#include "MyGUI.h"
#define RAYGUI_IMPLEMENTATION
#include <extras\raygui.h>

#include <functional>

namespace ray = raylib;

raylib::Rectangle operator|(raylib::Rectangle r1, raylib::Rectangle r2) {

	if (r1.width == 0 || r1.height == 0)
		return r2;
	if (r2.width == 0 || r2.height == 0)
		return r1;

	raylib::Rectangle res(0, 0, 0, 0);
	
	res.x = ((r1.x < r2.x) ? r1.x : r2.x);
	res.y = ((r1.y < r2.y) ? r1.y : r2.y);

	res.width = ((r1.x + r1.width > r2.x + r2.width) ? (r1.x + r1.width) : (r2.x + r2.width)) - res.x;
	res.height = ((r1.y + r1.height > r2.y + r2.height) ? (r1.y + r1.height) : (r2.y + r2.height)) - res.y;
	
	return res;
}

raylib::Rectangle operator&(raylib::Rectangle r1, raylib::Rectangle r2) {
	raylib::Rectangle res(0, 0, 0, 0);

	res.x = ((r1.x > r2.x) ? r1.x : r2.x);
	res.y = ((r1.y > r2.y) ? r1.y : r2.y);

	res.width = ((r1.x + r1.width < r2.x + r2.width) ? (r1.x + r1.width) : (r2.x + r2.width)) - res.x;
	res.height = ((r1.y + r1.height < r2.y + r2.height) ? (r1.y + r1.height) : (r2.y + r2.height)) - res.y;

	return res;
}

void GUIContainer::doStuff() {
	for (auto& u : units)
		u->doStuff();
}

void GUIContainer::stackChildren(bool isVertical, bool resizeToFit) {

	//These are cases for when Vertical stacking is required
	std::function<float&(Rectangle&)> getx = [](Rectangle& r)->float& {
		return r.x;
	};
	
	std::function<float&(Rectangle&)> gety= [](Rectangle& r)->float& {
		return r.y;
	};
	
	std::function<float&(Rectangle&)> getw = [](Rectangle& r)->float& {
		return r.width;
	};
	
	std::function<float&(Rectangle&)> geth = [](Rectangle& r)->float& {
		return r.height;
	};
	std::function<void(GUIUnit&,float,float)> gshift = [](GUIUnit& u, float dx, float dy) {
		u.shift(dx, dy);
	};

	//x and y roles are reversed when horizontal stacking is required
	if (!isVertical) {
		getx = [](Rectangle& r)->float& {
			return r.y;
		};
		gety = [](Rectangle& r)->float& {
			return r.x;
		};
		getw = [](Rectangle& r)->float& {
			return r.height;
		};
		geth = [](Rectangle& r)->float& {
			return r.width;
		};
		gshift = [](GUIUnit& u, float dx, float dy) {
			u.shift(dy, dx);
		};
	}

	float maxwid = 0;
	if (resizeToFit) {
		for (auto& u : units)
			if (u != nullptr)
				if (getw(u->box) > maxwid)
					maxwid = getw(u->box);
	}

	float curry = geth(box);

	//Currently this aligns center
	float midx = getx(box) + getw(box) * 0.5;
	for (auto& u : units) {
		if (u == nullptr)
			continue;
		//Resize width if asked
		if (resizeToFit) {
			getw(u->box) = maxwid;
		}
		//Get changes in y and x of units required
		float dx = (midx - getw(u->box) * 0.5) - getx(u->box);
		float dy = (curry ) - gety(u->box);

		//Call shifting functions of each
		gshift(*u, dx, dy);
	
		curry += geth(u->box);

	}

}

