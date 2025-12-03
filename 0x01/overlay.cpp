#include "Overlay.h"

Overlay::Overlay(HWND hwnd) : hwnd(hwnd)
{
	HDC hdc = GetDC(hwnd);
	hdcMem = CreateCompatibleDC(hdc);
	ReleaseDC(hwnd, hdc);
}

Overlay::~Overlay() 
{
	if (hBitmap) DeleteObject(hBitmap);
	if (hdcMem) DeleteDC(hdcMem);
}

void Overlay::BeginDraw()
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;

	if (width != lastWidth || height != lastHeight) {
		if (hBitmap) DeleteObject(hBitmap);
		HDC hdc = GetDC(hwnd);
		hBitmap = CreateCompatibleBitmap(hdc, width, height);
		ReleaseDC(hwnd, hdc);
		SelectObject(hdcMem, hBitmap);
		lastWidth = width;
		lastHeight = height;
	}

	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(hdcMem, &rect, hBrush);
	DeleteObject(hBrush);
}

void Overlay::EndDraw()
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	HDC hdc = GetDC(hwnd);
	BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
	ReleaseDC(hwnd, hdc);
}

void Overlay::DrawText(const std::wstring& text, int x, int y, COLORREF color, HFONT hFont) 
{
	SetTextColor(hdcMem, color);
	SetBkMode(hdcMem, TRANSPARENT);
	HFONT oldFont = nullptr;
	if (hFont) oldFont = (HFONT)SelectObject(hdcMem, hFont);
	TextOutW(hdcMem, x, y, text.c_str(), (int)text.length());
	if (oldFont) SelectObject(hdcMem, oldFont);
}

HFONT Overlay::CreateFontScaled(float height) 
{
	int fontSize = (int)(height / 8.0f);
	if (fontSize < 10) fontSize = 10;
	if (fontSize > 24) fontSize = 24;

	return CreateFontW(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		NONANTIALIASED_QUALITY, FIXED_PITCH | FF_DONTCARE, L"Courier New");

}

SIZE Overlay::GetTextSize(const std::wstring& text, HFONT font)
{
	SIZE size = {};
	HFONT oldFont = nullptr;

	if (font) oldFont = (HFONT)SelectObject(hdcMem, font);
	GetTextExtentPoint32W(hdcMem, text.c_str(), (int)text.length(), &size);
	if (oldFont) SelectObject(hdcMem, oldFont);

	return size;
}



void Overlay::DrawLine(int x1, int y1, int x2, int y2, COLORREF color, int strokeWidth) 
{
	HPEN hPen = CreatePen(PS_SOLID, strokeWidth, color);
	HPEN oldPen = (HPEN)SelectObject(hdcMem, hPen);
	MoveToEx(hdcMem, x1, y1, NULL);
	LineTo(hdcMem, x2, y2);
	SelectObject(hdcMem, oldPen);
	DeleteObject(hPen);
}

void Overlay::DrawCornerBox(int x, int y, int width, int height, int length, COLORREF color, int strokeWidth) 
{
	if (width < 4 || height < 4)
		return;

	int adjustedLength = min(length, min(width / 3, height / 3));
	if (adjustedLength <= 0)
		return;

	COLORREF outlineColor = RGB(1, 1, 1);
	int outlineWidth = 1;

	DrawLine(x - 1, y - 1, x + adjustedLength + 1, y - 1, outlineColor, outlineWidth);
	DrawLine(x + width - adjustedLength - 1, y - 1, x + width + 1, y - 1, outlineColor, outlineWidth);
	DrawLine(x - 1, y - 1, x - 1, y + adjustedLength + 1, outlineColor, outlineWidth);
	DrawLine(x + width + 1, y - 1, x + width + 1, y + adjustedLength + 1, outlineColor, outlineWidth);

	DrawLine(x - 1, y + height + 1, x + adjustedLength + 1, y + height + 1, outlineColor, outlineWidth);
	DrawLine(x + width - adjustedLength - 1, y + height + 1, x + width + 1, y + height + 1, outlineColor, outlineWidth);
	DrawLine(x - 1, y + height - adjustedLength - 1, x - 1, y + height + 1, outlineColor, outlineWidth);
	DrawLine(x + width + 1, y + height - adjustedLength - 1, x + width + 1, y + height + 1, outlineColor, outlineWidth);

	DrawLine(x, y, x + adjustedLength, y, color, strokeWidth);
	DrawLine(x + width - adjustedLength, y, x + width, y, color, strokeWidth);
	DrawLine(x, y, x, y + adjustedLength, color, strokeWidth);
	DrawLine(x + width, y, x + width, y + adjustedLength, color, strokeWidth);

	DrawLine(x, y + height, x + adjustedLength, y + height, color, strokeWidth);
	DrawLine(x + width - adjustedLength, y + height, x + width, y + height, color, strokeWidth);
	DrawLine(x, y + height - adjustedLength, x, y + height, color, strokeWidth);
	DrawLine(x + width, y + height - adjustedLength, x + width, y + height, color, strokeWidth);
}


