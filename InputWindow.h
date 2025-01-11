#pragma once
#include <memory>
#include <blend2d.h>
class InputWindow
{
public:
	InputWindow();
	~InputWindow();
	void show();
private:
	void initFont();
	void paint();
	void initWindow();
	bool enableAlpha();
	static LRESULT CALLBACK routeWinMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK processWinMsg(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	int x, y, w, h;
	HWND hwnd;
	std::unique_ptr<BLImage> img;
	bool caretVisible{ true };
	float fontSize{ 26 };
	std::unique_ptr<BLFont> font;
};

