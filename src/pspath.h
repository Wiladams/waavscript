#pragma once

#include <vector>

namespace waavs {

    enum class PSPathCommand : uint8_t {
        MoveTo,
        LineTo,
        CurveTo,
        ArcTo,
        ClosePath
    };

    struct PSPathSegment {
        PSPathCommand command;

        // Holds up to 3 points depending on the command
        double x1 = 0, y1 = 0;
        double x2 = 0, y2 = 0;
        double x3 = 0, y3 = 0;
    };

    struct PSPath {
        std::vector<PSPathSegment> segments;

		bool fHasCurrentPoint = false;
        double fCurrentX{ 0 };
        double fCurrentY{ 0 };
        double fStartX{ 0 };
        double fStartY{ 0 };


        void clear() {
            segments.clear();
			fCurrentX = 0;
			fCurrentY = 0;
			fHasCurrentPoint = false;
        }

        bool empty() const {
            return segments.empty();
		}

        bool getCurrentPoint(double& x, double& y) const {
            if (!fHasCurrentPoint) return false;
            x = fCurrentX;
            y = fCurrentY;
            return true;
		}

        // Movement commands to build path
        void moveto(double x, double y) {
            segments.push_back({ PSPathCommand::MoveTo, x, y });
            fCurrentX = fStartX = x;
            fCurrentY = fStartY = y;
            fHasCurrentPoint = true;
        }

        void lineto(double x, double y) {
            segments.push_back({ PSPathCommand::LineTo, x, y });
			fCurrentX = x;
			fCurrentY = y;
        }

        void curveto(double x1, double y1,
            double x2, double y2,
            double x3, double y3) {
			//if (!fHasCurrentPoint) return;

            segments.push_back({ PSPathCommand::CurveTo, x1, y1, x2, y2, x3, y3 });

			fCurrentX = x3; // Assuming x3 is the end point of the curve
			fCurrentY = y3;
        }

        // For ArcTo:
        //   x1, y1 = center
        //   x2     = radius
        //   x3     = startAngleRadians
        //   y3     = sweepAngleRadians
        void arcTo(double cx, double cy, double radius, double startAngleRad, double sweepAngleRad) {
            //if (!fHasCurrentPoint) return;

            segments.push_back({
                PSPathCommand::ArcTo,
                cx, cy,                      // x1, y1 = center
                radius, 0,                   // x2 = radius (y2 unused)
                startAngleRad, sweepAngleRad // x3 = start, y3 = sweep
                });

			fCurrentX = cx + radius * cos(startAngleRad + sweepAngleRad); // End point of arc
			fCurrentY = cy + radius * sin(startAngleRad + sweepAngleRad);

        }

        void close() {
            segments.push_back({ PSPathCommand::ClosePath });
			fCurrentX = fStartX; // Reset current point to start point
			fCurrentY = fStartY;
        }

    };

} // namespace waavs

