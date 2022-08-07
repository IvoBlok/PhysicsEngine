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

struct BufferObjectGroup {
	unsigned int vertexBufferObject;
	unsigned int elementBufferObject;

	unsigned int vertexArrayObject;

	unsigned int uniformBufferObject;
	unsigned int shaderStorageBufferObject;

private:
	bool buffersBound = false;

public:
	void generateBuffers(bool genUniformBuffer) {
		glGenVertexArrays(1, &vertexArrayObject);
		glGenBuffers(1, &vertexBufferObject);
		glGenBuffers(1, &elementBufferObject);
		glGenBuffers(1, &shaderStorageBufferObject);
		if(genUniformBuffer){ glGenBuffers(1, &uniformBufferObject); }
			
		buffersBound = false;
	}

	void generateBuffers(Shader& shader, bool genUniformBuffer = true) {
		shader.use();
		generateBuffers(genUniformBuffer);
	}

	void bindBufferObjectGroup() {
		glBindVertexArray(vertexArrayObject);
		if (!buffersBound) {
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, shaderStorageBufferObject);
			if(uniformBufferObject != 0) { glBindBuffer(GL_UNIFORM_BUFFER, uniformBufferObject); }
			
			buffersBound = true;
		}
	}

	void bindBufferObjectGroup(Shader& shader) {
		shader.use();
		bindBufferObjectGroup();
	}

	~BufferObjectGroup() {
		glDeleteVertexArrays(1, &vertexArrayObject);
		glDeleteBuffers(1, &vertexBufferObject);
		glDeleteBuffers(1, &elementBufferObject);
		glDeleteBuffers(1, &shaderStorageBufferObject);
		glDeleteBuffers(1, &uniformBufferObject);
	}
};

class BufferHandler {
public:
	Shader instancingShader;											//-> stores the shader class instance used for rendering instancing 'groups'
	Shader defaultShader;												//-> stores the shader class instance used for rendering the individual objects
	GLFWwindow* window;

private:
	unsigned int frame = 0;														//-> stores the unique index of the frame being worked on
	glm::mat4 view;														//-> stores (temporarily) the view matrix
	glm::mat4 projection;												//-> stores (temporarily) the projection matrix
	std::vector<std::shared_ptr<EngineObject>> engineObjects;			//-> stores all existing loaded engine objects in the scene

	// ---------
	std::vector<objectTypes> instancingTypes;							//-> stores the object types that will be rendered by instancing

	std::vector<dynamicFloatArrayData> instancingVerticesVector;		//-> stores per instancing group the vertices, the count and capacity
	std::vector<dynamicIntArrayData> instancingIndicesVector;			//-> stores per instancing group the indices, the count and capacity
	std::vector<dynamicObjectInfoArrayData> instancingObjectInfoVector;	//-> stores per instancing group the objectInfo structs
	std::vector<BufferObjectGroup> instancingBufferObjectGroup;

	// ---------
	dynamicFloatArrayData defaultObjectVertices;						//-> stores default objectgroup vertices, the count and capacity
	dynamicIntArrayData defaultObjectIndices;							//-> stores default objectgroup indices, the count and capacity
	dynamicObjectInfoArrayData defaultObjectGroupInfo;					//-> stores objectInfo struct for every engineobject in the default group
	BufferObjectGroup defaultBufferObjectGroup;
	
	int defaultObjectGroupVertexLimits[MAX_PER_OBJECTS_COUNT];					//-> stores the starting vertex_IDs for every objects. Is linked to a uniform for the per_object vertex shader

	bool isUniformBufferInitialized = false;

public:
	dynamicFloatArrayData& getDefaultObjectVertices() { return defaultObjectVertices; }
	dynamicIntArrayData& getDefaultObjectIndices() { return defaultObjectIndices; }
	dynamicObjectInfoArrayData& getDefaultObjectGroupInfo() { return defaultObjectGroupInfo; }

