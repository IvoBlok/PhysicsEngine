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

//TODO if objectMeshes change, bufferhandler should check if that object was in an instanced group and update accordingly

class BufferHandler {
public:
	float* vertices = new float[0];
	unsigned int* indices = new unsigned int[0];
    
    unsigned int vertexCount, indexCount, objectCount;

	ObjectInfo_t* objectInfo = new ObjectInfo_t[0];
	EngineObject* engineObjects = new EngineObject[0];

    std::vector<std::vector<int>> instancingGroups; // every group is a vector of indices to the relevant objectinfo object
    std::vector<int> combinedDrawCallGroup;

	BufferHandler(Camera& camera_, Shader& shader_) : camera(camera_), shader(shader_) {
		vertices = new float[INITIAL_VERTEX_BUFFER_CAPACITY];
		indices = new unsigned int[INITIAL_INDEX_BUFFER_CAPACITY];
        objectInfo = new ObjectInfo_t[INITIAL_OBJECT_CAPACITY];
        engineObjects = new EngineObject[INITIAL_OBJECT_CAPACITY];

		vertexCount = indexCount = objectCount = 0;
	};

	// function responsible for creating a new engine object, loading the model data, classifying it as instanced or not and updating the buffers
	unsigned int createEngineObject(std::string objectType, glm::vec3 position = glm::vec3(0.f), glm::vec3 color = glm::vec3(1.f)) {

        Mesh mesh = getPrimaryShapeMesh(objectType);
        if (mesh.vertices.size() == 0) { return NULL; }

        addObjectDataSlot();

        // check if the new object can be drawn as an instance of others
        int matchingObjectIndex = matchObjectMeshToKnow(mesh);
        std::cout << "matchingObjectIndex: " << matchingObjectIndex << std::endl;
        if (matchingObjectIndex != -1 && ALLOW_INSTANCED_DRAWING) {
            int groupIndex = findValueInNestedVector(matchingObjectIndex);
            if (groupIndex != NULL) {
                instancingGroups[groupIndex].push_back(objectCount - 1);
            }
            else {
                instancingGroups.push_back(std::vector<int>{});
                instancingGroups[instancingGroups.size() - 1].push_back(matchingObjectIndex);
                instancingGroups[instancingGroups.size() - 1].push_back(objectCount - 1);
            }
            
            std::cout << "INSTANCING WORKING!";

            objectInfo[objectCount - 1].indices = objectInfo[matchingObjectIndex].indices;
        }
        else {
            std::vector<int> indices = addToArrays(mesh);
            objectInfo[objectCount - 1].indices = glm::vec4(indices[0], indices[1], indices[2], indices[3]);
        }
        
        // Update the object and objectinfo arrays
        objectInfo[objectCount - 1].color = glm::vec4(color, 0);
        objectInfo[objectCount - 1].geometryMatrix = glm::translate(objectInfo[objectCount - 1].geometryMatrix, position);

        engineObjects[objectCount - 1].objectInfoIndex = objectCount - 1;

        updateBuffers();
        
        // return the new engine object
        return objectCount - 1;
	};

	void updateBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount, vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexCount, indices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        //configure the uniform buffer objects
        // ---------------------------------
        // first. We get the relevant block indices
        unsigned int uniformBlockIndexMatrices = glGetUniformBlockIndex(shader.ID, "Matrices");
        // then we link each shader's uniform block to this uniform binding point
        glUniformBlockBinding(shader.ID, uniformBlockIndexMatrices, 0);
        // Now actually create the buffer
        glGenBuffers(1, &UBO);
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        // define the range of the buffer that links to a uniform binding point
        glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, 2 * sizeof(glm::mat4));

        // calculate projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // store the projection matrix
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        //configure the storage buffer objects
        // ---------------------------------
        glGenBuffers(1, &SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ObjectInfo_t) * objectCount, objectInfo, GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    };

    // function responsible for executing the sorted draw operations. a potentially large list of instanced draw calls and a final draw call for the leftovers
    void draw() {
        /* OLD ?
        glm::mat4 view = camera.GetViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        shader.setVec3("viewPos", camera.Position);

        // render boxes
        glBindVertexArray(VAO);
        
        //ourShader.setBool("instancing", true);
        //glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)(sizeof(indices) / sizeof(*indices)), GL_UNSIGNED_INT, 0, (GLsizei)(sizeof(cubePositions) / sizeof(*cubePositions)));
        
        glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, 0); */
        // NEW?
        shader.setVec3("viewPos", camera.Position);
        glm::mat4 view = camera.GetViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        for (size_t i = 0; i < instancingGroups.size(); i++)
        {
            glBindVertexArray(VAOs[i + 1]);
            glBindBuffer(GL_ARRAY_BUFFER, VBOs[i + 1]);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[i + 1]);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOs[i + 1]);

            GLsizei indicesSize = objectInfo[instancingGroups[i][0]].indices[3] - objectInfo[instancingGroups[i][0]].indices[2] + 1;
            glDrawElementsInstanced(GL_TRIANGLES, indicesSize, GL_UNSIGNED_INT, 0, (GLsizei)instancingGroups[i].size());
        }
        glBindVertexArray(VAOs[0]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOs[0]);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOs[0]);
        shader.setBool("instancing", false);
        glDrawElements(GL_TRIANGLES, (GLsizei)combinedDrawCallIndexSize, GL_UNSIGNED_INT, 0);
    }

    ~BufferHandler() {
        for (size_t i = 0; i < VBOs.size(); i++)
        {
            glDeleteVertexArrays(1, &VAOs[i]);
            glDeleteBuffers(1, &VBOs[i]);
            glDeleteBuffers(1, &EBOs[i]);
            glDeleteBuffers(1, &SSBOs[i]);
        }

        glDeleteBuffers(1, &UBO);

        /* I got no clue why this gives a 'break' error
        delete[] vertices;
        delete[] indices;
        delete[] objectInfo;
        delete[] engineObjects;
        */
    }

