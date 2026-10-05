// b2dcontext.h

#pragma once

#pragma comment(lib, "blend2d.lib")

#include <algorithm>
#include <blend2d/blend2d.h>

#include "ps_type_graphicscontext.h"
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


    static inline BLRgba32 convertPaint(const PSPaint& p)  
    {
        switch (p.kind) {
            case PSPaintKind::GRAY:
                return BLRgba32(uint8_t(p.gray * 255), uint8_t(p.gray * 255), uint8_t(p.gray * 255), 255);

            case PSPaintKind::RGB:
                return BLRgba32(uint8_t(p.r * 255), uint8_t(p.g * 255), uint8_t(p.b * 255), uint8_t(p.a * 255));

            case PSPaintKind::CMYK: {
                double r = std::min(1.0, (1.0 - p.c) * (1.0 - p.k));
                double g = std::min(1.0, (1.0 - p.m) * (1.0 - p.k));
                double b = std::min(1.0, (1.0 - p.y) * (1.0 - p.k));

                return BLRgba32(uint8_t(r * 255), uint8_t(g * 255), uint8_t(b * 255), 255);
            }

            default:
                return BLRgba32(0, 0, 0, 255); // fallback to black
        }
    }

    struct BLPathSink
    {
        BLPath& path;

        bool onMoveTo(float x, float y) noexcept
        {
            path.move_to(x, y);
            return true;
        }

        bool onLineTo(float x, float y) noexcept
        {
            path.line_to(x, y);
            return true;
        }

        bool onQuadTo(float, float, float, float) noexcept
        {
            // PSPath should already be normalized to moveto/lineto/cubicto/close.
            assert(false && "BLPathSink: unexpected quadratic segment");
            return false;
        }

        bool onCubicTo(float x1, float y1, float x2, float y2, float x, float y) noexcept
        {
            path.cubic_to(x1, y1, x2, y2, x, y);
            return true;
        }

        bool onArcTo(float, float, float, float, float, float, float) noexcept
        {
            // PostScript arcs are normalized to cubic Beziers in PSPath.
            assert(false && "BLPathSink: unexpected arc segment");
            return false;
        }

        bool onClose() noexcept
        {
            path.close();
            return true;
        }

        bool onEnd() noexcept
        {
            return true;
        }
    };


    inline bool convertPSPathToBLPath(const PSPath& path, BLPath& out)
    {
        BLPathSink sink{ out };
        return pathprogram_dispatch(path.program(), sink);
    }

