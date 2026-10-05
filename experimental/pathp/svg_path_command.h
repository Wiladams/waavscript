// svg_path_command.h

#pragma once

#include <cstdint>

namespace waavs
{
    // SVGPathCommand
    // Represents the individual commands in an SVG path
    enum class SVGPathCommand : uint8_t
    {
        // Move to
        M = 'M',  // absolute moveto
        m = 'm',  // relative moveto

        // Line to
        L = 'L',  // absolute lineto
        l = 'l',  // relative lineto
        H = 'H',  // absolute horizontal lineto
        h = 'h',  // relative horizontal lineto
        V = 'V',  // absolute vertical lineto
        v = 'v',  // relative vertical lineto

        // Cubic Bezier
        C = 'C',  // absolute cubic Bezier
        c = 'c',  // relative cubic Bezier
        S = 'S',  // absolute smooth cubic Bezier
        s = 's',  // relative smooth cubic Bezier

        // Quadratic Bezier
        Q = 'Q',  // absolute quadratic Bezier
        q = 'q',  // relative quadratic Bezier
        T = 'T',  // absolute smooth quadratic Bezier
        t = 't',  // relative smooth quadratic Bezier

        // Elliptical arc
        A = 'A',  // absolute arc
        a = 'a',  // relative arc

        // Close path
        Z = 'Z',  // absolute closepath
        z = 'z'   // relative closepath (treated the same as Z in most renderers)
    };

    // ------------------------------------------------------------
    // SVG command arity
    //
    // 0xff means the byte is not an SVG path command.
    //
    // inline constexpr gives us one logical header-only definition,
    // initialized entirely at compile time.
    // ------------------------------------------------------------

    inline constexpr uint8_t kSVGPathInvalidArity = 0xff;

    inline constexpr std::array<uint8_t, 256> kSVGPathCommandArity = []()
        {
            std::array<uint8_t, 256> table{};
            table.fill(kSVGPathInvalidArity);


            table['M'] = table['m'] = 2;
            table['L'] = table['l'] = 2;
            table['H'] = table['h'] = 1;
            table['V'] = table['v'] = 1;
            table['C'] = table['c'] = 6;
            table['S'] = table['s'] = 4;
            table['Q'] = table['q'] = 4;
            table['T'] = table['t'] = 2;
            table['A'] = table['a'] = 7;
            table['Z'] = table['z'] = 0;

            return table;
        }();

    constexpr uint8_t svgPathCommandArity(uint8_t ch) noexcept
    {
        return kSVGPathCommandArity[ch];
    }

    constexpr uint8_t svgPathCommandArity(SVGPathCommand cmd) noexcept
    {
        return kSVGPathCommandArity[static_cast<uint8_t>(cmd)];
    }

    constexpr bool isSVGPathCommand(uint8_t ch) noexcept
    {
        return svgPathCommandArity(ch) != kSVGPathInvalidArity;
    }
}