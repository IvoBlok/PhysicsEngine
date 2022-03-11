#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H

// external
#include <GLM/gtx/string_cast.hpp>
#include <GLM/gtx/euler_angles.hpp>

// internal
#include "settings.h"
#include "EngineObject.h"
#include "Mesh.h"

// std
#include <algorithm>

// main class
// --------
class BufferHandler {
public:
	// public variables
	// ---------
	Shader instancingShader;											//-> stores the shader class instance used for rendering instancing 'groups'
	Shader perObjectShader;												//-> stores the shader class instance used for rendering the individual objects
	GLFWwindow* window;

	// basic functions
	// ---------
	BufferHandler() {
		glGenVertexArrays(1, &perObjectVAO);
		glGenBuffers(1, &perObjectVBO);
		glGenBuffers(1, &perObjectEBO);
		glGenBuffers(1, &perObjectSSBO);
		glGenBuffers(1, &perObjectUBO);
	};

	~BufferHandler() {
		// destroy objects
		for (size_t i = 0; i < engineObjects.size(); i++)
		{
			engineObjects[i]->~EngineObject();
		}

		// delete GPU buffers
		for (size_t i = 0; i < instancingVBOs.size(); i++)
		{
			glDeleteVertexArrays(1, &instancingVAOs[i]);
			glDeleteBuffers(1, &instancingVBOs[i]);
			glDeleteBuffers(1, &instancingEBOs[i]);
			glDeleteBuffers(1, &instancingSSBOs[i]);
		}
		glDeleteVertexArrays(1, &perObjectVAO);
		glDeleteBuffers(1, &perObjectVBO);
		glDeleteBuffers(1, &perObjectEBO);
		glDeleteBuffers(1, &perObjectSSBO);
		glDeleteBuffers(1, &perObjectUBO);

		// delete vertex/index/objectinfo data
		for (size_t i = 0; i < instancingVerticesVector.size(); i++)
		{
			delete[] instancingVerticesVector[i].data;
			delete[] instancingIndicesVector[i].data;
			delete[] instancingObjectInfoVector[i].data;
		}
		//delete[] perObjectVertices.data;
		//delete[] perObjectIndices.data;
	};

	Shader& createShader(bool instanceType, std::string vertexPath, std::string fragmentPath, std::string geometryPath) {
		if (ExternalDebug) {
			vertexPath = "../" + vertexPath;
			fragmentPath = "../" + fragmentPath;
			geometryPath = "../" + geometryPath;
		}
		if (instanceType) {
			instancingShader = Shader{ vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str() };
			return instancingShader;
		}
		else {
			perObjectShader = Shader{ vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str() };
			return perObjectShader;
		}
	};

	void setDirLight(DirLightData directionalLight) {
		perObjectShader.use();
		perObjectShader.setDirLight(directionalLight);

		instancingShader.use();
		instancingShader.setDirLight(directionalLight);
	}

	void updateEngineObjectMatrix(std::shared_ptr<EngineObject> object) {
		ObjectInfo_t newObjectInfo;
		newObjectInfo.color = glm::vec4{ object->color, 0 };
		newObjectInfo.geometryMatrix = glm::mat4{ 1 };
	
		newObjectInfo.geometryMatrix = glm::rotate(newObjectInfo.geometryMatrix, object->angle, object->rotationAxis);
		newObjectInfo.geometryMatrix = glm::scale(newObjectInfo.geometryMatrix, object->scale);

		// translation
		newObjectInfo.geometryMatrix[3][0] = object->position.x;
		newObjectInfo.geometryMatrix[3][1] = object->position.y;
		newObjectInfo.geometryMatrix[3][2] = object->position.z;

		if (object->instancedObject) {
			instancingObjectInfoVector[object->verticesIndex].data[object->objectInfoIndex] = newObjectInfo;
		}
		else {
			perObjectObjectInfoArray.data[object->objectInfoIndex] = newObjectInfo;
		}
	}

	// complex functions
	// ---------
	void draw(bool wireframe = false) {
		if (wireframe) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }

