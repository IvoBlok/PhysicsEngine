// external
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// internal
#include "MathFunctions.h"

#include "GLFWHandler.h"
#include "TimeHandler.h"
#include "BufferHandler.h"

#include "shaders/Shader.h"

// std headers
#include <iostream>
#include <cmath>

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
    BufferHandler bufferHandler{camera, ourShader};

    EngineObject& cube1 = bufferHandler.engineObjects[bufferHandler.createEngineObject("cube")];
    EngineObject& cube2 = bufferHandler.engineObjects[bufferHandler.createEngineObject("cube", glm::vec3(2, 6, -13), glm::vec3(1, 0, 0))];
    
    // initialize openGL settings
    // -------------------------------------------------------------------------------------------
    
    glEnable(GL_DEPTH_TEST);
    glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

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
        if (inputs.size() == 3) {
            directionalLight.direction = glm::vec3(inputs[0], inputs[1], inputs[2]);
            ourShader.setDirLight(directionalLight);
        }
        else if (inputs.size() == 1 && inputs[0] == -1) {
            bufferHandler.engineObjects[bufferHandler.createEngineObject("cube", glm::vec3(randomRange(-5, 5), randomRange(-5, 5), randomRange(-5, 5)))];
        }

        //std::cout << "Objects: " << bufferHandler.objectCount << "   FPS: " << 1 / deltaTime << std::endl;

        // render
        // -----------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bufferHandler.draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -----------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // deallocate buffers
    // ------------------------------------------------------------------------
    bufferHandler.~BufferHandler();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    GLFWHandler::terminateGLFW();
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
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        std::vector<float> returnValues = { -1 };
        return returnValues;
    }
    return std::vector<float>{};
}