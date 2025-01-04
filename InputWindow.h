#pragma once
class InputWindow
{
public:
	InputWindow();
	~InputWindow();
	void show();
private:
	void paint();
	void initWindow();
	static LRESULT CALLBACK routeWinMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK processWinMsg(UINT msg, WPARAM wParam, LPARAM lParam);
private:
	int x, y, w, h;
	HWND hwnd;
};

