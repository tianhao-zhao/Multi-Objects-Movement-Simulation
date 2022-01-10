/*

Author: Tianhao Zhao
Class: ECE 6122
Last Date Modified: 2021/12/07

Description: Use opengl to simulate a uav show on football field.

Credit: the skeleton code is from opengl tutorial 09 several objects chapter.

*/

// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
#include <string>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include "../build/ECE_UAV.h"



/*
Function: collideWith
Purpose: when p1 and p2 collide, they switch their velocity, and add flags to both p1 and p2,
		 so they dont count as collision in next second.
Argument: ECE_UAV pointer p1 and p2
Return: void
*/
void collideWith(ECE_UAV* p1, ECE_UAV* p2)
{
	// cheeck for flags of collision first
	if (p1->getCollision() || p2->getCollision()) 
	{
		return;
	}
	double tempV1[3] = {};
	double tempv2[3] = {};
	p1->getVelocity(tempV1);
	p2->getVelocity(tempv2);
	p1->setVelocity(tempv2);
	p2->setVelocity(tempV1);
	p1->setCollision(true);
	p2->setCollision(true);
	std::cout << "collide" << std::endl;
}

/*
Function: checkCollision
Purpose: check if array p1 and array p2 are with a certain distance
Argument: pointer p1 and p2
Return: bool
*/
bool checkCollision(double* p1, double* p2)
{
	double detectingDistance = 1.0;
	double dpx = p1[0] - p2[0];
	double dpy = p1[1] - p2[1];
	double dpz = p1[2] - p2[2];
	if (abs(dpx) < detectingDistance && abs(dpy) < detectingDistance && abs(dpz) < detectingDistance)
	{
		return true;
	}
	else 
	{
		return false;
	}
}

