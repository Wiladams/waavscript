#pragma once

#include <vector>
#include <cmath>

namespace waavs {

    enum class PSPathCommand : uint8_t {
        MoveTo,
        LineTo,
        Arc,        // clockwise arc from start to end angle
        ArcCCW, // counter-clockwise arc from start to end angle
        ArcTo,
        CurveTo,
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
        static constexpr double P_PI = 3.14159265358979323846; // Pi constant

        std::vector<PSPathSegment> segments;

		bool fHasCurrentPoint = false;
        double fCurrentX{ 0 };
        double fCurrentY{ 0 };
        double fStartX{ 0 };
        double fStartY{ 0 };


        bool reset() {
            segments.clear();
			fCurrentX = 0;
			fCurrentY = 0;
            fStartX = 0;
            fStartY = 0;

			fHasCurrentPoint = false;

            return true;
        }

        bool empty() const {
            return segments.empty();
		}

        constexpr bool hasCurrentPoint() const {
            return fHasCurrentPoint;
        }

        bool getCurrentPoint(double& x, double& y) const {
            if (!fHasCurrentPoint) return false;
            x = fCurrentX;
            y = fCurrentY;
            return true;
		}

        // Movement commands to build path
        bool moveto(double x, double y) {
            segments.push_back({ PSPathCommand::MoveTo, x, y });
            fCurrentX = fStartX = x;
            fCurrentY = fStartY = y;
            fHasCurrentPoint = true;

            return true;
        }

        bool lineto(double x, double y) {
			if (!fHasCurrentPoint) return false;
            segments.push_back({ PSPathCommand::LineTo, x, y });
			fCurrentX = x;
			fCurrentY = y;

            return true;
        }



        // Arc command:
        //   cx-x1, cy-y1 = center of the arc 
        //   radius-x2 = radius of the arc
        //   startDeg-x3 = starting angle in degrees
        //   endDeg-y3 = ending angle in degrees
        // Note: Angles are in degrees, with 0 degrees pointing to the right (positive X axis)
        bool arc(double cx, double cy, double radius, double startDeg, double endDeg) {
            // Arc is valid even if there is no currentpoint yet (like moveto)
            PSPathSegment seg;
            seg.command = PSPathCommand::Arc;
            seg.x1 = cx;        // Center X
            seg.y1 = cy;        // Center Y
            seg.x2 = radius;    // Radius
            seg.y2 = 0;         // Unused
            seg.x3 = startDeg;  // Start angle (degrees)
            seg.y3 = endDeg;    // End angle (degrees)

            segments.push_back(seg);

            // Update currentpoint to arc endpoint (angle = endDeg)
            double thetaRad = endDeg * (P_PI / 180.0);
            fCurrentX = cx + radius * std::cos(thetaRad);
            fCurrentY = cy + radius * std::sin(thetaRad);
            fHasCurrentPoint = true;

            return true;
        }

        bool arcCCW(double cx, double cy, double radius, double startDeg, double endDeg) {
            segments.push_back({
                PSPathCommand::ArcCCW,
                cx, cy,                      // x1, y1 = center
                radius, 0,                   // x2 = radius (y2 unused)
                startDeg, endDeg             // x3 = start angle, y3 = end angle (in degrees)
                });

            // Update currentpoint to the arc’s endpoint
            double endRad = endDeg * (P_PI / 180.0);
            fCurrentX = cx + radius * cos(endRad);
            fCurrentY = cy + radius * sin(endRad);
            fHasCurrentPoint = true;

            return true;
        }

        bool arcto(double x0, double y0,
            double x1, double y1,
            double x2, double y2,
            double r,
            double& xt1, double& yt1,
            double& xt2, double& yt2)
        {
            // Compute vectors
            double v1x = x0 - x1;
            double v1y = y0 - y1;
            double v2x = x2 - x1;
            double v2y = y2 - y1;

            double len1 = std::sqrt(v1x * v1x + v1y * v1y);
            double len2 = std::sqrt(v2x * v2x + v2y * v2y);

            if (len1 < 1e-6 || len2 < 1e-6)
                return false; // Degenerate input

            // Normalize
            v1x /= len1; v1y /= len1;
            v2x /= len2; v2y /= len2;

            // Compute bisector
            double bisectX = v1x + v2x;
            double bisectY = v1y + v2y;
            double bisectLen = std::sqrt(bisectX * bisectX + bisectY * bisectY);
            if (bisectLen < 1e-6)
                return false; // 180 deg corner, cannot round

            bisectX /= bisectLen;
            bisectY /= bisectLen;

            // sin(theta / 2) = sqrt((1 - cos(theta))/2)
            double dot = v1x * v2x + v1y * v2y;
            double sinHalfTheta = std::sqrt((1 - dot) / 2);
            if (sinHalfTheta < 1e-6)
                return false;

            // Distance from corner to arc start/end
            double dist = r / sinHalfTheta;

            // Tangent points
            xt1 = x1 + v1x * (dist - r);
            yt1 = y1 + v1y * (dist - r);
            xt2 = x1 + v2x * (dist - r);
            yt2 = y1 + v2y * (dist - r);

            // Line to arc start
            if (!lineto(xt1, yt1)) return false;

            // Approximate the arc from (xt1, yt1) to (xt2, yt2) around center (x1, y1)
            // You can either:
            //   - Insert an ArcTo segment here with center (x1, y1), radius r, and angles
            //   - Or approximate using one or more bezier segments (if ArcTo isn't supported downstream)
            // For now, we’ll use an ArcTo

            double a1 = std::atan2(yt1 - y1, xt1 - x1);
            double a2 = std::atan2(yt2 - y1, xt2 - x1);
            double sweep = a2 - a1;

            // Normalize sweep to positive CW arc
            if (sweep <= 0)
                sweep += 2 * P_PI;

            segments.push_back({
                PSPathCommand::ArcTo,
                x1, y1,         // center
                r, 0,           // radius
                a1, sweep       // angles
                });

            // Line to endpoint of arc (p2)
            if (!lineto(x2, y2)) return false;

            fCurrentX = x2;
            fCurrentY = y2;
            fHasCurrentPoint = true;

            return true;
        }

        bool curveto(double x1, double y1,
            double x2, double y2,
            double x3, double y3) {

            if (!fHasCurrentPoint) return false;

            segments.push_back({ PSPathCommand::CurveTo, x1, y1, x2, y2, x3, y3 });

            fCurrentX = x3; // Assuming x3 is the end point of the curve
            fCurrentY = y3;

            return true;
        }

        bool close() {
            segments.push_back({ PSPathCommand::ClosePath });
			fCurrentX = fStartX; // Reset current point to start point
			fCurrentY = fStartY;

            return true;
        }

    };

} // namespace waavs

