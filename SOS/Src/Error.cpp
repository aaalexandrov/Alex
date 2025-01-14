#include "stdafx.h"
#include "Error.h"
#include "Token.h"

CValue2String::TValueString g_arrIERR2Str[] = {
	VAL2STR(IERR_OK),
	VAL2STR(IERR_UNKNOWN),
	VAL2STR(IERR_INVALID_OPERAND),
  VAL2STR(IERR_OPERAND_TYPE),
  VAL2STR(IERR_INVALID_INSTRUCTION),
	VAL2STR(IERR_UNKNOWN_TOKEN),
	VAL2STR(IERR_PARSING_FAILED),
	VAL2STR(IERR_GRAMMAR_TRANSFORM_FAILED),
	VAL2STR(IERR_COMPILE_FAILED),
	VAL2STR(IERR_DUPLICATE_VARIABLE),
	VAL2STR(IERR_TOO_MANY_INSTRUCTIONS),
};

CValue2String g_IERR2Str(g_arrIERR2Str, ARRSIZE(g_arrIERR2Str));
