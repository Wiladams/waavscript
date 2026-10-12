#pragma once

#include <algorithm>
#include <cmath>

#include "definitions.h"

namespace waavs
{
    static constexpr double BEZIER_EPSILON = 1e-12;


    // ------------------------------------------------------
    // Common helpers

    static INLINE bool bez_is_interior_t(double t) noexcept
    {
        return t > 0.0 && t < 1.0;
    }

    static INLINE void bez_bounds_expand(double& minX, double& minY, double& maxX, double& maxY, double x, double y) noexcept
    {
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    }


    // ------------------------------------------------------
    // Quadratic Bezier

    static INLINE double bez_quad_eval(double p0, double p1, double p2, double t) noexcept
    {
        const double mt = 1.0 - t;
        return mt * mt * p0 + 2.0 * mt * t * p1 + t * t * p2;
    }

    static INLINE void bez_quad_eval_point(double x0, double y0, double x1, double y1, double x2, double y2, double t, double& x, double& y) noexcept
    {
        x = bez_quad_eval(x0, x1, x2, t);
        y = bez_quad_eval(y0, y1, y2, t);
    }

    static INLINE bool bez_quad_solve_extrema(double p0, double p1, double p2, double& t) noexcept
    {
        const double d = p0 - 2.0 * p1 + p2;

        if (std::abs(d) < BEZIER_EPSILON)
            return false;

        t = (p0 - p1) / d;
        return bez_is_interior_t(t);
    }

    // The bounds must already contain the starting point (x0, y0).
    static INLINE void bez_quad_bounds(double x0, double y0, double x1, double y1, double x2, double y2,
        double& minX, double& minY, double& maxX, double& maxY) noexcept
    {
        bez_bounds_expand(minX, minY, maxX, maxY, x2, y2);

        double t;

        if (bez_quad_solve_extrema(x0, x1, x2, t)) {
            double x, y;
            bez_quad_eval_point(x0, y0, x1, y1, x2, y2, t, x, y);
            bez_bounds_expand(minX, minY, maxX, maxY, x, y);
        }

        if (bez_quad_solve_extrema(y0, y1, y2, t)) {
            double x, y;
            bez_quad_eval_point(x0, y0, x1, y1, x2, y2, t, x, y);
            bez_bounds_expand(minX, minY, maxX, maxY, x, y);
        }
    }


    // ------------------------------------------------------
    // Cubic Bezier

    static INLINE double bez_cubic_eval(double p0, double p1, double p2, double p3, double t) noexcept
    {
        const double mt = 1.0 - t;

        return mt * mt * mt * p0
            + 3.0 * mt * mt * t * p1
            + 3.0 * mt * t * t * p2
            + t * t * t * p3;
    }

    static INLINE void bez_cubic_eval_point(double x0, double y0, double x1, double y1, double x2, double y2,
        double x3, double y3, double t, double& x, double& y) noexcept
    {
        x = bez_cubic_eval(x0, x1, x2, x3, t);
        y = bez_cubic_eval(y0, y1, y2, y3, t);
    }

    static INLINE int bez_cubic_solve_extrema(double p0, double p1, double p2, double p3, double tOut[2]) noexcept
    {
        const double a = -p0 + 3.0 * p1 - 3.0 * p2 + p3;
        const double b = 2.0 * (p0 - 2.0 * p1 + p2);
        const double c = -p0 + p1;

        if (std::abs(a) < BEZIER_EPSILON) {
            if (std::abs(b) < BEZIER_EPSILON)
                return 0;

            const double t = -c / b;
            if (!bez_is_interior_t(t))
                return 0;

            tOut[0] = t;
            return 1;
        }

        const double disc = b * b - 4.0 * a * c;
        if (disc < 0.0)
            return 0;

        const double s = std::sqrt(disc);
        const double invDenom = 1.0 / (2.0 * a);

        const double t1 = (-b + s) * invDenom;
        const double t2 = (-b - s) * invDenom;

        int count = 0;

        if (bez_is_interior_t(t1))
            tOut[count++] = t1;

        if (bez_is_interior_t(t2) && std::abs(t2 - t1) >= BEZIER_EPSILON)
            tOut[count++] = t2;

        return count;
    }

    // The bounds must already contain the starting point (x0, y0).
    static INLINE void bez_cubic_bounds(double x0, double y0, double x1, double y1, double x2, double y2,
        double x3, double y3, double& minX, double& minY, double& maxX, double& maxY) noexcept
    {
        bez_bounds_expand(minX, minY, maxX, maxY, x3, y3);

        double tValues[2];

        const int xCount = bez_cubic_solve_extrema(x0, x1, x2, x3, tValues);
        for (int i = 0; i < xCount; ++i) {
            double x, y;
            bez_cubic_eval_point(x0, y0, x1, y1, x2, y2, x3, y3, tValues[i], x, y);
            bez_bounds_expand(minX, minY, maxX, maxY, x, y);
        }

        const int yCount = bez_cubic_solve_extrema(y0, y1, y2, y3, tValues);
        for (int i = 0; i < yCount; ++i) {
            double x, y;
            bez_cubic_eval_point(x0, y0, x1, y1, x2, y2, x3, y3, tValues[i], x, y);
            bez_bounds_expand(minX, minY, maxX, maxY, x, y);
        }
    }
}