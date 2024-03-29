// external

// internal
#include "BufferHandler.h"

// std
#include <vector>
#include <map>

//TODO: Add support for line intersecting. If a line of the other object passes through the currently checked rectangle, it also represents a collision. Not just points.

void getBoundaryBox(std::shared_ptr<EngineObject> object, glm::vec3& minPoint, glm::vec3& maxPoint, BufferHandler& bufferHandler) {
	for (int i = 0; i < object->mesh.vertices.size(); i++) {
		// retrieve the vertex
		glm::vec4 vertex = glm::vec4{
			object->mesh.vertices[i],
			1
		};

		// transform it to the effective world position
		vertex = vertex * bufferHandler.getDefaultObjectGroupInfo().data[object->getObjectInfoIndex()].geometryMatrix;

		// initialize the boundary points with a value that is guaranteed to be within the mesh
		minPoint = (minPoint == glm::vec3{ 0 }) ? vertex : minPoint;
		maxPoint = (maxPoint == glm::vec3{ 0 }) ? vertex : maxPoint;

		// check whether the point falls outside the current boundary box
		for (int axis = 0; axis < 3; axis++)
		{
			if (vertex[axis] > maxPoint[axis]) { maxPoint[axis] = vertex[axis]; }
			if (vertex[axis] < minPoint[axis]) { minPoint[axis] = vertex[axis]; }
		}
	}
}

