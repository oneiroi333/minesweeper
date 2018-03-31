#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utf8_lib.h"

const char *UTF8_BOM = "\xEF\xBB\xBF";

#define VALID_CONT_BYTE(byte) \
	((byte & 0xc0) == 0x80)

#define UTF8_LENGTH(utf8, len)				\
	do {						\
		if (utf8 < 0x80) {			\
			len = 1;			\
		} else if ((utf8 & 0xe0) == 0xc0) {	\
			len = 2;			\
		} else if ((utf8 & 0xf0) == 0xe0) {	\
			len = 3;			\
		} else if ((utf8 & 0xf8) == 0xf0) {	\
			len = 4;			\
		} else {				\
			len = 0;			\
		}					\
	} while (0)

#define UTF8_TO_UCS4(utf8, len, ucs4)													\
	do {																\
		switch (len) {														\
		case 1:															\
			ucs4 = utf8[0];													\
			break;														\
		case 2:															\
			ucs4 = ((utf8[0] & 0x1f) << 6) | (utf8[1] & 0x3f);								\
			break;														\
		case 3:															\
			ucs4 = ((utf8[0] & 0x1f) << 12) | ((utf8[1] & 0x3f) << 6) | (utf8[2] & 0x3f);					\
			break;														\
		case 4:															\
			ucs4 = ((utf8[0] & 0x1f) << 18) | ((utf8[1] & 0x3f) << 12) | ((utf8[2] & 0x3f) << 6) | (utf8[3] & 0x3f);	\
			break;														\
		default:														\
			;														\
		}															\
	} while (0)

uint32_t *
utf8_to_ucs4(const uint8_t *utf8, size_t utf8_size, size_t *ucs4_size)
{
	uint32_t *ucs4;
	const uint8_t *utf8_cur;
	size_t i, n;
	uint8_t len;

	if (!utf8 || utf8_size < 1 || !ucs4_size) {
		return NULL;
	}

	if ((n = utf8_strlen(utf8, utf8_size)) == 0) {
		return NULL;
	}
	*ucs4_size = n;

	ucs4 = (uint32_t *) malloc(n * sizeof(*ucs4));
	if (ucs4 == NULL) {
		return NULL;
	}

	utf8_cur = utf8;
	for (i = 0; i < n; ++i) {
		UTF8_LENGTH(*utf8_cur, len);
		UTF8_TO_UCS4(utf8_cur, len, ucs4[i]);
		utf8_cur = utf8_next_char(utf8_cur);
	}

	return ucs4;
}

size_t
utf8_strlen(const uint8_t *utf8, size_t utf8_size)
{
	const uint8_t *cur, *end;
	size_t strlen, len;

	if (!utf8 || utf8_size < 1) {
		return 0;
	}

	strlen = 0;
	cur = utf8;
	end = utf8 + utf8_size;
	while (cur < end) {
		UTF8_LENGTH(*cur, len);
		if (len == 0) {
			return 0;
		}
		cur += len;
		++strlen;
	}

	return strlen;
}

const uint8_t *
utf8_next_char(const uint8_t *utf8)
{
	size_t len;

	if (!utf8) {
		return 0;
	}

	UTF8_LENGTH(*utf8, len);
	if (len == 0) {
		return 0;
	}

	return utf8 + len;
}

uint8_t
is_valid_utf8(const uint8_t *utf8, size_t utf8_size)
{
	uint32_t ucs4;
	size_t ucs4_size;
	const uint8_t *cur, *end;
	uint8_t i;

	if (!utf8 || utf8_size < 1) {
		return 0;
	}

	cur = utf8;
	end = utf8 + utf8_size;
	/* Skip BOM bytes */
	if (utf8_size >= 3) {
		if (memcmp(cur, UTF8_BOM, 3) == 0) {
			cur += 3;
		}
	}
	while (cur < end) {
		UTF8_LENGTH(*cur, ucs4_size);
		if (ucs4_size == 0) {
			return 0;
		}
		if (ucs4_size == 1) {
			++cur;
			continue;
		}
		if (cur + ucs4_size >= end) {
			return 0;
		}
		/* Check if continuation bytes of multibyte char are valid */
		for (i = 1; i < ucs4_size; ++i) {
			if (!VALID_CONT_BYTE(cur[i])) {
				return 0;
			}
		}
		ucs4 = 0;
		UTF8_TO_UCS4(cur, ucs4_size, ucs4);
		/* Check if the value of the multibyte char is valid */
		if (ucs4_size == 2) {
			if (ucs4 < 0x80 || ucs4 > 0x07ff) {
				return 0;
			}
		} else if (ucs4_size == 3) {
			if (ucs4 < 0x0800 || ucs4 > 0xffff) {
				return 0;
			}
		} else { /* ucs4_size == 4 */
			if (ucs4 < 0x10000 || ucs4 > 0x10ffff) {
				return 0;
			}
		}
		cur += ucs4_size;
	}

	return 1;
}
