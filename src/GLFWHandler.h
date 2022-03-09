#ifndef GLFW_HANDLER_H
#define GLFW_HANDLER_H

//external
#include <GLAD-GL4.6-Core-NoExt/glad/glad.h>
#include <GLFW-3.3/glfw3.h>

//internal
#include "settings.h"
#include "Camera.h"
 
//std
#include <iostream>

float lastX = (1 / 2) * SCR_WIDTH;
float lastY = (1 / 2) * SCR_HEIGHT;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
std::vector<float> processInput(GLFWwindow* window);

static class GLFWHandler {
public:
	static void init() {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 4);
	};

	static void setGLSettings() {
		glEnable(GL_DEPTH_TEST);                                // enable depth testing
		glEnable(GL_MULTISAMPLE);                               // enable multisampling ( MSAA / anti-aliasing)
		glProvokingVertex(GL_FIRST_VERTEX_CONVENTION);          // set the vertex order convention for proper face culling
		glCullFace(GL_BACK);                                   // set the culling side
		glEnable(GL_CULL_FACE);                                 // enable face culling
	}

	static GLFWwindow* getGLFWWindow(int width, int height, const char* title) {
		GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return nullptr;
		}
		glfwMakeContextCurrent(window);
		
		setCallbacks(window);
		setSettings(window);

		return window;
	}

	static void loadGLAD() {
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
		}
	}

	static void terminateGLFW() {
		glfwTerminate();
	}

private:
	static void setCallbacks(GLFWwindow* window) {
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);
	}

	static void setSettings(GLFWwindow* window) {
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
};

// function declarations

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}

	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos; // reversed since y-coordinates go from bottom to top

	lastX = (float)xpos;
	lastY = (float)ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
#endif