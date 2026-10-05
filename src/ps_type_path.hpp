#pragma once

#include <vector>
#include <cmath>

#include "ps_type_matrix.h"

namespace waavs {
#define CLAMP(x, low, high) std::min(std::max(x, low), high)

    enum class PSPathCommand : uint8_t {
        MoveTo,
        LineTo,
        EllipticArc,
        CurveTo,
        ClosePath
    };

    struct PSPathSegment {
        PSPathCommand command;
        PSMatrix fTransform;    // transformation matrix for this segment

        // Holds up to 3 points depending on the command
        double x1 = 0, y1 = 0;
        double x2 = 0, y2 = 0;
        double x3 = 0, y3 = 0;

    };

    // calArcTangents
    //
    // Given three points (x0, y0), (x1, y1), (x2, y2) 
    // where (x0,y0) is the starting point
    //       (x1, y1) is the corner point
    //       (x2, y2) is the end point
    // and a radius r, calculates the tangent points (xt1, yt1) and (xt2, yt2)
    // Returns true if successful, false if the angle is too small to draw an arc
    // The tangent points are the points on the circle of radius r that touch the lines
    // going from the two endpoints to the corner point.
    static bool calcArcTangents(double x0, double y0,
        double x1, double y1,
        double x2, double y2,
        double r,
        double& xt1, double& yt1,
        double& xt2, double& yt2)
    {
        // 1. Compute direction vectors away from the corner towards the tangent points
        // From start point (x0, y0) to corner (x1, y1)
        double dx1 = x0 - x1;
        double dy1 = y0 - y1;
        double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
        double vx1 = dx1 / len1; // Normalize
        double vy1 = dy1 / len1;

        // From end point (x2, y2) to corner (x1, y1)
        double dx2 = x2 - x1;
        double dy2 = y2 - y1;
        double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
        double vx2 = dx2 / len2; // Normalize
        double vy2 = dy2 / len2;

        // 2. Compute the angle between the two vectors
        // This gives us the interior angle at the corner
        double dot = vx1 * vx2 + vy1 * vy2; // Dot product
        double theta = std::acos(CLAMP(dot, -1.0, 1.0)); // Angle in radians

        // 3. Calculate the distance from the corner to the tangent points
        // We want to back up along the vectors from the corner point along the normals
        // This comes from from geometry of circle segments tangent to both lines
        // Note:  if the angle is too small, we can't draw an arc, so we should exit, or
        // just draw the two lines instead
        double d = r / std::tan(theta / 2.0);


        // 4.0 Compute tangent points
        xt1 = x1 + vx1 * d; // Tangent point 1
        yt1 = y1 + vy1 * d; // Tangent point 1
        xt2 = x1 + vx2 * d; // Tangent point 2
        yt2 = y1 + vy2 * d; // Tangent point 2

        return true;
    }


    struct PSPath {

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
        bool moveto(const PSMatrix &ctm, double x, double y) {
            PSPathSegment seg;
            seg.command = PSPathCommand::MoveTo;
            seg.fTransform = ctm; // Store the transformation matrix
            seg.x1 = x;
            seg.y1 = y;

            segments.push_back(seg);

            fCurrentX = fStartX = x;
            fCurrentY = fStartY = y;

            fHasCurrentPoint = true;

            return true;
        }
        bool moveto(double x, double y) {
            PSMatrix identity; // Identity matrix for no transformation
            return moveto(identity, x, y);
        }

        bool lineto(const PSMatrix& ctm, double x, double y) {
			if (!fHasCurrentPoint) return false;

            PSPathSegment seg;
            seg.command = PSPathCommand::LineTo;
            seg.fTransform = ctm;
            seg.x1 = x;
            seg.y1 = y;

            segments.push_back(seg);

            fCurrentX = x;
            fCurrentY = y;

            return true;
        }
        bool lineto(double x, double y) {
            PSMatrix identity; // Identity matrix for no transformation
            return lineto(identity, x, y);
        }



