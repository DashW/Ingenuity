#pragma once

#include "InputState.h"

#include <string>

namespace Ingenuity {

const unsigned MAX_TOUCHES = 10;
const unsigned MAX_KEYS = 256;

struct MouseState
{
	unsigned x, y, prevX, prevY;
	float dX, dY;
	bool leftDown, middleDown, rightDown,
		leftUp, middleUp, rightUp,
		entered, exited, over,
		left, middle, right;

	int scroll;

	MouseState()
		: x(0), y(0), prevX(0), prevY(0), dX(0.0f), dY(0.0f),
		leftDown(false), middleDown(false), rightDown(false),
		leftUp(false), middleUp(false), rightUp(false),
		entered(false), exited(false), over(false),
		left(false), middle(false), right(false) {}
};

struct TouchState
{
	unsigned
		x[MAX_TOUCHES],
		y[MAX_TOUCHES],
		prevX[MAX_TOUCHES],
		prevY[MAX_TOUCHES];
};

static const char charToScanCode[128]
{
	0x0,	//NUL
	0x0,	//SOH
	0x0,	//STX
	0x0,	//ETX
	0x0,	//EOT
	0x0,	//ENQ
	0x0,	//ACK
	0x0,	//BEL
	0x0e,	//BS
	0x0f,	//HT
	0x0,	//LF
	0x0,	//VT
	0x0,	//FF
	0x1c,	//CR
	0x0,	//SO
	0x0,	//SI
	0x0,	//DLE
	0x0,	//DC1
	0x0,	//DC2
	0x0,	//DC3
	0x0,	//DC4
	0x0,	//NAK
	0x0,	//SYN
	0x0,	//ETB
	0x0,	//CAN
	0x0,	//EM
	0x0,	//SUB
	0x01,	//ESC
	0x0,	//FS
	0x0,	//GS
	0x0,	//RS
	0x0,	//US
	0x39,	//
	0x02,	//!
	0x28,	//"
	0x04,	//#
	0x05,	//$
	0x06,	//%
	0x08,	//&
	0x0,	//'
	0x0a,	//(
	0x0b,	//)
	0x0,	//*
	0x0d,	//+
	0x33,	//,
	0x0c,	//-
	0x34,	//.
	0x35,	///
	0x0b,	//0
	0x02,	//1
	0x03,	//2
	0x04,	//3
	0x05,	//4
	0x06,	//5
	0x07,	//6
	0x08,	//7
	0x09,	//8
	0x0a,	//9
	0x27,	//:
	0x27,	//;
	0x33,	//<
	0x0d,	//=
	0x34,	//>
	0x35,	//?
	0x0,	//@
	0x1e,	//A
	0x30,	//B
	0x2e,	//C
	0x20,	//D
	0x12,	//E
	0x21,	//F
	0x22,	//G
	0x23,	//H
	0x17,	//I
	0x24,	//J
	0x25,	//K
	0x26,	//L
	0x32,	//M
	0x31,	//N
	0x18,	//O
	0x19,	//P
	0x10,	//Q
	0x13,	//R
	0x1f,	//S
	0x14,	//T
	0x16,	//U
	0x2f,	//V
	0x11,	//W
	0x2d,	//X
	0x15,	//Y
	0x2c,	//Z
	0x1a,	//[
	0x2b,	//\ 
	0x1b,	//]
	0x07,	//^
	0x0c,	//_
	0x29,	//`
	0x1e,	//a
	0x30,	//b
	0x2e,	//c
	0x20,	//d
	0x12,	//e
	0x21,	//f
	0x22,	//g
	0x23,	//h
	0x17,	//i
	0x24,	//j
	0x25,	//k
	0x26,	//l
	0x32,	//m
	0x31,	//n
	0x18,	//o
	0x19,	//p
	0x10,	//q
	0x13,	//r
	0x1f,	//s
	0x14,	//t
	0x16,	//u
	0x2f,	//v
	0x11,	//w
	0x2d,	//x
	0x15,	//y
	0x2c,	//z
	0x1a,	//{
	0x2b,	//|
	0x1b,	//}
	0x0,	//~
	0x0,	//
};

struct KeyState
{
	// Helper text characters, NOT to be used as scan codes
	static const unsigned char BACKSP_CHAR = 0x08;
	static const unsigned char TAB_CHAR = 0x11;
	static const unsigned char ENTER_CHAR = 0x15; // '\n'
	static const unsigned char ESCAPE_CHAR = 0x1B;
	static const unsigned char SPACE_CHAR = 0x20;
	static const unsigned char CTRLBACK_CHAR = 0x7F;

	static const unsigned char UP_CODE = 0x48;
	static const unsigned char DOWN_CODE = 0x50;
	static const unsigned char LEFT_CODE = 0x4b;
	static const unsigned char RIGHT_CODE = 0x4d;

	bool keys[MAX_KEYS];
	bool upKeys[MAX_KEYS];
	bool downKeys[MAX_KEYS];

	std::string text;

	KeyState()
	{
		memset(keys, 0, sizeof(bool) * MAX_KEYS);
		memset(upKeys, 0, sizeof(bool) * MAX_KEYS);
		memset(downKeys, 0, sizeof(bool) * MAX_KEYS);
		text = "";
	}
};

class InputState
{
public:
	MouseState mouse;
	MouseState mousePrev;
	KeyState keyboard;
	KeyState keyboardPrev;

	InputState() {}

	virtual MouseState& GetMouseState()
	{
		return mousePrev;
	};
	virtual KeyState& GetKeyState()
	{
		return keyboardPrev;
	};
	virtual void Update();

	virtual char CharToScanCode(char inputChar)
	{
		if(inputChar < 128)
			return charToScanCode[inputChar];
		return 0;
	}
};

} // namespace Ingenuity