	~BufferHandler() {
		// delete vertex/index/objectinfo data
		for (size_t i = 0; i < instancingVerticesVector.size(); i++)
		{
			delete[] instancingVerticesVector[i].data;
			delete[] instancingIndicesVector[i].data;
			delete[] instancingObjectInfoVector[i].data;
		}
		//delete[] defaultObjectVertices.data;
		//delete[] defaultObjectIndices.data;
	};

	Shader& createShader(bool instancing, std::string vertexPath, std::string fragmentPath, std::string geometryPath) {
		if (ExternalDebug) {
			vertexPath = "../" + vertexPath;
			fragmentPath = "../" + fragmentPath;
			geometryPath = "../" + geometryPath;
		}

		if (instancing) {
			instancingShader = Shader{ vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str() };
			return instancingShader;
		}
		else {
			defaultShader = Shader{ vertexPath.c_str(), fragmentPath.c_str(), geometryPath.c_str() };
			defaultBufferObjectGroup.generateBuffers(defaultShader);
			defaultBufferObjectGroup.bindBufferObjectGroup();
			return defaultShader;
		}
	};

	void updateEngineObjectMatrix(std::shared_ptr<EngineObject> object) {
		ObjectInfo_t newObjectInfo;
		newObjectInfo.color = glm::vec4{ object->color, 0 };
		newObjectInfo.geometryMatrix = glm::mat4{ 1 };
	
		newObjectInfo.geometryMatrix = glm::rotate(newObjectInfo.geometryMatrix, object->orientation.angle, object->orientation.axis);
		newObjectInfo.geometryMatrix = glm::scale(newObjectInfo.geometryMatrix, object->scale);

		// translation
		newObjectInfo.geometryMatrix[3][0] = object->position.x;
		newObjectInfo.geometryMatrix[3][1] = object->position.y;
		newObjectInfo.geometryMatrix[3][2] = object->position.z;

		if (object->getIsInstanced()) {
			instancingObjectInfoVector[object->getVerticesIndex()].data[object->getObjectInfoIndex()] = newObjectInfo;
		}
		else {
			defaultObjectGroupInfo.data[object->getObjectInfoIndex()] = newObjectInfo;
		}
	}

	void draw(bool wireframe = false) {
		if (wireframe) { glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); } else { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

		glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		initDefaultBufferObjectGroup();

		// please don't ask me why this needs to be called, I just know it works!
		updateDefaultBuffers();
		updateDefaultBuffers();
		glDrawElements(GL_TRIANGLES, (GLsizei)defaultObjectIndices.size, GL_UNSIGNED_INT, 0);

		drawInstancingObjects();
		frame++;
	};

	void drawInstancingObjects() {
		instancingShader.use();
		for (size_t i = 0; i < instancingObjectInfoVector.size(); i++)
		{
			// please don't ask me why this needs to be called, I just know it works!
			updateInstancingBuffers(i);
			updateInstancingBuffers(i);
			
			glDrawElementsInstanced(
				GL_TRIANGLES, 
				(GLsizei)instancingIndicesVector[i].size, 
				GL_UNSIGNED_INT, 
				0, 
				(GLsizei)instancingObjectInfoVector[i].size
			);
		}
	}

	void initEngineObjectReferences(EngineObject& engineObject, bool instancing, int verticesIndex, int indicesIndex, int objectInfoIndex, int engineObjectListIndex) {
		engineObject.setIsInstanced(instancing);
		engineObject.setVerticesIndex(verticesIndex);
		engineObject.setIndicesIndex(indicesIndex);
		engineObject.setObjectInfoIndex(objectInfoIndex);
		engineObject.setEngineObjectListIndex(engineObjectListIndex);
	}

	void initDefaultEngineObjectReferences(EngineObject& engineObject) {
		initEngineObjectReferences(
			engineObject,
			false,
			defaultObjectVertices.size - engineObject.mesh.vertices.size() * 3,
			defaultObjectIndices.size - engineObject.mesh.indices.size(),
			defaultObjectGroupInfo.size - 1,
			engineObjects.size()
		);
	}

	void initInstancingEngineObjectReferences(EngineObject& engineObject, int instancingGroup) {
		initEngineObjectReferences(
			engineObject,
			true,
			instancingGroup,
			instancingGroup,
			instancingObjectInfoVector[instancingGroup].size - 1,
			engineObjects.size() // removed -1, not sure why I had it. Assuming the push to engineobjects list is after this is called. IK it's bad
		);
	}

