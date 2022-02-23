#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H

//external
#include <GLM/gtx/string_cast.hpp>

//internal
#include "settings.h"
#include "EngineObject.h"
#include "Mesh.h"

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
		}

		glDeleteBuffers(1, &UBO);

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
		delete[] perObjectObjectInfoArray.data;
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

	// complex functions
	// ---------
	void draw() {
		// configure universal UBO
		// ---------
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = camera.GetViewMatrix();

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);

		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));

		// clean old-frame buffers and set the background color
		// ---------
		glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// draw single objects
		// ---------
		for (int i = 0; i < perObjectObjectInfoArray.size; i++)
		{
			// activate correct shader
			perObjectShader.use();
			perObjectShader.setInt("objectInfoIndex", i);

			// bind relevant buffer objects
			glBindVertexArray(perObjectVAOs[i]);
			glBindBuffer(GL_ARRAY_BUFFER, perObjectVBOs[i]);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, perObjectEBOs[i]);
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, perObjectSSBOs[i]);

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

			// draw
			glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)instancingIndicesVector[i].size, GL_UNSIGNED_INT, 0, (GLsizei)instancingObjectInfoVector[i].size );
		}

		frame++;
	};

	EngineObject& createObject(objectTypes type_);

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

	unsigned int UBO = NULL;

	// instancing variables
	// ---------
	std::vector<objectTypes> instancingTypes;							//-> stores the object types that will be rendered by instancing

	std::vector<dynamicVec3ArrayData> instancingVerticesVector;		//-> stores per instancing group the vertices, the count and capacity
	std::vector<dynamicIntArrayData> instancingIndicesVector;			//-> stores per instancing group the indices, the count and capacity
	std::vector<dynamicObjectInfoArrayData> instancingObjectInfoVector;	//-> stores per instancing group the objectInfo structs

	std::vector<unsigned int> instancingVBOs;
	std::vector<unsigned int> instancingEBOs;
	std::vector<unsigned int> instancingVAOs;
	std::vector<unsigned int> instancingSSBOs;

	// per object variables
	// ---------
	std::vector<objectTypes> perObjectTypes;							//-> stores the object types that will be rendered one by one

	std::vector<dynamicVec3ArrayData> perObjectVerticesVector;			//-> stores per object the vertices, the count and capacity
	std::vector<dynamicIntArrayData> perObjectIndicesVector;			//-> stores per object the indices, the count and capacity
	dynamicObjectInfoArrayData perObjectObjectInfoArray;				//-> stores per object the objectInfo struct

	std::vector<unsigned int> perObjectVBOs;
	std::vector<unsigned int> perObjectEBOs;
	std::vector<unsigned int> perObjectVAOs;
	std::vector<unsigned int> perObjectSSBOs;

	// basic functions
	// ---------
	

	// complex functions
	// ---------

};

#endif