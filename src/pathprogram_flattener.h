// pathprogram_flattener.h
#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility>

#include "pathprogram.h"
#include "pathprogram_builder.h"

namespace waavs
{
    struct PathFlattenOptions
    {
        double flatness{ 0.25 };
        int maxDepth{ 16 };
    };

    template <class Sink>
    struct PathFlattener
    {
        Sink& out;
        PathFlattenOptions opt{};

        bool fHasCurrentPoint{ false };
        bool fSubpathOpen{ false };
        double fCurX{ 0.0 };
        double fCurY{ 0.0 };
        double fStartX{ 0.0 };
        double fStartY{ 0.0 };

        explicit PathFlattener(Sink& sink) noexcept
            : out(sink)
        {}

        void resetState() noexcept
        {
            fHasCurrentPoint = false;
            fSubpathOpen = false;
            fCurX = 0.0;
            fCurY = 0.0;
            fStartX = 0.0;
            fStartY = 0.0;
        }

        bool onMoveTo(float x, float y) noexcept
        {
            fCurX = x;
            fCurY = y;
            fStartX = x;
            fStartY = y;
            fHasCurrentPoint = true;
            fSubpathOpen = true;
            return out.onMoveTo(x, y);
        }

        bool onLineTo(float x, float y) noexcept
        {
            if (!fHasCurrentPoint)
                return fail_();

            fCurX = x;
            fCurY = y;
            return out.onLineTo(x, y);
        }

        bool onQuadTo(float x1, float y1, float x, float y) noexcept
        {
            if (!fHasCurrentPoint)
                return fail_();

            return flattenQuadRecursive_(fCurX, fCurY, x1, y1, x, y, 0);
        }

        bool onCubicTo(float x1, float y1, float x2, float y2, float x, float y) noexcept
        {
            if (!fHasCurrentPoint)
                return fail_();

            return flattenCubicRecursive_(fCurX, fCurY, x1, y1, x2, y2, x, y, 0);
        }

        bool onArcTo(float rx, float ry, float xAxisRotation, float largeArcFlag, float sweepFlag, float x, float y) noexcept
        {
            if (!fHasCurrentPoint)
                return fail_();

            return flattenArc_(rx, ry, xAxisRotation, largeArcFlag, sweepFlag, x, y);
        }

        bool onClose() noexcept
        {
            if (!fHasCurrentPoint || !fSubpathOpen)
                return fail_();

            fCurX = fStartX;
            fCurY = fStartY;
            fSubpathOpen = false;
            return out.onClose();
        }

        bool onEnd() noexcept
        {
            return out.onEnd();
        }

    private:
        static constexpr double kPi = 3.14159265358979323846264338327950288;
        static constexpr double kPi2 = kPi * 2.0;
        static constexpr double kDegToRad = kPi / 180.0;
        static constexpr double kEpsilon = 1e-12;

        bool fail_() noexcept
        {
            assert(false && "PathFlattener: invalid path state");
            return false;
        }

        bool emitLineTo_(double x, double y) noexcept
        {
            fCurX = x;
            fCurY = y;
            return out.onLineTo(static_cast<float>(x), static_cast<float>(y));
        }

        static double distanceToLine_(double px, double py, double ax, double ay, double bx, double by) noexcept
        {
            const double dx = bx - ax;
            const double dy = by - ay;
            const double len2 = dx * dx + dy * dy;

            if (len2 <= kEpsilon * kEpsilon)
            {
                const double ex = px - ax;
                const double ey = py - ay;
                return std::sqrt(ex * ex + ey * ey);
            }

            const double cross = dx * (py - ay) - dy * (px - ax);
            return std::abs(cross) / std::sqrt(len2);
        }

        bool flattenQuadRecursive_(
            double x0, double y0,
            double x1, double y1,
            double x2, double y2,
            int depth) noexcept
        {
            if (depth >= opt.maxDepth)
                return emitLineTo_(x2, y2);

            const double d = distanceToLine_(x1, y1, x0, y0, x2, y2);
            if (d <= opt.flatness)
                return emitLineTo_(x2, y2);

            const double x01 = (x0 + x1) * 0.5;
            const double y01 = (y0 + y1) * 0.5;
            const double x12 = (x1 + x2) * 0.5;
            const double y12 = (y1 + y2) * 0.5;
            const double x012 = (x01 + x12) * 0.5;
            const double y012 = (y01 + y12) * 0.5;

            if (!flattenQuadRecursive_(x0, y0, x01, y01, x012, y012, depth + 1))
                return false;

            return flattenQuadRecursive_(x012, y012, x12, y12, x2, y2, depth + 1);
        }