		// clean old-frame buffers and set the background color
		// ---------
		glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw single objects
		// ---------
		updatePerObjectBuffers();
		glDrawElements(GL_TRIANGLES, (GLsizei)perObjectIndices.size, GL_UNSIGNED_INT, 0);

		// draw instanced objects
		// ---------
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		instancingShader.use();

		for (size_t i = 0; i < instancingObjectInfoVector.size(); i++)
		{
			updateInstancingBuffers(i);
			glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)instancingIndicesVector[i].size, GL_UNSIGNED_INT, 0, (GLsizei)instancingObjectInfoVector[i].size );
		}

		frame++;
	};

	std::shared_ptr<EngineObject> createObject(objectTypes type_, bool instancing, glm::vec3 position = glm::vec3{ 0 }, glm::vec3 scale = glm::vec3{ 1 }, glm::vec3 color = glm::vec3{ 1, 1, 1 }, glm::vec3 direction = glm::vec3{ 0, 1, 0 }) {
		
		Mesh mesh;
		if (!instancing || std::find(instancingTypes.begin(), instancingTypes.end(), type_) == instancingTypes.end()) {
			mesh = getPrimaryShapeMesh(type_);
		}

		EngineObject engineObject;
		engineObject.color = color;
		engineObject.position = position;
		engineObject.scale = scale;
		engineObject.setDirection(direction);

		// set objectinfo struct
		ObjectInfo_t newObjectInfo;
		newObjectInfo.color = glm::vec4{ color, 0 };
		newObjectInfo.geometryMatrix = glm::mat4{ 1 };

		newObjectInfo.geometryMatrix = glm::rotate(newObjectInfo.geometryMatrix, engineObject.angle, engineObject.rotationAxis);
		newObjectInfo.geometryMatrix = glm::scale(newObjectInfo.geometryMatrix, scale);

		// translation
		newObjectInfo.geometryMatrix[3][0] = position.x;
		newObjectInfo.geometryMatrix[3][1] = position.y;
		newObjectInfo.geometryMatrix[3][2] = position.z;

		if (!instancing) {
			perObjectShader.use();

			// first lets get it running with only per object rendering
			int newObjectIndex = perObjectObjectInfoArray.size;

			// every object's first index in the total storage is stored for use in the shader
			perObjectVertexLimits[newObjectIndex] = perObjectVertices.size / 3;
			perObjectVertexLimits[newObjectIndex + 1] = -1; // required so a small optimization in the shader can be made

			for (size_t i = 0; i < mesh.indices.size(); i++)
			{
				mesh.indices[i] += perObjectVertices.size / 3;
			}

			// set vertices and indices
			perObjectVertices.addData(mesh.vertices);
			perObjectIndices.addData(mesh.indices);

			perObjectObjectInfoArray.addData(newObjectInfo);

			// create engineobject
			// -----------
			engineObject.instancedObject = false;
			engineObject.verticesIndex = newObjectIndex;
			engineObject.indicesIndex = newObjectIndex;
			engineObject.objectInfoIndex = newObjectIndex;
			engineObject.engineObjectListIndex = engineObjects.size();
		}
		else {
			instancingShader.use();
			int instancingGroup;

			// check if there is a group with the current type
			// ---------
			if (std::find(instancingTypes.begin(), instancingTypes.end(), type_) != instancingTypes.end()) {
				instancingGroup = std::distance(instancingTypes.begin(), std::find(instancingTypes.begin(), instancingTypes.end(), type_));

				instancingObjectInfoVector[instancingGroup].addData(newObjectInfo);
			}
			else {
				// add the new type to the group
				instancingTypes.push_back(type_);

				// if this is the first object of instancing type, create a new instancing group for it's type
				// ---------
				instancingGroup = instancingObjectInfoVector.size();
				instancingVerticesVector.push_back(dynamicFloatArrayData{});
				instancingIndicesVector.push_back(dynamicIntArrayData{});
				instancingObjectInfoVector.push_back(dynamicObjectInfoArrayData{});

				// vertex and index data only has to be assigned for the first in the instancing group
				instancingVerticesVector[instancingGroup].addData(mesh.vertices);
				instancingIndicesVector[instancingGroup].addData(mesh.indices);

				instancingObjectInfoVector[instancingGroup].addData(newObjectInfo);
				//create bufferobjects
				// ---------
				unsigned int VAO, VBO, EBO, SSBO;

				glGenVertexArrays(1, &VAO);
				glGenBuffers(1, &VBO);
				glGenBuffers(1, &EBO);
				glGenBuffers(1, &SSBO);
				glBindVertexArray(VAO);

				instancingVAOs.push_back(VAO);
				instancingVBOs.push_back(VBO);
				instancingEBOs.push_back(EBO);
				instancingSSBOs.push_back(SSBO);
			}
			
			// create engineobject
			// -----------
			engineObject.instancedObject = true;
			engineObject.verticesIndex = instancingGroup;
			engineObject.indicesIndex = instancingGroup;
			engineObject.objectInfoIndex = instancingObjectInfoVector[instancingGroup].size - 1;
			engineObject.engineObjectListIndex = engineObjects.size();
		}

		engineObjects.push_back(std::make_shared<EngineObject>(engineObject));
		return engineObjects[engineObject.engineObjectListIndex];
	}

	// public object data manipulation functions
	// ---------
	void moveObjectOriginToAvg(std::shared_ptr<EngineObject> object) {
		//TODO! IMPLEMENTATION: MOVE ORIGIN TO AVERAGE OF ALL VERTICES
	}

