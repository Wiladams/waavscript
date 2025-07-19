#pragma once


#include <unordered_map>
#include <string>

#include "definitions.h"
#include "bithacks.h"


namespace waavs
{
	// OctetCursor
	// A cursor over a set of 8-bit bytes.
	// We are restricting to C++17, so limited in the usage of std::length() and std::string_view.
	//
	struct OctetCursor
	{
		const uint8_t* fStart; // Pointer to the start of the span
		const uint8_t* fEnd;   // Pointer to the end of the span

		constexpr OctetCursor() noexcept : fStart(nullptr), fEnd(nullptr) {}

		constexpr OctetCursor(const void* start, size_t len) noexcept
			: fStart(static_cast<const uint8_t*>(start))
			, fEnd(fStart + len) 
		{
		}

		// Non-constexpr constructor for runtime C-strings
		// OctetCursor c1("Hello, World!");
		//
		OctetCursor(const char* cstr)
			: fStart(reinterpret_cast<const uint8_t*>(cstr)),
			fEnd(reinterpret_cast<const uint8_t*>(cstr) + std::strlen(cstr)) 
		{
		}

		// constexpr constructor for string literals
		// constexpr OctetCursor c2("Hello");  // uses template constructor
		// static_assert(c2.size() == 5, "Size should be 5");
		// 
		//template <size_t N>
		//constexpr OctetCursor(const char(&str)[N])
		//	: fStart(reinterpret_cast<const uint8_t*>(str)),
		//	fEnd(reinterpret_cast<const uint8_t*>(str + N - 1)) 
		//{
			// exclude null terminator
		//}

		constexpr size_t size() const noexcept { return fEnd - fStart; }
		constexpr bool empty() const noexcept { return fStart == fEnd; }
		constexpr const uint8_t* data() const noexcept { return fStart; }
		constexpr const uint8_t* begin() const noexcept { return fStart; }
		constexpr const uint8_t* end() const noexcept { return fEnd; }

		// Subtle error is swallowed: if offset is out of bounds, return 0.
		constexpr uint8_t operator*() const noexcept { return (fStart < fEnd) ? *fStart : 0; }

		// pre-increment operator
		// This operator will increment the cursor first, then return a reference to the incremented object.
		constexpr OctetCursor& operator++() noexcept { if (fStart < fEnd) ++fStart; return *this; }

		// post-increment is more than just syntactic sugar.
		// There is a subtle difference between pre-increment and post-increment.
		// The post-increment operator returns a copy of the object before it was incremented.
		// We're not going to implement this for now, as we want to force a clear decision about
		// the semantics at the time of usage, and we'll start with only pre-increment.
		// because that's the less expensive operation.
		//constexpr OctetCursor operator++(int) noexcept
		//{
		//	OctetCursor temp = *this; // Copy the current object
		//	if (fStart < fEnd) ++fStart; // Increment the cursor
		//	return temp; // Return the copy of the object before it was incremented
		//}

		// Helpers
		// peek()
		// Peek at a byte at a particular offset, without advancing the cursor.
		// Subtle error is swallowed: if offset is out of bounds, return 0.
		constexpr uint8_t peek(size_t offset = 0) const noexcept
		{
			return (fStart + offset < fEnd) ? fStart[offset] : 0;
		}

		// advance()
		// Advance the cursor by n bytes, but not beyond the end of the span.
		//constexpr void skip(size_t n) noexcept { fStart = (fStart + n <= fEnd) ? fStart + n : fEnd; }
		constexpr void advance(size_t n) noexcept { fStart = (fStart + n <= fEnd) ? fStart + n : fEnd; }

	};

	// Enforce some constraints
	ASSERT_MEMCPY_SAFE(OctetCursor);
	ASSERT_POD_TYPE(OctetCursor);
	ASSERT_STRUCT_SIZE(OctetCursor, 16);


	inline bool same_span(const OctetCursor& lhs, const OctetCursor& rhs) noexcept
	{
		return (lhs.fStart == rhs.fStart) && (lhs.fEnd == rhs.fEnd);
	}

	// Content comparison between two cursors
	inline bool operator==(const OctetCursor& lhs, const OctetCursor& rhs) noexcept
	{
		size_t len = lhs.size();
		return len == rhs.size() &&
			std::memcmp(lhs.fStart, rhs.fStart, len) == 0;
	}

	inline bool operator!=(const OctetCursor& lhs, const OctetCursor& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	// Comparing to a null terminated c-string
	inline bool operator==(const OctetCursor& a, const char* b) noexcept
	{
		if (!b) return a.size() == 0;
		size_t len = std::strlen(b);
		return a.size() == len && std::memcmp(a.fStart, b, len) == 0;
	}

	inline bool operator==(const char* b, const OctetCursor& a) noexcept
	{
		return a == b;
	}

	inline bool operator!=(const OctetCursor& a, const char* b) noexcept
	{
		return !(a == b);
	}

	inline bool operator!=(const char* b, const OctetCursor& a) noexcept
	{
		return !(a == b);
	}
}



namespace waavs {

	struct OCursorHash {
		size_t operator()(const OctetCursor& span) const noexcept {
			return fnv1a_32(span.data(), span.size());
		}
	};

	// Case insensitive 'string' comparison
	//struct ByteSpanInsensitiveHash {
	//	size_t operator()(const ByteSpan& span) const noexcept {
	//		return waavs::fnv1a_32_case_insensitive(span.data(), span.size());
	//	}
	//};

	// Don't need to implement the following, as long as the operator==
	// does a content comparison.
	//struct OCursorEquivalent {
	//	bool operator()(const OctetCursor& a, const OctetCursor& b) const noexcept {
	//		if (a.size() != b.size())
	//			return false;
	//		return memcmp(a.fStart, b.fStart, a.size()) == 0;
	//	}
	//};



	struct OCursorCaseInsensitive {
		bool operator()(const OctetCursor& a, const OctetCursor& b) const noexcept {
			if (a.size() != b.size())
				return false;

			for (size_t i = 0; i < a.size(); ++i) {
				//if (TOLOWER(a[i]) != TOLOWER(b[i]))  // Case-insensitive comparison
				if (std::tolower(a.data()[i]) != std::tolower(b.data()[i]))  // Case-insensitive comparison

					return false;
			}

			return true;
		}
	};
}



namespace std {
	// Create this hash for OctetCursor to be used in unordered_map, unordered_set, etc.
	template <>
	struct hash<waavs::OctetCursor> {
		size_t operator()(const waavs::OctetCursor& span) const noexcept
		{
			return waavs::fnv1a_32(span.data(), span.size());
		}
	};
}