#ifndef MESH_H
#define MESH_H

// external
#include <GLM/glm.hpp>
#include <ASSIMP-3.3.1/Importer.hpp>
#include <ASSIMP-3.3.1/scene.h>
#include <ASSIMP-3.3.1/postprocess.h>

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
	std::vector<glm::vec3> normals;

	Mesh(std::vector<float> vertices_ = {}, std::vector<unsigned int> indices_ = {}) {
		for (size_t i = 0; i < vertices_.size(); i+= 3)
		{
			vertices.push_back(glm::vec3(vertices_[i], vertices_[i + 1], vertices_[i + 2]));
		}
		for (size_t i = 0; i < indices_.size(); i++)
		{
			indices.push_back(indices_[i]);
		}
	}

	Mesh(std::string const& path) {
		loadModel(path);
	}

	void loadModel(std::string const& path) {
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}
		// retrieve the directory path of the filepath
		std::string directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode* node, const aiScene* scene)
	{
		if (node->mNumMeshes > 1) {
			std::cout << "ERROR::ASSIMP: number of meshes too large" << std::endl;
			return;
		}
		aiMesh* mesh = scene->mMeshes[node->mMeshes[0]];
		processMesh(mesh, scene);

		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}
	}

    void processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            glm::vec3 vertex;
            glm::vec3 normal;
            // positions
            vertex.x = mesh->mVertices[i].x;
            vertex.y = mesh->mVertices[i].y;
            vertex.z = mesh->mVertices[i].z;
            // normals
            if (mesh->HasNormals())
            {
                normal.x = mesh->mNormals[i].x;
                normal.y = mesh->mNormals[i].y;
                normal.z = mesh->mNormals[i].z;
            }
            vertices.push_back(vertex);
            normals.push_back(normal);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
    }
};

#endif