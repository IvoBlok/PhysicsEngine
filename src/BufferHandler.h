#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H

//external
#include <GLM/gtx/string_cast.hpp>

//internal
#include "settings.h"
#include "EngineObject.h"
#include "Mesh.h"

//std
#include <algorithm>

// main class
// --------
class BufferHandler {
public:
	// public variables
	// ---------
	Shader instancingShader;											//-> stores the shader class instance used for rendering instancing 'groups'
	Shader perObjectShader;												//-> stores the shader class instance used for rendering the individual objects

	// basic functions
	// ---------
	BufferHandler() {};

	~BufferHandler() {
		// destroy objects
		for (size_t i = 0; i < engineObjects.size(); i++)
		{
			engineObjects[i].~EngineObject();
		}

		// delete GPU buffers
		for (size_t i = 0; i < instancingVBOs.size(); i++)
		{
			glDeleteVertexArrays(1, &instancingVAOs[i]);
			glDeleteBuffers(1, &instancingVBOs[i]);
			glDeleteBuffers(1, &instancingEBOs[i]);
			glDeleteBuffers(1, &instancingSSBOs[i]);
		}
		for (size_t i = 0; i < perObjectVBOs.size(); i++)
		{
			glDeleteVertexArrays(1, &perObjectVAOs[i]);
			glDeleteBuffers(1, &perObjectVBOs[i]);
			glDeleteBuffers(1, &perObjectEBOs[i]);
			glDeleteBuffers(1, &perObjectSSBOs[i]);
			glDeleteBuffers(1, &perObjectUBOs[i]);
		}

		// delete vertex/index/objectinfo data
		for (size_t i = 0; i < instancingVerticesVector.size(); i++)
		{
			delete[] instancingVerticesVector[i].data;
			delete[] instancingIndicesVector[i].data;
			delete[] instancingObjectInfoVector[i].data;
		}
		for (size_t i = 0; i < perObjectVerticesVector.size(); i++)
		{
			delete[] perObjectVerticesVector[i].data;
			delete[] perObjectIndicesVector[i].data;
		}
	};

	Shader& createShader(bool instanceType, const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
		if (instanceType) {
			instancingShader = Shader{ vertexPath, fragmentPath, geometryPath };
			return instancingShader;
		}
		else {
			perObjectShader = Shader{ vertexPath, fragmentPath, geometryPath };
			return perObjectShader;
		}
	};

	void setDirLight(DirLightData directionalLight) {
		perObjectShader.use();
		perObjectShader.setDirLight(directionalLight);

		instancingShader.use();
		instancingShader.setDirLight(directionalLight);
	}

