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

	glm::vec3 flowDirection = glm::vec3{ 0, 0, 1 };
	const int ARROWS_PER_AREA = 40;

	FlowFieldVisualizer(BufferHandler& bufferHandler, std::shared_ptr<EngineObject> object) : bufferHandler(bufferHandler), object(object) {
		getBoundaryBox(object, minPoint, maxPoint, bufferHandler);
	}

	void updateVisualization(glm::vec3 (*func)(glm::vec3)) {
		glm::vec2 arrowOriginDimensions;
		glm::vec3 xUnitVec = glm::vec3{1, 0, 0};
		glm::vec3 yUnitVec = glm::vec3{ 0, 1, 0 };
		int j = 0;
		for (int i = 0; i < 3; i++)
		{
			if (flowDirection[i] != 1) {
				arrowOriginDimensions[j++] = (maxPoint[i] - minPoint[i]);
			}
		}
		
		int totalAmountArrowsAllowed = (int)(arrowOriginDimensions[0] * arrowOriginDimensions[1] * (float)ARROWS_PER_AREA);

		while (arrows.size() < totalAmountArrowsAllowed)
		{
			float arrowSpacing = sqrt(1.f/ARROWS_PER_AREA);
			int arrowGridPosX = arrows.size() % (int)round(arrowOriginDimensions[0] / arrowSpacing);
			int arrowGridPosY = (int)round((float)arrows.size() / (arrowOriginDimensions[0] / arrowSpacing));

			glm::vec3 newArrowPosition = (minPoint + maxPoint) * 0.5f + xUnitVec * (float)arrowGridPosX * arrowSpacing + yUnitVec * (float)arrowGridPosY * arrowSpacing - glm::vec3{arrowOriginDimensions/2.f, 0};

			arrows.push_back(bufferHandler.createEngineObject(
				objectTypes::VECTOR,
				true,
				newArrowPosition,
				glm::vec3{ 0.3 },
				glm::vec3{ 1, 0, 0 },
				flowDirection
			));
		}

		for (size_t i = 0; i < arrows.size(); i++)
		{
			arrows[i]->position += deltaTime * func(arrows[i]->position);
			arrows[i]->orientation.setDirection(func(arrows[i]->position));
			bufferHandler.updateEngineObjectMatrix(arrows[i]);
		}
	}
};