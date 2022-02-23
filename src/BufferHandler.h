#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H

//external
#include <GLM/gtx/string_cast.hpp>

//internal
#include "settings.h"
#include "Camera.h"
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

	Camera& camera;														//-> stores a reference to the camera to be used for the next frame

	// basic functions
	// ---------
	BufferHandler(Camera& camera_);
	~BufferHandler();

	Shader& createShader(bool instanceType, const char* vertexPath, const char* fragmentPath, const char* geometryPath);


	// complex functions
	// ---------
	void draw();			

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
	std::vector<unsigned int> instancingUBOs;

	// per object variables
	// ---------
	std::vector<objectTypes> perObjectTypes;							//-> stores the object types that will be rendered one by one

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
	

	// complex functions
	// ---------

};

#endif