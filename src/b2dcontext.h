// b2dcontext.h

#pragma once


#include <algorithm>


#include "blend2d_connect.h"
#include "ps_type_graphicscontext.h"



namespace waavs 
{

    struct PSTransformedLineSink
    {
        PathProgramBuilder& out;
        const PSMatrix& transform;

        bool onMoveTo(float x, float y) noexcept
        {
            double tx, ty;
            transform.transformPoint(x, y, tx, ty);
            return out.onMoveTo(static_cast<float>(tx), static_cast<float>(ty));
        }

        bool onLineTo(float x, float y) noexcept
        {
            double tx, ty;
            transform.transformPoint(x, y, tx, ty);
            return out.onLineTo(static_cast<float>(tx), static_cast<float>(ty));
        }

        // All curves must be flattened before they reach this sink.
        bool onQuadTo(float, float, float, float) noexcept
        {
            return false;
        }

        bool onCubicTo(float, float, float, float, float, float) noexcept
        {
            return false;
        }

        bool onArcTo(float, float, float, float, float, float, float) noexcept
        {
            return false;
        }

        bool onClose() noexcept
        {
            return out.onClose();
        }

        bool onEnd() noexcept
        {
            return out.onEnd();
        }
    };




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

            // write out to a file with a page number in the filename???
        }

        void erasePage() override {
            // Clear the canvas
            ctx.clear_all();
            ctx.flush(BLContextFlushFlags::BL_CONTEXT_FLUSH_SYNC);
        }



        // Painting - filling and stroking paths

        bool fillWithRule(PathFillRule rule) override
        {
            BLPath blPath;
            const bool clipped = isClippingEnabled() && currentState()->hasClip;

            if (clipped) {
                PathProgram prepared;

                if (!prepareClippedFillPath(currentPath(), rule, prepared))
                    return false;

                if (!convertPSPathToBLPath(prepared, blPath))
                    return false;
            }
            else {
                if (!convertPSPathToBLPath(currentPath().program(), blPath))
                    return false;
            }

            ctx.save();

            const BLRgba32 fillColor = convertPaint(currentState()->fillPaint);
            const BLFillRule blRule = clipped || rule == PathFillRule::NonZero ? BL_FILL_RULE_NON_ZERO : BL_FILL_RULE_EVEN_ODD;

            ctx.set_fill_rule(blRule);
            ctx.set_fill_style(fillColor);

            const BLResult err = ctx.fill_path(blPath);

            ctx.restore();

            if (err != BL_SUCCESS)
                return false;

            currentPath().reset();
            return true;
        }


        bool stroke() override
        {
            auto* state = currentState();

            BLPath blPath;
            if (!convertPSPathToBLPath(currentPath().program(), blPath))
                return false;

            const BLRgba32 strokeColor = convertPaint(state->strokePaint);

            double wx;
            double wy;
            getCTM().dtransform(state->lineWidth, 0.0, wx, wy);
            double lineWidth = std::hypot(wx, wy);

            const BLStrokeJoin join = convertLineJoin(state->lineJoin);
            const BLStrokeCap cap = static_cast<BLStrokeCap>(state->lineCap);

            const bool hairline = lineWidth <= 0.0;
            if (hairline)
                lineWidth = 1.0;

            // Preserve the original Blend2D stroke behavior when clipping
            // is not required.
            if (!isClippingEnabled() || !state->hasClip)
            {
                ctx.save();

                if (hairline)
                    ctx.set_stroke_transform_order(BL_STROKE_TRANSFORM_ORDER_BEFORE);

                ctx.set_stroke_style(strokeColor);
                ctx.set_stroke_width(lineWidth);
                ctx.set_stroke_caps(cap);
                ctx.set_stroke_join(join);
                ctx.set_stroke_miter_limit(state->miterLimit);

                const BLResult err = ctx.stroke_path(blPath);

                ctx.restore();

                if (err != BL_SUCCESS)
                    return false;

                currentPath().reset();
                return true;
            }

            // ------------------------------------------------------
            // Expand the stroke into a filled outline.

            BLStrokeOptions strokeOptions;
            strokeOptions.width = lineWidth;
            strokeOptions.join = join;
            strokeOptions.miter_limit = state->miterLimit;
            strokeOptions.set_caps(cap);

            BLApproximationOptions approximationOptions{};

            BLPath strokeOutline;

            if (strokeOutline.add_stroked_path(blPath, strokeOptions, approximationOptions) != BL_SUCCESS)
                return false;

            // ------------------------------------------------------
            // Convert the Blend2D stroke outline into PathProgram.

            PathProgram outlineProgram;

            if (!convertBLPathToPathProgram(strokeOutline, outlineProgram))
                return false;

            PSPath outlinePath;

            if (!outlinePath.replaceProgram(outlineProgram))
                return false;

            // ------------------------------------------------------
            // Intersect the expanded stroke with the effective clip.

            PathProgram clippedProgram;

            if (!prepareClippedFillPath(outlinePath, PathFillRule::NonZero, clippedProgram))
                return false;

            // ------------------------------------------------------
            // Render the resulting geometry as a fill, using the
            // original stroke color.

            BLPath clippedPath;

            if (!convertPSPathToBLPath(clippedProgram, clippedPath))
                return false;

            ctx.save();

            ctx.set_fill_rule(BL_FILL_RULE_NON_ZERO);
            ctx.set_fill_style(strokeColor);

            const BLResult err = ctx.fill_path(clippedPath);

            ctx.restore();

            if (err != BL_SUCCESS)
                return false;

            // PostScript stroke consumes the current path.
            currentPath().reset();

            return true;
        }


        /*
        bool stroke() override
        {
            BLPath blPath;

            if (!convertPSPathToBLPath(currentPath().program(), blPath))
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
        */


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


        bool fillGlyphPath(const PathProgram& path, const PSMatrix& transform) override
        {
            // Fast path: preserve the original Blend2D glyph rendering.
            if (!isClippingEnabled() || !currentState()->hasClip)
            {
                BLPath blPath;
                
                if (!convertPSPathToBLPath(path, blPath))
                    return false;

                ctx.save();

                ctx.set_fill_rule(BL_FILL_RULE_NON_ZERO);
                ctx.set_fill_style(convertPaint(currentState()->fillPaint));
                ctx.set_transform(blTransform(transform));

                const BLResult err = ctx.fill_path(blPath);

                ctx.restore();
                return err == BL_SUCCESS;
            }

            // The glyph transform maps glyph-local coordinates into
            // the same canonical space as the current clipping path.
            PathProgramBuilder builder;
            PSTransformedLineSink transformed{ builder, transform };
            PathFlattener<PSTransformedLineSink> flattener(transformed);

            // Estimate the maximum scaling of the affine transform.
            // This keeps the flattening tolerance in device-space units.
            const double sx = std::hypot(transform.m[0], transform.m[1]);
            const double sy = std::hypot(transform.m[2], transform.m[3]);
            const double scaleBound = sx + sy;

            flattener.opt.flatness = scaleBound > 0.0 ? currentState()->flatness / scaleBound : currentState()->flatness;
            flattener.opt.maxDepth = 16;

            if (!pathprogram_dispatch(path, flattener))
                return false;

            // Glyph programs may not contain OP_END.
            if (path.ops.empty() || path.ops.back() != OP_END) {
                if (!flattener.onEnd())
                    return false;
            }

            PSPath canonicalGlyph;

            if (!canonicalGlyph.replaceProgram(builder.prog))
                return false;

            PathProgram clipped;

            if (!prepareClippedFillPath(canonicalGlyph, PathFillRule::NonZero, clipped))
                return false;

            BLPath blPath;

            if (!convertPSPathToBLPath(clipped, blPath))
                return false;

            ctx.save();

            ctx.set_fill_rule(BL_FILL_RULE_NON_ZERO);
            ctx.set_fill_style(convertPaint(currentState()->fillPaint));

            // Do NOT set the glyph transform here.
            // All vertices have already been transformed into canonical space.
            const BLResult err = ctx.fill_path(blPath);

            ctx.restore();

            return err == BL_SUCCESS;
        }

        /*
        bool fillGlyphPath(const PathProgram& path, const PSMatrix& transform) override
        {
            BLPath blPath;
            if (!convertPSPathToBLPath(path, blPath))
                return false;

            BLMatrix2D glyphTransform = blTransform(transform);

            ctx.save();

            BLRgba32 fillColor = convertPaint(currentState()->fillPaint);

            ctx.set_fill_rule(BL_FILL_RULE_NON_ZERO);
            ctx.set_fill_style(fillColor);

            ctx.set_transform(glyphTransform);
            ctx.fill_path(blPath);

            ctx.restore();

            return true;
        }
        */

    };

} // namespace waavs