/*
Function: main
Purpose: main function
Argument: none
Return: int
*/
int main(void)
{
	// Color oscillate counter
	int colorOscillatorCnt = 0;

	// Store positions to check for collision
	double uavPositions[15][3] = {};

	// Convert unit from pix to m
	double mConvert = 2.93;

	// Rotate suzi 90 degree
	GLfloat rotAngle = 90.0;

	// Create 15 uavs
	ECE_UAV uavs[15];
	// Set their initial locations
	double xStart = -140.0;
	double yStart = -70.0;
	double xOffset = 67.0;
	double yOffset = 70.0;
	double zStart = 10.0 / mConvert;
	double initPos[3] = { xStart, yStart, zStart };
	for (int ii = 0; ii < 15; ii++)
	{
		int xIndex = ii % 5;
		int yIndex = ii / 5;
		initPos[0] = (xStart + xIndex * xOffset) / mConvert;
		initPos[1] = (yStart + yIndex * yOffset) / mConvert;
		uavs[ii].setPosition(initPos);
	}

	// Set the destination for each uav 
	for (int ii = 0; ii < 15; ii++)
	{

	}
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 09 - Rendering several models", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited mouvement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");
	GLuint programIDHalf = LoadShaders("StandardShading.vertexshader", "StandardShadingHalf.fragmentshader");
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M");\

	GLuint MatrixIDHalf = glGetUniformLocation(programIDHalf, "MVP");
	GLuint ViewMatrixIDHalf = glGetUniformLocation(programIDHalf, "V");
	GLuint ModelMatrixIDHalf = glGetUniformLocation(programIDHalf, "M");

	// Load the texture
	
	GLuint Texture = loadDDS("uvmap.DDS");

	// Get a handle for our "myTextureSampler" uniform

	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");
	GLuint TextureIDHalf = glGetUniformLocation(programIDHalf, "myTextureSampler");
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("suzanne.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);

	// Load it into a VBO

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);

	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	
	GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	
	glUseProgram(programIDHalf);
	GLuint LightIDHalf = glGetUniformLocation(programIDHalf, "LightPosition_worldspace");
	// For speed computation
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	/*
	Set up the football field
	*/
	// Load the texture
	GLuint TextureField = loadBMP_custom("ff.bmp");
	

	// Get a handle for our "myTextureSampler" uniform
	
	GLuint TextureIDField = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> verticesField;
	std::vector<glm::vec2> uvsField;
	std::vector<glm::vec3> normalsField;
	bool resField = loadOBJ("footballfield.obj", verticesField, uvsField, normalsField);

	std::vector<unsigned short> indicesField;
	std::vector<glm::vec3> indexed_verticesField;
	std::vector<glm::vec2> indexed_uvsField;
	std::vector<glm::vec3> indexed_normalsField;
	indexVBO(verticesField, uvsField, normalsField, indicesField, indexed_verticesField, indexed_uvsField, indexed_normalsField);

	// Load it into a VBO

	GLuint vertexbufferField;
	glGenBuffers(1, &vertexbufferField);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferField);
	glBufferData(GL_ARRAY_BUFFER, indexed_verticesField.size() * sizeof(glm::vec3), &indexed_verticesField[0], GL_STATIC_DRAW);

	GLuint uvbufferField;
	glGenBuffers(1, &uvbufferField);
	glBindBuffer(GL_ARRAY_BUFFER, uvbufferField);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvsField.size() * sizeof(glm::vec2), &indexed_uvsField[0], GL_STATIC_DRAW);

	GLuint normalbufferField;
	glGenBuffers(1, &normalbufferField);
	glBindBuffer(GL_ARRAY_BUFFER, normalbufferField);
	glBufferData(GL_ARRAY_BUFFER, indexed_normalsField.size() * sizeof(glm::vec3), &indexed_normalsField[0], GL_STATIC_DRAW);

	// Generate a buffer for the indices as well
	GLuint elementbufferField;
	glGenBuffers(1, &elementbufferField);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufferField);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesField.size() * sizeof(unsigned short), &indicesField[0], GL_STATIC_DRAW);


	// Start all the uavs
	for (int ii = 0; ii < 15; ii++)
	{
		uavs[ii].start();
	}

	// Main loop to draw the uavs
	do {

		colorOscillatorCnt++;
		colorOscillatorCnt = colorOscillatorCnt % 200;

		// Measure speed
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
			// printf and reset
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// Compute the MVP matrix from keyboard and mouse input
		//computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(40.0f), 4.0f / 3.0f, 0.1f, 1000.0f);
		glm::mat4 ViewMatrix = glm::lookAt(
			glm::vec3(0, -300, 325), // Camera is at (4,3,3), in World Space
			glm::vec3(0, 0, 50), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
		);


		////// Start of the rendering of the first object //////

		// Use our shader
		glUseProgram(programID);
		
		


		glm::vec3 lightPos = glm::vec3(0, -300, 400);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"


		/*
		Draw the football field

		*/
		glm::mat4 ModelMatrix1 = glm::mat4(1.0);
		glm::mat4 MVP1 = ProjectionMatrix * ViewMatrix * ModelMatrix1;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);


		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		
		glBindTexture(GL_TEXTURE_2D, TextureField);
		
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureIDField, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferField);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbufferField);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbufferField);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufferField);

		// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP1[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix1[0][0]);

		// Draw
		glDrawElements(
			GL_TRIANGLES,      // mode
			indices.size(),    // count
			GL_UNSIGNED_SHORT,   // type
			(void*)0           // element array buffer offset
		);


		// Oscillate the color of suzi
		if (colorOscillatorCnt < 100)
		{
			glUseProgram(programID);
		}
		else
		{
			glUseProgram(programIDHalf);
		}

		// Set the light again
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"


		// Start of suzis
		glm::mat4 ModelMatrixSuzi = glm::mat4(1.0);
		glm::mat4 MVPSuzi = ProjectionMatrix * ViewMatrix * ModelMatrixSuzi;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPSuzi[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixSuzi[0][0]);


		// Bind our texture in Texture Unit 0

		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);


		/*
		Modify the MVP matrix and draw them in a loop
		*/

		double currPos[3] = { 0.0, 0.0, 0.0 };
		for (int ii = 0; ii < 15; ii++)
		{
			/*
			Get x, y, z from UAV and modify the MVP matrix
			*/
			uavs[ii].getPosition(currPos);

			// Insert the current position into collision array
			for (int jj = 0; jj < 3; jj++)
			{
				uavPositions[ii][jj] = currPos[jj];
			}

			// Check with previous positions
			for (int kk = 0; kk < ii; kk++)
			{
				if (checkCollision(uavPositions[kk], uavPositions[ii]))
				{
					collideWith(&uavs[kk], &uavs[ii]);
				}
			}
			// BUT the Model matrix is different (and the MVP too)
			glm::mat4 ModelMatrixSuzi = glm::mat4(1.0);
			ModelMatrixSuzi = glm::translate(ModelMatrixSuzi, glm::vec3(currPos[0] * mConvert, currPos[1] * mConvert, currPos[2] * mConvert));
			ModelMatrixSuzi = glm::scale(ModelMatrixSuzi, glm::vec3(3.0f, 3.0f, 3.0f));
			ModelMatrixSuzi = glm::rotate(ModelMatrixSuzi, glm::radians(rotAngle), glm::vec3(1.0f, 0.0f, 0.0f));
			//ModelMatrix1 = glm::rotate(ModelMatrix1, rotAngle)
			glm::mat4 MVPSuzi = ProjectionMatrix * ViewMatrix * ModelMatrixSuzi;

			
			// Send our transformation to the currently bound shader, 
			// in the "MVP" uniform
			glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVPSuzi[0][0]);
			glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrixSuzi[0][0]);

			// Draw the triangles !
			glDrawElements(
				GL_TRIANGLES,      // mode
				indices.size(),    // count
				GL_UNSIGNED_SHORT,   // type
				(void*)0           // element array buffer offset
			);
		}




		////// End of rendering of the first object //////
		////// Start of the rendering of the second object //////

		// In our very specific case, the 2 objects use the same shader.
		// So it's useless to re-bind the "programID" shader, since it's already the current one.
		//glUseProgram(programID);

		// Similarly : don't re-set the light position and camera matrix in programID,
		// it's still valid !
		// *** You would have to do it if you used another shader ! ***
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		//glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]); // This one doesn't change between objects, so this can be done once for all objects that use "programID"


		// Again : this is already done, but this only works because we use the same shader.
		//// Bind our texture in Texture Unit 0
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, Texture);
		//// Set our "myTextureSampler" sampler to use Texture Unit 0
		//glUniform1i(TextureID, 0);

		/*
		
		// BUT the Model matrix is different (and the MVP too)
		glm::mat4 ModelMatrix2 = glm::mat4(1.0);
		ModelMatrix2 = glm::translate(ModelMatrix2, glm::vec3(2.0f, 0.0f, 0.0f));
		glm::mat4 MVP2 = ProjectionMatrix * ViewMatrix * ModelMatrix2;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP2[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix2[0][0]);


		// The rest is exactly the same as the first object

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

		// Draw the triangles !
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);

		*/
		////// End of rendering of the second object //////




		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


	// Stop all uavs
	for (int ii = 0; ii < 15; ii++)
	{
		uavs[ii].stop();
	}
	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteTextures(1, &Texture);
	

	// Cleanup football field
	glDeleteBuffers(1, &vertexbufferField);
	glDeleteBuffers(1, &uvbufferField);
	glDeleteBuffers(1, &normalbufferField);
	glDeleteBuffers(1, &elementbufferField);
	glDeleteTextures(1, &TextureField);
    
	glDeleteVertexArrays(1, &VertexArrayID);
	glDeleteProgram(programID);
	glDeleteProgram(programIDHalf);
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
