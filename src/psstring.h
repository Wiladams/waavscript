#pragma once

#include "definitions.h"

#include <memory>
#include <string>
#include <vector>

namespace waavs {
    // --------------------
    // PSString
    // 
    //     Core type representing a string, with a PS interface
    //     This is NOT null terminated.  the length tells you 
    //     the size of the string.  The capacity tells you the size
    //     of the allocated buffer.
    // --------------------
    struct PSString {
    private:
        std::unique_ptr<uint8_t[]> fData;
        uint32_t fLength = 0;
        uint32_t fCapacity = 0;

    public:
        PSString() = default;

        explicit PSString(size_t cap)
            : fData(new uint8_t[cap]()), fLength(0), fCapacity(static_cast<uint32_t>(cap)) {
        }

        PSString(const uint8_t* src, size_t len)
            : fData(new uint8_t[len]), fLength(static_cast<uint32_t>(len)), fCapacity(static_cast<uint32_t>(len)) {
            std::memcpy(fData.get(), src, len);
        }

        PSString(const char* cstr) {
            if (cstr) {
                size_t len = std::strlen(cstr);
                fData = std::unique_ptr<uint8_t[]>(new uint8_t[len]);
                std::memcpy(fData.get(), cstr, len);
                fLength = fCapacity = static_cast<uint32_t>(len);
            }
        }

        // Copy constructor
        PSString(const PSString& other)
            : fData(new uint8_t[other.fCapacity])
            , fLength(other.fLength)
            , fCapacity(other.fCapacity) {
            std::memcpy(fData.get(), other.fData.get(), fLength);
        }

        // Copy assignment
        PSString& operator=(const PSString& other) {
            if (this != &other) {
                fData.reset(new uint8_t[other.fCapacity]);
                fLength = other.fLength;
                fCapacity = other.fCapacity;
                std::memcpy(fData.get(), other.fData.get(), fLength);
            }
            return *this;
        }

        // Move constructor
        PSString(PSString&&) noexcept = default;

        // Move assignment
        PSString& operator=(PSString&&) noexcept = default;

        // Public interface
        size_t length() const noexcept { return fLength; }
        size_t capacity() const noexcept { return fCapacity; }
        uint8_t* data() noexcept { return fData.get(); }
        const uint8_t* data() const noexcept { return fData.get(); }

        void reset() noexcept { fLength = 0; }

        void setLength(uint32_t len) noexcept {
            fLength = (len <= fCapacity) ? len : fCapacity;
        }

        std::string toString() const {
            return std::string(reinterpret_cast<const char*>(fData.get()), fLength);
        }

        uint8_t get(uint32_t i) const noexcept {
            return (i < fLength) ? fData[i] : 0;
        }

        bool get(uint32_t i, uint8_t& out) const noexcept {
            if (i >= fLength) return false;
            out = fData[i];
            return true;
        }

        bool put(uint32_t i, uint8_t value) {
            if (i >= fCapacity) return false;
            fData[i] = value;
            if (i >= fLength) fLength = i + 1;
            return true;
        }

        PSString getInterval(uint32_t offset, uint32_t count) const {
            if (offset >= fLength) return PSString();
            if (count > fLength - offset) count = fLength - offset;
            return PSString(fData.get() + offset, count);
        }

        bool putInterval(uint32_t offset, const PSString& src) {
            if (offset >= fCapacity) return false;
            uint32_t count = src.fLength;
            if (offset + count > fCapacity) count = fCapacity - offset;
            std::memcpy(fData.get() + offset, src.fData.get(), count);
            if (offset + count > fLength) fLength = offset + count;
            return true;
        }

        // Search 
        // Adds to PSString class
        inline bool search(const PSString& target, PSString& pre, PSString& match, PSString& post) const {
            if (target.length() == 0 || this->length() < target.length())
                return false;

            const uint8_t* haystack = this->data();
            const uint8_t* needle = target.data();
            size_t haystackLen = this->length();
            size_t needleLen = target.length();

            for (size_t i = 0; i <= haystackLen - needleLen; ++i) {
                if (std::memcmp(haystack + i, needle, needleLen) == 0) {
                    pre = this->getInterval(0, static_cast<uint32_t>(i));
                    match = target;
                    post = this->getInterval(static_cast<uint32_t>(i + needleLen), this->length() - (i + needleLen));
                    return true;
                }
            }
            return false;
        }



        // Helpful factory constructors
        static PSString fromSpan(const uint8_t* src, size_t len) {
            return len == 0 ? PSString() : PSString(src, len);
		}

        static PSString fromVector(const std::vector<uint8_t>& v) {
            return v.empty() ? PSString() : PSString(v.data(), v.size());
        }

        static PSString fromCString(const char* s) {
            return s ? PSString(s) : PSString();
        }
    };
}