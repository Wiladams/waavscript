#pragma once

#include "definitions.h"

#include <memory>
#include <string>

namespace waavs {
    // --------------------
    // PSString
    // 
    //     Core type representing a string, with a PS interface
    //     This is NOT null terminated.  the length tells you 
    //     the size of the string.  The capacity tells you the size
    //     of the allocated buffer.
    // --------------------
    struct PSString
    {
    private:
        uint32_t fCapacity = 0;
        uint8_t* fData = nullptr;
        uint32_t fLength = 0;

        PSString() = default;

        PSString(size_t cap)
            : fCapacity(cap)
            , fLength(0)
        {
            fData = new uint8_t[cap];       // Allocate memory for the string
            memset(fData, 0, fCapacity);    // Fill with 0s
        }

        PSString(size_t len, const uint8_t* src)
            :fCapacity(len)
            , fLength(len)
        {
            fData = new uint8_t[len];
            std::memcpy(fData, src, len);
        }

        PSString(const char* literal)
        {
            fLength = fCapacity = std::strlen(literal);
            fData = new uint8_t[fCapacity];
			std::copy_n(reinterpret_cast<const uint8_t*>(literal), fCapacity, fData);
        }

    public:
        // brute force
        void setLength(size_t len) 
        { 
            if (len > fCapacity) 
                len = fCapacity; // Ensure we don't exceed capacity
            fLength = len; 
		}

		// Factory methods to create PSString instances
        static std::shared_ptr<PSString> createFromSize(size_t len)
        {
            auto ptr = std::shared_ptr<PSString>(new PSString(len));

            return ptr;
        }

        static std::shared_ptr<PSString> createFromSpan(size_t len, const uint8_t * src)
        {
            if (!src)
				return PSString::createFromSize((size_t)0); // Return empty string if src is null

            auto ptr = std::shared_ptr<PSString>(new PSString(len, src));

            return ptr;
        }

        static std::shared_ptr<PSString> createFromCString(const char* src)
        {
            if (!src)
                return PSString::createFromSize((size_t)0); // Return empty string if src is null

            auto ptr = std::shared_ptr<PSString>(new PSString(src));

            return ptr;
		}


        ~PSString() 
        { 
            delete[] fData; 
        }

        size_t length() const { return fLength; }
        size_t capacity() const { return fCapacity; }
        uint8_t* data() { return fData; }

        void reset() {
            fLength = 0;
        }

        std::string toString() const {
			return fLength > 0 ? std::string(reinterpret_cast<const char*>(fData), fLength) : std::string();
        }

        uint8_t get(uint32_t index) const {
            if (index >= fLength) return 0; // or throw an error
            return fData[index];
        }
        bool get(uint32_t index, uint8_t& out) const {
            if (index >= fLength) return false;
            out = fData[index];
            return true;
        }

        bool put(uint32_t index, uint8_t value) {
            if (index >= fCapacity) return false;
            fData[index] = value;
            if (index >= fLength) fLength = index + 1;
            return true;
        }

        std::shared_ptr<PSString> getInterval(uint32_t index, uint32_t count) const {
            if (index >= fCapacity) 
                return PSString::createFromSize((size_t)0);
            
            if (count > fCapacity - index) count = fCapacity - index;

            auto out = PSString::createFromSpan(count, fData);

            return out;
        }

        bool putInterval(uint32_t offset, const PSString& other) {
            if (offset >= fCapacity) return false;
            uint32_t count = other.fLength;
            if (offset + count > fCapacity) count = fCapacity - offset;

            for (uint32_t i = 0; i < count; ++i)
                put(offset + i, other.fData[i]);

            return true;
        }


    };
}