	// complex functions
	// ---------
	void draw() {
		// check to make sure at least the UBO has been updated. it's an ugly solution, but for my goal it should work fine
		if (perObjectObjectInfoArray.size == 0 && instancingObjectInfoVector.size() > 0) {
			std::cout << "ERROR: CAN'T DRAW WHEN ONLY INSTANCINGOBJECTS EXIST" << std::endl;
			return;
		}

		// configure universal UBO
		// ---------
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		// clean old-frame buffers and set the background color
		// ---------
		glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw single objects
		// ---------
		
		// activate correct shader
		perObjectShader.use();

		for (int i = 0; i < perObjectObjectInfoArray.size; i++)
		{
			// bind relevant buffer objects
			glBindVertexArray(perObjectVAOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, perObjectVBOs[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, perObjectEBOs[i]);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, perObjectSSBOs[i]);
			glBindBuffer(GL_UNIFORM_BUFFER, perObjectUBOs[i]);

			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

			perObjectShader.setInt("objectInfoIndex", i);

			// draw
			glDrawElements(GL_TRIANGLES, (GLsizei)perObjectIndicesVector[i].size, GL_UNSIGNED_INT, 0);
		}

		// draw instanced objects
		// ---------
		
		for (size_t i = 0; i < instancingObjectInfoVector.size(); i++)
		{
			// activate correct shader
			instancingShader.use();

			// bind relevant buffer objects
			glBindVertexArray(instancingVAOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, instancingVBOs[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, instancingEBOs[i]);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancingSSBOs[i]);
			//glBindBuffer(GL_UNIFORM_BUFFER, instancingUBOs[i]);

			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

			// draw
			glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)instancingIndicesVector[i].size, GL_UNSIGNED_INT, 0, (GLsizei)instancingObjectInfoVector[i].size );
		}
		
		frame++;
	};
	
	EngineObject& createObject(objectTypes type_, bool instancing, glm::vec3 color = glm::vec3{ 1, 1, 1 }, glm::vec3 position = glm::vec3{randomRange(-2, 2), randomRange(-2, 2), randomRange(-2, 2) }) {
		
		Mesh mesh = getPrimaryShapeMesh(type_);
		EngineObject engineObject;

		if (!instancing) {
			perObjectShader.use();

			// first lets get it running with only per object rendering
			int newObjectIndex = perObjectObjectInfoArray.size;
			perObjectVerticesVector.push_back(dynamicFloatArrayData{});
			perObjectIndicesVector.push_back(dynamicIntArrayData{});
			ObjectInfo_t newObjectInfo;

			// set vertices and indices
			perObjectVerticesVector[newObjectIndex].addData(mesh.vertices);
			perObjectIndicesVector[newObjectIndex].addData(mesh.indices);

			// set objectinfo struct
			newObjectInfo.color = glm::vec4{ color, 0 };
			newObjectInfo.geometryMatrix = glm::translate(glm::mat4{ 1 }, position);

			perObjectObjectInfoArray.addData(newObjectInfo);

			//create bufferobjects
			// ---------

			unsigned int VAO, VBO, EBO, SSBO, UBO;

			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);
			glGenBuffers(1, &SSBO);
			glGenBuffers(1, &UBO);
			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * perObjectVerticesVector[newObjectIndex].size, perObjectVerticesVector[newObjectIndex].data, GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * perObjectIndicesVector[newObjectIndex].size, perObjectIndicesVector[newObjectIndex].data, GL_STATIC_DRAW);

			glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
			glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t) * perObjectObjectInfoArray.size, perObjectObjectInfoArray.data, GL_DYNAMIC_COPY);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);

			GLuint uniformBlockIndexMatrices = glGetUniformBlockIndex(perObjectShader.ID, "Matrices");
			if (uniformBlockIndexMatrices < 0) { std::cout << "unfiromblockindexmatrices not found ..." << std::endl; }

			// then we link each shader's uniform block to this uniform binding point
			glUniformBlockBinding(perObjectShader.ID, uniformBlockIndexMatrices, 0);
			glBindBuffer(GL_UNIFORM_BUFFER, UBO);
			glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
			// define the range of the buffer that links to a uniform binding point
			glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));


			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);


			perObjectVAOs.push_back(VAO);
			perObjectVBOs.push_back(VBO);
			perObjectEBOs.push_back(EBO);
			perObjectSSBOs.push_back(SSBO);
			perObjectUBOs.push_back(UBO);

			// create engineobject
			// -----------
			engineObject.instancedObject = false;
			engineObject.verticesIndex = newObjectIndex;
			engineObject.indicesIndex = newObjectIndex;
			engineObject.objectInfoIndex = newObjectIndex;

			engineObjects.push_back(engineObject);
		}
		else {
			
			instancingShader.use();

			color = glm::vec3{ 1, 0.5, 0 };

			// objectinfo data
			ObjectInfo_t newObjectInfo;
			// set objectinfo struct
			newObjectInfo.color = glm::vec4{ color, 0 };
			newObjectInfo.geometryMatrix = glm::translate(glm::mat4{ 1 }, position);

			int instancingGroup;
			// check if there is a group with the current type
			// ---------
			
			if (std::find(instancingTypes.begin(), instancingTypes.end(), type_) != instancingTypes.end()) {
				instancingGroup = std::distance(instancingTypes.begin(), std::find(instancingTypes.begin(), instancingTypes.end(), type_));

				// only thing that needs to be updated is the SSBO and objectinfovector
				// ---------
				instancingObjectInfoVector[instancingGroup].addData(newObjectInfo);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, instancingSSBOs[instancingGroup]);
				glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t) * instancingObjectInfoVector[instancingGroup].size, instancingObjectInfoVector[instancingGroup].data, GL_DYNAMIC_COPY);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, instancingSSBOs[instancingGroup]);
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


				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glBufferData(GL_ARRAY_BUFFER, sizeof(float)* instancingVerticesVector[instancingGroup].size, instancingVerticesVector[instancingGroup].data, GL_STATIC_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)* instancingIndicesVector[instancingGroup].size, instancingIndicesVector[instancingGroup].data, GL_STATIC_DRAW);

				glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
				glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t)* instancingObjectInfoVector[instancingGroup].size, instancingObjectInfoVector[instancingGroup].data, GL_DYNAMIC_COPY);
				glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO);


				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);

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

			engineObjects.push_back(engineObject);
			
		}
		return engineObject;
	}

	// object functions
	// ---------
	void moveObject(EngineObject&);
	void removeObject(EngineObject&);