        bool ellipticArcTo(double radius,bool sweepFlag,double x2, double y2) {

            if (!fHasCurrentPoint) 
                return false;

            // Store the elliptic arc segment
            PSPathSegment seg;
            seg.command = PSPathCommand::EllipticArc;
            seg.x1 = radius;        // radius
            seg.y1 = sweepFlag;     // sweep flag (0 or 1)
            seg.x2 = x2;            // endpoint x
            seg.y2 = y2;            // endpoint y

            segments.push_back( seg);

            fCurrentX = x2; // Update current point to end point
            fCurrentY = y2;
            fHasCurrentPoint = true;
            
            return true;
        }
        

        bool arcto(const PSMatrix &ctm, double x0, double y0,
            double x1, double y1,
            double x2, double y2,
            double r,
            double& xt1, double& yt1,
            double& xt2, double& yt2)
        {
            // 1. Compute direction vectors away from the corner towards the tangent points
            // From start point (x0, y0) to corner (x1, y1)
            double dx1 = x0 - x1;
            double dy1 = y0 - y1;
            double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
            double vx1 = dx1 / len1; // Normalize
            double vy1 = dy1 / len1;

            // From end point (x2, y2) to corner (x1, y1)
            double dx2 = x2 - x1;
            double dy2 = y2 - y1;
            double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);
            double vx2 = dx2 / len2; // Normalize
            double vy2 = dy2 / len2;
        
            // 2. Compute the angle between the two vectors
            // This gives us the interior angle at the corner
            double dot = vx1 * vx2 + vy1 * vy2; // Dot product
            double theta = std::acos(CLAMP(dot,-1.0,1.0)); // Angle in radians

            // 3. Calculate the distance from the corner to the tangent points
            // We want to back up along the vectors from the corner point along the normals
            // This comes from from geometry of circle segments tangent to both lines
            // Note:  if the angle is too small, we can't draw an arc, so we should exit, or
            // just draw the two lines instead
            double d = r / std::tan(theta / 2.0);


            // 4.0 Compute tangent points
            xt1 = x1 + vx1 * d; // Tangent point 1
            yt1 = y1 + vy1 * d; // Tangent point 1
            xt2 = x1 + vx2 * d; // Tangent point 2
            yt2 = y1 + vy2 * d; // Tangent point 2

            
            // 5.0 Compute Arc Center
            // 5a - We need vectors that point towards the corner from the tangent points,
            // which are the opposite of what was previously calculated
            double dux1 = x1 - xt1; // Inverse direction of vx1
            double duy1 = y1 - yt1; // Inverse direction of vy1
            double u1len = std::sqrt(dux1 * dux1 + duy1 * duy1);
            double ux1 = dux1 / u1len;
            double uy1 = duy1 / u1len;

            double dux2 = x1 - xt2; // Inverse direction of vx2
            double duy2 = y1 - yt2; // Inverse direction of vy2
            double u2len = std::sqrt(dux2 * dux2 + duy2 * duy2);
            double ux2 = dux2 / u2len;
            double uy2 = duy2 / u2len;

            // 5b - Compute the angle bisector vector
            double bx = ux1 + ux2; // Bisector X
            double by = uy1 + uy2; // Bisector Y
            double blen = std::sqrt(bx * bx + by * by); // Length of bisector
            bx /= -blen; // Normalize
            by /= -blen; // Normalize

            // 5c Compute the center of offset distance
            // Now 'b' is a unit vector pointing from the corner towards the center of the arc
            double h = r / std::sin(theta / 2.0); // Distance from corner to center

            double cx = x1 + bx * h; // Center X
            double cy = y1 + by * h; // Center Y

            // 6.0 Determine sweep flag
            // Take the signed area of the triangle
            double cross = (xt1 - cx) * (yt2 - cy) - (xt2 - cx) * (yt1 - cy);
            double sweepFlag = (cross > 0);
            

