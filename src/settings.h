#ifndef SETTINGS_H
#define SETTINGS_H

#include "Camera.h"

// Camera
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

Camera camera{ glm::vec3(1.0f, 2.0f, 5.0f) };

// Engine Objects
const unsigned int INITIAL_VERTEX_BUFFER_CAPACITY = 100;
const unsigned int INITIAL_INDEX_BUFFER_CAPACITY = 300;
const unsigned int INITIAL_OBJECT_CAPACITY = 10;

// Draw
const float backgroundColor[4] = { 0.2f, 0.3f, 0.3f, 1.0f };
const unsigned int MAX_PER_OBJECTS_COUNT = 100; // also update this constant in the shader_per_object.vert shader

// Debugging
const bool ExternalDebug = false;
#endif