private:
	// private variables
	// ---------
	int frame = 0;														//-> stores the unique index of the frame being worked on
	glm::mat4 view;														//-> stores (temporarily) the view matrix
	glm::mat4 projection;												//-> stores (temporarily) the projection matrix
	std::vector<EngineObject> engineObjects;							//-> stores all existing loaded engine objects in the scene

	// instancing variables
	// ---------
	std::vector<objectTypes> instancingTypes;							//-> stores the object types that will be rendered by instancing

	std::vector<dynamicFloatArrayData> instancingVerticesVector;			//-> stores per instancing group the vertices, the count and capacity
	std::vector<dynamicIntArrayData> instancingIndicesVector;			//-> stores per instancing group the indices, the count and capacity
	std::vector<dynamicObjectInfoArrayData> instancingObjectInfoVector;	//-> stores per instancing group the objectInfo structs

	std::vector<unsigned int> instancingVBOs;
	std::vector<unsigned int> instancingEBOs;
	std::vector<unsigned int> instancingVAOs;
	std::vector<unsigned int> instancingSSBOs;

	// per object variables
	// ---------

	std::vector<dynamicFloatArrayData> perObjectVerticesVector;			//-> stores per object the vertices, the count and capacity
	std::vector<dynamicIntArrayData> perObjectIndicesVector;			//-> stores per object the indices, the count and capacity
	dynamicObjectInfoArrayData perObjectObjectInfoArray;				//-> stores per object the objectInfo struct

	std::vector<unsigned int> perObjectVBOs;
	std::vector<unsigned int> perObjectEBOs;
	std::vector<unsigned int> perObjectVAOs;
	std::vector<unsigned int> perObjectSSBOs;
	std::vector<unsigned int> perObjectUBOs;

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
				3, 1, 0, // back side
				3, 2, 1,

				4, 5, 7, // front side
				5, 6, 7,

				4, 3, 0, // y+ side
				4, 7, 3,

				1, 2, 5, // y- side
				2, 6, 5,

				0, 1, 4,// x+ side
				1, 5, 4,

				7, 2, 3,// x- side
				7, 6, 2,
			};
			Mesh mesh{ vertices, indices };

			return mesh;
		}
		if (type == objectTypes::ARROW) {
			std::vector<float> vertices = {
				// positions      
				 1.5f,  0.5f, -0.2f,
				 1.5f, -0.5f, -0.5f,
				-1.5f, -0.5f, -0.5f,
				-1.5f,  0.5f, -0.5f,

				 1.5f,  0.5f,  0.5f,
				 1.5f, -0.5f,  0.5f,
				-1.5f, -0.5f,  0.5f,
				-1.5f,  0.5f,  0.5f,
			};

			std::vector<unsigned int> indices = {
				3, 1, 0, // back side
				3, 2, 1,

				4, 5, 7, // front side
				5, 6, 7,

				4, 3, 0, // y+ side
				4, 7, 3,

				1, 2, 5, // y- side
				2, 6, 5,

				0, 1, 4,// x+ side
				1, 5, 4,

				7, 2, 3,// x- side
				7, 6, 2,
			};
			Mesh mesh{ vertices, indices };

			return mesh;
		}
	}

	// complex functions
	// ---------

};

#endif