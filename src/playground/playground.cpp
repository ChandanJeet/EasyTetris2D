#include "playground.h"

// Include standard headers
#include <stdio.h>
#include <stdlib.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Tetris Game files
//#include <tetrisUtils.h>

// some libraries for sleeping, time measurement, calculation
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <math.h>
#include <sstream>

GLuint tetrisVao;
GLuint tetrisVbo;
tetrisBlock obj = tetrisBlock(0, 0, 0, 1);

// Include GLM
#include <glm/glm.hpp>
using namespace glm;
#include <common/shader.hpp>
using namespace std;
float x = 0.0, y = 0.0;
clock_t begin_time, sword_cd, move_cd;
int vecRealTile[10][25];
int vecBlockedTile[10][25];
bool noPiece = false;
int counter = 0;
int score = 0;

int main(void)
{

	//Initialize window
	bool windowInitialized = initializeWindow();
	if (!windowInitialized) return -1;

	//Initialize vertex buffer
	bool vertexbufferInitialized = initializeVertexbuffer();
	if (!vertexbufferInitialized) return -1;

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");

	startGame();
	//start animation loop until escape key is pressed
	do {
		while (((double)(clock() - begin_time) / CLOCKS_PER_SEC) < (1 / 120.0)) {
		}
		begin_time = clock();
		if (!gameOver()) {
			gameLogic();
			removeLines();
			checkBoard();
			drawBoard();
		}
		else {
			drawRectangle(200, 100, 0, 0, 1);
			checkBoard();
			drawBoard();
			if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
				startGame();
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	//Cleanup and close window
	cleanupVertexbuffer();
	glDeleteProgram(programID);
	closeWindow();

	return 0;
}

bool initializeWindow()
{
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context width x height
	window = glfwCreateWindow(600, 900, "", NULL, NULL);
	gameScore();

	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return false;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	return true;
}

bool initializeVertexbuffer()
{
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	vertexbuffer_size = 3;
	static const GLfloat g_vertex_buffer_data[] = {
	  -1.0f, -1.0f, 0.0f,
	  1.0f, -1.0f, 0.0f,
	  0.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	return true;
}

void startGame() {
	obj = tetrisBlock(4, 4, 0, 1);
	bool noPiece = false;
	int counter = 0;
	int score = 0;

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 25; j++) {
			vecRealTile[i][j] = 0;
			vecBlockedTile[i][j] = 0;

		}
	}
}

void gameScore() {
	string stringScore = to_string(score);
	string windowTitle = "EasyTetris2D -------- SCORE: " + stringScore;
	char* charVar;
	charVar = &windowTitle[0];
	glfwSetWindowTitle(window, charVar);

}


void gameLogic()
{
	if (noPiece) {
		int range = 5 - 1 + 1;
		int num = rand() % range + 1;
		obj = tetrisBlock(4, 4, 0, num);
		noPiece = false;
	}
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT);

	if (((double)(move_cd - clock()) / CLOCKS_PER_SEC) < -(1 / 1.8f)) {
		if ((glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)) {
			move_cd = clock();
			if (checkMovement(tetrisBlock(obj.x + 1, obj.y, obj.rotation, obj.type)))
				obj.moveRight();
		}
		else if ((glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)) {
			move_cd = clock();
			if (checkMovement(tetrisBlock(obj.x - 1, obj.y, obj.rotation, obj.type)))
				obj.moveLeft();
		}
	}

	if (((double)(sword_cd - clock()) / CLOCKS_PER_SEC) < -(1 / 1.5f)) {
		sword_cd = clock();
		//Check keyboard input
		if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
			sword_cd = clock();
			if (checkMovement(tetrisBlock(obj.x, obj.y, obj.rotation - 1, obj.type)))
				obj.rotateLeft();
		}
		else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			sword_cd = clock();
			if (checkMovement(tetrisBlock(obj.x, obj.y, obj.rotation + 1, obj.type)))
				obj.rotateRight();
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (checkMovement(tetrisBlock(obj.x, obj.y + 1, obj.rotation, obj.type)))
			obj.y = obj.y + 1; //increase tile speed
	}
	//game speed
	counter++;
	if (counter == 60) {
		if (checkMovement(tetrisBlock(obj.x, obj.y + 1, obj.rotation, obj.type))) {
			obj.y = obj.y + 1;
			counter = 0;
		}
		else {
			noPiece = true;
			freezeTile();
			counter = 0;
		}
	}
}

