#ifndef SETTINGS_H
#define SETTINGS_H

#include "Camera.h"

// Camera
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// Engine Objects
const unsigned int INITIAL_VERTEX_BUFFER_CAPACITY = 100;
const unsigned int INITIAL_INDEX_BUFFER_CAPACITY = 300;
const unsigned int INITIAL_OBJECT_CAPACITY = 10;

#endif