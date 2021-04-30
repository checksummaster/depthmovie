
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>




using namespace std;

static GLFWwindow* window;
static float angleX = 0.0f, angleY = 0.0f;

static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float d = 1.f;
	if (width <= height)
		glOrtho(-d, d, -d * (GLfloat)height / (GLfloat)width,
			d * (GLfloat)height / (GLfloat)width, -d, d);
	else
		glOrtho(-d * (GLfloat)width / (GLfloat)height,
			d * (GLfloat)width / (GLfloat)height, -d, d, -d, d);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static double xpos, ypos;
static bool press = false;
static float angleXatPRess, angleYatPRess;
static double xposatPRess, yposatPRess;

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (press) {
		angleX = angleXatPRess + (xpos - xposatPRess);
		angleY = angleYatPRess + (ypos - yposatPRess);
	}
}



void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{


	glfwGetCursorPos(window, &xpos, &ypos); 
	if (button == GLFW_MOUSE_BUTTON_LEFT )
	{
		if (action == GLFW_PRESS) {
			press = true;
			angleXatPRess = angleX;
			angleYatPRess = angleY;
			xposatPRess = xpos;
			yposatPRess = ypos;
		} 
		if (action == GLFW_RELEASE) {
			press = false;
		}
	}
	

}

void view_init(void)
{
	
	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
		exit(EXIT_FAILURE);

	window = glfwCreateWindow(800, 600, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetWindowSizeCallback(window, window_size_callback);


	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_FLAT);

}


#include "opencv2/imgproc.hpp"



bool view_loop(cv::Mat m, cv::Mat c, float factor)
{
	if (!glfwWindowShouldClose(window))
	{
		
		float* pixelPtr = (float*)m.data;
		uint8_t* pixelPtrColor = (uint8_t*)c.data;
		int cn = c.channels();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor3f(1.0, 1.0, 1.0);
		glPushMatrix();
		//glRotatef((float)glfwGetTime() * 50.f, (float)glfwGetTime() * 25.f, (float)glfwGetTime() * 12.5f, 1.0);
		//glRotatef((float)glfwGetTime() * 25.f,  0.0, 1.0, 0.0);

		glRotatef(angleY, 1.0, 0.0, 0.0);
		glRotatef(angleX, 0.0, 1.0, 0.0);


		glBegin(GL_POINTS);
		for (int y = 0; y <= m.rows; y++) {
			
			for (int x = 0; x <= m.cols; x++) {
				float pix = -pixelPtr[y * m.cols + x];
				//if (pix >= 0) {
					uint8_t b = pixelPtrColor[y * c.cols * cn + x * cn + 0]; // B
					uint8_t g = pixelPtrColor[y * c.cols * cn + x * cn + 1]; // G
					uint8_t r = pixelPtrColor[y * c.cols * cn + x * cn + 2]; // R
					//glColor3ub(v[n].r, v[n].g, v[n].b);
					glColor3f((float)r / 256, (float)g / 256, (float)b / 256);
					glVertex3f((float)(x - m.cols / 2) / m.cols, -(float)(y - m.rows / 2) / m.rows, pix *factor);
				//}
			}			
		}
		glEnd();

		glPopMatrix();
		glFlush();

		glfwSwapBuffers(window);
		glfwPollEvents();
		
		return true;
	}
	return false;
}

void view_term()
{
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}