        bool flattenCubicRecursive_(
            double x0, double y0,
            double x1, double y1,
            double x2, double y2,
            double x3, double y3,
            int depth) noexcept
        {
            if (depth >= opt.maxDepth)
                return emitLineTo_(x3, y3);

            const double d1 = distanceToLine_(x1, y1, x0, y0, x3, y3);
            const double d2 = distanceToLine_(x2, y2, x0, y0, x3, y3);

            if (std::max(d1, d2) <= opt.flatness)
                return emitLineTo_(x3, y3);

            const double x01 = (x0 + x1) * 0.5;
            const double y01 = (y0 + y1) * 0.5;
            const double x12 = (x1 + x2) * 0.5;
            const double y12 = (y1 + y2) * 0.5;
            const double x23 = (x2 + x3) * 0.5;
            const double y23 = (y2 + y3) * 0.5;

            const double x012 = (x01 + x12) * 0.5;
            const double y012 = (y01 + y12) * 0.5;
            const double x123 = (x12 + x23) * 0.5;
            const double y123 = (y12 + y23) * 0.5;

            const double x0123 = (x012 + x123) * 0.5;
            const double y0123 = (y012 + y123) * 0.5;

            if (!flattenCubicRecursive_(x0, y0, x01, y01, x012, y012, x0123, y0123, depth + 1))
                return false;

            return flattenCubicRecursive_(x0123, y0123, x123, y123, x23, y23, x3, y3, depth + 1);
        }

        bool flattenArc_(
            double rx, double ry, double xAxisRotation,
            double largeArcFlag, double sweepFlag,
            double x, double y) noexcept
        {
            const double x0 = fCurX;
            const double y0 = fCurY;
            const double x1 = x;
            const double y1 = y;

            if (std::abs(rx) <= kEpsilon || std::abs(ry) <= kEpsilon)
                return emitLineTo_(x1, y1);

            rx = std::abs(rx);
            ry = std::abs(ry);

            const double phi = xAxisRotation * kDegToRad;
            const double cosPhi = std::cos(phi);
            const double sinPhi = std::sin(phi);

            const double dx2 = (x0 - x1) * 0.5;
            const double dy2 = (y0 - y1) * 0.5;

            const double x1p = cosPhi * dx2 + sinPhi * dy2;
            const double y1p = -sinPhi * dx2 + cosPhi * dy2;

            if (std::abs(x1p) <= kEpsilon && std::abs(y1p) <= kEpsilon)
                return emitLineTo_(x1, y1);

            double lambda = (x1p * x1p) / (rx * rx) + (y1p * y1p) / (ry * ry);

            if (lambda > 1.0)
            {
                const double scale = std::sqrt(lambda);
                rx *= scale;
                ry *= scale;
            }

            const double rx2 = rx * rx;
            const double ry2 = ry * ry;
            const double x1p2 = x1p * x1p;
            const double y1p2 = y1p * y1p;

            const bool largeArc = largeArcFlag != 0.0;
            const bool sweep = sweepFlag != 0.0;

            double num = rx2 * ry2 - rx2 * y1p2 - ry2 * x1p2;
            const double den = rx2 * y1p2 + ry2 * x1p2;

            if (den <= kEpsilon)
                return emitLineTo_(x1, y1);

            if (num < 0.0)
                num = 0.0;

            double coef = std::sqrt(num / den);

            if (largeArc == sweep)
                coef = -coef;

            const double cxp = coef * ((rx * y1p) / ry);
            const double cyp = coef * (-(ry * x1p) / rx);

            const double cx = cosPhi * cxp - sinPhi * cyp + (x0 + x1) * 0.5;
            const double cy = sinPhi * cxp + cosPhi * cyp + (y0 + y1) * 0.5;

            auto angleBetween = [](double ux, double uy, double vx, double vy) noexcept
                {
                    return std::atan2(ux * vy - uy * vx, ux * vx + uy * vy);
                };

            const double ux = (x1p - cxp) / rx;
            const double uy = (y1p - cyp) / ry;
            const double vx = (-x1p - cxp) / rx;
            const double vy = (-y1p - cyp) / ry;

            const double theta1 = std::atan2(uy, ux);
            double delta = angleBetween(ux, uy, vx, vy);

            if (!sweep && delta > 0.0)
                delta -= kPi2;
            else if (sweep && delta < 0.0)
                delta += kPi2;

            const double rmax = std::max(rx, ry);

            double maxStep;

            if (rmax <= opt.flatness || rmax <= kEpsilon)
            {
                maxStep = std::abs(delta);
            }
            else
            {
                double c = 1.0 - opt.flatness / rmax;
                c = std::clamp(c, -1.0, 1.0);
                maxStep = 2.0 * std::acos(c);
            }

            if (maxStep <= 1e-6 || !std::isfinite(maxStep))
                maxStep = 0.25;

            int steps = static_cast<int>(std::ceil(std::abs(delta) / maxStep));
            if (steps < 1)
                steps = 1;

            for (int i = 1; i <= steps; ++i)
            {
                const double t = static_cast<double>(i) / static_cast<double>(steps);
                const double a = theta1 + delta * t;

                const double ca = std::cos(a);
                const double sa = std::sin(a);

                const double px = cx + cosPhi * rx * ca - sinPhi * ry * sa;
                const double py = cy + sinPhi * rx * ca + cosPhi * ry * sa;

                if (!emitLineTo_(px, py))
                    return false;
            }

            return true;
        }
    };


    inline bool flattenPathProgram(const PathProgram& src, PathProgram& dst, double flatness = 0.25, int maxDepth = 16) noexcept
    {
        PathProgramBuilder builder;
        builder.reserve(src.ops.size() * 2u, src.args.size() * 4u);

        PathFlattener<PathProgramBuilder> flattener(builder);
        flattener.opt.flatness = flatness;
        flattener.opt.maxDepth = maxDepth;

        if (!pathprogram_dispatch(src, flattener))
            return false;

        dst = std::move(builder.prog);
        return true;
    }

} // namespace waavs