void Overlay::DrawRotatedText(const std::wstring& text, int x, int y, COLORREF color, HFONT hFont, float angle) 
{
	SetTextColor(hdcMem, color);
	SetBkMode(hdcMem, TRANSPARENT);

	int oldGraphicsMode = SetGraphicsMode(hdcMem, GM_ADVANCED);
	XFORM oldTransform;
	GetWorldTransform(hdcMem, &oldTransform);

	XFORM transform;
	float radians = angle * 3.14159265f / 180.0f;
	transform.eM11 = cosf(radians);
	transform.eM12 = sinf(radians);
	transform.eM21 = -sinf(radians);
	transform.eM22 = cosf(radians);
	transform.eDx = (float)x;
	transform.eDy = (float)y;

	SetWorldTransform(hdcMem, &transform);

	HFONT oldFont = nullptr;
	if (hFont) oldFont = (HFONT)SelectObject(hdcMem, hFont);
	TextOutW(hdcMem, 0, 0, text.c_str(), (int)text.length());
	if (oldFont) SelectObject(hdcMem, oldFont);

	SetWorldTransform(hdcMem, &oldTransform);
	SetGraphicsMode(hdcMem, oldGraphicsMode);
}

void Overlay::DrawBox(int x, int y, int width, int height, COLORREF color, int strokeWidth, bool outlineEnabled)
{
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

	HPEN oldPen = nullptr;

	if (outlineEnabled) {
		COLORREF outlineColor = RGB(1, 1, 1);
		int outlineOffset = 1;

		HPEN hOutlinePen = CreatePen(PS_SOLID, strokeWidth, outlineColor);
		oldPen = (HPEN)SelectObject(hdcMem, hOutlinePen);

		Rectangle(hdcMem,
			x - outlineOffset,
			y - outlineOffset,
			x + width + outlineOffset,
			y + height + outlineOffset);

		DeleteObject(SelectObject(hdcMem, oldPen));
	}

	HPEN hPen = CreatePen(PS_SOLID, strokeWidth, color);
	oldPen = (HPEN)SelectObject(hdcMem, hPen);

	Rectangle(hdcMem, x, y, x + width, y + height);

	SelectObject(hdcMem, oldBrush);
	SelectObject(hdcMem, oldPen);

	DeleteObject(hPen);
}


void Overlay::DrawFilledBox(int x, int y, int width, int height, COLORREF fillColor, bool outlineEnabled, COLORREF outlineColor) 
{
	if (outlineEnabled) 
	{
		HPEN hOutlinePen = CreatePen(PS_SOLID, 1, outlineColor);
		HPEN oldPen = (HPEN)SelectObject(hdcMem, hOutlinePen);
		HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hNullBrush);

		Rectangle(hdcMem, x - 1, y - 1, x + width + 1, y + height + 1);

		SelectObject(hdcMem, oldBrush);
		SelectObject(hdcMem, oldPen);
		DeleteObject(hOutlinePen);
	}

	HBRUSH hBrush = CreateSolidBrush(fillColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
	Rectangle(hdcMem, x, y, x + width, y + height);
	SelectObject(hdcMem, oldBrush);
	DeleteObject(hBrush);
}

