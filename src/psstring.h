#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include "pstypes.h"

namespace waavsps {
    struct PSString
    {
    public:
        PSString(size_t capacity)
            : fCapacity(capacity), fLength(0), fIsExecutable(false) {
            fData = new uint8_t[capacity];
        }

        PSString(const char* literal) {
            fLength = fCapacity = std::strlen(literal);
            fData = new uint8_t[fCapacity];
            std::memcpy(fData, literal, fCapacity);
            fIsExecutable = false;
        }

        ~PSString() {
            delete[] fData;
        }

        // Copy constructor
        PSString(const PSString& other)
            : fCapacity(other.fCapacity),
            fLength(other.fLength),
            fIsExecutable(other.fIsExecutable) {
            fData = new uint8_t[fCapacity];
            std::memcpy(fData, other.fData, fLength);
        }

        // Assignment
        PSString& operator=(const PSString& other) {
            if (this != &other) {
                delete[] fData;
                fCapacity = other.fCapacity;
                fLength = other.fLength;
                fIsExecutable = other.fIsExecutable;
                fData = new uint8_t[fCapacity];
                std::memcpy(fData, other.fData, fLength);
            }
            return *this;
        }

        // Capacity and length
        uint32_t capacity() const { return fCapacity; }
        uint32_t length()   const { return fLength; }
        void reset() { fLength = 0; }

        // Attribute
        void setExecutable(bool value) { fIsExecutable = value; }
        bool isExecutable() const { return fIsExecutable; }

        // Access
        bool get(uint32_t index, uint8_t& out) const {
            if (index >= fLength) return false;
            out = fData[index];
            return true;
        }

        bool put(uint32_t index, uint8_t value) {
            if (index >= fCapacity) return false;
            fData[index] = value;
            if (index >= fLength) {
                fLength = index + 1;
            }
            return true;
        }

        // Get a subrange as new PSString
        PSString getInterval(uint32_t index, uint32_t count) const {
            if (index >= fCapacity || count == 0)
                return PSString((size_t)0);

            count = (index + count > fCapacity) ? (fCapacity - index) : count;
            PSString result(count);
            for (uint32_t i = 0; i < count; ++i) {
                result.put(i, fData[index + i]);
            }
            return result;
        }

        // Put contents of another PSString
        bool putInterval(uint32_t offset, const PSString& other) {
            if (offset >= fCapacity) return false;

            uint32_t count = other.length();
            if (offset + count > fCapacity)
                count = fCapacity - offset;

            for (uint32_t i = 0; i < count; ++i) {
                fData[offset + i] = other.fData[i];
            }
            if (offset + count > fLength) {
                fLength = offset + count;
            }
            return true;
        }

        // View as a null-terminated string (not always safe)
        std::string toString() const {
            return std::string(reinterpret_cast<const char*>(fData), fLength);
        }

    private:
        uint8_t* fData = nullptr;
        uint32_t fCapacity = 0;
        uint32_t fLength = 0;
        bool fIsExecutable = false;
    };
}