void freezeTile() {

	switch (obj.type)
	{
	case 1:
		switch (obj.rotation)
		{
		case 0:
			vecBlockedTile[obj.x][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 3][obj.y + 1] = obj.type;
			break;
		case 1:
			vecBlockedTile[obj.x + 2][obj.y] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 3] = obj.type;
			break;
		case 2:
			vecBlockedTile[obj.x][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 3][obj.y + 2] = obj.type;
			break;
		case 3:
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 3] = obj.type;
			break;
		}
		break;
	case 2:
		vecBlockedTile[obj.x][obj.y] = obj.type;
		vecBlockedTile[obj.x + 1][obj.y] = obj.type;
		vecBlockedTile[obj.x][obj.y + 1] = obj.type;
		vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
		break;

	case 3:
		switch (obj.rotation)
		{
		case 0:
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x][obj.y] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y] = obj.type;
			break;
		case 1:
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y] = obj.type;
			break;
		case 2:
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 2] = obj.type;
			break;
		case 3:
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x][obj.y] = obj.type;
			break;
		}
		break;
	case 4:
		switch (obj.rotation)
		{
		case 0:
			vecBlockedTile[obj.x][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 2][obj.y + 1] = obj.type;
			break;
		case 1:
			vecBlockedTile[obj.x][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x][obj.y + 2] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			break;
		case 2:
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y] = obj.type;
			vecBlockedTile[obj.x - 1][obj.y] = obj.type;
			break;
		case 3:
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y - 1] = obj.type;
			break;

		}
		break;
	case 5:
		switch (obj.rotation)
		{
		case 0:
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y - 1] = obj.type;
			break;
		case 1:
			vecBlockedTile[obj.x + 2][obj.y] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y + 1] = obj.type;
			break;
		case 2:
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y - 1] = obj.type;
			vecBlockedTile[obj.x][obj.y] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			break;
		case 3:
			vecBlockedTile[obj.x + 2][obj.y] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y + 1] = obj.type;
			vecBlockedTile[obj.x + 1][obj.y] = obj.type;
			vecBlockedTile[obj.x][obj.y + 1] = obj.type;
			break;
		}
	default:
		break;
	}
}

void removeLines() {
	int linePosition[4];
	int numRemovedLines = 0;

	for (int i = 0; i < 25; i++) {
		int numberBlocked = 0;
		for (int j = 0; j < 10; j++) {
			if (vecBlockedTile[j][i])
				numberBlocked++;
		}
		if (numberBlocked == 10) {
			linePosition[numRemovedLines] = i;
			numRemovedLines++;
			score += 10;
			gameScore();
		}
	}

	for (int i = 0; i < numRemovedLines; i++) {
		for (int s = linePosition[i]; s > 0; s--) {
			for (int j = 0; j < 10; j++) {
				vecBlockedTile[j][s] = vecBlockedTile[j][s - 1];
			}
		}
	}
}

bool gameOver() {
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 5; j++) {
			if (vecBlockedTile[i][j] > 0)
				return true;
		}
	}
	return false;
}

