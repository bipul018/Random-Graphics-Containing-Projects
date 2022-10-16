#pragma once
#include "raylib-cpp.hpp"

//A function that does or/union of rectangles , that results a rectangles including both 
raylib::Rectangle operator | (raylib::Rectangle r1, raylib::Rectangle r2);
//A function that does and/intersection of rectangles , that results a rectangles included by both 
raylib::Rectangle operator & (raylib::Rectangle r1, raylib::Rectangle r2);

//Containers for stuff only , needs to be derived for each use cases
class GUIUnit {
public:
	virtual void doStuff() = 0;
	inline virtual void packToUnits() {}
	//Relocates the unit to particular relative position based on x and y factors
	//facx and facy need to be between 0 and 1 and they determine how to be positioned
	//inside the rectangle, it wont let content go outside the rectangle
	virtual void relocate(raylib::Rectangle rec,float facx, float facy);
	//Plain old shifting function
	virtual void shift(float dx, float dy);
	//Resizes keeping center intact
	virtual void resize(float fac);
	raylib::Rectangle box;
	inline virtual ~GUIUnit() {}
};

class GUIContainer: public GUIUnit {

public:
	 virtual void doStuff() override;
	 virtual void packToUnits() override;
	 virtual void stackChildren(bool isVertical = true, bool resizeToFit = true);
	 void relocate(raylib::Rectangle rec, float facx, float facy) override;
	 void shift(float dx, float dy) override;
	 virtual void resize(float fac) override;
	 std::vector<GUIUnit*> units;
};

class Button : public GUIUnit {
public:
	bool isactive = false;
	std::string msg = "Button";
	int txtSize = 29;
	void doStuff() override;
	void packToUnits() override;
	void resize(float fac) override;
};

class Panel : public GUIContainer {
public:
	void doStuff() override;
};

class WindowBox : public GUIContainer {
public:
	std::string title = "Title Here";
	bool isactive = false;

	void doStuff() override;
	void packToUnits() override;

};

class GroupBox : public GUIContainer {
public:
	std::string title = "Group Title";
	int txtSize = 33;
	void doStuff() override;
	void packToUnits() override;
	void resize(float fac) override;
};

class Label : public GUIUnit {
public:

	std::string msg = "Label";
	int txtSize = 29;
	void doStuff() override;
	void packToUnits() override;
	void resize(float fac) override;

};

class TextBox : public GUIUnit {

public:
	int txtSize = 29;
	unsigned maxSize = 0;		//0 implies infinite actually
	bool enterPressed = false;
	bool editable = false;

	//Needs to guarentee that content wont be changed, rules basically same of c_str() in strings
	const char* getStr() const;	

	//Sets internal string value, truncates if > maxSize
	void setStr(const char* str);
	void doStuff() override;
	void packToUnits() override;
	void resize(float fac) override;
private:
	std::vector<char> data;
};

class Toggle : public GUIUnit {
public:

	std::string msg = "Toggle";
	int txtSize = 29;
	bool isactive = false;
	void doStuff() override;
	void packToUnits() override;
	void resize(float fac) override;
};

class Slider : public GUIUnit {
public:
	int txtSize = 23;
	std::string leftmsg = "0.0";
	std::string rightmsg = "1.0";
	float leftval = 0;
	float rightval = 1;
	float val = 0.5;
	bool showVal = true;
	void doStuff() override;
	//This only sets height of the slider by the text size
	void packToUnits() override;
	void resize(float fac) override;
};

class ScrollBar : public GUIUnit {
public:
	int value;
	int minValue;
	int maxValue;
	void doStuff() override;
};

//Have to change the just changed flag to bring about any change
class DropBox : public GUIUnit {
public:
	int txtSize = 23;
	std::vector<std::string> list;
	bool justchanged = true;
	int choice = 0;
	bool editmode = false;

	void doStuff() override;
	void resize(float fac) override;
	void packToUnits() override;

	~DropBox();
private:
	char* buffer = nullptr;

};


int guitest();