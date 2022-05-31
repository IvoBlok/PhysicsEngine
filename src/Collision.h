// external

// internal
#include "BufferHandler.h"

// std


void generateCollisionCubes(BufferHandler& bufferHandler, std::shared_ptr<EngineObject> object) {
	if (object->instancedObject) { std::cout << "ERROR: given object can not be run for collision because it is an instanced object; unsupported behaviour" << std::endl; return; }
	
#pragma region get rectangular bounding box

	glm::vec3 minBoundingBoxPoint = glm::vec3{ 0 };
	glm::vec3 maxBoundingBoxPoint = glm::vec3{ 0 };

	for (int i = object->verticesIndex; i < object->verticesIndex + object->mesh.vertices.size(); i++) {
		// retrieve the vertex
		glm::vec4 vertex = glm::vec4{
			bufferHandler.perObjectVertices.data[i * 3],
			bufferHandler.perObjectVertices.data[i * 3 + 1],
			bufferHandler.perObjectVertices.data[i * 3 + 2],
			1
		};

		// transform it to the effective world position
		vertex = vertex * bufferHandler.perObjectObjectInfoArray.data[object->objectInfoIndex].geometryMatrix;

		// initialize the boundary points with a value that is guaranteed to be within the mesh
		minBoundingBoxPoint = (minBoundingBoxPoint == glm::vec3{ 0 }) ? vertex : minBoundingBoxPoint;
		maxBoundingBoxPoint = (maxBoundingBoxPoint == glm::vec3{ 0 }) ? vertex : maxBoundingBoxPoint;

		// check whether the point falls outside the current boundary box
		if (vertex.x > maxBoundingBoxPoint.x) { maxBoundingBoxPoint.x = vertex.x; }
		if (vertex.y > maxBoundingBoxPoint.y) { maxBoundingBoxPoint.y = vertex.y; }
		if (vertex.z > maxBoundingBoxPoint.z) { maxBoundingBoxPoint.z = vertex.z; }

		if (vertex.x < minBoundingBoxPoint.x) { minBoundingBoxPoint.x = vertex.x; }
		if (vertex.y < minBoundingBoxPoint.y) { minBoundingBoxPoint.y = vertex.y; }
		if (vertex.z < minBoundingBoxPoint.z) { minBoundingBoxPoint.z = vertex.z; }
	}
	glm::vec3 bounderBoxSize = abs(maxBoundingBoxPoint) - abs(minBoundingBoxPoint);

	std::shared_ptr<EngineObject> boundaryBox = bufferHandler.createObject(
		objectTypes::CUBE,
		true,
		(minBoundingBoxPoint + maxBoundingBoxPoint) / 2.f,
		bounderBoxSize,
		glm::vec3{1, 0, 1}
	);

#pragma endregion

	/*
	std::cout << object->indicesIndex << " : " << object->indicesIndex + object->mesh.indices.size() << std::endl;
	for (int i = object->verticesIndex; i < object->verticesIndex + object->mesh.vertices.size(); i++)
	{
		glm::vec4 currentVector = glm::vec4{
			bufferHandler.perObjectVertices.data[i*3],
			bufferHandler.perObjectVertices.data[i*3 + 1],
			bufferHandler.perObjectVertices.data[i*3 + 2],
			1
		};
		currentVector = currentVector * bufferHandler.perObjectObjectInfoArray.data[object->objectInfoIndex].geometryMatrix;

		currentVector.x = round(currentVector.x * 10)/10;
		currentVector.y = round(currentVector.y * 10)/10;
		currentVector.z = round(currentVector.z * 10)/10;

		bufferHandler.createObject(objectTypes::CUBE, true, currentVector, glm::vec3{ 0.1 }, glm::vec3{1, 0, 0});
	}*/
}