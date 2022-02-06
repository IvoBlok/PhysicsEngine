#ifndef MESH_H
#define MESH_H

// external
#include <GLM/glm.hpp>

// internal
#include <shaders/Shader.h>

//std headers
#include <string>
#include <vector>

class Mesh {
public:
	// mesh data
	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	Mesh(std::vector<float> vertices_, std::vector<unsigned int> indices_) {
		for (size_t i = 0; i < vertices_.size(); i+= 3)
		{
			vertices.push_back(glm::vec3(vertices_[i], vertices_[i + 1], vertices_[i + 2]));
		}
		for (size_t i = 0; i < indices_.size(); i++)
		{
			indices.push_back(indices_[i]);
		}
	}
};

#endif