bool checkMovement(tetrisBlock solid) {
	int vec[4][2];

	switch (solid.type)
	{
	case 1:
		switch (solid.rotation)
		{
		case 0:
			vec[0][0] = solid.x;
			vec[0][1] = solid.y + 1;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x + 2;
			vec[2][1] = solid.y + 1;

			vec[3][0] = solid.x + 3;
			vec[3][1] = solid.y + 1;
			break;
		case 1:
			vec[0][0] = solid.x + 2;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x + 2;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x + 2;
			vec[2][1] = solid.y + 2;

			vec[3][0] = solid.x + 2;
			vec[3][1] = solid.y + 3;
			break;
		case 2:
			vec[0][0] = solid.x;
			vec[0][1] = solid.y + 2;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y + 2;

			vec[2][0] = solid.x + 2;
			vec[2][1] = solid.y + 2;

			vec[3][0] = solid.x + 3;
			vec[3][1] = solid.y + 2;
			break;
		case 3:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x + 1;
			vec[2][1] = solid.y + 2;

			vec[3][0] = solid.x + 1;
			vec[3][1] = solid.y + 3;
			break;
		}
		break;
	case 2:
		vec[0][0] = solid.x;
		vec[0][1] = solid.y;

		vec[1][0] = solid.x + 1;
		vec[1][1] = solid.y;

		vec[2][0] = solid.x;
		vec[2][1] = solid.y + 1;

		vec[3][0] = solid.x + 1;
		vec[3][1] = solid.y + 1;
		break;

		break;
	case 3:
		switch (solid.rotation)
		{
		case 0:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y + 1;

			vec[1][0] = solid.x;
			vec[1][1] = solid.y;

			vec[2][0] = solid.x + 1;
			vec[2][1] = solid.y;

			vec[3][0] = solid.x + 2;
			vec[3][1] = solid.y;
			break;
		case 1:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y + 1;

			vec[1][0] = solid.x + 2;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x + 2;
			vec[2][1] = solid.y + 2;

			vec[3][0] = solid.x + 2;
			vec[3][1] = solid.y;
			break;
		case 2:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y + 1;

			vec[1][0] = solid.x;
			vec[1][1] = solid.y + 2;

			vec[2][0] = solid.x + 1;
			vec[2][1] = solid.y + 2;

			vec[3][0] = solid.x + 2;
			vec[3][1] = solid.y + 2;
			break;
		case 3:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y + 1;

			vec[1][0] = solid.x;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x;
			vec[2][1] = solid.y + 2;

			vec[3][0] = solid.x;
			vec[3][1] = solid.y;
			break;
		}
		break;
	case 4:
		switch (solid.rotation)
		{
		case 0:
			vec[0][0] = solid.x;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x + 1;
			vec[2][1] = solid.y + 1;

			vec[3][0] = solid.x + 2;
			vec[3][1] = solid.y + 1;
			break;
		case 1:
			vec[0][0] = solid.x;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x;
			vec[2][1] = solid.y + 2;

			vec[3][0] = solid.x + 1;
			vec[3][1] = solid.y;
			break;
		case 2:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y + 1;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y;

			vec[2][0] = solid.x;
			vec[2][1] = solid.y;

			vec[3][0] = solid.x - 1;
			vec[3][1] = solid.y;
			break;
		case 3:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y + 1;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y;

			vec[2][0] = solid.x;
			vec[2][1] = solid.y + 1;

			vec[3][0] = solid.x + 1;
			vec[3][1] = solid.y - 1;
			break;
		}
		break;
	case 5:
		switch (solid.rotation)
		{
		case 0:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x;
			vec[2][1] = solid.y;

			vec[3][0] = solid.x;
			vec[3][1] = solid.y - 1;
			break;
		case 1:
			vec[0][0] = solid.x + 2;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x + 1;
			vec[2][1] = solid.y;

			vec[3][0] = solid.x;
			vec[3][1] = solid.y + 1;
			break;
		case 2:
			vec[0][0] = solid.x + 1;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x;
			vec[1][1] = solid.y - 1;

			vec[2][0] = solid.x;
			vec[2][1] = solid.y;

			vec[3][0] = solid.x + 1;
			vec[3][1] = solid.y + 1;
			break;
		case 3:
			vec[0][0] = solid.x + 2;
			vec[0][1] = solid.y;

			vec[1][0] = solid.x + 1;
			vec[1][1] = solid.y + 1;

			vec[2][0] = solid.x + 1;
			vec[2][1] = solid.y;

			vec[3][0] = solid.x;
			vec[3][1] = solid.y + 1;
			break;
		}
		break;
	default:
		break;
	}
	for (int i = 0; i < 4; i++) {
		if (vec[i][0] > 9 || vec[i][0] < 0 || vec[i][1] > 24 || vec[i][1] < 0)
			return false;
		if (vecBlockedTile[vec[i][0]][vec[i][1]] > 0)
			return false;
	}
	return true;
}

