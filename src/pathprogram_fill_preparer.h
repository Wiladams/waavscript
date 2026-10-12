
#pragma once

#include <cstddef>
#include <new>
#include <utility>
#include <vector>

#include "pathprogram.h"
#include "pathprogram_builder.h"

namespace waavs
{
    template <class Sink>
    struct PathFillPreparer
    {
        struct Point
        {
            float x;
            float y;
        };

        Sink& out;

        std::vector<Point> fPoints;
        bool fHasCurrentPoint{ false };
        bool fSubpathOpen{ false };
        float fCurX{ 0.0f };
        float fCurY{ 0.0f };
        float fStartX{ 0.0f };
        float fStartY{ 0.0f };

        explicit PathFillPreparer(Sink& sink) noexcept
            : out(sink)
        {}

        void resetState() noexcept
        {
            fPoints.clear();
            fHasCurrentPoint = false;
            fSubpathOpen = false;
            fCurX = fCurY = 0.0f;
            fStartX = fStartY = 0.0f;
        }

        bool onMoveTo(float x, float y) noexcept
        {
            if (!finishContour_())
                return false;

            fHasCurrentPoint = true;
            fSubpathOpen = true;
            fCurX = fStartX = x;
            fCurY = fStartY = y;

            return addPoint_(x, y);
        }

        bool onLineTo(float x, float y) noexcept
        {
            if (!fHasCurrentPoint)
                return false;

            if (!fSubpathOpen) {
                if (!beginAtCurrent_())
                    return false;
            }

            fCurX = x;
            fCurY = y;
            return addPoint_(x, y);
        }

        // This sink expects a flattened path.
        // Curved segments must be handled by PathFlattener upstream.

        bool onQuadTo(float x1, float y1, float x, float y) noexcept
        {
            return false;
        }

        bool onCubicTo(float x1, float y1, float x2, float y2, float x, float y) noexcept
        {
            return false;
        }

        bool onArcTo(float rx, float ry, float xAxisRotation, float largeArcFlag, float sweepFlag, float x, float y) noexcept
        {
            return false;
        }

        bool onClose() noexcept
        {
            if (!fHasCurrentPoint)
                return false;

            if (fSubpathOpen) {
                if (!finishContour_())
                    return false;
            }

            fCurX = fStartX;
            fCurY = fStartY;
            fSubpathOpen = false;
            return true;
        }

        bool onEnd() noexcept
        {
            if (!finishContour_())
                return false;

            return out.onEnd();
        }

    private:
        static bool samePoint_(const Point& a, const Point& b) noexcept
        {
            return a.x == b.x && a.y == b.y;
        }

        bool addPoint_(float x, float y) noexcept
        {
            Point pt{ x, y };

            if (!fPoints.empty() && samePoint_(fPoints.back(), pt))
                return true;

            try {
                fPoints.push_back(pt);
            }
            catch (const std::bad_alloc&) {
                return false;
            }

            return true;
        }

        bool beginAtCurrent_() noexcept
        {
            fStartX = fCurX;
            fStartY = fCurY;
            fSubpathOpen = true;
            return addPoint_(fCurX, fCurY);
        }

        bool finishContour_() noexcept
        {
            if (fPoints.empty()) {
                fSubpathOpen = false;
                return true;
            }

            // Remove an explicit final vertex equal to the start.
            if (fPoints.size() > 1 && samePoint_(fPoints.front(), fPoints.back()))
                fPoints.pop_back();

            // Fewer than three distinct vertices cannot enclose area.
            // Do not reject by signed area: self-intersecting contours
            // can have zero net signed area yet still have filled regions.
            bool valid = false;

            if (fPoints.size() >= 3) {
                size_t distinct = 1;

                for (size_t i = 1; i < fPoints.size(); ++i) {
                    bool seen = false;

                    for (size_t j = 0; j < i; ++j) {
                        if (samePoint_(fPoints[i], fPoints[j])) {
                            seen = true;
                            break;
                        }
                    }

                    if (!seen && ++distinct >= 3) {
                        valid = true;
                        break;
                    }
                }
            }

            if (valid) {
                if (!out.onMoveTo(fPoints[0].x, fPoints[0].y))
                    return false;

                for (size_t i = 1; i < fPoints.size(); ++i) {
                    if (!out.onLineTo(fPoints[i].x, fPoints[i].y))
                        return false;
                }

                if (!out.onClose())
                    return false;
            }

            fPoints.clear();
            fSubpathOpen = false;
            return true;
        }
    };


    inline bool preparePathForFill(const PathProgram& src, PathProgram& dst) noexcept
    {
        PathProgramBuilder builder;
        PathFillPreparer<PathProgramBuilder> preparer(builder);

        if (!pathprogram_dispatch(src, preparer))
            return false;

        dst = std::move(builder.prog);
        return true;
    }

} // namespace waavs