void GUIContainer::relocate(raylib::Rectangle rec, float facx, float facy) {

	raylib::Rectangle tmprec = box;
	//First relocate self by GUIUnit rules
	GUIUnit::relocate(rec, facx, facy);

	//Now relocate child element 
	//Probably better to use locking/unlocking feature to relocate or not these children
	for (auto& u : units) {
		if (u == nullptr)
			continue;

		//Used a new function to simplify
		u->shift(box.x - tmprec.x, box.y - tmprec.y);

		//This is still a backup for in cases later needed
		////Find the rectangle containing both unit and original container 
		//ray::Rectangle enclose = u->box | tmprec;
		//
		//float xf, yf;
		////Calculate the factor that they were arranged originally
		//xf = (u->box.x + u->box.width * 0.5 - tmprec.x) / tmprec.width;
		//yf = (u->box.y + u->box.height * 0.5 - tmprec.y) / tmprec.height;
		////Shift the enclosing box as per container
		//enclose.x += box.x - tmprec.x;
		//enclose.y += box.y - tmprec.y;
		////Tell the unit to relocate as per these rules
		////This going round way to calculate factor allows the GUIUnit unit that is also 
		////a GUIContainer to relocate so as their units are also relocated
		//
		//u->relocate(enclose, xf, yf);


	}

}

void GUIContainer::shift(float dx, float dy) {
	GUIUnit::shift(dx, dy);

	//Need later to add locking unlocking feature here too
	for (auto& u : units)
		u->shift(dx, dy);
}

void GUIContainer::packToUnits() {
	if (units.empty()) {
		box.width = 0;
		box.height = 0;
		return;
	}

	box = ray::Rectangle(0, 0, 0, 0);
	for (auto& u : units)
		box = box | u->box;
	return;

	box = units.at(0)->box;
	Vector2 farend;
	farend.x = box.x+box.width;
	farend.y = box.y + box.height;
	for (auto& u : units) {

		if (box.x > u->box.x)
			box.x = u->box.x;
		if (box.y > u->box.y)
			box.y = u->box.y;
		if (farend.x < (u->box.x + u->box.width))
			farend.x = u->box.x + u->box.width;
		if (farend.y < (u->box.y + u->box.height))
			farend.y = u->box.y + u->box.height;

	}
	box.width = farend.x - box.x;
	box.height = farend.y - box.y;


}

void GUIUnit::relocate(raylib::Rectangle rec, float facx, float facy) {

	if (facx < 0 || facy < 0 || facx> 1 || facy > 1)
		return;

	box.x = rec.x + rec.width * facx - box.width / 2;
	box.y = rec.y + rec.height * facy - box.height / 2;

	//If box overflows right to source rec
	if (box.x + box.width > rec.x + rec.width)
		box.x = rec.x + rec.width - box.width;

	//If box overflows down to source rec
	if (box.y + box.height > rec.y + rec.height)
		box.y = rec.y + rec.height - box.height;

	//If box overflows up to source rec
	if (box.x < rec.x)
		box.x = rec.x;

	//If box overflows left to source rec
	if (box.y < rec.y)
		box.y = rec.y;


}

void GUIUnit::shift(float dx, float dy) {
	box.x += dx;
	box.y += dy;
}


int guitest() {
	int width = 1200;
	int height = 900;
	InitWindow(width, height, "GUIEE");


	Button exit;
	exit.msg = "EXIT";
	exit.txtSize = 80;
	exit.packToUnits();
	TextBox noexit;
	noexit.setStr("_");
	noexit.editable = true;
	noexit.maxSize = 10;
	noexit.txtSize = 40;
	noexit.packToUnits();
	Label dummy;
	dummy.msg = "THIS IS A DUMMY ONE";
	dummy.txtSize = 80;
	dummy.packToUnits();
	Slider whee;
	whee.box = ray::Rectangle(50, 40, 150, 30);
	DropBox dropp;
	dropp.txtSize = 80;
	dropp.list.push_back("Mpples");
	dropp.list.push_back("Oranges");
	dropp.list.push_back("Banana");
	dropp.packToUnits();

	Panel panh;
	panh.units.push_back(&noexit);
	panh.units.push_back(&exit);
	panh.units.push_back(&dropp);
	panh.stackChildren(false);
	panh.packToUnits();

	WindowBox panv;
	panv.title = "Mini Window";
	panv.units.push_back(&dummy);
	panv.units.push_back(&whee);
	panv.units.push_back(&panh);
	panv.stackChildren(true);
	panv.packToUnits();
	panv.relocate(ray::Rectangle(0, 0, width, height), 0.5, 0.5);

	//Now let's fill remaining region of panh extended by panv through droplist dropp
	dropp.box.width = panh.box.width - noexit.box.width - exit.box.width;

	while (!WindowShouldClose() && !exit.isactive ) {
		ClearBackground(RAYWHITE);
		BeginDrawing();

		panv.doStuff();
		EndDrawing();
	}
	CloseWindow();
	return 0;
}