void drawRectangle(float width, float height, float x, float y, int filled) {

	float red = 0.4f, green = 0.4f, blue = 0.4f;
	if (filled) {
		//tiles colors
		switch (filled)
		{
		case 1:
			red = 0.1;
			green = 0.8;
			blue = 0.3;
			break;
		case 2:
			red = 0.5;
			green = 0.1;
			blue = 0.9;
			break;
		case 3:
			red = 0.4;
			green = 0.2;
			blue = 0.5;
			break;
		case 4:
			red = 0.2;
			green = 0.6;
			blue = 0.8;
			break;
		case 5:
			red = 0.8;
			green = 0.3;
			blue = 0.4;
			break;
		default:
			break;
		}
	}
	float vertices2[] =
	{
		-width / 2, height / 2, red, green, blue, // top left
		width / 2, height / 2, red, green, blue,// top right
		-width / 2, -height / 2, red, green, blue, // bottom left

		width / 2, height / 2, red, green, blue,// top right
		width / 2, -height / 2, red, green, blue, // bottom right
		-width / 2, -height / 2, red, green, blue, // bottom left
	};

	glGenVertexArrays(1, &tetrisVao);
	glGenBuffers(1, &tetrisVbo);

	glBindVertexArray(tetrisVao);
	glBindBuffer(GL_ARRAY_BUFFER, tetrisVbo);


	glGenBuffers(1, &tetrisVbo);
	glBindBuffer(GL_ARRAY_BUFFER, tetrisVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(programID);
	int offsetLocation = glGetUniformLocation(programID, "movement");
	glUniform2f(offsetLocation, x, y);

	glBindVertexArray(tetrisVao);
	glDrawArrays(GL_TRIANGLES, 0, 6); // 3 indices starting at 0 -> 1 triangle
	glBindVertexArray(0);
	glDisableVertexAttribArray(0);
	glDeleteProgram(programID);
	//glDeleteBuffers(tetrisVao);
}

void drawBoard() {

	for (int i = 0; i < 10; i++) {
		for (int j = 5; j < 25; j++) {
			drawRectangle(35, 35, -265 + 40 * i, 415 - 40 * (j - 5), vecRealTile[i][j]);

		}
	}
}

void checkBoard() {

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 25; j++) {
			vecRealTile[i][j] = vecBlockedTile[i][j];
		}
	}
	switch (obj.type)
	{
	case 1:
		switch (obj.rotation)
		{
		case 0:
			vecRealTile[obj.x][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 3][obj.y + 1] = obj.type;
			break;
		case 1:
			vecRealTile[obj.x + 2][obj.y] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 3] = obj.type;
			break;
		case 2:
			vecRealTile[obj.x][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 3][obj.y + 2] = obj.type;
			break;
		case 3:
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 3] = obj.type;
			break;
		}
		break;
	case 2:
		vecRealTile[obj.x][obj.y] = obj.type;
		vecRealTile[obj.x + 1][obj.y] = obj.type;
		vecRealTile[obj.x][obj.y + 1] = obj.type;
		vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
		break;

	case 3:
		switch (obj.rotation)
		{
		case 0:
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x][obj.y] = obj.type;
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x + 2][obj.y] = obj.type;
			break;
		case 1:
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 2][obj.y] = obj.type;
			break;
		case 2:
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 2] = obj.type;
			break;
		case 3:
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x][obj.y + 1] = obj.type;
			vecRealTile[obj.x][obj.y + 2] = obj.type;
			vecRealTile[obj.x][obj.y] = obj.type;
			break;
		}
		break;
	case 4:
		switch (obj.rotation)
		{
		case 0:
			vecRealTile[obj.x][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 2][obj.y + 1] = obj.type;
			break;
		case 1:
			vecRealTile[obj.x][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y + 1] = obj.type;
			vecRealTile[obj.x][obj.y + 2] = obj.type;
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			break;
		case 2:
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y] = obj.type;
			vecRealTile[obj.x - 1][obj.y] = obj.type;
			break;
		case 3:
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y - 1] = obj.type;
			break;

		}
		break;
	case 5:
		switch (obj.rotation)
		{
		case 0:
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y - 1] = obj.type;
			break;
		case 1:
			vecRealTile[obj.x + 2][obj.y] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y + 1] = obj.type;
			break;
		case 2:
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y - 1] = obj.type;
			vecRealTile[obj.x][obj.y] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			break;
		case 3:
			vecRealTile[obj.x + 2][obj.y] = obj.type;
			vecRealTile[obj.x + 1][obj.y + 1] = obj.type;
			vecRealTile[obj.x + 1][obj.y] = obj.type;
			vecRealTile[obj.x][obj.y + 1] = obj.type;
			break;
		}
	default:
		break;
	}
}

bool cleanupVertexbuffer()
{
	// Cleanup VBO
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteVertexArrays(1, &VertexArrayID);
	return true;
}

bool closeWindow()
{
	glfwTerminate();
	return true;
}

