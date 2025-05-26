#pragma once

#include "bspan.h"

namespace waavs {

	// Efficiently reads the next key-value attribute pair from `src`
	// Attributes are separated by '=' and values are enclosed in '"' or '\''
	static bool readNextKeyAttribute(ByteSpan& src, ByteSpan& key, ByteSpan& value) noexcept
	{
		key.reset();
		value.reset();

		// Trim leading whitespace
		src.skipWhile(chrWspChars);

		if (!src)
			return false;

		// Handle end tag scenario (e.g., `/>`)
		if (*src == '/')
			return false;

		// Capture attribute name up to '='
		const uint8_t* keyStart = src.fStart;
		const uint8_t* keyEnd = keyStart; // track last non-whitespace char seen
		while (src.fStart < src.fEnd && *src.fStart != '=')
		{
			if (!chrWspChars(*src.fStart)) 
			{
				keyEnd = src.fStart + 1; // past the last non-space character
			}
			++src.fStart;
		}

		// If no '=' found, return false
		if (src.empty())
			return false;

		// Assign key � trimmed to exclude any trailing whitespace
		key = ByteSpan(keyStart, keyEnd);

		// Move past '='
		++src.fStart;

		// Skip any whitespace
		src.skipWhile(chrWspChars);

		if (src.empty())
			return false;

		// Ensure we have a quoted value
		uint8_t quoteChar = *src;
		if (quoteChar !='"' && quoteChar !='\'')
			return false;

		// Move past the opening quote
		src++;

		// Locate the closing quote using `memchr`
		const uint8_t* endQuote = static_cast<const uint8_t*>(std::memchr(src.fStart, quoteChar, src.size()));

		if (!endQuote)
			return false; // No closing quote found

		// Assign the attribute value (excluding quotes)
		value = { src.fStart, endQuote };

		// Move past the closing quote
		src.fStart = endQuote + 1;

		return true;
	}

	// Searches `inChunk` for an attribute `key` and returns its value if found
	static bool getKeyValue(const ByteSpan& inChunk, const ByteSpan& key, ByteSpan& value) noexcept
	{
		ByteSpan src = inChunk;
		ByteSpan name{};
		bool insideQuotes = false;
		uint8_t quoteChar = 0;

		while (src)
		{
			// Skip leading whitespace
			src = chunk_ltrim(src, chrWspChars);

			if (!src)
				return false;

			// If we hit a quote, skip the entire quoted section
			if (*src == '"' || *src == '\'')
			{
				quoteChar = *src;
				src++; // Move past opening quote

				// Use `chunk_find_char` to efficiently skip to closing quote
				src = chunk_find_char(src, quoteChar);
				if (!src)
					return false;

				src++; // Move past closing quote
				continue;
			}

			// Extract the next token as a potential key
			ByteSpan keyCandidate = chunk_token_char(src, '=');
			keyCandidate = chunk_trim(keyCandidate, chrWspChars);

			// If this matches the requested key, extract the value
			if (keyCandidate == key)
			{
				// Skip whitespace before value
				src = chunk_ltrim(src, chrWspChars);

				if (!src)
					return false;

				// **Only accept quoted values**
				if (*src == '"' || *src == '\'')
				{
					quoteChar = *src;
					src++;
					value.fStart = src.fStart;

					// Find the closing quote **quickly**
					src = chunk_find_char(src, quoteChar);
					if (!src)
						return false;

					value.fEnd = src.fStart;  // Exclude the closing quote
					src++;
					return true; // Successfully found key and value
				}

				// **Reject unquoted values in XML**
				return false;
			}

			// If there was no `=`, continue scanning
			src = chunk_ltrim(src, chrWspChars);
			if (src && *src == '=')
				src++; // Skip past '=' and continue parsing
		}

		return false; // Key not found
	}

}

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