// ps_type_path.h 
#pragma once

#include <cmath>
#include <cstdint>

#include "pathprogram_builder.h"
#include "ps_type_matrix.h"

namespace waavs {

    struct PSPath
    {
    private:
        PathProgramBuilder fBuilder;

        static bool transformPoint_(const PSMatrix& ctm, double x, double y, float& tx, float& ty) noexcept
        {
            double dx;
            double dy;
            ctm.transformPoint(x, y, dx, dy);

            tx = static_cast<float>(dx);
            ty = static_cast<float>(dy);
            return true;
        }

        bool emitArc_(const PSMatrix& ctm, double cx, double cy, double radius, double startDeg, double endDeg, bool clockwise)
        {
            if (radius < 0.0)
                return false;

            constexpr double pi = 3.14159265358979323846;
            constexpr double degToRad = pi / 180.0;
            constexpr double quarterArc = pi * 0.5;

            double start = startDeg * degToRad;
            double end = endDeg * degToRad;
            double sweep = end - start;

            if (clockwise)
            {
                while (sweep >= 0.0)
                    sweep -= 2.0 * pi;
            }
            else
            {
                while (sweep <= 0.0)
                    sweep += 2.0 * pi;
            }

            int segmentCount = static_cast<int>(std::ceil(std::abs(sweep) / quarterArc));
            if (segmentCount < 1)
                segmentCount = 1;

            const double delta = sweep / static_cast<double>(segmentCount);

            double startX = cx + radius * std::cos(start);
            double startY = cy + radius * std::sin(start);

            if (!hasCurrentPoint())
            {
                if (!moveto(ctm, startX, startY))
                    return false;
            }
            else
            {
                double curX;
                double curY;

                if (!getCurrentPoint(ctm, curX, curY))
                    return false;

                constexpr double epsilon = 1e-10;

                if (std::abs(curX - startX) > epsilon || std::abs(curY - startY) > epsilon)
                {
                    if (!lineto(ctm, startX, startY))
                        return false;
                }
            }

            for (int i = 0; i < segmentCount; ++i)
            {
                const double t0 = start + static_cast<double>(i) * delta;
                const double t1 = t0 + delta;

                const double cos0 = std::cos(t0);
                const double sin0 = std::sin(t0);
                const double cos1 = std::cos(t1);
                const double sin1 = std::sin(t1);

                const double alpha = std::tan((t1 - t0) * 0.25) * (4.0 / 3.0);

                const double x0 = cx + radius * cos0;
                const double y0 = cy + radius * sin0;

                const double x1 = x0 - radius * alpha * sin0;
                const double y1 = y0 + radius * alpha * cos0;

                const double x3 = cx + radius * cos1;
                const double y3 = cy + radius * sin1;

                const double x2 = x3 + radius * alpha * sin1;
                const double y2 = y3 - radius * alpha * cos1;

                if (!curveto(ctm, x1, y1, x2, y2, x3, y3))
                    return false;
            }

            return true;
        }

    public:
        PSPath() = default;


        // ------------------------------------------------------------
        // Lifecycle / state
        // ------------------------------------------------------------

        bool reset() noexcept
        {
            fBuilder.reset();
            return true;
        }

        bool empty() const noexcept
        {
            return fBuilder.prog.ops.empty();
        }

        bool rsetCurrentPoint(const PSMatrix& ctm, double dx, double dy)
        {
            double x;
            double y;

            if (!getCurrentPoint(ctm, x, y))
                return false;

            return setCurrentPoint(ctm, x + dx, y + dy);
        }

        bool setCurrentPoint(const PSMatrix& ctm, double x, double y)
        {
            float tx;
            float ty;

            if (!transformPoint_(ctm, x, y, tx, ty))
                return false;

            return fBuilder.setCurrentPoint(tx, ty);
        }

        bool hasCurrentPoint() const noexcept
        {
            return fBuilder.hasCurrentPoint();
        }

        const PathProgram& program() const noexcept
        {
            return fBuilder.prog;
        }

        //PathProgram& program() noexcept
        //{
        //    return fBuilder.prog;
        //}

