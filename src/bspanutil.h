#pragma once

#include "bspan.h"

namespace waavs {
	// readNextCSSKeyValue()
// 
// Properties are separated by ';'
// values are separated from the key with ':'
// Ex: <tagname style="stroke:black;fill:white" />
// Return
//   true - if a valid key/value pair was found
//      in this case, key, and value will be populated
//   false - if no key/value pair was found, or end of string
//      in this case, key, and value will be undefined
//
	static bool readNextCSSKeyValue(ByteSpan& src, ByteSpan& key, ByteSpan& value, const unsigned char fieldDelimeter = ';', const unsigned char keyValueSeparator = ':') noexcept
	{
		// Trim leading whitespace to begin
		src = chunk_ltrim(src, chrWspChars);

		// If the string is now blank, return immediately
		if (!src)
			return false;

		// peel off a key/value pair by taking a token up to the fieldDelimeter
		value = chunk_token_char(src, fieldDelimeter);

		// Now, separate the key from the value using the keyValueSeparator
		key = chunk_token_char(value, keyValueSeparator);

		// trim the key and value fields of whitespace
		key = chunk_trim(key, chrWspChars);
		value = chunk_trim(value, chrWspChars);

		return true;
	}


	INLINE void writeChunk(const ByteSpan& chunk) noexcept
	{
		ByteSpan s = chunk;

		while (s && *s) {
			printf("%c", *s);
			s++;
		}
	}

	INLINE void writeChunkBordered(const ByteSpan& chunk) noexcept
	{
		ByteSpan s = chunk;

		printf("||");
		while (s && *s) {
			printf("%c", *s);
			s++;
		}
		printf("||");
	}

	INLINE void printChunk(const ByteSpan& chunk) noexcept
	{
		if (chunk)
		{
			writeChunk(chunk);
			printf("\n");
		}
		else
			printf("BLANK==CHUNK\n");

	}
}