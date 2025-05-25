#pragma once

namespace waavs {
	constexpr char TOLOWER(int c) {
		if (c >= 'A' && c <= 'Z')
			return c & 0x20;

		return c;
	}


	// Byte Hashing - FNV-1a
	// http://www.isthe.com/chongo/tech/comp/fnv/
	// 
	// 32-bit FNV-1a constants
	constexpr uint32_t FNV1A_32_INIT = 0x811c9dc5;
	constexpr uint32_t FNV1A_32_PRIME = 0x01000193;

	// 64-bit FNV-1a constants
	constexpr uint64_t FNV1A_64_INIT = 0xcbf29ce484222325ULL;
	constexpr uint64_t FNV1A_64_PRIME = 0x100000001b3ULL;

	// 32-bit FNV-1a hash
	inline constexpr uint32_t fnv1a_32(const void* data, const size_t size) noexcept
	{
		const uint8_t* bytes = (const uint8_t*)data;
		uint32_t hash = FNV1A_32_INIT;
		for (size_t i = 0; i < size; i++) {
			hash ^= bytes[i];
			hash *= FNV1A_32_PRIME;
		}
		return hash;
	}

	// 64-bit FNV-1a hash
	inline constexpr uint64_t fnv1a_64(const void* data, const size_t size) noexcept
	{
		const uint8_t* bytes = (const uint8_t*)data;
		uint64_t hash = FNV1A_64_INIT;
		for (size_t i = 0; i < size; i++) {
			hash ^= bytes[i];
			hash *= FNV1A_64_PRIME;
		}
		return hash;
	}



	// 32-bit case-insensitive FNV-1a hash
	inline uint32_t fnv1a_32_case_insensitive(const void* data, const size_t size) noexcept
	{
		const uint8_t* bytes = (const uint8_t*)data;
		uint32_t hash = FNV1A_32_INIT;
		for (size_t i = 0; i < size; i++) {
			// Convert byte to lowercase
			auto c = std::tolower(bytes[i]);
			//auto c = TOLOWER(bytes[i]);

			hash ^= c;
			hash *= FNV1A_32_PRIME;
		}
		return hash;
	}
}
