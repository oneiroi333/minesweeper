#ifndef UTF8_LIB_H
#define UTF8_LIB_H

#include <stdint.h>

/*
 * Decodes UTF8 to UCS4
 */
uint32_t *utf8_to_ucs4(const uint8_t *utf8, size_t utf8_size, size_t *ucs4_size);

/*
 * Number of UCS4 characters in UTF8
 */
size_t utf8_strlen(const uint8_t *utf8, size_t utf8_size);

/*
 * Returns a pointer to the next UTF8 character
 */
const uint8_t *utf8_next_char(const uint8_t *utf8);

/*
 * Checks if the UTF8 is valid
 * Returns: 1 if valid
 * 	    0 if not valid
 */
uint8_t is_valid_utf8(const uint8_t *utf8, size_t utf8_size);

#endif /* UTF8_LIB_H */
