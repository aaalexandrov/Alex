#include "utf8.h"
#include "dbg.h"

namespace utf8 {

uint8_t cp_size(uint32_t cp)
{
	if (cp <= 0x7f)
		return 1;
	if (cp <= 0x7ff)
		return 2;
	if (cp <= 0xffff)
		return 3;
	if (cp <= 0x10ffff)
		return 4;
	return 0;
}

uint8_t cp_size(uint8_t *p)
{
	uint8_t len;
	if (*p <= 0x7f) {
		len = 1;
	} else if ((*p & 0xe0) == 0xc0) {
		len = 2;
	} else if ((*p & 0xf0) == 0xe0) {
		len = 3;
	} else if ((*p & 0xf8) == 0xf0) {
		len = 4;
	} else {
		len = 0;
	}
	return len;
}

uint32_t read_cp(uint8_t *&p, uint8_t cpSize)
{
	ASSERT(cpSize > 4 || cpSize == cp_size(p));
	uint8_t len = cpSize <= 4 ? cpSize : cp_size(p);
	uint8_t mask = len >= 2 ? (0x1f >> (len - 2)) : 0xff;
	uint32_t result = p[0] & mask;
	int i = 0;
	switch (len) {
		case 0:
			return InvalidCP;
		case 1:
			break;
		case 4:
			if ((p[++i] & 0xc0) != 0x80)
				return InvalidCP;
			result = (result << 6) | (p[i] & 0x3f);
		case 3:
			if ((p[++i] & 0xc0) != 0x80)
				return InvalidCP;
			result = (result << 6) | (p[i] & 0x3f);
		case 2:
			if ((p[++i] & 0xc0) != 0x80)
				return InvalidCP;
			result = (result << 6) | (p[i] & 0x3f);
	}
	p += len;
	return result;
}

uint8_t write_cp(uint8_t *&p, uint32_t cp, uint8_t cpSize)
{
	ASSERT(cpSize > 4 || cpSize == cp_size(cp));
	uint8_t len = cpSize <= 4 ? cpSize : cp_size(cp);
	switch (len) {
		case 4:
			p[0] = uint8_t((cp >> 18) | 0xf0);
			p[1] = uint8_t(((cp >> 12) & 0x3f) | 0x80);
			p[2] = uint8_t(((cp >> 6) & 0x3f) | 0x80);
			p[3] = uint8_t(((cp >> 0) & 0x3f) | 0x80);
			break;
		case 3:
			p[0] = uint8_t((cp >> 12) | 0xe0);
			p[1] = uint8_t(((cp >> 6) & 0x3f) | 0x80);
			p[2] = uint8_t(((cp >> 0) & 0x3f) | 0x80);
			break;
		case 2:
			p[0] = uint8_t((cp >> 6) | 0xc0);
			p[1] = uint8_t(((cp >> 0) & 0x3f) | 0x80);
			break;
		case 1:
			p[0] = uint8_t(cp);
			break;
	}
	p += len;
	return len;
}

}