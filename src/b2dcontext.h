#pragma once

#pragma comment(lib, "blend2d.lib")

#include <algorithm>
#include <blend2d/blend2d.h>

#include "psgraphicscontext.h"
#include "fontmonger.h"


namespace waavs {
    static inline BLMatrix2D blTransform(const PSMatrix& m) {
        return BLMatrix2D(m.m[0], m.m[1], m.m[2], m.m[3], m.m[4], m.m[5]);
    }

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


    static inline BLRgba32 convertPaint(const PSPaint& p)  {
        switch (p.kind) {
        case PSPaintKind::GRAY:
            return BLRgba32(uint8_t(p.gray * 255), uint8_t(p.gray * 255), uint8_t(p.gray * 255), 255);
        case PSPaintKind::RGB:
            return BLRgba32(uint8_t(p.r * 255), uint8_t(p.g * 255), uint8_t(p.b * 255), uint8_t(p.a * 255));
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
                // transform the points using the segment's transformation matrix
                //out.cubicTo(seg.x1, seg.y1, seg.x2, seg.y2, seg.x3, seg.y3);

                double tx1, ty1;
                double tx2, ty2;
                double tx3, ty3;

                seg.fTransform.transformPoint(seg.x1, seg.y1, tx1, ty1);
                seg.fTransform.transformPoint(seg.x2, seg.y2, tx2, ty2);
                seg.fTransform.transformPoint(seg.x3, seg.y3, tx3, ty3);

                out.cubicTo(tx1, ty1, tx2, ty2, tx3, ty3);
            break;


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

    bool convertBLPathToPSPath(const BLPath& inPath, const PSMatrix& ctm, PSPath& outPSPath)
    {
        const uint8_t* cmds = inPath.commandData();
        const BLPoint* pts = inPath.vertexData();
        size_t cmdCount = inPath.size();

        for (size_t i = 0; i < cmdCount; ++i)
        {
            BLPathCmd cmd = (BLPathCmd)cmds[i];
            switch (cmd) {
            case BLPathCmd::BL_PATH_CMD_MOVE:
                outPSPath.moveto(ctm, pts[i].x, pts[i].y);
                break;

            case BLPathCmd::BL_PATH_CMD_ON:
                outPSPath.lineto(ctm, pts[i].x, pts[i].y);
                break;



            case BL_PATH_CMD_QUAD:
            {
                const BLPoint& p0 = outPSPath.hasCurrentPoint()
                    ? BLPoint(outPSPath.fCurrentX, outPSPath.fCurrentY)
                    : pts[i];  // fallback if no current point (unlikely for charpath)

                const BLPoint& p1 = pts[i + 0]; // control
                const BLPoint& p2 = pts[i + 1]; // end

                // Convert quad to cubic
                BLPoint c1 = BLPoint(
                    p0.x + (2.0 / 3.0) * (p1.x - p0.x),
                    p0.y + (2.0 / 3.0) * (p1.y - p0.y)
                );

                BLPoint c2 = BLPoint(
                    p2.x + (2.0 / 3.0) * (p1.x - p2.x),
                    p2.y + (2.0 / 3.0) * (p1.y - p2.y)
                );

                // Emit as cubic
                outPSPath.curveto(ctm, c1.x, c1.y, c2.x, c2.y, p2.x, p2.y);
                i++;
                break;
            }

            case BLPathCmd::BL_PATH_CMD_CUBIC:
                outPSPath.curveto(
                    ctm,
                    pts[i + 0].x, pts[i + 0].y,
                    pts[i + 1].x, pts[i + 1].y,
                    pts[i + 2].x, pts[i + 2].y
                );
                i += 2;
                break;

            case BL_PATH_CMD_CLOSE:
                outPSPath.close();
                break;

            default:
                // Skip unsupported/unknown commands
                printf("UNKNOWN COMMAND: %d\n", cmd);
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
            flipY.scale(2.77, 2.77);

            ctx.setTransform(flipY);
            ctx.userToMeta();

        }

        ~Blend2DGraphicsContext() {
            ctx.end();
        }

        const BLImage& getImage() const { return fCanvas; }

        void showPage() override {
            //printf("onShowPage: show the current page\n", pageWidth, pageHeight);
            ctx.flush(BLContextFlushFlags::BL_CONTEXT_FLUSH_SYNC);
        }

        void erasePage() override {
            // Clear the canvas
            ctx.clearAll();
            ctx.flush(BLContextFlushFlags::BL_CONTEXT_FLUSH_SYNC);
        }

        // Font related methods
        bool findFont(PSVirtualMachine &vm, const PSName& faceName, PSObject& outObj) override
        {
            auto& ostk = vm.opStack();
            auto& estk = vm.execStack();

            ostk.pushLiteralName(faceName);
            ostk.pushLiteralName("Font");
            estk.pushExecName("findresource");


            if (!vm.run())
                return false;

            ostk.pop(outObj);
            
            return true;
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

        void strokeAxis(BLRgba32 xColor, BLRgba32 yColor)
        {
            // Draw postscript axes as they currently sit
            BLPath xAxisPath, yAxisPath;
            xAxisPath.moveTo(0, 0);
            xAxisPath.lineTo(300, 0);
            yAxisPath.moveTo(0, 0);
            yAxisPath.lineTo(0, 300);

            ctx.strokePath(xAxisPath, xColor);
            ctx.strokePath(yAxisPath, yColor);

        }

        bool showText(const PSMatrix& ctm, const PSString& text) override
        {
            // select the current font
            // encode the string
            // draw it
            auto fontHandle =  currentState()->getFont();
            BLFont* font = (BLFont *)fontHandle->fSystemHandle;
            double x, y;
            currentState()->fCurrentPath.getCurrentPoint(x, y);

            double dx, dy;
            getStringWidth(fontHandle, text, dx, dy); 

            ctx.save();

            // DEBUG - Postscript axis before anything else
            //ctx.setStrokeWidth(12.0);
            //strokeAxis(BLRgba32(0xff0000ff), BLRgba32(0xffff0000));


            // Apply CTM matrix before the coordinates of the text
           // PSMatrix ctm = currentState()->ctm;
            BLMatrix2D bctm(ctm.m[0], ctm.m[1], ctm.m[2], ctm.m[3], ctm.m[4], ctm.m[5]);
            ctx.applyTransform(bctm);

            // Finally, get into the right coordinate space 
            // To draw the text
            ctx.translate(x, y);

            // draw translated axis
            //ctx.setStrokeWidth(3.0);
            //strokeAxis(BLRgba32(0xff0000ff), BLRgba32(0xffff0000));

            // flip the y-axis
            ctx.scale(1, -1);

            // draw final flipped axes
            //ctx.setStrokeWidth(1.0);
            //strokeAxis(BLRgba32(0xff000000), BLRgba32(0xffff00ff));

            // Finally, draw the actual text
            BLRgba32 fillColor = convertPaint(currentState()->fillPaint);
            ctx.setFillStyle(fillColor);
            ctx.fillUtf8Text(BLPoint(0, 0), *font, (const char *)text.data(), text.length());
            
            ctx.restore();

            // Reset the current path after drawing text
            currentState()->fCurrentPath.fCurrentX = x + dx; 
            currentState()->fCurrentPath.fCurrentY = y + dy;

            return false;
        }

        bool getStringWidth(PSFontHandle fontHandle, const PSString& str, double& dx, double& dy) const override
        {
            dx = 0;
            dy = 0;

            BLFont* font = (BLFont*)fontHandle->fSystemHandle;
            PSMatrix ctm = currentState()->ctm;

            BLTextMetrics tm;
            BLGlyphBuffer gb;
            BLFontMetrics fm = font->metrics();

            gb.setUtf8Text(str.data(), str.length());
            font->shape(gb);
            font->getTextMetrics(gb, tm);

            dx = tm.boundingBox.x1 - tm.boundingBox.x0;
            dy = 0.0;

            return true;
        }

        bool getCharPath(PSFontHandle fontHandle, const PSMatrix& ctm, const PSString& str, PSPath &outPSPath) const // override 
        {
            BLFont* font = (BLFont*)fontHandle->fSystemHandle;

            BLGlyphBuffer gb;
            gb.setUtf8Text(str.data(), str.length());
            font->shape(gb);

            const BLGlyphRun& grun = gb.glyphRun();

            BLPath glyphPath{};
            font->getGlyphRunOutlines(grun, glyphPath);

            // Now turn the BLPath into a PSPath
            //double h = fCanvas.height();
            PSMatrix tmat = ctm;
            tmat.scale(1, -1);

            bool success = convertBLPathToPSPath(glyphPath, tmat, outPSPath);

            return success;
        }

    };

} // namespace waavs