	void updateObjectInfo(ObjectInfo_t& objectInfo, EngineObject& engineObject) {
		objectInfo.color = glm::vec4{ engineObject.color, 0 };
		objectInfo.geometryMatrix = glm::mat4{ 1 };

		objectInfo.geometryMatrix = glm::rotate(objectInfo.geometryMatrix, engineObject.orientation.angle, engineObject.orientation.axis);
		objectInfo.geometryMatrix = glm::scale(objectInfo.geometryMatrix, engineObject.scale);

		// translation
		objectInfo.geometryMatrix[3][0] = engineObject.position.x;
		objectInfo.geometryMatrix[3][1] = engineObject.position.y;
		objectInfo.geometryMatrix[3][2] = engineObject.position.z;
	}

	void updateObjectVertices(std::shared_ptr<EngineObject> object) {
		if (!object->getIsInstanced()) {
			for (size_t i = 0; i < object->mesh.vertices.size(); i++)
			{
				defaultObjectVertices.data[object->getVerticesIndex() + 3 * i] = object->mesh.vertices[i].x;
				defaultObjectVertices.data[object->getVerticesIndex() + 3 * i + 1] = object->mesh.vertices[i].y;
				defaultObjectVertices.data[object->getVerticesIndex() + 3 * i + 2] = object->mesh.vertices[i].z;
			}
		}
		else {
			std::cout << "ERROR: tried to update object vertices of instanced object. This is unsupported behaviour (for now) \n";
		}
	}

	std::shared_ptr<EngineObject> createEngineObject(objectTypes objectType, bool instancing, glm::vec3 position = glm::vec3{ 0 }, glm::vec3 scale = glm::vec3{ 1 }, glm::vec3 color = glm::vec3{ 1, 1, 1 }, glm::vec3 direction = glm::vec3{ 0, 1, 0 }) {

		EngineObject newEngineObject{position, scale, color, direction};
		newEngineObject.mesh = getPrimaryShapeMesh(objectType);

		ObjectInfo_t newEngineObjectInfo;
		updateObjectInfo(newEngineObjectInfo, newEngineObject);

		if (!instancing) {
			defaultShader.use();
			int newEngineObjectIndex = defaultObjectGroupInfo.size;

			// every object's first index in the total storage is stored for use in the shader
			defaultObjectGroupVertexLimits[newEngineObjectIndex] = defaultObjectVertices.size / 3;
			defaultObjectGroupVertexLimits[newEngineObjectIndex + 1] = -1; // required so a small optimization in the shader can be made

			// now that the object is stored as part of a larger set, update the indices
			std::for_each(newEngineObject.mesh.indices.begin(), newEngineObject.mesh.indices.end(), [=](unsigned int& value) {value += defaultObjectVertices.size / 3; });

			defaultObjectVertices.addData(newEngineObject.mesh.vertices);
			defaultObjectIndices.addData(newEngineObject.mesh.indices);
			defaultObjectGroupInfo.addData(newEngineObjectInfo);

			initDefaultEngineObjectReferences(newEngineObject);
		}
		else {
			instancingShader.use();
			int instancingGroup;

			// check if there is a group with the given type
			if (std::find(instancingTypes.begin(), instancingTypes.end(), objectType) != instancingTypes.end()) {
				instancingGroup = std::distance(instancingTypes.begin(), std::find(instancingTypes.begin(), instancingTypes.end(), objectType));
			}
			else {
				// add the new type to the group
				instancingTypes.push_back(objectType);

				// if this is the first object of instancing type, create a new instancing group for it's type
				instancingGroup = instancingObjectInfoVector.size();

				instancingVerticesVector.push_back(dynamicFloatArrayData{});
				instancingIndicesVector.push_back(dynamicIntArrayData{});
				instancingObjectInfoVector.push_back(dynamicObjectInfoArrayData{});

				// vertex and index data only has to be assigned for the first in the instancing group
				instancingVerticesVector.back().addData(newEngineObject.mesh.vertices);
				instancingIndicesVector.back().addData(newEngineObject.mesh.indices);
				
				instancingBufferObjectGroup.push_back(BufferObjectGroup{});
				instancingBufferObjectGroup.back().generateBuffers(instancingShader, false);
			}
			instancingObjectInfoVector[instancingGroup].addData(newEngineObjectInfo);

			initInstancingEngineObjectReferences(newEngineObject, instancingGroup);
		}

		engineObjects.push_back(std::make_shared<EngineObject>(newEngineObject));
		return engineObjects.back();
	}