        bool replaceProgram(const PathProgram& prog)
        {
            PathProgramBuilder builder;
            builder.reserve(prog.ops.size(), prog.args.size());

            if (!pathprogram_dispatch(prog, builder))
                return false;

            fBuilder = std::move(builder);
            return true;
        }


        // ------------------------------------------------------------
        // Current point
        //
        // Internally the builder current point is in canonical path space.
        // PostScript currentpoint must be returned in CURRENT user space.
        // ------------------------------------------------------------

        bool getCurrentPointCanonical(double& x, double& y) const noexcept
        {
            if (!fBuilder.hasCurrentPoint())
                return false;

            x = static_cast<double>(fBuilder.curX());
            y = static_cast<double>(fBuilder.curY());
            return true;
        }

        bool getCurrentPoint(const PSMatrix& ctm, double& x, double& y) const
        {
            if (!fBuilder.hasCurrentPoint())
                return false;

            PSMatrix inverse;
            if (!ctm.inverse(inverse))
                return false;

            inverse.transformPoint(
                static_cast<double>(fBuilder.curX()),
                static_cast<double>(fBuilder.curY()),
                x, y);

            return true;
        }


        // ------------------------------------------------------------
        // Basic path construction
        //
        // Input coordinates are PostScript user-space coordinates.
        // They are transformed immediately and stored canonically.
        // ------------------------------------------------------------

        bool moveto(const PSMatrix& ctm, double x, double y)
        {
            float tx;
            float ty;

            if (!transformPoint_(ctm, x, y, tx, ty))
                return false;

            return fBuilder.moveTo(tx, ty);
        }

        bool lineto(const PSMatrix& ctm, double x, double y)
        {
            if (!fBuilder.hasCurrentPoint())
                return false;

            float tx;
            float ty;

            if (!transformPoint_(ctm, x, y, tx, ty))
                return false;

            return fBuilder.lineTo(tx, ty);
        }

        bool curveto(const PSMatrix& ctm,
            double x1, double y1,
            double x2, double y2,
            double x3, double y3)
        {
            if (!fBuilder.hasCurrentPoint())
                return false;

            float tx1;
            float ty1;
            float tx2;
            float ty2;
            float tx3;
            float ty3;

            if (!transformPoint_(ctm, x1, y1, tx1, ty1) ||
                !transformPoint_(ctm, x2, y2, tx2, ty2) ||
                !transformPoint_(ctm, x3, y3, tx3, ty3))
                return false;

            return fBuilder.cubicTo(tx1, ty1, tx2, ty2, tx3, ty3);
        }

        bool quadto(const PSMatrix& ctm, double x1, double y1, double x2, double y2)
        {
            if (!fBuilder.hasCurrentPoint())
                return false;

            float tx1;
            float ty1;
            float tx2;
            float ty2;

            if (!transformPoint_(ctm, x1, y1, tx1, ty1) ||
                !transformPoint_(ctm, x2, y2, tx2, ty2))
                return false;

            return fBuilder.quadTo(tx1, ty1, tx2, ty2);
        }

        bool close() noexcept
        {
            return fBuilder.close();
        }


        // ------------------------------------------------------------
        // Relative PostScript operations
        //
        // These are useful here because PSPath owns the coordinate-space
        // transition. Operators do not need to know how the path is stored.
        // ------------------------------------------------------------

        bool rmoveto(const PSMatrix& ctm, double dx, double dy)
        {
            double x;
            double y;

            if (!getCurrentPoint(ctm, x, y))
                return false;

            return moveto(ctm, x + dx, y + dy);
        }

        bool rlineto(const PSMatrix& ctm, double dx, double dy)
        {
            double x;
            double y;

            if (!getCurrentPoint(ctm, x, y))
                return false;

            return lineto(ctm, x + dx, y + dy);
        }

        bool rcurveto(const PSMatrix& ctm,
            double dx1, double dy1,
            double dx2, double dy2,
            double dx3, double dy3)
        {
            double x;
            double y;

            if (!getCurrentPoint(ctm, x, y))
                return false;

            return curveto(ctm,
                x + dx1, y + dy1,
                x + dx2, y + dy2,
                x + dx3, y + dy3);
        }


