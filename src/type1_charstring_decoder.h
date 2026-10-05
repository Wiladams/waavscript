// type1_charstring_decoder.h
#pragma once

#include <cstdint>
#include <cstdio>
#include <vector>

#include "ocspan.h"
#include "pathprogram_builder.h"

namespace waavs
{
    struct Type1GlyphMetrics
    {
        float sideBearingX = 0.0f;
        float sideBearingY = 0.0f;
        float advanceX = 0.0f;
        float advanceY = 0.0f;
    };


    class Type1CharStringDecoder
    {
    public:
        Type1CharStringDecoder(PathProgramBuilder& builder, const std::vector<OctetCursor>& subrs, int lenIV = 4) noexcept
            : fBuilder(builder),
              fSubrs(subrs),
              fLenIV(lenIV)
        {
        }

        bool decode(OctetCursor encryptedCharString, Type1GlyphMetrics& metrics)
        {
            fBuilder.reset();

            fStack.clear();
            fMetrics = {};
            fX = 0.0f;
            fY = 0.0f;
            fHaveWidth = false;
            fEnded = false;

            std::vector<uint8_t> decrypted;
            if (!decrypt(encryptedCharString, decrypted))
                return false;

            if (!execute(OctetCursor(decrypted.data(), decrypted.size()), false, 0))
                return false;

            if (!fEnded)
                return false;

            metrics = fMetrics;
            return fBuilder.finalize();
        }

    private:
        static constexpr uint16_t kCharStringKey = 4330u;
        static constexpr uint16_t kC1 = 52845u;
        static constexpr uint16_t kC2 = 22719u;
        static constexpr size_t kMaxStack = 48;
        static constexpr unsigned kMaxSubrDepth = 32;

        PathProgramBuilder& fBuilder;
        const std::vector<OctetCursor>& fSubrs;
        int fLenIV = 4;

        std::vector<double> fStack;
        Type1GlyphMetrics fMetrics;

        float fX = 0.0f;
        float fY = 0.0f;

        bool fHaveWidth = false;
        bool fEnded = false;


        bool decrypt(OctetCursor encrypted, std::vector<uint8_t>& out) const
        {
            out.clear();

            if (fLenIV < 0)
            {
                out.assign(encrypted.begin(), encrypted.end());
                return true;
            }

            const size_t discard = static_cast<size_t>(fLenIV);

            if (encrypted.size() < discard)
                return false;

            uint16_t r = kCharStringKey;
            size_t index = 0;

            out.reserve(encrypted.size() - discard);

            while (!encrypted.empty())
            {
                const uint8_t cipher = *encrypted;
                ++encrypted;

                const uint8_t plain = static_cast<uint8_t>(cipher ^ (r >> 8));

                r = static_cast<uint16_t>((static_cast<uint32_t>(cipher + r) * kC1 + kC2) & 0xFFFFu);

                if (index >= discard)
                    out.push_back(plain);

                ++index;
            }

            return true;
        }


        bool execute(OctetCursor code, bool isSubr, unsigned depth)
        {
            if (depth > kMaxSubrDepth)
                return false;

            while (!code.empty())
            {
                const uint8_t b = *code;
                ++code;

                if (b >= 32)
                {
                    double value;

                    if (!decodeNumber(b, code, value))
                        return false;

                    if (fStack.size() >= kMaxStack)
                        return false;

                    fStack.push_back(value);
                    continue;
                }

                switch (b)
                {
                case 1:     // hstem
                    if (!opHint()) return false;
                    break;

                case 3:     // vstem
                    if (!opHint()) return false;
                    break;

                case 4:     // vmoveto
                    if (!opVMoveTo()) return false;
                    break;

                case 5:     // rlineto
                    if (!opRLineTo()) return false;
                    break;

                case 6:     // hlineto
                    if (!opHLineTo()) return false;
                    break;

                case 7:     // vlineto
                    if (!opVLineTo()) return false;
                    break;

                case 8:     // rrcurveto
                    if (!opRRCurveTo()) return false;
                    break;

                case 9:     // closepath
                    if (!opClosePath()) return false;
                    break;

                case 10:    // callsubr
                    if (!opCallSubr(depth)) return false;
                    break;

                case 11:    // return
                    if (!isSubr)
                        return false;
                    return true;

                case 12:    // escape
                    if (code.empty())
                        return false;
                    {
                        const uint8_t escaped = *code;
                        ++code;

                        if (!executeEscape(escaped))
                            return false;
                    }
                    break;

                case 13:    // hsbw
                    if (!opHSBW()) return false;
                    break;

                case 14:    // endchar
                    if (isSubr)
                        return false;

                    fStack.clear();
                    fEnded = true;
                    return true;

                case 21:    // rmoveto
                    if (!opRMoveTo()) return false;
                    break;

                case 22:    // hmoveto
                    if (!opHMoveTo()) return false;
                    break;

                case 30:    // vhcurveto
                    if (!opVHCurveTo()) return false;
                    break;

                case 31:    // hvcurveto
                    if (!opHVCurveTo()) return false;
                    break;

                default:
                    std::printf("Type1: unsupported operator %u\n", static_cast<unsigned>(b));
                    return false;
                }
            }

            return isSubr;
        }


