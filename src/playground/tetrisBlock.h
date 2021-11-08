
class tetrisBlock
{
public:
	int x, y;
	int type;
	int rotation;
	void rotateLeft();
	void rotateRight();
	void fall();
	void moveLeft();
	void moveRight();
	tetrisBlock();
	tetrisBlock(int x, int y, int rotation, int type);
};




/*
I-Block = 1
J-Block = 2
S-Block = 3
L-Block = 4
T-Block = 5
O-Block = 6
Z-Block = 7
*/