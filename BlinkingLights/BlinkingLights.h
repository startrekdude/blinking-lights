#pragma once

#include "resource.h"

#include <random>

#define WIN32_LEAN_AND_MEAN
#include <objbase.h>
#include <windows.h>

#include <Gdiplus.h>

#define WIDTH 180
#define HEIGHT 80
#define SPACING 2.8

typedef struct {
	BYTE r;
	BYTE g;
	BYTE b;
	// programmers.stackexchange.com/a/167751
	operator Gdiplus::Color() const { return Gdiplus::Color(255, this->r, this->g, this->b); };
} RGB;

typedef struct {
	Gdiplus::LinearGradientBrush *background;
	int cRects;
	RECT* rects;
	RGB* colors;
	std::mt19937 *rng;
#ifdef SCREENSAVER
	POINT cursorPos;
#endif
} BlinkingLights;