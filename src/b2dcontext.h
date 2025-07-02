#pragma once

#pragma comment(lib, "blend2d.lib")

#include <algorithm>
#include <blend2d/blend2d.h>

#include "psgraphicscontext.h"
#include "fontmonger.h"


namespace waavs {
    //static inline BLMatrix2D blTransform(const PSMatrix& m) {
    //    return BLMatrix2D(m.m[0], m.m[1], m.m[2], m.m[3], m.m[4], m.m[5]);
    //}

    static inline BLStrokeJoin convertLineJoin(PSLineJoin join) {
        switch (join) {
        case PSLineJoin::Miter:
            return BLStrokeJoin::BL_STROKE_JOIN_MITER_CLIP;
        case PSLineJoin::Round:
            return BLStrokeJoin::BL_STROKE_JOIN_ROUND;
        case PSLineJoin::Bevel:
            return BLStrokeJoin::BL_STROKE_JOIN_BEVEL;
        default:
            return BLStrokeJoin::BL_STROKE_JOIN_MITER_CLIP; // Default to miter
        }
    }

    // From current point to: angleStart + sweep
    // With radius = BLPoint(r, r), rotation = 0
    // largeArcFlag = sweepDeg > 180
    // sweepFlag = true  // clockwise
    /*
    bool emitArcToBlend2D(BLPath& path, double cx, double cy, double r, double a1Deg, double sweepDeg)
    {
        double a2Deg = a1Deg + sweepDeg;

        // Start and end point in radians
        double a1Rad = a1Deg * (3.14159265358979323846 / 180.0);
        double a2Rad = a2Deg * (3.14159265358979323846 / 180.0);

        double x0 = cx + r * std::cos(a1Rad);
        double y0 = cy + r * std::sin(a1Rad);
        double x1 = cx + r * std::cos(a2Rad);
        double y1 = cy + r * std::sin(a2Rad);

        // Inject moveTo if needed (depends on current path state)
        BLPoint lastPos;
        path.getLastVertex(&lastPos);
        if (std::abs(lastPos.x - x0) > 1e-6 || std::abs(lastPos.y - y0) > 1e-6)
            path.lineTo(x0, y0);

        bool largeArc = sweepDeg > 180.0;
        bool sweep = true; // clockwise

        return path.ellipticArcTo(BLPoint(r, r), 0.0, largeArc, sweep, BLPoint(x1, y1)) == BL_SUCCESS;
    }
    */
    
    static inline void emitArcSegmentAsBezier(BLPath& out, double cx, double cy, double r, double t0, double t1, const PSMatrix &ctm) {
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

        double tx0, ty0;
        double tx1, ty1;
        double tx2, ty2;
        double tx3, ty3;

        ctm.transformPoint(x0, y0, tx0, ty0);
        ctm.transformPoint(x1, y1, tx1, ty1);
        ctm.transformPoint(x2, y2, tx2, ty2);
        ctm.transformPoint(x3, y3, tx3, ty3);

        out.cubicTo(tx1, ty1, tx2, ty2, tx3, ty3);
    }
    

    bool convertPSPathToBLPath(const PSPath &path, BLPath& out) {
        static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
        static constexpr double QUARTER_ARC = 3.14159265358979323846 / 2.0;

        for (const auto& seg : path.segments) {
            switch (seg.command) {
            case PSPathCommand::MoveTo: {
                double tx, ty;
                seg.fTransform.transformPoint(seg.x1, seg.y1, tx, ty);
                out.moveTo(tx, ty);
            }

                break;

            case PSPathCommand::LineTo: {
                double tx, ty;
                seg.fTransform.transformPoint(seg.x1, seg.y1, tx, ty);

                out.lineTo(tx, ty);
            }
                break;

            case PSPathCommand::CurveTo:
                out.cubicTo(seg.x1, seg.y1, seg.x2, seg.y2, seg.x3, seg.y3);
                break;


            case PSPathCommand::Arc: 
            case PSPathCommand::ArcCCW:
            {

                double cx = seg.x1;
                double cy = seg.y1;
                double radius = seg.x2;
                double startDeg = seg.x3;
                double endDeg = seg.y3;

                // Convert angles to radians
                double startRad = startDeg * DEG_TO_RAD;
                double endRad = endDeg * DEG_TO_RAD;
                double sweep = endRad - startRad;

                // Determine number of segments (max 90 degrees per segment)
                int steps = std::ceil(std::abs(sweep) / QUARTER_ARC);
                if (steps < 1) steps = 1;
                double delta = sweep / steps;

                // Add moveTo for start point
                double startX = cx + radius * std::cos(startRad);
                double startY = cy + radius * std::sin(startRad);
                double tx, ty;
                seg.fTransform.transformPoint(startX, startY, tx, ty);
                out.moveTo(tx, ty);

                for (int i = 0; i < steps; ++i) {
                    double t0 = startRad + i * delta;
                    double t1 = t0 + delta;
                    emitArcSegmentAsBezier(out, cx, cy, radius, t0, t1, seg.fTransform);
                }
                break;
            }

            case PSPathCommand::EllipticArc: {
                double x1 = seg.x2;
                double y1 = seg.y2;
                double r = seg.x1; // Radius
                bool sweepFlag = seg.y1>0.0 ? true : false;
               
                out.ellipticArcTo(r, r, 0.0, false, sweepFlag, x1, y1);
                break;
            }

            case PSPathCommand::ClosePath:
                out.close();
                break;
            }
        }

        return true;
    }

