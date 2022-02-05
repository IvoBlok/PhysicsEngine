#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H

//external
#include <ASSIMP-3.3.1/Importer.hpp>
#include <ASSIMP-3.3.1/scene.h>
#include <ASSIMP-3.3.1/postprocess.h>
#include <external/stb_image.h>

//internal
#include "settings.h"
#include "Camera.h"
#include "EngineObject.h"
#include "Mesh.h"

class BufferHandler {
public:
	float* vertices = new float[0];
	unsigned int* indices = new unsigned int[0];

	ObjectInfo_t* objectInfo = new ObjectInfo_t[0];
	EngineObject* engineObjects = new EngineObject[0];

	BufferHandler(Camera& camera_) : camera(camera_) {
		vertices = new float[INITIAL_VERTEX_BUFFER_CAPACITY];
		indices = new unsigned int[INITIAL_INDEX_BUFFER_CAPACITY];
        objectInfo = new ObjectInfo_t[INITIAL_OBJECT_CAPACITY];
        engineObjects = new EngineObject[INITIAL_OBJECT_CAPACITY];

		vertexCount = indexCount = objectCount = 0;
	};

    ~BufferHandler() {
        if (vertices) delete[] vertices;
        if (indices) delete[] indices;
        if (objectInfo) delete[] objectInfo;
        if (engineObjects) delete[] engineObjects;
    }

	// function responsible for creating a new engine object, loading the model data, classifying it as instanced or not and updating the buffers
	EngineObject& createEngineObject(const char* path, glm::vec3 position = glm::vec3(0.f), glm::vec3 color = glm::vec3(1.f)) {
		std::vector<Mesh>    meshes;
		std::string directory;

        if (meshes.size() > 1) { std::cout << "ERROR::OBJECTS WITH MULTIPLE MESHES ARE NOT SUPPORTED YET! " << std::endl; return; }

        loadModel(path, directory, meshes);
        std::vector<int> indices = addToArrays(meshes[0]);

        // Update the object and objectinfo arrays
        EngineObject object_ = EngineObject{};
        ObjectInfo_t objectInfo_ = ObjectInfo_t{};
        objectInfo_.indices[0] = indices[0];
        objectInfo_.indices[1] = indices[1];
        objectInfo_.color = glm::vec4(color, 0);
        objectInfo_.geometryMatrix = glm::translate(objectInfo_.geometryMatrix, position);

        addObjectData(objectInfo_, object_);

        updateBuffers();

        // return the new engine object
        return object_;
	};

	// function responsible for updating the buffers with the current data held in 'vertices', 'indices' and 'objectInfo'
	void updateBuffers() {};

	// function responsible for executing the sorted draw operations. a potentially large list of instanced draw calls and a final draw call for the leftovers
	void draw() {};

private:
	unsigned int vertexCount, indexCount, objectCount;
    int vertexCapacity = INITIAL_VERTEX_BUFFER_CAPACITY; int indexCapacity = INITIAL_INDEX_BUFFER_CAPACITY; int objectCapacity = INITIAL_OBJECT_CAPACITY;
	Camera& camera;

    std::vector<int> addToArrays(Mesh& mesh) {
        std::vector<int> returnValues;

        //TODO: IMPLEMENT SYSTEM TO CHECK IF THE NEW VERTICES AREN'T ALREADY IN THE VERTEX BUFFER, IF SO THE INDICES CORRESPONDING TO THAT VERTEX NEED TO BE CHANGED
        float* tempVertices;
        unsigned int* tempIndices;

        int newIndexValuesCount = mesh.indices.size();
        int newVertexValuesCount = mesh.vertices.size() * 3;

        // Vertices
        // ==================================
        // If the max capacity of the array has been reached, update the array to be longer and copy over the old values
        if (vertexCount + newVertexValuesCount > vertexCapacity) {
            vertexCapacity *= 2;
            tempVertices = new float[vertexCapacity];
            for (int i = 0; i < vertexCount; i++)
            {
                tempVertices[i] = vertices[i];
            }
            delete[] vertices;
            vertices = tempVertices;
            tempVertices = nullptr;
        }
        //! All vertices of an object need to be besides each other
        returnValues.push_back(vertexCount); // This will be the index to the first vertex in the new object
        returnValues.push_back(vertexCount + newVertexValuesCount - 1); // This will be the index to the last vertex of the new object
        // Copy over the new values
        for (int i = vertexCount; i < vertexCount + newVertexValuesCount; i += 3) // go over every position with an increase of 3 every cycle to make the indexing easier
        {
            for (int j = 0; j < 3; j++)
            {
                vertices[i + j] = mesh.vertices[(size_t)i - (size_t)vertexCount].position[j];
            }
        }

        // Indices
        // ==================================
        // If the max capacity of the array has been reached, update the array to be longer and copy over the old values
        if (indexCount + newIndexValuesCount > indexCapacity) {
            indexCapacity *= 2;
            tempIndices = new unsigned int[indexCapacity];
            for (int i = 0; i < indexCount; i++)
            {
                tempIndices[i] = indices[i];
            }
            delete[] indices;
            indices = tempIndices;
            tempIndices = nullptr;
        }
        // Copy over the new values
        for (int i = indexCount; i < indexCount + newIndexValuesCount; i++) // go over every index value
        {
            indices[i] = mesh.indices[(size_t)i - (size_t)indexCount] + vertexCount; // the old VertexCount is added cause THE INDEX THAT IS WRITTEN TO THE INDEX BUFFER NEEDS TO BE UPDATED, AS THE VERTICES THEY WERE REFERRING TO DONT (HAVE TO) START AT INDEX 0
        }
        // Update the new Count
        indexCount += newIndexValuesCount;
        vertexCount += newVertexValuesCount;

        return returnValues;
    };

    void addObjectData(ObjectInfo_t& objectInfo_, EngineObject& object) {
        ObjectInfo_t* tempObjectInfo;
        EngineObject* tempEngineObjects;

        if (objectCount + 1 > objectCapacity) {
            objectCapacity *= 2;
            tempObjectInfo = new ObjectInfo_t[objectCapacity];
            tempEngineObjects = new EngineObject[objectCapacity];
            for (int i = 0; i < objectCount; i++)
            {
                tempObjectInfo[i] = objectInfo[i];
                tempEngineObjects[i] = engineObjects[i];
            }
            delete[] objectInfo;
            delete[] engineObjects;
            objectInfo = tempObjectInfo;
            engineObjects = tempEngineObjects;

            tempObjectInfo = nullptr;
            tempEngineObjects = nullptr;
        }
        objectInfo[objectCount] = objectInfo_;
        engineObjects[objectCount] = object;
        object.objectInfoIndex = objectCount;
        objectCount++;
    }

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const& path, std::string& directory, std::vector<Mesh>& meshes)
    {
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
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene, meshes);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, meshes);
        }
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            }
            else
                vertex.texCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:


        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }
};

#endif