void Overlay::DrawRoundedBox(int x, int y, int width, int height, COLORREF color, int strokeWidth, bool outlineEnabled, int borderRadius) 
{
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

	HPEN oldPen = nullptr;

	if (outlineEnabled)
	{
		COLORREF outlineColor = RGB(1, 1, 1);
		HPEN hOutlinePen = CreatePen(PS_SOLID, strokeWidth, outlineColor);
		oldPen = (HPEN)SelectObject(hdcMem, hOutlinePen);

		RoundRect(hdcMem,
			x - 1,
			y - 1,
			x + width + 1,
			y + height + 1,
			borderRadius,
			borderRadius);

		DeleteObject(SelectObject(hdcMem, oldPen));
	}

	HPEN hPen = CreatePen(PS_SOLID, strokeWidth, color);
	oldPen = (HPEN)SelectObject(hdcMem, hPen);

	RoundRect(hdcMem, x, y, x + width, y + height, borderRadius, borderRadius);

	SelectObject(hdcMem, oldBrush);
	SelectObject(hdcMem, oldPen);
	DeleteObject(hPen);
}

void Overlay::DrawFilledRoundedBox(int x, int y, int width, int height, COLORREF fillColor, bool outlineEnabled, COLORREF outlineColor, int borderRadius) 
{
	if (outlineEnabled) 
	{
		HPEN hOutlinePen = CreatePen(PS_SOLID, 1, outlineColor);
		HPEN oldPen = (HPEN)SelectObject(hdcMem, hOutlinePen);
		HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hNullBrush);

		RoundRect(hdcMem, x - 1, y - 1, x + width + 1, y + height + 1, borderRadius, borderRadius);

		SelectObject(hdcMem, oldBrush);
		SelectObject(hdcMem, oldPen);
		DeleteObject(hOutlinePen);
	}

	HBRUSH hBrush = CreateSolidBrush(fillColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);
	RoundRect(hdcMem, x, y, x + width, y + height, borderRadius, borderRadius);
	SelectObject(hdcMem, oldBrush);
	DeleteObject(hBrush);
}

void Overlay::DrawCircle(int x, int y, int radius, COLORREF color, int strokeWidth)
{
	HPEN hPen = CreatePen(PS_SOLID, strokeWidth, color);
	HPEN oldPen = (HPEN)SelectObject(hdcMem, hPen);
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

	Ellipse(hdcMem, x - radius, y - radius, x + radius, y + radius);

	SelectObject(hdcMem, oldBrush);
	SelectObject(hdcMem, oldPen);
	DeleteObject(hPen);
}

void Overlay::DrawFilledCircle(int x, int y, int radius, COLORREF fillColor)
{
	HBRUSH hBrush = CreateSolidBrush(fillColor);
	HBRUSH oldBrush = (HBRUSH)SelectObject(hdcMem, hBrush);

	HPEN hPen = (HPEN)GetStockObject(NULL_PEN);
	HPEN oldPen = (HPEN)SelectObject(hdcMem, hPen);

	Ellipse(hdcMem, x - radius, y - radius, x + radius, y + radius);

	SelectObject(hdcMem, oldBrush);
	SelectObject(hdcMem, oldPen);
	DeleteObject(hBrush);
}

void Overlay::DrawArrow(int x1, int y1, int x2, int y2, COLORREF color, int strokeWidth, float headLength, float headAngleDeg)
{
	DrawLine(x1, y1, x2, y2, color, strokeWidth);

	float angle = atan2f((float)(y2 - y1), (float)(x2 - x1));

	float headAngleRad = headAngleDeg * (3.14159265f / 180.0f);

	int x3 = (int)(x2 - headLength * cosf(angle - headAngleRad));
	int y3 = (int)(y2 - headLength * sinf(angle - headAngleRad));

	int x4 = (int)(x2 - headLength * cosf(angle + headAngleRad));
	int y4 = (int)(y2 - headLength * sinf(angle + headAngleRad));

	DrawLine(x2, y2, x3, y3, color, strokeWidth);
	DrawLine(x2, y2, x4, y4, color, strokeWidth);
}


