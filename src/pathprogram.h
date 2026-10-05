
// pathprogram.h
#pragma once


static_assert(__cplusplus >= 202002L, "pathp requires C++20 or later");

#include <cstdint>
#include <cstddef>
#include <vector>
#include <array>


// Machinery for a PathProgram
// That represents a sequence of path segment operations
namespace waavs
{
    // These ops represent normalized commands.  That means...
    //  - no relative commands exist
    //  - no implicit lineto after moveto
    //  - arcs are already endpoint-form
    //  - smooth curves are expanded
    enum PathOp : uint8_t
    {
        OP_END = 0,
        OP_MOVETO,
        OP_LINETO,
        OP_CUBICTO,
        OP_QUADTO,
        OP_ARCTO,
        OP_CLOSE,
        //OP_COUNT
    };

    inline constexpr uint8_t kPathOpInvalidArity = 0xff;

    inline constexpr std::array<uint8_t, 256> kPathOpArity = []()
        {
            std::array<uint8_t, 256> table{};
            table.fill(kPathOpInvalidArity);

            table[OP_END] = 0;
            table[OP_MOVETO] = 2;
            table[OP_LINETO] = 2;
            table[OP_CUBICTO] = 6;
            table[OP_QUADTO] = 4;
            table[OP_ARCTO] = 7;
            table[OP_CLOSE] = 0;

            return table;
        }();

    constexpr uint8_t pathOpArity(PathOp op) noexcept
    {
        return kPathOpArity[static_cast<uint8_t>(op)];
    }



    // The container for a path program
    //	canonical
    //  comparable
    //  cacheable
    // 
    struct PathProgram
    {
        std::vector<uint8_t> ops;
        std::vector<float> args;

        void clear() noexcept
        {
            ops.clear();
            args.clear();
        }
    };
}