private:
    int vertexCapacity = INITIAL_VERTEX_BUFFER_CAPACITY; int indexCapacity = INITIAL_INDEX_BUFFER_CAPACITY; int objectCapacity = INITIAL_OBJECT_CAPACITY;
	Camera& camera;
    Shader& shader;
    std::vector<unsigned int> VBOs;
    std::vector<unsigned int> EBOs;
    std::vector<unsigned int> VAOs;
    std::vector<unsigned int> SSBOs;
    unsigned int UBO;

    int combinedDrawCallIndexSize;

    std::vector<int> addToArrays(Mesh& mesh) {
        std::vector<int> returnValues;

        //TODO: IMPLEMENT SYSTEM TO CHECK IF THE NEW VERTICES AREN'T ALREADY IN THE VERTEX BUFFER, IF SO THE INDICES CORRESPONDING TO THAT VERTEX NEED TO BE CHANGED
        float* tempVertices;
        unsigned int* tempIndices;

        int newIndexValuesCount = (int)mesh.indices.size();
        int newVertexValuesCount = (int)mesh.vertices.size() * 3; // multiply by 3 to compensate for the fact that mesh.vertices is stored as a vec3, and bufferhandler vertices in float array

        // Vertices
        // ==================================
        // If the max capacity of the array has been reached, update the array to be longer and copy over the old values
        if ((int)vertexCount + newVertexValuesCount > vertexCapacity) {
            std::cout << "vertex capacity reached" << std::endl;
            vertexCapacity *= 2;
            tempVertices = new float[vertexCapacity];
            for (int i = 0; i < (int)vertexCount; i++)
            {
                tempVertices[i] = vertices[i];
            }
            delete[] vertices;
            vertices = tempVertices;
            tempVertices = nullptr;
        }
        //! All vertices of an object need to be besides each other
        returnValues.push_back(vertexCount / 3); // This will be the index to the first vertex in the new object
        returnValues.push_back((vertexCount + newVertexValuesCount) / 3 - 1); // This will be the index to the last vertex of the new object
        // Copy over the new values

        for (int i = vertexCount; i < (int)vertexCount + newVertexValuesCount; i += 3) // go over every position with an increase of 3 every cycle to make the indexing easier
        {
            for (int j = 0; j < 3; j++)
            {
               // std::cout << i + j << " : " << i - vertexCount << " " << j << std::endl;
                vertices[i + j] = mesh.vertices[((size_t)i - (size_t)vertexCount)/3][j];
            }
        }

        // Indices
        // ==================================
        // If the max capacity of the array has been reached, update the array to be longer and copy over the old values
        if ((int)indexCount + newIndexValuesCount > indexCapacity) {
            std::cout << "index capacity reached" << std::endl;
            indexCapacity *= 2;
            tempIndices = new unsigned int[indexCapacity];
            for (int i = 0; i < (int)indexCount; i++)
            {
                tempIndices[i] = indices[i];
            }
            delete[] indices;
            indices = tempIndices;
            tempIndices = nullptr;
        }
        // Copy over the new values
        for (int i = indexCount; i < (int)indexCount + newIndexValuesCount; i++) // go over every index value
        {
            indices[i] = mesh.indices[(size_t)i - (size_t)indexCount] + vertexCount/3; // the old VertexCount is added cause THE INDEX THAT IS WRITTEN TO THE INDEX BUFFER NEEDS TO BE UPDATED, AS THE VERTICES THEY WERE REFERRING TO DONT (HAVE TO) START AT INDEX 0
        }

        returnValues.push_back(indexCount); // This will be the index to the first vertex in the new object
        returnValues.push_back(indexCount + newIndexValuesCount - 1); // This will be the index to the last vertex of the new object
        // Update the new Count
        indexCount += newIndexValuesCount;
        vertexCount += newVertexValuesCount;

        return returnValues;
    };

    Mesh getPrimaryShapeMesh(std::string type) {
        if (type == "cube") {
            std::vector<float> vertices = {
                // positions      
                 0.5f,  0.5f, -0.5f,
                 0.5f, -0.5f, -0.5f,
                -0.5f, -0.5f, -0.5f,
                -0.5f,  0.5f, -0.5f,

                 0.5f,  0.5f,  0.5f,
                 0.5f, -0.5f,  0.5f,
                -0.5f, -0.5f,  0.5f,
                -0.5f,  0.5f,  0.5f,
            };

            std::vector<unsigned int> indices = {
                3, 1, 0, // back side
                3, 2, 1,

                4, 5, 7, // front side
                5, 6, 7,

                4, 3, 0, // y+ side
                4, 7, 3,

                1, 2, 5, // y- side
                2, 6, 5,

                0, 1, 4,// x+ side
                1, 5, 4,

                7, 2, 3,// x- side
                7, 6, 2,
            };
            Mesh mesh{vertices, indices};
            
            return mesh;
        }
    }

    void addObjectDataSlot() {
        ObjectInfo_t* tempObjectInfo;
        EngineObject* tempEngineObjects;

        if ((int)objectCount + 1 > objectCapacity) {
            std::cout << "object capacity reached" << std::endl;
            objectCapacity *= 2;
            tempObjectInfo = new ObjectInfo_t[objectCapacity];
            tempEngineObjects = new EngineObject[objectCapacity];
            for (int i = 0; i < (int)objectCount; i++)
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
        objectCount++;
    }

    int matchObjectMeshToKnow(Mesh mesh_) {
        for (int i = 0; i < (int)objectCount - 1; i++)
        {
            std::cout << "i: " << i << std::endl;
            int firstVertexIndex = objectInfo[i].indices[0];
            int firstIndexIndex = objectInfo[i].indices[2];
            // first simple checks to minimize the amount of times we go over every vertex
            
            //std::cout << mesh_.vertices.size() << " " << objectInfo[i].indices[1] - firstVertexIndex + 1 << std::endl;
            //std::cout << mesh_.indices.size() << " " << objectInfo[i].indices[3] - firstIndexIndex + 1<< std::endl;

            if (mesh_.vertices.size() != objectInfo[i].indices[1] - firstVertexIndex + 1) { goto next; }
            if (mesh_.indices.size() != objectInfo[i].indices[3] - firstIndexIndex + 1) { goto next; }

            // check if the vertices overlap
            for (size_t j = 0; j < mesh_.vertices.size(); j++)
            {
                for (int k = 0; k < 3; k++)
                {
                    //std::cout << "1: " << j << " " << k << " : " << firstVertexIndex + 3 * j + k << std::endl;
                    //std::cout << "2: " << mesh_.vertices[j][k] << " : " << vertices[firstVertexIndex + 3 * j + k] << std::endl;
                    if (mesh_.vertices[j][k] != vertices[firstVertexIndex + 3 * j + k]) { goto next; }
                }
            }

            // check if the indices overlap
            std::cout << "Index size: " << mesh_.indices.size();
            for (size_t j = 0; j < mesh_.indices.size(); j++)
            {
                std::cout << "1: " << j << " : " << firstIndexIndex + j - firstVertexIndex << std::endl;
                std::cout << "2: " << mesh_.indices[j] << " : " << indices[firstIndexIndex + j - firstVertexIndex] << std::endl;
                if (mesh_.indices[j] != indices[firstIndexIndex + j - firstVertexIndex]) { goto next; }
            }
            return i;

            next:
            continue;
        }
        return -1;
    }

    int findValueInNestedVector(int index) {
        for (size_t i = 0; i < instancingGroups.size(); i++)
        {
            for (size_t j = 0; j < instancingGroups[i].size(); j++)
            {
                if (instancingGroups[i][j] == index) { return i; }
            }
        }
        return NULL;
    }
};

#endif