            // Build the actual path segments
            lineto(ctm, xt1, yt1);
            ellipticArcTo(r, sweepFlag, xt2, yt2);

            fCurrentX = xt2;
            fCurrentY = yt2;
            fHasCurrentPoint = true;

            return true;
        }


        bool curveto(const PSMatrix &ctm, 
            double x1, double y1,
            double x2, double y2,
            double x3, double y3) {

            if (!fHasCurrentPoint) 
                return false;

            PSPathSegment seg;
            seg.command = PSPathCommand::CurveTo;
            seg.fTransform = ctm;
            seg.x1 = x1;
            seg.y1 = y1;
            seg.x2 = x2;
            seg.y2 = y2;
            seg.x3 = x3;
            seg.y3 = y3;
            segments.push_back(seg);

            fCurrentX = x3; // Assuming x3 is the end point of the curve
            fCurrentY = y3;

            return true;
        }

        bool close() {
            if (!fHasCurrentPoint) 
                return false;
            
            PSPathSegment seg;
            seg.command = PSPathCommand::ClosePath;
            seg.fTransform = PSMatrix(); // No transformation for close path
            seg.x1 = fCurrentX;
            seg.y1 = fCurrentY;

            segments.push_back(seg);

            fCurrentX = fStartX; // Reset current point to start point
			fCurrentY = fStartY;

            return true;
        }

        // Get the boundary box for the path
        bool getBoundingBox(double& minX, double& minY, double& maxX, double& maxY) const {
            bool found = false;

            auto includePoint = [&](double x, double y) {
                if (!found) {
                    minX = maxX = x;
                    minY = maxY = y;
                    found = true;
                }
                else {
                    if (x < minX) minX = x;
                    if (x > maxX) maxX = x;
                    if (y < minY) minY = y;
                    if (y > maxY) maxY = y;
                }
                };

            auto includeArcBounds = [&](double cx, double cy, double r, double startDeg, double sweepDeg) {
                auto normDeg = [](double deg) {
                    deg = std::fmod(deg, 360.0);
                    return deg < 0 ? deg + 360.0 : deg;
                    };

                startDeg = normDeg(startDeg);
                double endDeg = normDeg(startDeg + sweepDeg);

                auto isAngleBetween = [](double angle, double start, double end) {
                    if (end < start) end += 360.0;
                    if (angle < start) angle += 360.0;
                    return angle >= start && angle <= end;
                    };

                auto include = [&](double deg) {
                    double rad = deg * (PI / 180.0);
                    double x = cx + r * std::cos(rad);
                    double y = cy + r * std::sin(rad);
                    includePoint(x, y);
                    };

                include(startDeg);
                include(endDeg);

                for (int i = 0; i < 4; ++i) {
                    double axisDeg = i * 90.0;
                    if (isAngleBetween(axisDeg, startDeg, endDeg))
                        include(axisDeg);
                }
                };

            for (const auto& seg : segments) {
                switch (seg.command) {
                case PSPathCommand::MoveTo:
                case PSPathCommand::LineTo:
                case PSPathCommand::ClosePath:
                    includePoint(seg.x1, seg.y1);
                    break;

                case PSPathCommand::CurveTo:
                    for (int i = 0; i < 3; ++i) {
                        double x = (&seg.x1)[i * 2];
                        double y = (&seg.y1)[i * 2];
                        includePoint(x, y);
                    }
                    break;

                case PSPathCommand::EllipticArc: {
                    double cx = seg.x1;
                    double cy = seg.y1;
                    double r = seg.x2;
                    double startDeg = seg.x3;
                    double sweepDeg = seg.y3;
                    includeArcBounds(cx, cy, r, startDeg, sweepDeg);
                    break;
                }

                default:
                    break;
                }
            }

            return found;
        }
    };

} // namespace waavs