	void setDirLight(DirLightData directionalLight) {
		defaultShader.use();
		defaultShader.setDirLight(directionalLight);

		instancingShader.use();
		instancingShader.setDirLight(directionalLight);
	}

private:

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
		else if (type == objectTypes::VECTOR) {
			std::string path = "src/external/models/vector.stl";
			if (ExternalDebug) { path = "../" + path; }

			Mesh mesh{ path };
			return mesh;
		}
		else if (type == objectTypes::MODEL) {
			std::string path = "src/external/models/SolarCarTestModel.stl";
			if (ExternalDebug) { path = "../" + path; }

			Mesh mesh{ path };
			return mesh;
		}
		else if (type == objectTypes::GRID) {
			std::string path = "src/external/models/grid.stl";
			if (ExternalDebug) { path = "../" + path; }

			Mesh mesh{ path };
			return mesh;
		}
	}

	void initDefaultBufferObjectGroup() {
		if (isUniformBufferInitialized) { return; }

		defaultBufferObjectGroup.bindBufferObjectGroup(defaultShader);

		GLuint matricesShaderIndex = glGetUniformBlockIndex(defaultShader.ID, "Matrices");
		if (matricesShaderIndex < 0) { std::cout << "unfiromblockindexmatrices not found ..." << std::endl; return; }

		// then we link each shader's uniform block to this uniform binding point
		glUniformBlockBinding(defaultShader.ID, matricesShaderIndex, 0);
		glBindBuffer(GL_UNIFORM_BUFFER, defaultBufferObjectGroup.uniformBufferObject);
		glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
		// define the range of the buffer that links to a uniform binding point
		glBindBufferRange(GL_UNIFORM_BUFFER, 0, defaultBufferObjectGroup.uniformBufferObject, 0, 2 * sizeof(glm::mat4));
		isUniformBufferInitialized = true;
	}

	void updateUniformBuffer() {
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	}
	
	void updateDefaultBuffers() {

		defaultBufferObjectGroup.bindBufferObjectGroup(defaultShader);

		// make sure there is the appropriate amount of memory reserved
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * defaultObjectVertices.size, defaultObjectVertices.data, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * defaultObjectIndices.size, defaultObjectIndices.data, GL_DYNAMIC_DRAW);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t) * defaultObjectGroupInfo.size, defaultObjectGroupInfo.data, GL_DYNAMIC_COPY);
		
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, defaultBufferObjectGroup.shaderStorageBufferObject);

		updateUniformBuffer();

		defaultShader.setIntArr("vertexLimits", defaultObjectGroupVertexLimits);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void updateInstancingBuffers(unsigned int instancingGroupIndex) {
		// setup
		instancingBufferObjectGroup[instancingGroupIndex].bindBufferObjectGroup(instancingShader);

		// all rendering groups use the same uniform buffer object, so the instancing bufferobjectgroups just contain a reference to the unique one

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * instancingVerticesVector[instancingGroupIndex].size, instancingVerticesVector[instancingGroupIndex].data, GL_STATIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * instancingIndicesVector[instancingGroupIndex].size, instancingIndicesVector[instancingGroupIndex].data, GL_STATIC_DRAW);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t) * instancingObjectInfoVector[instancingGroupIndex].size, instancingObjectInfoVector[instancingGroupIndex].data, GL_DYNAMIC_COPY);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, instancingBufferObjectGroup[instancingGroupIndex].shaderStorageBufferObject);

		updateUniformBuffer();

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
};
#endif