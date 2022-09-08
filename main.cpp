#include <iostream>
int sudoku();
int emfield();
int chess();

struct PointPot {
	float x;
	float y;
	float pot;

};

struct MyColor {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

int main() {

	std::cout << "Enter e for em simulation and s for sudoku :  c for chess:";
	char c = 'x';
	std::cin >> c;
	switch (c) {
	case 'e':
		return emfield();
	case 's':
		return sudoku();
	case 'c':
		return chess();
	}

}