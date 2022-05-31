// external

// internal
#include "BufferHandler.h"

// std
#include <vector>

void generateCollisionCubes(BufferHandler& bufferHandler, std::shared_ptr<EngineObject> object) {
	if (object->instancedObject) { std::cout << "ERROR: given object can not be run for collision because it is an instanced object; unsupported behaviour" << std::endl; return; }
	
	struct BoundaryBox {
		std::shared_ptr<EngineObject> object;
		glm::vec3 minPoint;
		glm::vec3 maxPoint;
	};

	std::vector<std::vector<BoundaryBox>> boundaryBoxes;

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
	glm::vec3 boundaryBoxSize = abs(maxBoundingBoxPoint - minBoundingBoxPoint);

	boundaryBoxes.push_back(std::vector<BoundaryBox>{});
	boundaryBoxes[0].push_back(BoundaryBox{});
	boundaryBoxes[0][0].object = bufferHandler.createObject(
		objectTypes::CUBE,
		false,
		(minBoundingBoxPoint + maxBoundingBoxPoint) / 2.f,
		boundaryBoxSize,
		glm::vec3{1, 0, 1}
	);
	boundaryBoxes[0][0].minPoint = minBoundingBoxPoint;
	boundaryBoxes[0][0].maxPoint = maxBoundingBoxPoint;

#pragma endregion

#pragma region block overlaying
	const unsigned short MAX_LAYER_DEPTH = 3;
	const unsigned short LAYER_DIVISION_FACTOR = 2;

	for (int l = 0; l < MAX_LAYER_DEPTH; l++)
	{
		// push the new vector where the to be generated boxes can be stored
		boundaryBoxes.push_back(std::vector<BoundaryBox>{});

		for (size_t b = 0; b < boundaryBoxes[l].size(); b++)
		{
			glm::vec3 currentBoxBoundarySize = abs(boundaryBoxes[l][b].maxPoint - boundaryBoxes[l][b].minPoint);

			for (int i = 0; i < LAYER_DIVISION_FACTOR; i++)
			{
				for (int j = 0; j < LAYER_DIVISION_FACTOR; j++)
				{
					for (int k = 0; k < LAYER_DIVISION_FACTOR; k++)
					{
						glm::vec3 newMinBoundingPoint = boundaryBoxes[l][b].minPoint
							+ glm::vec3{ (float)i * (currentBoxBoundarySize.x / (float)LAYER_DIVISION_FACTOR),
										(float)j * (currentBoxBoundarySize.y / (float)LAYER_DIVISION_FACTOR),
										(float)k * (currentBoxBoundarySize.z / (float)LAYER_DIVISION_FACTOR)
						};

						glm::vec3 newMaxBoundingPoint = newMinBoundingPoint + (currentBoxBoundarySize / (float)LAYER_DIVISION_FACTOR);
			
						// check if there are any points from the mesh in the newly defined region...
						for (int iv = object->verticesIndex; iv < object->verticesIndex + object->mesh.vertices.size(); iv++)
						{
							// retrieve the vertex
							glm::vec4 vertex = glm::vec4{
								bufferHandler.perObjectVertices.data[iv * 3],
								bufferHandler.perObjectVertices.data[iv * 3 + 1],
								bufferHandler.perObjectVertices.data[iv * 3 + 2],
								1
							};
							// transform it to the effective world position
							vertex = vertex * bufferHandler.perObjectObjectInfoArray.data[object->objectInfoIndex].geometryMatrix;

							// check whether the point falls inside the current boundary box
							if (vertex.x <= newMaxBoundingPoint.x && vertex.x >= newMinBoundingPoint.x &&
								vertex.y <= newMaxBoundingPoint.y && vertex.y >= newMinBoundingPoint.y &&
								vertex.z <= newMaxBoundingPoint.z && vertex.z >= newMinBoundingPoint.z ) {
								
								// generator box
								boundaryBoxes[l + 1].push_back(BoundaryBox{});
								boundaryBoxes[l + 1][boundaryBoxes[l + 1].size() - 1].maxPoint = newMaxBoundingPoint;
								boundaryBoxes[l + 1][boundaryBoxes[l + 1].size() - 1].minPoint = newMinBoundingPoint;
								if (l == MAX_LAYER_DEPTH - 1) {
									boundaryBoxes[l + 1][boundaryBoxes[l + 1].size() - 1].object = bufferHandler.createObject(
										objectTypes::CUBE,
										true,
										(newMinBoundingPoint + newMaxBoundingPoint) / 2.f,
										abs(newMaxBoundingPoint - newMinBoundingPoint),
										glm::vec3{ randomRange(0, 100) / 100.f, randomRange(0, 100) / 100.f, randomRange(0, 100) / 100.f }
									);
								}
								break;
							}
						}
					}
				}
			}
		}
	}




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