    // Use blend2d library to do actual rendering
    class Blend2DGraphicsContext : public PSGraphicsContext {
    private:
        BLImage fCanvas;
        BLContext ctx;

    public:
        Blend2DGraphicsContext(int width, int height)
            : fCanvas(width, height, BL_FORMAT_PRGB32)
        {
            ctx.begin(fCanvas);
            ctx.clearAll();
			
            ctx.setFillRule(BL_FILL_RULE_NON_ZERO); // Non-zero winding rule
            ctx.setCompOp(BL_COMP_OP_SRC_OVER);
            ctx.setGlobalAlpha(1.0); // optional - opaque rendering
			ctx.fillAll(BLRgba32(0xff, 0xff, 0xff, 255)); // Fill with white background

			ctx.setStrokeAlpha(1.0); // optional - opaque stroke
            setRGB(0, 0, 0);


            // Flip coordinate system: origin to bottom-left, Y+ goes up
            double h = fCanvas.height();
            BLMatrix2D flipY = BLMatrix2D::makeScaling(1, -1);

            flipY.translate(0, -h);

            ctx.setTransform(flipY);
            ctx.userToMeta();

        }

        ~Blend2DGraphicsContext() {
            ctx.end();
        }

        const BLImage& getImage() const { return fCanvas; }

        void onShowPage() override {
            //printf("onShowPage: show the current page\n", pageWidth, pageHeight);
            ctx.flush(BLContextFlushFlags::BL_CONTEXT_FLUSH_SYNC);
        }

        // Font related methods
        bool findFont(const PSName& name, PSObject& outObj) override
        {
            return FontMonger::instance().findFontFaceByName(name, outObj);
        }

        // Painting - filling and stroking paths
        bool fill() override {
            BLPath blPath;
            if (!convertPSPathToBLPath(currentPath(), blPath))
                return false;


            ctx.save(); // Save current state

            BLRgba32 fillColor = convertPaint(currentState()->fillPaint);
            BLFillRule fillRule = BL_FILL_RULE_NON_ZERO;

            ctx.setFillRule(fillRule);
            ctx.setFillStyle(fillColor);

            ctx.fillPath(blPath);

            currentPath().reset();

            ctx.restore(); // Restore to previous state

            return true;
        }

        bool eofill() override {
            BLPath blPath;

            if (!convertPSPathToBLPath(currentPath(), blPath))
                return false;

            ctx.save(); // Save current state

            BLRgba32 fillColor = convertPaint(currentState()->fillPaint);
            BLFillRule fillRule = BL_FILL_RULE_EVEN_ODD;

            ctx.setFillRule(fillRule); // Set even-odd fill rule
            ctx.setFillStyle(fillColor);
            ctx.fillPath(blPath);

            currentPath().reset();

            ctx.restore(); // Restore to previous state

            return true;
        }

        bool stroke() override {
            BLPath blPath;

            if (!convertPSPathToBLPath(currentPath(), blPath))
                return false;

			ctx.save(); // Save current state

            BLRgba32 strokeColor = convertPaint(currentState()->strokePaint);
            double lineWidth = currentState()->lineWidth;
            BLStrokeJoin join = convertLineJoin(currentState()->lineJoin);

            ctx.setStrokeStyle(strokeColor);
            ctx.setStrokeWidth(lineWidth);
            ctx.setStrokeCaps(static_cast<BLStrokeCap>(currentState()->lineCap));
            ctx.setStrokeJoin(join);
            ctx.setStrokeMiterLimit(currentState()->miterLimit);

            ctx.strokePath(blPath);
            currentPath().reset();

			ctx.restore(); // Restore to previous state

            return true;
        }


        bool image(PSImage& img) override
        {
            // Create a BLImage object
            BLImage blimg(img.width, img.height, BLFormat::BL_FORMAT_PRGB32);
            BLImageData imgData;
            blimg.getData(&imgData);

            // got pixel by pixel setting each value according to the grayscale
            // values in the PSImage
            for (int y = 0; y < img.height; ++y) {
                for (int x = 0; x < img.width; ++x) {
                    uint8_t grayValue = img.data[y * img.width + x];
                    uint32_t pixelValue = (255 << 24) | (grayValue << 16) | (grayValue << 8) | grayValue;
                    ((uint32_t*)(imgData.pixelData))[(img.height-1-y)*img.width+x] = pixelValue;
                }
            }

            //BLMatrix2D blTrans = blTransform(img.transform);
            ctx.save();
            //ctx.applyTransform(blTrans);
            ctx.blitImage(BLPoint(0, 0), blimg);
            ctx.restore();

            return true;
        }

    private:







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
