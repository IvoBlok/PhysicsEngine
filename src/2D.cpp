// external
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// internal
#include "GLFWHandler.h"
#include "TimeHandler.h"
#include "BufferHandler.h"

#include "shaders/Shader.h"

// std headers
#include <iostream>
#include <cmath>


struct ObjectInfo_t {
    glm::mat4 geometryMatrix;
    glm::vec4 color;
    glm::vec4 indices;
} objectInfo[15];

int main() {
    // glfw window creation
    // --------------------
    GLFWHandler::init();
    GLFWwindow* window = GLFWHandler::getGLFWWindow(SCR_WIDTH, SCR_HEIGHT, "2D Physics Engine :)");

    // load GLAD
    // --------------------
    GLFWHandler::loadGLAD();

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("src/shaders/simple_lighting_shader.vert",
                     "src/shaders/simple_lighting_shader.frag",
                     "src/shaders/simple_lighting_shader.geom");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
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

    unsigned int indices[] = {
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

        //11, 9, 8, // back side
        //11, 10, 9,

        //12, 13, 15, // front side
        //13, 14, 15,

        //12, 11, 8, // y+ side
        //12, 15, 11,

        //9, 10, 13, // y- side
        //10, 14, 13,

        //8, 9, 12,// x+ side
        //9, 13, 12,

        //15, 10, 11,// x- side
        //15, 14, 10,
    };

    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  6.0f, -13.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    for (int i = 0; i < sizeof(cubePositions) / sizeof(*cubePositions); i++)
    {
        objectInfo[i].geometryMatrix = glm::mat4(1.f);
        objectInfo[i].geometryMatrix = glm::translate(objectInfo[i].geometryMatrix, cubePositions[i]);
        objectInfo[i].color = glm::vec4(i * 0.3 + 0.4);
        objectInfo[i].indices = glm::vec4(8 * i, 8 * i + 8 - 1, 0, 0);
    }

    // initialize openGL settings
    // -------------------------------------------------------------------------------------------
    glEnable(GL_DEPTH_TEST);
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);

    unsigned int VBO, EBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //configure the uniform buffer objects
    // ---------------------------------
    // first. We get the relevant block indices
    unsigned int uniformBlockIndexMatrices = glGetUniformBlockIndex(ourShader.ID, "Matrices");
    // then we link each shader's uniform block to this uniform binding point
    glUniformBlockBinding(ourShader.ID, uniformBlockIndexMatrices, 0);
    // Now actually create the buffer
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    // define the range of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    // calculate projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // store the projection matrix
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //configure the storage buffer objects
    // ---------------------------------
    unsigned int SSBO;
    glGenBuffers(1, &SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(objectInfo), &objectInfo, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // lighting
    // ---------------------------------
    DirLightData directionalLight{ glm::vec3(-.2f, -1.f, -.3f), glm::vec3(.2f), glm::vec3(.4f)};
    ourShader.use();
    ourShader.setDirLight(directionalLight);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        updateTime();

        // input
        // -----------
        std::vector<float> inputs = processInput(window);
        if (inputs.size() > 0) {
            directionalLight.direction = glm::vec3(inputs[0], inputs[1], inputs[2]);
            ourShader.setDirLight(directionalLight);
        }

        // render
        // -----------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // pass view matrix to shader uniform buffer
        glm::mat4 view = camera.GetViewMatrix();
        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setBool("instancing", true);
        // render boxes
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, (GLsizei)(sizeof(indices) / sizeof(*indices)), GL_UNSIGNED_INT, 0, (GLsizei)(sizeof(cubePositions) / sizeof(*cubePositions)));
        //glDrawElements(GL_TRIANGLES, (GLsizei)(sizeof(indices) / sizeof(*indices)), GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -----------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
std::vector<float> processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        std::vector<float> values = {0, 0, 0};
        std::cout << "lightDir: " << std::endl;
        std::cin >> values[0];
        std::cin >> values[1];
        std::cin >> values[2];

        return values;
    }
    return std::vector<float>{};

}