// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef __STDAFX_H
#define __STDAFX_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <ASSERT.h>
#include <memory.h>
#define _USE_MATH_DEFINES
#include <math.h>

#include "Debug.h"
#include "Base.h"
#include "Util.h"
using namespace Util;

#include "Vector.h"
#include "Rect.h"
//#include "Matrix.h"

#include "List.h"
#include "Array.h"
#include "Bits.h"
#include "Hash.h"
#include "AVLTree.h"

#include "Str.h"
#include "Parse.h"
#include "Var.h"
#include "VarUtil.h"

#define INITGUID
#include <D3D11.h>
#include <Windows.h>

// TODO: reference additional headers your program requires here

#endif
