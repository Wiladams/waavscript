// ps_type_string.h

#pragma once

#include "definitions.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace waavs {

    struct PSStringStorage
    {
        std::vector<uint8_t> bytes;
    };


    // --------------------
    // PSString
    //
    // A PostScript string is a view into shared byte storage.
    // Substrings share the same storage with a different offset/length.
    // --------------------
    struct PSString
    {
    private:
        std::shared_ptr<PSStringStorage> fStorage;
        uint32_t fOffset = 0;
        uint32_t fLength = 0;

        PSString(std::shared_ptr<PSStringStorage> storage, uint32_t offset, uint32_t length)
            : fStorage(std::move(storage))
            , fOffset(offset)
            , fLength(length)
        {}

    public:
        PSString()
            : fStorage(std::make_shared<PSStringStorage>())
        {}

        explicit PSString(size_t length)
            : fStorage(std::make_shared<PSStringStorage>())
            , fLength(static_cast<uint32_t>(length))
        {
            fStorage->bytes.resize(length);
        }

        PSString(const uint8_t* src, size_t len)
            : fStorage(std::make_shared<PSStringStorage>())
            , fLength(static_cast<uint32_t>(len))
        {
            if (len != 0)
                fStorage->bytes.assign(src, src + len);
        }

        PSString(const char* cstr)
            : fStorage(std::make_shared<PSStringStorage>())
        {
            if (!cstr)
                return;

            size_t len = std::strlen(cstr);

            if (len != 0)
            {
                const auto* src = reinterpret_cast<const uint8_t*>(cstr);
                fStorage->bytes.assign(src, src + len);
            }

            fLength = static_cast<uint32_t>(len);
        }

        // Copying a PostScript string copies the view, not its bytes.
        PSString(const PSString&) = default;
        PSString(PSString&&) noexcept = default;
        PSString& operator=(const PSString&) = default;
        PSString& operator=(PSString&&) noexcept = default;


        size_t length() const noexcept
        {
            return fLength;
        }

        // Compatibility with existing runtime code.
        // PostScript strings are fixed-length objects, so their visible
        // capacity is their current view length.
        size_t capacity() const noexcept
        {
            return fLength;
        }

        bool empty() const noexcept
        {
            return fLength == 0;
        }


        uint8_t* data() noexcept
        {
            if (fStorage->bytes.empty())
                return nullptr;

            return fStorage->bytes.data() + fOffset;
        }

        const uint8_t* data() const noexcept
        {
            if (fStorage->bytes.empty())
                return nullptr;

            return fStorage->bytes.data() + fOffset;
        }


        void reset() noexcept
        {
            fStorage = std::make_shared<PSStringStorage>();
            fOffset = 0;
            fLength = 0;
        }


        // Retained for existing internal code such as readstring/cvs.
        // This only shortens the current view; it never grows it.
        void setLength(uint32_t length) noexcept
        {
            if (length < fLength)
                fLength = length;
        }


        std::string toString() const
        {
            if (fLength == 0)
                return std::string();

            return std::string(reinterpret_cast<const char*>(data()), fLength);
        }


        uint8_t get(uint32_t index) const noexcept
        {
            return index < fLength ? fStorage->bytes[fOffset + index] : 0;
        }

        bool get(uint32_t index, uint8_t& out) const noexcept
        {
            if (index >= fLength)
                return false;

            out = fStorage->bytes[fOffset + index];
            return true;
        }

        bool put(uint32_t index, uint8_t value) noexcept
        {
            if (index >= fLength)
                return false;

            fStorage->bytes[fOffset + index] = value;
            return true;
        }


        // Return a shared view into this string.
        PSString getInterval(uint32_t offset, uint32_t count) const
        {
            if (offset > fLength || count > fLength - offset)
                return PSString();

            return PSString(fStorage, fOffset + offset, count);
        }


        bool putInterval(uint32_t offset, const PSString& src)
        {
            if (offset > fLength || src.fLength > fLength - offset)
                return false;

            for (uint32_t i = 0; i < src.fLength; ++i)
            {
                uint8_t value;

                if (!src.get(i, value))
                    return false;

                if (!put(offset + i, value))
                    return false;
            }

            return true;
        }


        bool sameValue(const PSString& other) const noexcept
        {
            return fStorage == other.fStorage &&
                fOffset == other.fOffset &&
                fLength == other.fLength;
        }


        // Search for target and return views into this string.
        bool search(const PSString& target, PSString& pre, PSString& match, PSString& post) const
        {
            if (target.length() == 0 || length() < target.length())
                return false;

            const uint8_t* haystack = data();
            const uint8_t* needle = target.data();
            size_t haystackLen = length();
            size_t needleLen = target.length();

            for (size_t i = 0; i <= haystackLen - needleLen; ++i)
            {
                if (std::memcmp(haystack + i, needle, needleLen) == 0)
                {
                    pre = getInterval(0, static_cast<uint32_t>(i));
                    match = getInterval(static_cast<uint32_t>(i), static_cast<uint32_t>(needleLen));
                    post = getInterval(static_cast<uint32_t>(i + needleLen), static_cast<uint32_t>(haystackLen - i - needleLen));
                    return true;
                }
            }

            return false;
        }


        static PSString fromSpan(const uint8_t* src, size_t len)
        {
            return PSString(src, len);
        }

        static PSString fromVector(const std::vector<uint8_t>& v)
        {
            return PSString(v.data(), v.size());
        }

        static PSString fromCString(const char* str)
        {
            return PSString(str);
        }
    };

}