        bool decodeNumber(uint8_t b, OctetCursor& code, double& value) const noexcept
        {
            if (b >= 32 && b <= 246)
            {
                value = static_cast<int>(b) - 139;
                return true;
            }

            if (b >= 247 && b <= 250)
            {
                if (code.empty())
                    return false;

                const uint8_t w = *code;
                ++code;

                value = static_cast<int>((b - 247) * 256 + w + 108);
                return true;
            }

            if (b >= 251 && b <= 254)
            {
                if (code.empty())
                    return false;

                const uint8_t w = *code;
                ++code;

                value = -static_cast<int>((b - 251) * 256 + w + 108);
                return true;
            }

            if (b == 255)
            {
                if (code.size() < 4)
                    return false;

                const uint32_t raw =
                    (static_cast<uint32_t>(code.peek(0)) << 24) |
                    (static_cast<uint32_t>(code.peek(1)) << 16) |
                    (static_cast<uint32_t>(code.peek(2)) << 8) |
                    static_cast<uint32_t>(code.peek(3));

                code.advance(4);

                value = static_cast<double>(static_cast<int32_t>(raw));
                return true;
            }

            return false;
        }


        bool executeEscape(uint8_t op)
        {
            switch (op)
            {
            case 0:     // dotsection
                fStack.clear();
                return true;

            case 1:     // vstem3
            case 2:     // hstem3
                return opHint();

            case 6:     // seac
                std::printf("Type1: unsupported escape operator 12 6 (seac)\n");
                return false;

            case 7:     // sbw
                return opSBW();

            case 12:    // div
                return opDiv();

            case 16:    // callothersubr
                std::printf("Type1: unsupported escape operator 12 16 (callothersubr)\n");
                return false;

            case 17:    // pop
                std::printf("Type1: unsupported escape operator 12 17 (pop)\n");
                return false;

            case 33:    // setcurrentpoint
                return opSetCurrentPoint();

            default:
                std::printf("Type1: unsupported escape operator 12 %u\n", static_cast<unsigned>(op));
                return false;
            }
        }


        bool opHSBW()
        {
            if (fStack.size() != 2)
                return false;

            fMetrics.sideBearingX = static_cast<float>(fStack[0]);
            fMetrics.sideBearingY = 0.0f;
            fMetrics.advanceX = static_cast<float>(fStack[1]);
            fMetrics.advanceY = 0.0f;

            fX = fMetrics.sideBearingX;
            fY = 0.0f;

            fHaveWidth = true;
            fStack.clear();
            return true;
        }


        bool opSBW()
        {
            if (fStack.size() != 4)
                return false;

            fMetrics.sideBearingX = static_cast<float>(fStack[0]);
            fMetrics.sideBearingY = static_cast<float>(fStack[1]);
            fMetrics.advanceX = static_cast<float>(fStack[2]);
            fMetrics.advanceY = static_cast<float>(fStack[3]);

            fX = fMetrics.sideBearingX;
            fY = fMetrics.sideBearingY;

            fHaveWidth = true;
            fStack.clear();
            return true;
        }


        bool opRMoveTo()
        {
            if (fStack.size() != 2)
                return false;

            fX += static_cast<float>(fStack[0]);
            fY += static_cast<float>(fStack[1]);
            fStack.clear();

            return fBuilder.moveTo(fX, fY);
        }


        bool opHMoveTo()
        {
            if (fStack.size() != 1)
                return false;

            fX += static_cast<float>(fStack[0]);
            fStack.clear();

            return fBuilder.moveTo(fX, fY);
        }


