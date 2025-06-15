#pragma once

#pragma comment(lib, "blend2d.lib")

#include <algorithm>
#include <blend2d/blend2d.h>

#include "psgraphicscontext.h"



namespace waavs {

    class Blend2DGraphicsContext : public PSGraphicsContext {
    private:
        BLImage image;
        BLContext ctx;

    public:
        Blend2DGraphicsContext(int width, int height)
            : image(width, height, BL_FORMAT_PRGB32)
        {
            ctx.begin(image);
            ctx.clearAll();
			
            ctx.setFillRule(BL_FILL_RULE_NON_ZERO); // Non-zero winding rule
            ctx.setCompOp(BL_COMP_OP_SRC_OVER);
            ctx.setGlobalAlpha(1.0); // optional - opaque rendering
			ctx.fillAll(BLRgba32(255, 255, 255, 255)); // Fill with white background

			ctx.setStrokeAlpha(1.0); // optional - opaque stroke
            setRGB(0, 0, 0);


            // Flip coordinate system: origin to bottom-left, Y+ goes up
            double h = image.height();
            BLMatrix2D flipY = BLMatrix2D::makeScaling(1, -1);
            flipY.translate(0, -h);

            ctx.setTransform(flipY);
            ctx.userToMeta();

        }

        ~Blend2DGraphicsContext() {
            ctx.end();
        }

        const BLImage& getImage() const { return image; }

        void fill() override {
            BLPath blPath;
            BLMatrix2D blTrans;

            blTrans = blTransform(currentState()->ctm);
            buildBLPath(blPath);


            ctx.save(); // Save current state


            ctx.setTransform(blTrans);
            ctx.setFillStyle(convertPaint(currentState()->fillPaint));

            ctx.fillPath(blPath);

            currentPath().reset();

            ctx.restore(); // Restore to previous state

        }


        void stroke() override {
            BLPath blPath;
            BLMatrix2D blTrans;

            blTrans = blTransform(currentState()->ctm);
            buildBLPath(blPath);

			ctx.save(); // Save current state

            ctx.setTransform(blTrans);

            ctx.setStrokeStyle(convertPaint(currentState()->strokePaint));
            ctx.setStrokeWidth(currentState()->lineWidth);
            ctx.setStrokeCaps(static_cast<BLStrokeCap>(currentState()->lineCap));
            ctx.setStrokeJoin(static_cast<BLStrokeJoin>(currentState()->lineJoin));
            ctx.setStrokeMiterLimit(currentState()->miterLimit);

            ctx.strokePath(blPath);
            currentPath().reset();

			ctx.restore(); // Restore to previous state
        }




    private:
        inline void emitArcSegmentAsBezier(BLPath& out, double cx, double cy, double r, double t0, double t1) {
            double cos0 = std::cos(t0), sin0 = std::sin(t0);
            double cos1 = std::cos(t1), sin1 = std::sin(t1);

            double alpha = std::tan((t1 - t0) / 4) * 4.0 / 3.0;

            double x0 = cx + r * cos0;
            double y0 = cy + r * sin0;

            double x1 = x0 - r * alpha * sin0;
            double y1 = y0 + r * alpha * cos0;

            double x3 = cx + r * cos1;
            double y3 = cy + r * sin1;

            double x2 = x3 + r * alpha * sin1;
            double y2 = y3 - r * alpha * cos1;

            out.cubicTo(x1, y1, x2, y2, x3, y3);
        }

        void buildBLPath(BLPath& out) {
            static constexpr double B_PI = 3.14159265358979323846;
            static constexpr double B_PI_2 = B_PI / 2.0;

            for (const auto& seg : currentPath().segments) {
                switch (seg.command) {
                case PSPathCommand::MoveTo:
                    out.moveTo(seg.x1, seg.y1);
                    break;
                case PSPathCommand::LineTo:
                    out.lineTo(seg.x1, seg.y1);
                    break;
                case PSPathCommand::CurveTo:
                    out.cubicTo(seg.x1, seg.y1, seg.x2, seg.y2, seg.x3, seg.y3);
                    break;

                case PSPathCommand::ArcTo: {
                    double centerX = seg.x1;
                    double centerY = seg.y1;
                    double radius = seg.x2;
                    double startRad = seg.x3;
                    double sweepRad = seg.y3;
					double startX = centerX + radius * std::cos(startRad);
					double startY = centerY + radius * std::sin(startRad);

                    // Subdivide arc into Bézier segments
                    int steps = std::ceil(std::abs(sweepRad) / (B_PI_2)); // At most 90° per cubic
                    double delta = sweepRad / steps;

                    // do a moveTo before the curveTo that will follow
					out.moveTo(startX, startY);

                    for (int i = 0; i < steps; ++i) {
                        double t0 = startRad + i * delta;
                        double t1 = t0 + delta;
                        emitArcSegmentAsBezier(out, centerX, centerY, radius, t0, t1);
                        //cx = centerX + radius * std::cos(t1);
                        //cy = centerY + radius * std::sin(t1);
                    }
                    break;
                }

                case PSPathCommand::ClosePath:
                    out.close();
                    break;
                }
            }
        }

        BLMatrix2D blTransform(const PSMatrix& m) const {
            return BLMatrix2D(m.m[0], m.m[1], m.m[2], m.m[3], m.m[4], m.m[5]);
        }

        BLRgba32 convertPaint(const PSPaint& p) const {
            switch (p.kind) {
            case PSPaintKind::GRAY:
                return BLRgba32(uint8_t(p.gray * 255), uint8_t(p.gray * 255), uint8_t(p.gray * 255), 255);
            case PSPaintKind::RGB:
                return BLRgba32(uint8_t(p.r * 255), uint8_t(p.g * 255), uint8_t(p.b * 255));
            case PSPaintKind::CMYK: {
                auto clamp01 = [](double x) { return std::min(std::max(x, 0.0), 1.0); };
                double r = 1.0 - clamp01(p.c + p.k);
                double g = 1.0 - clamp01(p.m + p.k);
                double b = 1.0 - clamp01(p.y + p.k);
                return BLRgba32(uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255));
            }
            default:
                return BLRgba32(0, 0, 0, 255); // fallback to black
            }
        }
    };

} // namespace waavs
