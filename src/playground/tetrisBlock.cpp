#include "tetrisBlock.h"

tetrisBlock::tetrisBlock(int x, int y, int rotation, int type) {
	this->x = x;
	this->y = y;
	this->rotation = rotation;
	this->type = type;
	if (this->rotation < 0)
		this->rotation = 3;
	if (this->rotation > 3)
		this->rotation = 0;
}

void tetrisBlock::rotateLeft() {
	rotation = rotation - 1;
	if (rotation < 0)
		rotation = 3;
}

void tetrisBlock::rotateRight() {
	rotation = rotation + 1;
	if (rotation > 3)
		rotation = 0;
}

void tetrisBlock::moveLeft() {
	x = x - 1;
}

void tetrisBlock::moveRight() {
	x = x + 1;
}


