// pathprogram_metrics.h
#pragma once

#include "bezier_util.h"
#include "ps_type_matrix.h"

namespace waavs
{
    struct PathBoundsSink
    {
        PSMatrix transform;

        double minX{};
        double minY{};
        double maxX{};
        double maxY{};

        double curX{};
        double curY{};
        double startX{};
        double startY{};

        bool hasBounds{};
        bool hasCurrentPoint{};

        explicit PathBoundsSink(const PSMatrix& m) noexcept
            : transform(m)
        {}



        
        bool onMoveTo(float x, float y) noexcept
        {
            double tx, ty;
            transform.transformPoint(x, y, tx, ty);

            curX = tx;
            curY = ty;
            startX = tx;
            startY = ty;

            hasCurrentPoint = true;
            include(tx, ty);
            return true;
        }
        

        bool onLineTo(float x, float y) noexcept
        {
            if (!hasCurrentPoint)
                return false;

            double tx, ty;
            transform.transformPoint(x, y, tx, ty);

            include(tx, ty);

            curX = tx;
            curY = ty;
            return true;
        }

        bool onQuadTo(float x1, float y1, float x, float y) noexcept
        {
            if (!hasCurrentPoint)
                return false;

            double tx1, ty1;
            double tx, ty;

            transform.transformPoint(x1, y1, tx1, ty1);
            transform.transformPoint(x, y, tx, ty);

            bez_quad_bounds(curX, curY, tx1, ty1, tx, ty, minX, minY, maxX, maxY);

            curX = tx;
            curY = ty;
            return true;
        }

        bool onCubicTo(float x1, float y1, float x2, float y2, float x, float y) noexcept
        {
            if (!hasCurrentPoint)
                return false;

            double tx1, ty1;
            double tx2, ty2;
            double tx, ty;

            transform.transformPoint(x1, y1, tx1, ty1);
            transform.transformPoint(x2, y2, tx2, ty2);
            transform.transformPoint(x, y, tx, ty);

            bez_cubic_bounds(curX, curY, tx1, ty1, tx2, ty2, tx, ty, minX, minY, maxX, maxY);

            curX = tx;
            curY = ty;
            return true;
        }

        bool onArcTo(float rx, float ry, float rotation, float largeArc, float sweep, float x, float y) noexcept
        {
            // Endpoint-form arcs require their own extrema calculation.
            return false;
        }

        bool onClose() noexcept
        {
            if (!hasCurrentPoint)
                return false;

            include(startX, startY);

            curX = startX;
            curY = startY;
            return true;
        }

        void onEnd() noexcept
        {}

        bool getBounds(double& x, double& y, double& width, double& height) const noexcept
        {
            if (!hasBounds)
                return false;

            x = minX;
            y = minY;
            width = maxX - minX;
            height = maxY - minY;
            return true;
        }

    private:
        void include(double x, double y) noexcept
        {
            if (!hasBounds) {
                minX = maxX = x;
                minY = maxY = y;
                hasBounds = true;
                return;
            }

            bez_bounds_expand(minX, minY, maxX, maxY, x, y);
        }
    };
}