        // ------------------------------------------------------------
        // Circular PostScript arcs
        //
        // Arcs are normalized immediately to cubic Beziers. This avoids
        // retaining PostScript arc semantics or CTM state in PathProgram.
        // ------------------------------------------------------------

        bool arc(const PSMatrix& ctm, double cx, double cy, double radius, double startDeg, double endDeg)
        {
            return emitArc_(ctm, cx, cy, radius, startDeg, endDeg, false);
        }

        bool arcn(const PSMatrix& ctm, double cx, double cy, double radius, double startDeg, double endDeg)
        {
            return emitArc_(ctm, cx, cy, radius, startDeg, endDeg, true);
        }


        // ------------------------------------------------------------
        // arcto
        //
        // All geometry is computed in current user space, then normalized
        // into line/cubic operations through the same CTM-aware interface.
        // ------------------------------------------------------------

        bool arcto(const PSMatrix& ctm,
            double x1, double y1,
            double x2, double y2,
            double radius,
            double& xt1, double& yt1,
            double& xt2, double& yt2)
        {
            if (radius < 0.0)
                return false;

            double x0;
            double y0;

            if (!getCurrentPoint(ctm, x0, y0))
                return false;

            double dx1 = x0 - x1;
            double dy1 = y0 - y1;
            double dx2 = x2 - x1;
            double dy2 = y2 - y1;

            double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
            double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

            constexpr double epsilon = 1e-12;

            if (len1 <= epsilon || len2 <= epsilon)
                return false;

            double vx1 = dx1 / len1;
            double vy1 = dy1 / len1;
            double vx2 = dx2 / len2;
            double vy2 = dy2 / len2;

            double dot = vx1 * vx2 + vy1 * vy2;

            if (dot < -1.0)
                dot = -1.0;
            else if (dot > 1.0)
                dot = 1.0;

            double theta = std::acos(dot);

            if (theta <= epsilon || std::abs(3.14159265358979323846 - theta) <= epsilon)
                return false;

            double distance = radius / std::tan(theta * 0.5);

            xt1 = x1 + vx1 * distance;
            yt1 = y1 + vy1 * distance;
            xt2 = x1 + vx2 * distance;
            yt2 = y1 + vy2 * distance;

            if (!lineto(ctm, xt1, yt1))
                return false;

            if (radius <= epsilon)
                return lineto(ctm, xt2, yt2);

            // Unit vectors from corner toward tangent points.
            double ux1 = xt1 - x1;
            double uy1 = yt1 - y1;
            double ux2 = xt2 - x1;
            double uy2 = yt2 - y1;

            double ulen1 = std::sqrt(ux1 * ux1 + uy1 * uy1);
            double ulen2 = std::sqrt(ux2 * ux2 + uy2 * uy2);

            if (ulen1 <= epsilon || ulen2 <= epsilon)
                return false;

            ux1 /= ulen1;
            uy1 /= ulen1;
            ux2 /= ulen2;
            uy2 /= ulen2;

            double bx = ux1 + ux2;
            double by = uy1 + uy2;
            double blen = std::sqrt(bx * bx + by * by);

            if (blen <= epsilon)
                return false;

            bx /= blen;
            by /= blen;

            double centerDistance = radius / std::sin(theta * 0.5);

            double cx = x1 + bx * centerDistance;
            double cy = y1 + by * centerDistance;

            double startAngle = std::atan2(yt1 - cy, xt1 - cx);
            double endAngle = std::atan2(yt2 - cy, xt2 - cx);

            double cross =
                (xt1 - cx) * (yt2 - cy) -
                (yt1 - cy) * (xt2 - cx);

            constexpr double radToDeg = 180.0 / 3.14159265358979323846;

            double startDeg = startAngle * radToDeg;
            double endDeg = endAngle * radToDeg;

            bool clockwise = cross < 0.0;

            return emitArc_(ctm, cx, cy, radius, startDeg, endDeg, clockwise);
        }
    };

} // namespace waavs