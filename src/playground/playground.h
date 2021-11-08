#ifndef PLAYGROUND_H
#define PLAYGROUND_H

// Include GLEW
#include <GL/glew.h>
#include "../../../bin/tetrisBlock.h"

#include <./playground/playground.h>

//some global variables for handling the vertex buffer
GLuint vertexbuffer;
GLuint VertexArrayID;
GLuint vertexbuffer_size;

//program ID of the shaders, required for handling the shaders with OpenGL
GLuint programID;


int main(void); //<<< main function, called at startup
void drawRectangle(float width, float height, float x, float y, int filled); //<<< draw rectangle
bool initializeWindow(); //<<< initializes the window using GLFW and GLEW
bool initializeVertexbuffer(); //<<< initializes the vertex buffer array and binds it OpenGL
bool cleanupVertexbuffer(); //<<< frees all recources from the vertex buffer
bool closeWindow(); //<<< Closes the OpenGL window and terminates GLFW
void drawBoard();
void checkBoard();
void gameLogic();
bool checkMovement(tetrisBlock solid);
void freezeTile();
void removeLines();
bool gameOver();
void startGame();
void gameScore();
#endif