void WindowBox::doStuff() {
	isactive = GuiWindowBox(box, title.c_str());
	for (auto& u : units)
		u->doStuff();
}

void WindowBox::packToUnits() {

	//This the the definition used inside raygui implementation, so may change and might appear buggy
#ifndef WINDOW_STATUSBAR_HEIGHT
#define WINDOW_STATUSBAR_HEIGHT 22
#endif
	int statusBarHeight = WINDOW_STATUSBAR_HEIGHT + 2 * GuiGetStyle(STATUSBAR, BORDER_WIDTH);

	
	box.y += statusBarHeight;

	GUIContainer::packToUnits();
	box.y -= statusBarHeight;
	box.height += statusBarHeight;

}


void GroupBox::doStuff() {
	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	auto bbox = box;
	bbox.y += txtSize / 2;
	bbox.height -= txtSize / 2;

	GuiGroupBox(bbox, title.c_str());
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);

	GUIContainer::doStuff();
}

void GroupBox::packToUnits() {
	//For arranging child maintaining consistency
	box.y += txtSize;
	GUIContainer::packToUnits();

	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	box.y -= txtSize;
	box.height += txtSize;
	//This the the definition used inside raygui implementation, so may change and might appear buggy
#ifndef LINE_TEXT_PADDING
#define LINE_TEXT_PADDING 10
#endif
	float tmpw = GetTextWidth(title.c_str()) + 2 * (GuiGetStyle(DEFAULT, TEXT_SPACING) + LINE_TEXT_PADDING );
	if (box.width < tmpw)
		box.width = tmpw;
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);
	
}


void Button::doStuff() {
	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	isactive = GuiButton(box, msg.c_str());
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);

}

void Button::packToUnits() {
	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	box.width = GetTextWidth(msg.c_str()) + 2 * (GuiGetStyle(DEFAULT, TEXT_SPACING) + GuiGetStyle(DEFAULT, TEXT_PADDING) + GuiGetStyle(BUTTON, BORDER_WIDTH));

	box.height = txtSize;
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);
}

void Panel::doStuff() {

	GuiPanel(box);
	GUIContainer::doStuff();
}

void Toggle::packToUnits() {
	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	box.width = GetTextWidth(msg.c_str()) + 2 * (GuiGetStyle(DEFAULT, TEXT_SPACING) + GuiGetStyle(DEFAULT, TEXT_PADDING) + GuiGetStyle(BUTTON, BORDER_WIDTH));

	box.height = txtSize;
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);
}

void Toggle::doStuff() {
	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	isactive = GuiToggle(box, msg.c_str(),isactive);
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);

}

void Label::doStuff() {
	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	GuiLabel(box, msg.c_str());
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);
}

void Label::packToUnits() {

	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	box.width = GetTextWidth(msg.c_str()) + 2 * (GuiGetStyle(DEFAULT, TEXT_SPACING) + GuiGetStyle(DEFAULT, TEXT_PADDING) + GuiGetStyle(BUTTON, BORDER_WIDTH));

	box.height = txtSize;
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);
}

void TextBox::packToUnits() {

	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);

	int mSize = ((maxSize > 0) ? maxSize : (data.size() * 2 + 2));

	//This was simpler than creating whole function self
	std::string dummy(mSize, 'M');

	box.width = GetTextWidth(dummy.c_str()) + 2 * (GuiGetStyle(DEFAULT, TEXT_SPACING) + GuiGetStyle(DEFAULT, TEXT_PADDING) + GuiGetStyle(TEXTBOX, BORDER_WIDTH));

	//Cursor height
	box.height = txtSize * 2;
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);

}