        bool opVMoveTo()
        {
            if (fStack.size() != 1)
                return false;

            fY += static_cast<float>(fStack[0]);
            fStack.clear();

            return fBuilder.moveTo(fX, fY);
        }


        bool opRLineTo()
        {
            if (fStack.size() != 2)
                return false;

            fX += static_cast<float>(fStack[0]);
            fY += static_cast<float>(fStack[1]);
            fStack.clear();

            return fBuilder.lineTo(fX, fY);
        }


        bool opHLineTo()
        {
            if (fStack.size() != 1)
                return false;

            fX += static_cast<float>(fStack[0]);
            fStack.clear();

            return fBuilder.lineTo(fX, fY);
        }


        bool opVLineTo()
        {
            if (fStack.size() != 1)
                return false;

            fY += static_cast<float>(fStack[0]);
            fStack.clear();

            return fBuilder.lineTo(fX, fY);
        }


        bool opRRCurveTo()
        {
            if (fStack.size() != 6)
                return false;

            const float x1 = fX + static_cast<float>(fStack[0]);
            const float y1 = fY + static_cast<float>(fStack[1]);

            const float x2 = x1 + static_cast<float>(fStack[2]);
            const float y2 = y1 + static_cast<float>(fStack[3]);

            const float x3 = x2 + static_cast<float>(fStack[4]);
            const float y3 = y2 + static_cast<float>(fStack[5]);

            fStack.clear();

            if (!fBuilder.cubicTo(x1, y1, x2, y2, x3, y3))
                return false;

            fX = x3;
            fY = y3;
            return true;
        }


        bool opVHCurveTo()
        {
            if (fStack.size() != 4)
                return false;

            const float x1 = fX;
            const float y1 = fY + static_cast<float>(fStack[0]);

            const float x2 = x1 + static_cast<float>(fStack[1]);
            const float y2 = y1 + static_cast<float>(fStack[2]);

            const float x3 = x2 + static_cast<float>(fStack[3]);
            const float y3 = y2;

            fStack.clear();

            if (!fBuilder.cubicTo(x1, y1, x2, y2, x3, y3))
                return false;

            fX = x3;
            fY = y3;
            return true;
        }


        bool opHVCurveTo()
        {
            if (fStack.size() != 4)
                return false;

            const float x1 = fX + static_cast<float>(fStack[0]);
            const float y1 = fY;

            const float x2 = x1 + static_cast<float>(fStack[1]);
            const float y2 = y1 + static_cast<float>(fStack[2]);

            const float x3 = x2;
            const float y3 = y2 + static_cast<float>(fStack[3]);

            fStack.clear();

            if (!fBuilder.cubicTo(x1, y1, x2, y2, x3, y3))
                return false;

            fX = x3;
            fY = y3;
            return true;
        }


        bool opClosePath()
        {
            if (!fStack.empty())
                return false;

            if (!fBuilder.subpathOpen())
                return true;

            if (!fBuilder.close())
                return false;

            fX = fBuilder.curX();
            fY = fBuilder.curY();
            return true;
        }


        bool opCallSubr(unsigned depth)
        {
            if (fStack.empty())
                return false;

            const double value = fStack.back();
            fStack.pop_back();

            const int index = static_cast<int>(value);

            if (value != static_cast<double>(index) || index < 0 || static_cast<size_t>(index) >= fSubrs.size())
                return false;

            std::vector<uint8_t> decrypted;

            if (!decrypt(fSubrs[static_cast<size_t>(index)], decrypted))
                return false;

            return execute(OctetCursor(decrypted.data(), decrypted.size()), true, depth + 1);
        }


        bool opDiv()
        {
            if (fStack.size() < 2)
                return false;

            const double denominator = fStack.back();
            fStack.pop_back();

            const double numerator = fStack.back();
            fStack.pop_back();

            if (denominator == 0.0)
                return false;

            fStack.push_back(numerator / denominator);
            return true;
        }


        bool opSetCurrentPoint()
        {
            if (fStack.size() != 2)
                return false;

            fX = static_cast<float>(fStack[0]);
            fY = static_cast<float>(fStack[1]);
            fStack.clear();

            return true;
        }


        bool opHint()
        {
            // Geometry-only conversion. Type 1 hints affect rasterization,
            // not the underlying outline.
            fStack.clear();
            return true;
        }
    };
}