bool checkCollisionWithRectangleDomains(BufferHandler* bufferHandler, std::shared_ptr<EngineObject> object, std::shared_ptr<EngineObject> secondObject, bool visualize = false) {
	if (object->getIsInstanced()) { std::cout << "ERROR: given object can not be run for collision because it is an instanced object; unsupported behaviour" << std::endl; return false; }
	
	struct BoundaryBox {
		std::shared_ptr<EngineObject> object;
		glm::vec3 minPoint;
		glm::vec3 maxPoint;
	};
	std::vector<std::vector<BoundaryBox>> boundaryBoxes;

	#pragma region get rectangular bounding box
	glm::vec3 minBoundingBoxPoint = glm::vec3{ 0 };
	glm::vec3 maxBoundingBoxPoint = glm::vec3{ 0 };

	getBoundaryBox(object, minBoundingBoxPoint, maxBoundingBoxPoint, *bufferHandler);

	glm::vec3 boundaryBoxSize = abs(maxBoundingBoxPoint - minBoundingBoxPoint);

	boundaryBoxes.push_back(std::vector<BoundaryBox>{});
	boundaryBoxes[0].push_back(BoundaryBox{});
	if (visualize) {
		boundaryBoxes[0][0].object = bufferHandler->createEngineObject(
			objectTypes::CUBE,
			false,
			(minBoundingBoxPoint + maxBoundingBoxPoint) / 2.f + object->position,
			boundaryBoxSize,
			glm::vec3{ 1, 0, 1 }
		);
	}
	boundaryBoxes[0][0].minPoint = minBoundingBoxPoint;
	boundaryBoxes[0][0].maxPoint = maxBoundingBoxPoint;
	#pragma endregion

	#pragma region block overlaying
	const unsigned short MAX_LAYER_DEPTH = 4;
	const unsigned short LAYER_DIVISION_FACTOR = 3;

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
						bool pointInCurrentBoundaryBox = false;

						// check if there are any points from the mesh in the newly defined region...
						for (int iv = object->getVerticesIndex()/3; iv < object->getVerticesIndex()/3 + object->mesh.vertices.size(); iv++)
						{
							// retrieve the vertex
							glm::vec4 vertex = glm::vec4{
								object->mesh.vertices[iv],
								1
							};
							// transform it to the effective world position
							vertex = vertex * bufferHandler->getDefaultObjectGroupInfo().data[object->getObjectInfoIndex()].geometryMatrix;
							

							// check whether the point falls inside the current boundary box
							if (vertex.x <= newMaxBoundingPoint.x && vertex.x >= newMinBoundingPoint.x &&
								vertex.y <= newMaxBoundingPoint.y && vertex.y >= newMinBoundingPoint.y &&
								vertex.z <= newMaxBoundingPoint.z && vertex.z >= newMinBoundingPoint.z ) {
								
								pointInCurrentBoundaryBox = true;
								break;
							}
						}
						if (!pointInCurrentBoundaryBox) { break; }

						// check second object...
						for (int iv = secondObject->getVerticesIndex() / 3; iv < secondObject->getVerticesIndex() / 3 + secondObject->mesh.vertices.size(); iv++)
						{
							// retrieve the vertex
							glm::vec4 vertex = glm::vec4{
								bufferHandler->getDefaultObjectVertices().data[iv * 3],
								bufferHandler->getDefaultObjectVertices().data[iv * 3 + 1],
								bufferHandler->getDefaultObjectVertices().data[iv * 3 + 2],
								1
							};
							// transform it to the effective world position
							vertex = vertex * bufferHandler->getDefaultObjectGroupInfo().data[secondObject->getObjectInfoIndex()].geometryMatrix + glm::vec4{secondObject->position, 0};

							// check whether the point falls inside the current boundary box
							if (vertex.x <= newMaxBoundingPoint.x && vertex.x >= newMinBoundingPoint.x &&
								vertex.y <= newMaxBoundingPoint.y && vertex.y >= newMinBoundingPoint.y &&
								vertex.z <= newMaxBoundingPoint.z && vertex.z >= newMinBoundingPoint.z) {

								// generator box
								boundaryBoxes[l + 1].push_back(BoundaryBox{});
								boundaryBoxes[l + 1][boundaryBoxes[l + 1].size() - 1].maxPoint = newMaxBoundingPoint;
								boundaryBoxes[l + 1][boundaryBoxes[l + 1].size() - 1].minPoint = newMinBoundingPoint;
								if (l == MAX_LAYER_DEPTH - 1 && visualize) {
									boundaryBoxes[l + 1][boundaryBoxes[l + 1].size() - 1].object = bufferHandler->createEngineObject(
										objectTypes::CUBE,
										true,
										(newMinBoundingPoint + newMaxBoundingPoint) / 2.f + object->position,
										abs(newMaxBoundingPoint - newMinBoundingPoint),
										glm::vec3{ 1, 0.2, 0.2 }
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

	return (boundaryBoxes[1].size() > 0);
}

const size_t hashVec3(glm::vec3& k) {
	std::size_t seed = k.length();

	seed = seed ^ ((int)(k.x * 10000000) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
	seed = seed ^ ((int)(k.y * 1000000) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
	seed = seed ^ ((int)(k.z * 100000) + 0x9e3779b9 + (seed << 6) + (seed >> 2));


	return seed;
}

bool checkCollisionWithSTDMap(BufferHandler* bufferHandler, std::shared_ptr<EngineObject> object, std::shared_ptr<EngineObject> secondObject) {
	std::map<int, int> countMap;
	for (int i = 0; i < object->mesh.vertices.size(); i++) {
		glm::vec3 vertex = glm::vec4{ object->mesh.vertices[i],1 } *bufferHandler->getDefaultObjectGroupInfo().data[object->getObjectInfoIndex()].geometryMatrix;
		vertex += object->position;

		auto result = countMap.insert(std::pair<size_t, int>(hashVec3(vertex), 1));
	}
	for (int i = 0; i < secondObject->mesh.vertices.size(); i++) {
		glm::vec3 vertex = glm::vec4{ secondObject->mesh.vertices[i],1 } *bufferHandler->getDefaultObjectGroupInfo().data[secondObject->getObjectInfoIndex()].geometryMatrix;
		vertex += secondObject->position;

		auto result = countMap.insert(std::pair<size_t, int>(hashVec3(vertex), 1));
		if (result.second == false) {
			result.first->second++;
		}
	}
	return false;
}