void TextBox::doStuff() {

	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	int tmpalign = GuiGetStyle(DEFAULT, TEXT_ALIGNMENT);
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, GUI_TEXT_ALIGN_CENTER);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	int mSize = ((maxSize > 0) ? maxSize : (data.size() * 2 + 2));
	
	if (!data.empty()) {
		if (std::strlen(data.data()) >= data.size() - 2)
			data.resize(data.size() * 2 + 2);
	}
	else {
		data.resize(2);
		data.at(0) = '\0';
	}

	enterPressed = GuiTextBox(box, data.data(), mSize, editable);
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);
	GuiSetStyle(DEFAULT, TEXT_ALIGNMENT, tmpalign);

}

const char* TextBox::getStr() const {
	return data.data();
}

void TextBox::setStr(const char* str) {

	data.clear();
	data.reserve(maxSize);
	const char* ptr = str;
	while (*ptr != '\0' && ((maxSize == 0) || (ptr - str) < maxSize)) {
		data.push_back(*ptr);
		++ptr;
	}
	data.push_back('\0');

}

void Slider::doStuff() {
	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	auto ltextl = GetTextWidth(leftmsg.c_str());
	auto rtextl = GetTextWidth(rightmsg.c_str());
	auto bbox = box;
	bbox.x += ltextl + GuiGetStyle(SLIDER, TEXT_PADDING)*2;
	bbox.width -= ltextl + rtextl + 4 * GuiGetStyle(SLIDER, TEXT_PADDING);
	val = GuiSlider(bbox, leftmsg.c_str(), rightmsg.c_str(), val, leftval, rightval);
	if (showVal) {
		std::string sval = std::to_string(val);
		float x = box.x + box.width / 2 - GetTextWidth(sval.c_str()) / 2;
		float y = box.y + box.height / 2 - txtSize / 2;
		DrawText(sval.c_str(), x, y, txtSize, Fade(GetColor(GuiGetStyle(SLIDER, TEXT + (GuiGetState() * 3))), guiAlpha));
	}
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);

}

void Slider::packToUnits() {

	box.height = txtSize;

}

void DropBox::doStuff() {

	//if just changed then update buffer
	if (justchanged) {

		int reqsize = 0;
		for (auto& s : list) {
			reqsize += s.length() + 1;
		}

		if (buffer != nullptr) {
			delete[] buffer;
		}
		buffer = new char[reqsize];
		int ptr = 0;
		for (auto& s : list) {

			for (auto& c : s) {
				buffer[ptr] = c;
				++ptr;
			}
			buffer[ptr] = ';';
			++ptr;
			
		}
		buffer[ptr - 1] = '\0';
		justchanged = false;
	}
	if (buffer == nullptr)
		return;


	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);
	editmode = !(editmode == GuiDropdownBox(box, buffer, &choice, editmode));
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);


}

void DropBox::packToUnits() {

	if (list.empty())
		return;

	int tmpsize = GuiGetStyle(DEFAULT, TEXT_SIZE);
	GuiSetStyle(DEFAULT, TEXT_SIZE, txtSize);

	int largesize = 0;
	int largeIndex = -1;
	for (int i = 0; i < list.size(); ++i) {
		if (list.at(i).length() > largesize) {
			largeIndex = i;
			largesize = list.at(i).length();
		}
	}

	//one extra character width for drop down icon
	box.width =GetTextWidth("A")+ GetTextWidth(list.at(largeIndex).c_str()) + 2 * (GuiGetStyle(DEFAULT, TEXT_SPACING) + GuiGetStyle(DEFAULT, TEXT_PADDING) + GuiGetStyle(BUTTON, BORDER_WIDTH));

	box.height = txtSize;
	GuiSetStyle(DEFAULT, TEXT_SIZE, tmpsize);
}

DropBox::~DropBox() {
	if (buffer != nullptr)
		delete[]buffer;
	buffer = nullptr;

}
