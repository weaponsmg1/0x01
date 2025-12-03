#pragma once
#include <windows.h>
#include <string>

class Overlay 
{
private:
	HWND hwnd;
	HDC hdcMem = nullptr;
	HBITMAP hBitmap = nullptr;
	int lastWidth = 0;
	int lastHeight = 0;

public:
	Overlay(HWND hwnd);
	~Overlay();

	void BeginDraw();
	void EndDraw();
	void DrawText(const std::wstring& text, int x, int y, COLORREF color, HFONT hFont = nullptr);
	void DrawRotatedText(const std::wstring& text, int x, int y, COLORREF color, HFONT hFont, float angle);
	void DrawLine(int x1, int y1, int x2, int y2, COLORREF color, int strokeWidth = 1);
	void DrawBox(int x, int y, int width, int height, COLORREF color, int strokeWidth = 1, bool outlineEnabled = false);
	void DrawCornerBox(int x, int y, int width, int height, int length, COLORREF color, int strokeWidth);
	void DrawFilledBox(int x, int y, int width, int height, COLORREF fillColor, bool outlineEnabled = false, COLORREF outlineColor = RGB(0, 0, 0));
	void DrawRoundedBox(int x, int y, int width, int height, COLORREF color, int strokeWidth, bool outlineEnabled, int borderRadius);
	void DrawFilledRoundedBox(int x, int y, int width, int height, COLORREF fillColor, bool outlineEnabled, COLORREF outlineColor, int borderRadius);
	void DrawCircle(int x, int y, int radius, COLORREF color, int strokeWidth);
	void DrawFilledCircle(int x, int y, int radius, COLORREF fillColor);
	void DrawArrow(int x1, int y1, int x2, int y2, COLORREF color, int strokeWidth, float headLength = 10.0f, float headAngleDeg = 30.0f);
	HFONT CreateFontScaled(float height);
	SIZE GetTextSize(const std::wstring& text, HFONT font);
};