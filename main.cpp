#include <iostream>
#include <Windows.h>
int sudoku();
int emfield();
int chess();

int mousestuff();

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
	
	//FreeConsole();

	switch (c) {
	case 'e':
		return emfield();
	case 's':
		return sudoku();
	case 'c':
		//return mousestuff();
		return chess();
	}

}