#pragma once

#pragma comment(lib, "blend2d.lib")


#include <blend2d/blend2d.h>

namespace waavs
{
    struct BLPathSink
    {
        BLPath& path;

        bool onMoveTo(float x, float y) noexcept
        {
            return path.move_to(x, y) == BL_SUCCESS;
        }

        bool onLineTo(float x, float y) noexcept
        {
            return path.line_to(x, y) == BL_SUCCESS;
        }

        bool onQuadTo(float x1, float y1, float x, float y) noexcept
        {
            return path.quad_to(x1, y1, x, y) == BL_SUCCESS;
        }

        bool onCubicTo(float x1, float y1, float x2, float y2, float x, float y) noexcept
        {
            return path.cubic_to(x1, y1, x2, y2, x, y) == BL_SUCCESS;
        }

        bool onArcTo(float rx, float ry, float rotation, float largeArc, float sweep, float x, float y) noexcept
        {
            return path.elliptic_arc_to(rx, ry, rotation, largeArc != 0.0f, sweep != 0.0f, x, y) == BL_SUCCESS;
        }

        bool onClose() noexcept
        {
            return path.close() == BL_SUCCESS;
        }

        void onEnd() noexcept
        {}
    };

    static inline BLMatrix2D blTransform(const PSMatrix& m) {
        return BLMatrix2D(m.m[0], m.m[1], m.m[2], m.m[3], m.m[4], m.m[5]);
    }

    static inline BLStrokeJoin convertLineJoin(PSLineJoin join) {
        switch (join) {
        case PSLineJoin::Miter:
            return BLStrokeJoin::BL_STROKE_JOIN_MITER_CLIP;
        case PSLineJoin::Round:
            return BLStrokeJoin::BL_STROKE_JOIN_ROUND;
        case PSLineJoin::Bevel:
            return BLStrokeJoin::BL_STROKE_JOIN_BEVEL;
        default:
            return BLStrokeJoin::BL_STROKE_JOIN_MITER_CLIP; // Default to miter
        }
    }


    static inline BLRgba32 convertPaint(const PSPaint& p)
    {
        switch (p.kind) {
        case PSPaintKind::GRAY:
            return BLRgba32(uint8_t(p.gray * 255), uint8_t(p.gray * 255), uint8_t(p.gray * 255), 255);

        case PSPaintKind::RGB:
            return BLRgba32(uint8_t(p.r * 255), uint8_t(p.g * 255), uint8_t(p.b * 255), uint8_t(p.a * 255));

        case PSPaintKind::CMYK: {
            double r = std::min(1.0, (1.0 - p.c) * (1.0 - p.k));
            double g = std::min(1.0, (1.0 - p.m) * (1.0 - p.k));
            double b = std::min(1.0, (1.0 - p.y) * (1.0 - p.k));

            return BLRgba32(uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255), 255);
        }

        default:
            return BLRgba32(0, 0, 0, 255); // fallback to black
        }
    }
}

namespace waavs
{

    inline bool convertPSPathToBLPath(const PathProgram& prog, BLPath& out)
    {
        BLPathSink sink{ out };
        return pathprogram_dispatch(prog, sink);
    }

    static bool convertBLPathToPathProgram(const BLPath& path, PathProgramBuilder& builder, const PSMatrix& transform = PSMatrix())
    {
        const BLPathView view = path.view();
        const uint8_t* commands = view.command_data;
        const BLPoint* vertices = view.vertex_data;

        auto point = [&](size_t index, float& x, float& y) {
            double tx, ty;
            transform.transformPoint(vertices[index].x, vertices[index].y, tx, ty);
            x = static_cast<float>(tx);
            y = static_cast<float>(ty);
            };

        size_t i = 0;
        while (i < view.size) {
            const uint8_t command = commands[i];

            switch (command) {
            case BL_PATH_CMD_MOVE: {
                float x, y;
                point(i, x, y);
                if (!builder.moveTo(x, y))
                    return false;
                ++i;
                break;
            }

            case BL_PATH_CMD_ON: {
                float x, y;
                point(i, x, y);
                if (!builder.lineTo(x, y))
                    return false;
                ++i;
                break;
            }

            case BL_PATH_CMD_QUAD: {
                if (i + 1 >= view.size || commands[i + 1] != BL_PATH_CMD_ON)
                    return false;

                float x1, y1, x, y;
                point(i, x1, y1);
                point(i + 1, x, y);

                if (!builder.quadTo(x1, y1, x, y))
                    return false;

                i += 2;
                break;
            }

            case BL_PATH_CMD_CUBIC: {
                if (i + 2 >= view.size || commands[i + 1] != BL_PATH_CMD_CUBIC || commands[i + 2] != BL_PATH_CMD_ON)
                    return false;

                float x1, y1, x2, y2, x, y;
                point(i, x1, y1);
                point(i + 1, x2, y2);
                point(i + 2, x, y);

                if (!builder.cubicTo(x1, y1, x2, y2, x, y))
                    return false;

                i += 3;
                break;
            }

            case BL_PATH_CMD_CLOSE:
                if (!builder.close())
                    return false;
                ++i;
                break;

            case BL_PATH_CMD_CONIC:
            case BL_PATH_CMD_WEIGHT:
            default:
                return false;
            }
        }

        return true;
    }

    inline bool convertBLPathToPathProgram(const BLPath& path, PathProgram& result, const PSMatrix& transform = PSMatrix())
    {
        PathProgramBuilder builder;

        if (!convertBLPathToPathProgram(path, builder, transform))
            return false;

        result = std::move(builder.prog);

        return true;
    }
}