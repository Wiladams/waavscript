// pathcommand_normalizer.h

#pragma once

#include <cassert>

#include "pathprogram_builder.h"
#include "svg_path_command.h"

namespace waavs
{
    // ------------------------------------------------------------
    // PathCommandNormalizer
    //
    // Consumes raw SVG path commands and emits normalized PathProgram
    // operations through a PathProgramBuilder.
    //
    // Normalization includes:
    //
    //   - relative -> absolute coordinates
    //   - repeated M/m -> lineTo
    //   - H/V -> lineTo
    //   - S/s -> cubicTo
    //   - T/t -> quadTo
    //
    // Arcs remain in SVG endpoint form, but relative endpoints are
    // converted to absolute coordinates.
    // ------------------------------------------------------------

    struct PathCommandNormalizer
    {
        PathProgramBuilder& b;

        // Current point and current subpath start.
        float cx{ 0.0f }, cy{ 0.0f };
        float sx{ 0.0f }, sy{ 0.0f };
        bool hasCP{ false };
        bool subpathOpen{ false };

        // Smooth cubic reflection state.
        bool hasLastCubicCtrl{ false };
        float lastCubicCtrlX{ 0.0f }, lastCubicCtrlY{ 0.0f };

        // Smooth quadratic reflection state.
        bool hasLastQuadCtrl{ false };
        float lastQuadCtrlX{ 0.0f }, lastQuadCtrlY{ 0.0f };

        // Previous raw SVG command.
        SVGPathCommand prevCmd{ SVGPathCommand::M };

        // Records whether the immediately preceding command closed a subpath.
        bool justClosed{ false };


        explicit PathCommandNormalizer(PathProgramBuilder& builder) noexcept
            : b(builder)
        {}


        void resetState() noexcept
        {
            cx = cy = 0.0f;
            sx = sy = 0.0f;
            hasCP = false;
            subpathOpen = false;

            hasLastCubicCtrl = false;
            lastCubicCtrlX = lastCubicCtrlY = 0.0f;

            hasLastQuadCtrl = false;
            lastQuadCtrlX = lastQuadCtrlY = 0.0f;

            prevCmd = SVGPathCommand::M;
            justClosed = false;
        }


        bool consume(SVGPathCommand cmd, const float* a, bool repeated) noexcept
        {
            // SVG permits an initial relative moveto. With no current point,
            // its relative coordinate base is (0,0).
            auto ensureInitialRelMoveBase_ = [&]() noexcept
                {
                    if (!hasCP)
                    {
                        cx = 0.0f;
                        cy = 0.0f;
                        hasCP = true;
                    }
                };

            auto clearSmoothState_ = [&]() noexcept
                {
                    hasLastCubicCtrl = false;
                    hasLastQuadCtrl = false;
                };

            auto setCurrent_ = [&](float x, float y) noexcept
                {
                    cx = x;
                    cy = y;
                    hasCP = true;
                };


            // --------------------------------------------------------
            // M - absolute moveto
            //
            // Additional coordinate pairs after the first are lineto.
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::M)
            {
                const float x = a[0], y = a[1];

                if (!repeated)
                {
                    if (!b.moveTo(x, y))
                        return false;

                    sx = x;
                    sy = y;
                    setCurrent_(x, y);
                    subpathOpen = true;
                }
                else
                {
                    if (!b.lineTo(x, y))
                        return false;

                    setCurrent_(x, y);
                    subpathOpen = true;
                }

                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // m - relative moveto
            //
            // Additional coordinate pairs after the first are relative
            // lineto operations.
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::m)
            {
                ensureInitialRelMoveBase_();

                const float x = cx + a[0];
                const float y = cy + a[1];

                if (!repeated)
                {
                    if (!b.moveTo(x, y))
                        return false;

                    sx = x;
                    sy = y;
                    setCurrent_(x, y);
                    subpathOpen = true;
                }
                else
                {
                    if (!b.lineTo(x, y))
                        return false;

                    setCurrent_(x, y);
                    subpathOpen = true;
                }

                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // Everything except the initial M/m requires an existing
            // current point.
            if (!hasCP)
            {
                assert(false && "PathCommandNormalizer: command before initial moveto");
                return false;
            }


            // --------------------------------------------------------
            // L/l - lineto
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::L)
            {
                const float x = a[0], y = a[1];

                if (!b.lineTo(x, y))
                    return false;

                setCurrent_(x, y);
                subpathOpen = true;
                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }

            if (cmd == SVGPathCommand::l)
            {
                const float x = cx + a[0];
                const float y = cy + a[1];

                if (!b.lineTo(x, y))
                    return false;

                setCurrent_(x, y);
                subpathOpen = true;
                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // H/h - horizontal lineto
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::H)
            {
                const float x = a[0];

                if (!b.lineTo(x, cy))
                    return false;

                setCurrent_(x, cy);
                subpathOpen = true;
                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }

            if (cmd == SVGPathCommand::h)
            {
                const float x = cx + a[0];

                if (!b.lineTo(x, cy))
                    return false;

                setCurrent_(x, cy);
                subpathOpen = true;
                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // V/v - vertical lineto
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::V)
            {
                const float y = a[0];

                if (!b.lineTo(cx, y))
                    return false;

                setCurrent_(cx, y);
                subpathOpen = true;
                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }

            if (cmd == SVGPathCommand::v)
            {
                const float y = cy + a[0];

                if (!b.lineTo(cx, y))
                    return false;

                setCurrent_(cx, y);
                subpathOpen = true;
                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // C/c - cubic Bezier
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::C)
            {
                const float x1 = a[0], y1 = a[1];
                const float x2 = a[2], y2 = a[3];
                const float x = a[4], y = a[5];

                if (!b.cubicTo(x1, y1, x2, y2, x, y))
                    return false;

                setCurrent_(x, y);

                hasLastCubicCtrl = true;
                lastCubicCtrlX = x2;
                lastCubicCtrlY = y2;
                hasLastQuadCtrl = false;

                prevCmd = cmd;
                justClosed = false;
                return true;
            }

            if (cmd == SVGPathCommand::c)
            {
                const float x1 = cx + a[0], y1 = cy + a[1];
                const float x2 = cx + a[2], y2 = cy + a[3];
                const float x = cx + a[4], y = cy + a[5];

                if (!b.cubicTo(x1, y1, x2, y2, x, y))
                    return false;

                setCurrent_(x, y);

                hasLastCubicCtrl = true;
                lastCubicCtrlX = x2;
                lastCubicCtrlY = y2;
                hasLastQuadCtrl = false;

                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // S/s - smooth cubic Bezier
            //
            // Reflect the previous cubic control point when the previous
            // command was C/c/S/s. Otherwise the first control point is
            // the current point.
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::S || cmd == SVGPathCommand::s)
            {
                float x2, y2, x, y;

                if (cmd == SVGPathCommand::S)
                {
                    x2 = a[0];
                    y2 = a[1];
                    x = a[2];
                    y = a[3];
                }
                else
                {
                    x2 = cx + a[0];
                    y2 = cy + a[1];
                    x = cx + a[2];
                    y = cy + a[3];
                }

                float x1 = cx;
                float y1 = cy;

                if (hasLastCubicCtrl &&
                    (prevCmd == SVGPathCommand::C || prevCmd == SVGPathCommand::c ||
                        prevCmd == SVGPathCommand::S || prevCmd == SVGPathCommand::s))
                {
                    x1 = 2.0f * cx - lastCubicCtrlX;
                    y1 = 2.0f * cy - lastCubicCtrlY;
                }

                if (!b.cubicTo(x1, y1, x2, y2, x, y))
                    return false;

                setCurrent_(x, y);

                hasLastCubicCtrl = true;
                lastCubicCtrlX = x2;
                lastCubicCtrlY = y2;
                hasLastQuadCtrl = false;

                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // Q/q - quadratic Bezier
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::Q)
            {
                const float x1 = a[0], y1 = a[1];
                const float x = a[2], y = a[3];

                if (!b.quadTo(x1, y1, x, y))
                    return false;

                setCurrent_(x, y);

                hasLastQuadCtrl = true;
                lastQuadCtrlX = x1;
                lastQuadCtrlY = y1;
                hasLastCubicCtrl = false;

                prevCmd = cmd;
                justClosed = false;
                return true;
            }

            if (cmd == SVGPathCommand::q)
            {
                const float x1 = cx + a[0], y1 = cy + a[1];
                const float x = cx + a[2], y = cy + a[3];

                if (!b.quadTo(x1, y1, x, y))
                    return false;

                setCurrent_(x, y);

                hasLastQuadCtrl = true;
                lastQuadCtrlX = x1;
                lastQuadCtrlY = y1;
                hasLastCubicCtrl = false;

                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // T/t - smooth quadratic Bezier
            //
            // Reflect the previous quadratic control point when the
            // previous command was Q/q/T/t. Otherwise the control point
            // is the current point.
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::T || cmd == SVGPathCommand::t)
            {
                float x, y;

                if (cmd == SVGPathCommand::T)
                {
                    x = a[0];
                    y = a[1];
                }
                else
                {
                    x = cx + a[0];
                    y = cy + a[1];
                }

                float x1 = cx;
                float y1 = cy;

                if (hasLastQuadCtrl &&
                    (prevCmd == SVGPathCommand::Q || prevCmd == SVGPathCommand::q ||
                        prevCmd == SVGPathCommand::T || prevCmd == SVGPathCommand::t))
                {
                    x1 = 2.0f * cx - lastQuadCtrlX;
                    y1 = 2.0f * cy - lastQuadCtrlY;
                }

                if (!b.quadTo(x1, y1, x, y))
                    return false;

                setCurrent_(x, y);

                hasLastQuadCtrl = true;
                lastQuadCtrlX = x1;
                lastQuadCtrlY = y1;
                hasLastCubicCtrl = false;

                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // A/a - elliptical arc
            //
            // Preserve SVG endpoint arc representation. Relative endpoint
            // coordinates are converted to absolute coordinates.
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::A || cmd == SVGPathCommand::a)
            {
                const float rx = a[0];
                const float ry = a[1];
                const float xrot = a[2];
                const float largeArc = a[3];
                const float sweep = a[4];

                float x, y;

                if (cmd == SVGPathCommand::A)
                {
                    x = a[5];
                    y = a[6];
                }
                else
                {
                    x = cx + a[5];
                    y = cy + a[6];
                }

                if (!b.arcTo(rx, ry, xrot, largeArc, sweep, x, y))
                    return false;

                setCurrent_(x, y);
                subpathOpen = true;

                clearSmoothState_();
                prevCmd = cmd;
                justClosed = false;
                return true;
            }


            // --------------------------------------------------------
            // Z/z - close current subpath
            // --------------------------------------------------------

            if (cmd == SVGPathCommand::Z || cmd == SVGPathCommand::z)
            {
                if (!b.close())
                    return false;

                // SVG current point becomes the subpath start.
                setCurrent_(sx, sy);
                subpathOpen = false;

                clearSmoothState_();
                prevCmd = cmd;
                justClosed = true;
                return true;
            }


            assert(false && "PathCommandNormalizer: unknown SVG path command");
            return false;
        }
    };
}