// memcursor.h

#pragma once


#include <cstddef>
#include <cstdint>
#include <cstring>

//
// MemCursor
// 
// MemCursor is a non-owning contiguous byte view with cursor semantics.
// Unlike std::span, advancing a MemCursor mutates the beginning of the
// view, which is useful throughout the parser infrastructure.
//

namespace waavs 
{

    struct MemCursor final
    {
    private:
        const uint8_t* fStart{ nullptr };
        const uint8_t* fEnd{ nullptr };

    public:

        // Constructors
        constexpr MemCursor() noexcept = default;

        // Construct from start and end pointers
        constexpr MemCursor(const uint8_t* start, const uint8_t* end) noexcept 
            : fStart(start)
            , fEnd(end) {}
        
        // Construct from a pointer and size
        constexpr MemCursor(const uint8_t * start, size_t sz) noexcept
            : fStart(start)
            , fEnd(start ? start + sz : nullptr)
        {
        }

        // Construct from a null-terminated C string
        // Error:  If there is no null terminator, this will read past the end of the buffer
        MemCursor(const char* cstr) noexcept
        {
            if (!cstr)
            {
                fStart = nullptr;
                fEnd = nullptr;
                return;
            }

            fStart = reinterpret_cast<const uint8_t*>(cstr);
            fEnd = fStart + std::strlen(cstr);
        }


        constexpr void reset() noexcept { fStart = nullptr; fEnd = nullptr; }
        
        constexpr void resetPointers(const uint8_t * start, const uint8_t * end) noexcept
        {
            fStart = start;
            fEnd = end;
        }
        constexpr void resetStart(const uint8_t * start) noexcept { fStart = start; }
        constexpr void resetEnd(const uint8_t* end) noexcept {fEnd = end;}

        constexpr void resetFromSize(const void* data, size_t sz) noexcept
        {
            fStart = static_cast<const uint8_t*>(data);
            fEnd = fStart ? fStart + sz : nullptr;
        }
        


        // setting up for a range-based for loop
        // not actually that useful, as it's just memory traversal
        // but, having data() and size() hides the internals
        [[nodiscard]]
        constexpr const uint8_t* data() const noexcept { return fStart; }
        [[nodiscard]]
        constexpr size_t size() const noexcept { return (fStart && fEnd >= fStart) ? size_t(fEnd - fStart) : 0; }

        [[nodiscard]]
        constexpr const uint8_t* begin() const noexcept { return fStart; }
        [[nodiscard]]
        constexpr const uint8_t* end() const noexcept { return fEnd; }
        [[nodiscard]]
        constexpr bool empty() const noexcept { return size() == 0; }


        // Type conversions
        explicit constexpr operator bool() const noexcept { return !empty(); }


        // 
        // precondition: !empty()
        constexpr uint8_t operator*() const noexcept
        {
            //assert(!empty());
            return *fStart;
        }

        // indexed access
        // precondition: i < size()
        constexpr const uint8_t& operator[](size_t i) const noexcept 
        { 
            //assert(i < size());
            return fStart[i]; 
        }

        // Return a pointer to the i-th element, or nullptr if out of bounds
        constexpr const uint8_t* at(size_t i) const noexcept
        {
            return i < size() ? fStart + i : nullptr;
        }

        // Create a MemCursor beginning at startAt and extending
        // through the end of the current span.
        // If startAt is beyond the end, returns an empty span
        // positioned at the end.
        [[nodiscard]]
        constexpr MemCursor subCursor(size_t startAt) const noexcept
        {
            if (!fStart)
                return {};

            const size_t n = size();
            const size_t off = startAt < n ? startAt : n;

            return MemCursor(fStart + off, n - off);
        }

        // subSpan()
        // 
        // Create a MemCursor that is a view on the current span
        // If the requested position plus size is greater than the amount
        // of span remaining at that position, the size will be truncated 
        // to the amount remaining from the requested position.
        // So, it's more like an intersection of the desired subspan
        // and the current span.
        [[nodiscard]]
        constexpr MemCursor subCursor(size_t startAt, size_t sz) const noexcept
        {
            if (!fStart)
                return {};

            const size_t n = size();
            const size_t off = startAt < n ? startAt : n;
            const size_t len = sz < (n - off) ? sz : (n - off);
            
            return MemCursor(fStart + off, len);
        }

        // take()
        // 
        // Create a MemCursor that is a view on the current span
        // and mutate the current span by advancing the start pointer
        // by the requested size.
        constexpr MemCursor take(size_t n) noexcept
        {
            MemCursor result = subCursor(0, n);
            advance(result.size());
            return result;
        }

        // advance()
        // 
        // advance the start pointer the specified number of entries
        // constrain to end 
        // Protect against null pointers
        constexpr MemCursor& advance(size_t n) noexcept
        {
            if (!fStart || !fEnd)
                return *this;

            const size_t remaining = size();
            fStart += (n < remaining) ? n : remaining;
            return *this;
        }

        constexpr MemCursor& advanceToEnd() noexcept
        {
            fStart = fEnd;
            return *this;
        }

        constexpr MemCursor& operator+=(size_t n) noexcept  {  return advance(n); }

        constexpr MemCursor& operator++() noexcept { return advance(1); }
        //constexpr MemCursor operator++(int) noexcept
        //{
        //    MemCursor tmp = *this;
        //    advance(1);
        //    return tmp;
        //}






        // BUGBUG - not sure these should be used any more
        // favoring interned strings is probably a better approach
        // operators for comparison
        // 
        // operator==;
        // operator!=;
        // operator<=;
        // operator>=;
        
        // isEqual()
        // A pointer comparison
        constexpr bool isEqual(const MemCursor& b) const noexcept
        {
            return fStart == b.fStart && size() == b.size();
        }

        /*
        bool equivalent(const MemCursor& b) const noexcept
        {
            const size_t n = size();
            if (n != b.size())
                return false;

            if (n == 0)
                return true;

            if (!fStart || !b.fStart)
                return false;

            return std::memcmp(fStart, b.fStart, n) == 0;
        }
        */
        // operator==
        // Perform a full content comparison of the two spans
        //bool operator==(const MemCursor& b) const noexcept
        //{
        //    return equivalent(b);
        //}

        //bool operator==(const char* b) const noexcept
        //{
        //    if (!b)
        //        return false;
        //
        //    return equivalent(MemCursor(b));
        //}


        // -----------------------------------------
        // Static factory methods for convenience
        // -----------------------------------------
        static constexpr MemCursor fromPointers(const uint8_t* startAt, const uint8_t* endAt) noexcept
        {
            return MemCursor(startAt, endAt);
        }

        static constexpr MemCursor fromPointerAndSize(const uint8_t* start, size_t sz) noexcept
        {
            return MemCursor(start, sz);
        }
    };

    //ASSERT_MEMCPY_SAFE(MemCursor);
}