private:
	// private variables
	// ---------
	int frame = 0;														//-> stores the unique index of the frame being worked on
	glm::mat4 view;														//-> stores (temporarily) the view matrix
	glm::mat4 projection;												//-> stores (temporarily) the projection matrix
	std::vector<std::shared_ptr<EngineObject>> engineObjects;			//-> stores all existing loaded engine objects in the scene

	// instancing variables
	// ---------
	std::vector<objectTypes> instancingTypes;							//-> stores the object types that will be rendered by instancing

	std::vector<dynamicFloatArrayData> instancingVerticesVector;		//-> stores per instancing group the vertices, the count and capacity
	std::vector<dynamicIntArrayData> instancingIndicesVector;			//-> stores per instancing group the indices, the count and capacity
	std::vector<dynamicObjectInfoArrayData> instancingObjectInfoVector;	//-> stores per instancing group the objectInfo structs

	std::vector<unsigned int> instancingVBOs;
	std::vector<unsigned int> instancingEBOs;
	std::vector<unsigned int> instancingVAOs;
	std::vector<unsigned int> instancingSSBOs;

	// per object variables
	// ---------

	dynamicFloatArrayData perObjectVertices;							//-> stores per object the vertices, the count and capacity
	dynamicIntArrayData perObjectIndices;								//-> stores per object the indices, the count and capacity
	dynamicObjectInfoArrayData perObjectObjectInfoArray;				//-> stores per object the objectInfo struct

	unsigned int perObjectVBO;
	unsigned int perObjectEBO;
	unsigned int perObjectVAO;
	unsigned int perObjectSSBO;
	unsigned int perObjectUBO;
	bool UBOInitialized = false;
	int perObjectVertexLimits[MAX_PER_OBJECTS_COUNT];					//-> stores the starting vertex_IDs for every objects. Is linked to a uniform for the per_object vertex shader

	// basic functions
	// ---------
	Mesh getPrimaryShapeMesh(objectTypes type) {
		if (type == objectTypes::CUBE) {
			std::vector<float> vertices = {
				// positions      
				 0.5f,  0.5f, -0.5f,
				 0.5f, -0.5f, -0.5f,
				-0.5f, -0.5f, -0.5f,
				-0.5f,  0.5f, -0.5f,

				 0.5f,  0.5f,  0.5f,
				 0.5f, -0.5f,  0.5f,
				-0.5f, -0.5f,  0.5f,
				-0.5f,  0.5f,  0.5f,
			};

			std::vector<unsigned int> indices = {
				0, 1, 3, // back side
				1, 2, 3,

				7, 5, 4, // front side
				7, 6, 5,

				0, 3, 4, // y+ side
				3, 7, 4,

				5, 2, 1, // y- side
				5, 6, 2,

				4, 1, 0,// x+ side
				4, 5, 1,

				3, 2, 7,// x- side
				2, 6, 7,
			};
			Mesh mesh{ vertices, indices };

			return mesh;
		}
		if (type == objectTypes::VECTOR) {
			std::string path = "src/external/models/vector.stl";
			if (ExternalDebug) { path = "../" + path; }

			Mesh mesh{ path };
			return mesh;
		}
		if (type == objectTypes::MODEL) {
			std::string path = "src/external/models/SolarCarTestModel.stl";
			if (ExternalDebug) { path = "../" + path; }

			Mesh mesh{ path };
			return mesh;
		}
	}

	// complex functions
	// ---------
	void updatePerObjectBuffers() {
		perObjectShader.use();

		glBindVertexArray(perObjectVAO);
		glBindBuffer(GL_ARRAY_BUFFER, perObjectVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, perObjectEBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, perObjectSSBO);
		glBindBuffer(GL_UNIFORM_BUFFER, perObjectUBO);

		// setup
		if (!UBOInitialized) {
			GLuint uniformBlockIndexMatrices = glGetUniformBlockIndex(perObjectShader.ID, "Matrices");
			if (uniformBlockIndexMatrices < 0) { std::cout << "unfiromblockindexmatrices not found ..." << std::endl; }

			// then we link each shader's uniform block to this uniform binding point
			glUniformBlockBinding(perObjectShader.ID, uniformBlockIndexMatrices, 0);
			glBindBuffer(GL_UNIFORM_BUFFER, perObjectUBO);
			glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
			// define the range of the buffer that links to a uniform binding point
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, perObjectUBO, 0, 2 * sizeof(glm::mat4));
			UBOInitialized = true;
		}

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * perObjectVertices.size, perObjectVertices.data, GL_STATIC_DRAW);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * perObjectIndices.size, perObjectIndices.data, GL_STATIC_DRAW);

		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t) * perObjectObjectInfoArray.size, perObjectObjectInfoArray.data, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, perObjectSSBO);

		// configure universal UBO
		// ---------
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		perObjectShader.setIntArr("vertexLimits", perObjectVertexLimits);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void updateInstancingBuffers(int instancingGroupIndex) {
		// setup
		if (!UBOInitialized) {
			glBindVertexArray(perObjectVAO);
			glBindBuffer(GL_UNIFORM_BUFFER, perObjectUBO);

			GLuint uniformBlockIndexMatrices = glGetUniformBlockIndex(perObjectShader.ID, "Matrices");
			if (uniformBlockIndexMatrices < 0) { std::cout << "unfiromblockindexmatrices not found ..." << std::endl; }

			// then we link each shader's uniform block to this uniform binding point
			glUniformBlockBinding(perObjectShader.ID, uniformBlockIndexMatrices, 0);
			glBindBuffer(GL_UNIFORM_BUFFER, perObjectUBO);
			glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
			// define the range of the buffer that links to a uniform binding point
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, perObjectUBO, 0, 2 * sizeof(glm::mat4));
			UBOInitialized = true;
		}
		
		instancingShader.use();

		glBindVertexArray(instancingVAOs[instancingGroupIndex]);
		glBindBuffer(GL_ARRAY_BUFFER, instancingVBOs[instancingGroupIndex]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instancingEBOs[instancingGroupIndex]);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancingSSBOs[instancingGroupIndex]);


		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * instancingVerticesVector[instancingGroupIndex].size, instancingVerticesVector[instancingGroupIndex].data, GL_STATIC_DRAW);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * instancingIndicesVector[instancingGroupIndex].size, instancingIndicesVector[instancingGroupIndex].data, GL_STATIC_DRAW);

		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t) * instancingObjectInfoVector[instancingGroupIndex].size, instancingObjectInfoVector[instancingGroupIndex].data, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, instancingSSBOs[instancingGroupIndex]);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
};

#endif