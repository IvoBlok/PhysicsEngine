#ifndef TIMEHANDLER_H
#define TIMEHANDLER_H

#include "settings.h"

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

void updateTime() {
    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

#endif