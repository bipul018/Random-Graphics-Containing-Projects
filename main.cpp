
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "MyGUI.h"
int sudoku();
int chess();

int coaster();
int baghchal();

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

	std::cout << "Enter  s for sudoku :  c for chess:\n h for coaster: b for baghchal:\ng : guitest";
	char c = 'x';
	std::cin >> c;
	
	//FreeConsole();

	switch (c) {
	case 's':
		return sudoku();
	case 'c':
		return chess();
	case 'h':
		return coaster();
	case 'b':
		return baghchal();
	case 'g':
		return guitest();
	}

}