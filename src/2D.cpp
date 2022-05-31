// external
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>

// internal
#include "MathFunctions.h"
#include "settings.h"

#include "GLFWHandler.h"
#include "TimeHandler.h"
#include "BufferHandler.h"

#include "shaders/Shader.h"

#include "Collision.h"

// std headers
#include <iostream>
#include <cmath>

bool buttonPressed = false;

int main() {
    // glfw window creation
    // -----------
    GLFWHandler::init();
    GLFWwindow* window = GLFWHandler::getGLFWWindow(SCR_WIDTH, SCR_HEIGHT, "3D Engine :)");

    GLFWHandler::loadGLAD();
    GLFWHandler::setGLSettings();

    // Setup bufferhandler and shaders
    // -----------
    BufferHandler bufferHandler{};
    bufferHandler.window = window;

    Shader& instancingShader = bufferHandler.createShader(
        true,
        "src/shaders/shader_instancing.vert",
        "src/shaders/shader_instancing.frag",
        "src/shaders/shader_instancing.geom"
    );

    Shader& perObjectShader = bufferHandler.createShader(
        false,
        "src/shaders/shader_per_object.vert",
        "src/shaders/shader_per_object.frag",
        "src/shaders/shader_per_object.geom"
    );

    // lighting
    // -----------
    DirLightData directionalLight{ glm::vec3(0.6f, 0.6f, 0.6f), glm::vec3(.2f), glm::vec3(.8f)};
    bufferHandler.setDirLight(directionalLight);

    // Objects
    // -----------
    std::shared_ptr<EngineObject> vehicle = bufferHandler.createObject(objectTypes::MODEL, false, glm::vec3{ 0 }, glm::vec3{ 0.001 });
    std::shared_ptr<EngineObject> vehicle2 = bufferHandler.createObject(objectTypes::MODEL, false, glm::vec3{ 1.5,0,0 }, glm::vec3{ 0.001 });

    generateCollisionCubes(bufferHandler, vehicle, vehicle2);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        updateTime();
        processInput(window);

        bufferHandler.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    bufferHandler.~BufferHandler();

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
        std::cout << "translate: " << std::endl;
        std::cin >> values[0];
        std::cin >> values[1];
        std::cin >> values[2];

        return values;
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        std::vector<float> returnValues = { -1 };
        return returnValues;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        std::vector<float> returnValues = { -2 };
        return returnValues;
    }
    return std::vector<float>{};
}