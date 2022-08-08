#pragma once

#include "EngineObject.h"
#include "BufferHandler.h"
#include "Collision.h"
#include "TimeHandler.h"

#include <GLM/glm.hpp>
#include <vector>

class FlowFieldVisualizer {
public:
	BufferHandler& bufferHandler;
	std::shared_ptr<EngineObject> object;
	std::vector<std::shared_ptr<EngineObject>> arrows;

	glm::vec3 minPoint;
	glm::vec3 maxPoint;

	const int ARROWS_PER_AREA = 100;

	FlowFieldVisualizer(BufferHandler& bufferHandler, std::shared_ptr<EngineObject> object, bool visualizeBoundary = false) : bufferHandler(bufferHandler), object(object) {
		getBoundaryBox(object, minPoint, maxPoint, bufferHandler);
		
		if (visualizeBoundary) {
			bufferHandler.createEngineObject(
				objectTypes::CUBE,
				false,
				(minPoint + maxPoint) / 2.f + object->position,
				abs(maxPoint - minPoint),
				glm::vec3{ 1, 0, 1 }
			);
		}
	}

	void updateVisualization(glm::vec3(*func)(glm::vec3), glm::vec3 initialFlowDirection = glm::vec3{0, 0, 1}) {
		// check if flow direction is cardinal direction
		
		float directionSum = initialFlowDirection[0] + initialFlowDirection[1] + initialFlowDirection[2];
		float directionSize = sqrt(pow(initialFlowDirection[0], 2) + pow(initialFlowDirection[1], 2) + pow(initialFlowDirection[2], 2));
		if (directionSize != 1.f || abs(directionSum) != 1.f) { std::cout << "ERROR::UNSUPPORTED BEHAVIOUR! Given flow direction is not in a cardinal direction. \n"; return; }

		glm::vec2 arrowOriginDimensions;
		glm::vec3 arrowOriginPlaneMinPoint = minPoint;
		glm::vec3 arrowOriginPlaneMaxPoint = maxPoint;

		arrowOriginPlaneMinPoint -= glm::vec3{ abs(initialFlowDirection[0]) * arrowOriginPlaneMinPoint[0], abs(initialFlowDirection[1]) * arrowOriginPlaneMinPoint[1], abs(initialFlowDirection[2]) * arrowOriginPlaneMinPoint[2] };
		arrowOriginPlaneMaxPoint -= glm::vec3{abs(initialFlowDirection[0]) * arrowOriginPlaneMaxPoint[0], abs(initialFlowDirection[1]) * arrowOriginPlaneMaxPoint[1], abs(initialFlowDirection[2]) * arrowOriginPlaneMaxPoint[2] };

		glm::vec3 xUnitVec = glm::vec3{0};
		glm::vec3 yUnitVec = glm::vec3{0};

		if (glm::dot(initialFlowDirection, glm::vec3{ 1, 0, 0 }) == 0) { if (xUnitVec == glm::vec3{ 0 }) { xUnitVec = glm::vec3{ 1, 0, 0 }; } else { yUnitVec = glm::vec3{ 1, 0, 0 }; } }
		if (glm::dot(initialFlowDirection, glm::vec3{ 0, 1, 0 }) == 0) { if (xUnitVec == glm::vec3{ 0 }) { xUnitVec = glm::vec3{ 0, 1, 0 }; } else { yUnitVec = glm::vec3{ 0, 1, 0 }; } }
		if (glm::dot(initialFlowDirection, glm::vec3{ 0, 0, 1 }) == 0) { if (xUnitVec == glm::vec3{ 0 }) { xUnitVec = glm::vec3{ 0, 0, 1 }; } else { yUnitVec = glm::vec3{ 0, 0, 1 }; } }

		if (directionSum < 0) {
			arrowOriginPlaneMinPoint += glm::dot(maxPoint, initialFlowDirection) * initialFlowDirection;
			arrowOriginPlaneMaxPoint += glm::dot(maxPoint, initialFlowDirection) * initialFlowDirection;
		}
		else {
			arrowOriginPlaneMinPoint += glm::dot(minPoint, initialFlowDirection) * initialFlowDirection;
			arrowOriginPlaneMaxPoint += glm::dot(minPoint, initialFlowDirection) * initialFlowDirection;
		}
		arrowOriginDimensions = glm::vec2{glm::dot((arrowOriginPlaneMaxPoint - arrowOriginPlaneMinPoint), xUnitVec), glm::dot((arrowOriginPlaneMaxPoint - arrowOriginPlaneMinPoint), yUnitVec) };
		
		int totalAmountArrowsAllowed = (int)(arrowOriginDimensions[0] * arrowOriginDimensions[1] * (float)ARROWS_PER_AREA);

		// initialize arrows if needed
		while (arrows.size() < totalAmountArrowsAllowed)
		{
			float arrowSpacing = sqrt(1.f/ARROWS_PER_AREA);
			int arrowGridPosX = arrows.size() % (int)round(arrowOriginDimensions[0] / arrowSpacing);
			int arrowGridPosY = (int)round((float)arrows.size() / (arrowOriginDimensions[0] / arrowSpacing));

			glm::vec3 newArrowPosition = (arrowOriginPlaneMinPoint + arrowOriginPlaneMaxPoint) * 0.5f + xUnitVec * (float)arrowGridPosX * arrowSpacing + yUnitVec * (float)arrowGridPosY * arrowSpacing - glm::vec3{arrowOriginDimensions/2.f, 0};

			arrows.push_back(bufferHandler.createEngineObject(
				objectTypes::VECTOR,
				true,
				newArrowPosition,
				glm::vec3{ 0.2 },
				glm::vec3{ 1, 0, 0 },
				initialFlowDirection
			));
		}

		// update arrow positions and directions
		for (size_t i = 0; i < arrows.size(); i++)
		{
			arrows[i]->position += deltaTime * func(arrows[i]->position);
			if(!isPointInBoundaryBox(arrows[i]->position)) {
				float arrowSpacing = sqrt(1.f / ARROWS_PER_AREA);
				int arrowGridPosX = i % (int)round(arrowOriginDimensions[0] / arrowSpacing);
				int arrowGridPosY = (int)round((float)i / (arrowOriginDimensions[0] / arrowSpacing));

				glm::vec3 newArrowPosition = (arrowOriginPlaneMinPoint + arrowOriginPlaneMaxPoint) * 0.5f + xUnitVec * (float)arrowGridPosX * arrowSpacing + yUnitVec * (float)arrowGridPosY * arrowSpacing - glm::vec3{ arrowOriginDimensions / 2.f, 0 };
				arrows[i]->position = newArrowPosition;
			}
			arrows[i]->orientation.setDirection(func(arrows[i]->position));
			bufferHandler.updateEngineObjectMatrix(arrows[i]);
		}
	}

private:
	bool isPointInBoundaryBox(glm::vec3 point) {
		return !(point.x < minPoint.x || point.y < minPoint.y || point.z < minPoint.z || point.x > maxPoint.x || point.y > maxPoint.y || point.z > maxPoint.z);
	}
};