/*
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

        out.cubic_to(tx1, ty1, tx2, ty2, tx3, ty3);
    }
    */

    /*
    bool convertPSPathToBLPath(const PSPath &path, BLPath& out) {
        static constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
        static constexpr double QUARTER_ARC = 3.14159265358979323846 / 2.0;

        for (const auto& seg : path.segments) {
            switch (seg.command) {
            case PSPathCommand::MoveTo: {
                double tx, ty;
                seg.fTransform.transformPoint(seg.x1, seg.y1, tx, ty);
                out.move_to(tx, ty);
            }

            break;

            case PSPathCommand::LineTo: {
                double tx, ty;
                seg.fTransform.transformPoint(seg.x1, seg.y1, tx, ty);

                out.line_to(tx, ty);
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

                out.cubic_to(tx1, ty1, tx2, ty2, tx3, ty3);
            break;


            case PSPathCommand::EllipticArc: {
                double x1 = seg.x2;
                double y1 = seg.y2;
                double r = seg.x1; // Radius
                bool sweepFlag = seg.y1>0.0 ? true : false;
               
                out.elliptic_arc_to(r, r, 0.0, false, sweepFlag, x1, y1);
                break;
            }

            case PSPathCommand::ClosePath:
                out.close();
                break;
            }
        }

        return true;
    }
    */


    inline bool convertBLPathToPSPath(const BLPath& inPath, const PSMatrix& ctm, PSPath& outPath)
    {
        const uint8_t* cmds = inPath.command_data();
        const BLPoint* pts = inPath.vertex_data();
        const size_t count = inPath.size();

        double curX = 0.0;
        double curY = 0.0;
        double startX = 0.0;
        double startY = 0.0;
        bool hasCurrentPoint = false;

        for (size_t i = 0; i < count; ++i)
        {
            const BLPathCmd cmd = static_cast<BLPathCmd>(cmds[i]);

            switch (cmd)
            {
            case BLPathCmd::BL_PATH_CMD_MOVE:
                if (!outPath.moveto(ctm, pts[i].x, pts[i].y))
                    return false;

                curX = startX = pts[i].x;
                curY = startY = pts[i].y;
                hasCurrentPoint = true;
                break;

            case BLPathCmd::BL_PATH_CMD_ON:
                if (!hasCurrentPoint)
                    return false;

                if (!outPath.lineto(ctm, pts[i].x, pts[i].y))
                    return false;

                curX = pts[i].x;
                curY = pts[i].y;
                break;

            case BL_PATH_CMD_QUAD:
            {
                if (!hasCurrentPoint || i + 1 >= count)
                    return false;

                const BLPoint& p1 = pts[i];
                const BLPoint& p2 = pts[i + 1];

                const double c1x = curX + (2.0 / 3.0) * (p1.x - curX);
                const double c1y = curY + (2.0 / 3.0) * (p1.y - curY);
                const double c2x = p2.x + (2.0 / 3.0) * (p1.x - p2.x);
                const double c2y = p2.y + (2.0 / 3.0) * (p1.y - p2.y);

                if (!outPath.curveto(ctm, c1x, c1y, c2x, c2y, p2.x, p2.y))
                    return false;

                curX = p2.x;
                curY = p2.y;

                ++i;
                break;
            }

            case BLPathCmd::BL_PATH_CMD_CUBIC:
            {
                if (!hasCurrentPoint || i + 2 >= count)
                    return false;

                const BLPoint& p1 = pts[i];
                const BLPoint& p2 = pts[i + 1];
                const BLPoint& p3 = pts[i + 2];

                if (!outPath.curveto(ctm, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y))
                    return false;

                curX = p3.x;
                curY = p3.y;

                i += 2;
                break;
            }

            case BL_PATH_CMD_CLOSE:
                if (!outPath.close())
                    return false;

                curX = startX;
                curY = startY;
                break;

            default:
                return false;
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
            ctx.clear_all();
			
            ctx.set_fill_rule(BL_FILL_RULE_NON_ZERO); // Non-zero winding rule
            ctx.set_comp_op(BL_COMP_OP_SRC_OVER);
            ctx.set_global_alpha(1.0); // optional - opaque rendering
            ctx.fill_all(BLRgba32(0xff, 0xff, 0xff, 255)); // Fill with white background

			ctx.set_stroke_alpha(1.0); // optional - opaque stroke
            setRGB(0, 0, 0);


            // Flip coordinate system: origin to bottom-left, Y+ goes up
            double h = fCanvas.height();
            BLMatrix2D flipY = BLMatrix2D::make_scaling(1, -1);

            flipY.translate(0, -h);
            flipY.scale(2.77, 2.77);

            ctx.set_transform(flipY);
            ctx.user_to_meta();

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
            ctx.clear_all();
            ctx.flush(BLContextFlushFlags::BL_CONTEXT_FLUSH_SYNC);
        }

        // Font related methods
        bool findFont(PSVirtualMachine& vm, const PSName& faceName, PSObject& outObj) override
        {
            if (!vm.findResource(faceName, "Font", outObj))
                return vm.error("findFont: font resource not found", faceName.c_str());

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

            ctx.set_fill_rule(fillRule);
            ctx.set_fill_style(fillColor);

            ctx.fill_path(blPath);

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

            ctx.set_fill_rule(fillRule); // Set even-odd fill rule
            ctx.set_fill_style(fillColor);
            ctx.fill_path(blPath);

            currentPath().reset();

            ctx.restore(); // Restore to previous state

            return true;
        }

        bool stroke() override
        {
            BLPath blPath;

            if (!convertPSPathToBLPath(currentPath(), blPath))
                return false;

            ctx.save();

            BLRgba32 strokeColor = convertPaint(currentState()->strokePaint);

            double wx;
            double wy;
            getCTM().dtransform(currentState()->lineWidth, 0.0, wx, wy);
            double lineWidth = std::hypot(wx, wy);

            BLStrokeJoin join = convertLineJoin(currentState()->lineJoin);

            if (lineWidth <= 0.0) {
                // If the line width is zero or negative, we can choose a hairline
                // stroke
                ctx.set_stroke_transform_order(BL_STROKE_TRANSFORM_ORDER_BEFORE);
                lineWidth = 1.0; // Hairline stroke
            }

            ctx.set_stroke_style(strokeColor);
            ctx.set_stroke_width(lineWidth);
            ctx.set_stroke_caps(static_cast<BLStrokeCap>(currentState()->lineCap));
            ctx.set_stroke_join(join);
            ctx.set_stroke_miter_limit(currentState()->miterLimit);
            ctx.stroke_path(blPath);

            currentPath().reset();

            ctx.restore();

            return true;
        }


        bool image(PSImage& img, PSFileHandle src) override
        {
            // Create a BLImage object
            BLImage blimg(img.width, img.height, BLFormat::BL_FORMAT_PRGB32);
            BLImageData imgData;
            blimg.get_data(&imgData);

            // got pixel by pixel setting each value according to the grayscale
            // values in the PSImage
            for (int y = 0; y < img.height; ++y) {
                for (int x = 0; x < img.width; ++x) {
                    uint8_t grayValue = 0;
                    if (!src->readByte(grayValue))
                        return false;
                    //uint8_t grayValue = img.data[y * img.width + x];
                    uint32_t pixelValue = (255 << 24) | (grayValue << 16) | (grayValue << 8) | grayValue;
                    ((uint32_t*)(imgData.pixel_data))[(img.height-1-y)*img.width+x] = pixelValue;
                }
            }

            //BLMatrix2D blTrans = blTransform(img.transform);
            ctx.save();
            //double cx, cy;
            //currentState()->fCurrentPath.getCurrentPoint(cx, cy);

            ctx.blit_image(BLPoint(0, 0), blimg);
            ctx.restore();

            return true;
        }

        void strokeAxis(BLRgba32 xColor, BLRgba32 yColor)
        {
            // Draw postscript axes as they currently sit
            BLPath xAxisPath, yAxisPath;
            xAxisPath.move_to(0, 0);
            xAxisPath.line_to(300, 0);
            yAxisPath.move_to(0, 0);
            yAxisPath.line_to(0, 300);

            ctx.stroke_path(xAxisPath, xColor);
            ctx.stroke_path(yAxisPath, yColor);

        }

        bool showText(const PSMatrix& ctm, const uint8_t *txt, const size_t txtSize) override
        {
            auto fontHandle = currentState()->getFont();
            BLFont* font = static_cast<BLFont*>(fontHandle->fSystemHandle);

            PSPath& path = currentState()->fCurrentPath;

            double x;
            double y;
            if (!path.getCurrentPoint(ctm, x, y))
                return false;

            double dx;
            double dy;
            if (!getStringWidth(fontHandle, txt, txtSize, dx, dy))
                return false;

            ctx.save();

            // Text position is expressed in current PostScript user space.
            BLMatrix2D bctm(ctm.m[0], ctm.m[1], ctm.m[2], ctm.m[3], ctm.m[4], ctm.m[5]);
            ctx.apply_transform(bctm);
            ctx.translate(x, y);

            // Blend2D glyph outlines use the opposite Y orientation.
            ctx.scale(1.0, -1.0);

            ctx.set_fill_style(convertPaint(currentState()->fillPaint));
            ctx.fill_utf8_text(BLPoint(0, 0), *font, reinterpret_cast<const char*>(txt), txtSize);

            ctx.restore();

            // show advances the PostScript current point.
            if (!path.setCurrentPoint(ctm, x + dx, y + dy))
                return false;

            return true;
        }


        bool getStringWidth(PSFontHandle fontHandle, const uint8_t *txt, const size_t txtSize, double& dx, double& dy) const
        {
            dx = 0.0;
            dy = 0.0;

            BLFont* font = (BLFont*)fontHandle->fSystemHandle;

            BLTextMetrics tm;
            BLGlyphBuffer gb;

            gb.set_utf8_text(txt, txtSize);
            font->shape(gb);
            font->get_text_metrics(gb, tm);

            dx = tm.advance.x;
            dy = tm.advance.y;

            return true;
        }

        bool getStringWidth(PSFontHandle fontHandle, const PSString& str, double& dx, double& dy) const override
        {
            return getStringWidth(fontHandle, str.data(), str.length(), dx, dy);
        }

        bool getCharPath(PSFontHandle fontHandle, const PSMatrix& ctm, const PSString& str, PSPath &outPSPath) const // override 
        {
            BLFont* font = (BLFont*)fontHandle->fSystemHandle;

            BLGlyphBuffer gb;
            gb.set_utf8_text(str.data(), str.length());
            font->shape(gb);

            const BLGlyphRun& grun = gb.glyph_run();

            BLPath glyphPath{};
            font->get_glyph_run_outlines(grun, glyphPath);

            // Now turn the BLPath into a PSPath
            //double h = fCanvas.height();
            PSMatrix tmat = ctm;
            tmat.scale(1, -1);

            bool success = convertBLPathToPSPath(glyphPath, tmat, outPSPath);

            return success;
        }

    };

} // namespace waavs
