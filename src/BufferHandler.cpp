#include "BufferHandler.h"

BufferHandler::BufferHandler(Camera& camera_) : camera(camera_) {};

BufferHandler::~BufferHandler() {
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
		glDeleteBuffers(1, &instancingUBOs[i]);
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
	delete[] perObjectObjectInfoArray.data;

}

Shader& BufferHandler::createShader(bool instanceType, const char* vertexPath, const char* fragmentPath, const char* geometryPath) {
	if (instanceType) {
		instancingShader = Shader{ vertexPath, fragmentPath, geometryPath };
		return instancingShader;
	}
	else {
		perObjectShader = Shader{ vertexPath, fragmentPath, geometryPath };
		return perObjectShader;
	}
}

void BufferHandler::draw() {
	// draw single objects
	// ---------
	for (int i = 0; i < perObjectObjectInfoArray.size; i++)
	{

	}
}