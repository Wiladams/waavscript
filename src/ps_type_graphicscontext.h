// ps_type_graphicscontext.h

#pragma once

#include <memory>
#include <vector>
#include <cstdio>

#include "ps_type_graphicstate.h"
#include "ps_type_matrix.h"
#include "ps_type_image.h"
#include "ps_type_font.h"
#include "pathprogram_sinks.h"
#include "pathprogram_intersection.h"


namespace waavs 
{

    struct PSResolvedGlyph
    {
        PSGlyphId glyphId{ kInvalidPSGlyphId };
        PSGlyphMetrics metrics{};
        const PathProgram* path{ nullptr };
    };

    class PSGraphicsContext {
        bool fClippingEnabled{ true };

    protected:
        PSGraphicsStack stateStack;
		double pageWidth = 612; // Default A4 width in points
		double pageHeight = 792; // Default A4 height in points

    public:
        virtual ~PSGraphicsContext() = default;

        virtual PSMatrix getDeviceDefaultMatrix() const {
            return PSMatrix(); // PSMatrix::makeIdentity(); // Default to identity matrix
        }

        bool isClippingEnabled() const noexcept { return fClippingEnabled; }
        void setClippingEnabled(bool enabled) noexcept { fClippingEnabled = enabled; }

        // --- State access ---
        PSGraphicsState* currentState() const { return stateStack.get(); }
        PSGraphicsStack& states() { return stateStack; }



        // Device information
        virtual void setPageSize(double w, double h) {
            pageWidth = w;
            pageHeight = h;
        }

        virtual void getPageSize(double& w, double& h) const {
            w = pageWidth;
            h = pageHeight;
        }

        virtual void reset() {
            stateStack.reset();
        }

        // Do whatever is needed to show the page
        virtual void showPage() {
            printf("showpage: flush and start new page (%gx%g)\n", pageWidth, pageHeight);
        }

        virtual void erasePage() {
            printf("erasepage: clear page content (%gx%g)\n", pageWidth, pageHeight);
        }

        // --- Stack operations ---
        virtual void gsave() {
            stateStack.gsave();
        }

        virtual void grestore() {
            stateStack.grestore();
        }

        virtual void initClipPath()
        {
            auto* state = currentState();

            if (!isClippingEnabled()) {
                state->fCurrentClipPath.reset();
                state->hasClip = false;
                return;
            }

            PSPath& clip = state->fCurrentClipPath;
            const PSMatrix& ctm = state->ctm;

            clip.reset();
            clip.moveto(ctm, 0, 0);
            clip.lineto(ctm, pageWidth, 0);
            clip.lineto(ctm, pageWidth, pageHeight);
            clip.lineto(ctm, 0, pageHeight);
            clip.close();

            state->hasClip = true;
        }

        virtual void initGraphics() {
            reset();              // Clear state stack
            setLineWidth(1);      // Default line width
            initClipPath();      // Default clip path
        }


        virtual PSMatrix & getCTM()  { return currentState()->ctm; }
        virtual const PSMatrix& getCTM() const { return currentState()->ctm; }


        // Handling paint
        virtual void setGray(double gray) {
            currentState()->strokePaint = PSPaint::fromGray(gray);
            currentState()->fillPaint = PSPaint::fromGray(gray);
        }

        virtual bool getCurrentRgb(double &r, double &g, double &b) const {
            PSPaint& paint = currentState()->strokePaint;
            if (paint.isRGB()) {
                r = paint.r;
                g = paint.g;
                b = paint.b;
                return true;
            }
            return false;
        }

        virtual void setRGB(double r, double g, double b) {
            currentState()->strokePaint = PSPaint::fromRGB(r, g, b);
            currentState()->fillPaint = PSPaint::fromRGB(r, g, b);
        }

        virtual void setCMYK(double c, double m, double y, double k) {
            currentState()->strokePaint = PSPaint::fromCMYK(c, m, y, k);
            currentState()->fillPaint = PSPaint::fromCMYK(c, m, y, k);
        }


        // --- Graphics state operators ---
        virtual void setLineWidth(double w) {
            currentState()->lineWidth = w;
        }

        virtual void setLineCap(PSLineCap cap) {
            currentState()->lineCap = cap;
        }

        virtual void setLineJoin(PSLineJoin join) {
            currentState()->lineJoin = join;
        }

        double getMiterLimit() { return currentState()->getMiterLimit(); }
        virtual void setMiterLimit(double limit) {currentState()->miterLimit = limit;}

        double getFlatness() { return currentState()->getFlatness(); }
        virtual void setFlatness(double f) {currentState()->flatness = f;}

        virtual void setDashPattern(const std::vector<double>& pattern, double offset) {
            currentState()->dashArray = pattern;
            currentState()->dashOffset = offset;
        }

        virtual void setDashPattern(std::vector<double>&& pattern, double offset) {
            currentState()->dashArray = std::move(pattern);
            currentState()->dashOffset = offset;
        }

        // --- Paint (color) ---
        virtual void setStrokePaint(const PSPaint& paint) {
            currentState()->strokePaint = paint;
        }

        virtual void setFillPaint(const PSPaint& paint) {
            currentState()->fillPaint = paint;
        }



        // --- Clip Management ---

        bool prepareClippedFillPath(const PSPath& source, PathFillRule fillRule, PathProgram& result) const
        {
            if (!isClippingEnabled()) 
            {
                result = source.program();
                return true;
            }

            const auto* state = currentState();

            // Convert the drawing path into closed, flattened fill geometry.
            PathProgram preparedDrawing;

            if (!flattenPathForFill(source.program(), preparedDrawing, state->flatness))
                return false;

            // Without an active clipping region, return the prepared path.
            if (!state->hasClip) {
                result = std::move(preparedDrawing);
                return true;
            }

            // The current clipping path already represents the effective clip.
            PathProgram preparedClip;

            if (!flattenPathForFill(state->fCurrentClipPath.program(), preparedClip, state->flatness))
                return false;

            // The effective clip is interpreted with nonzero winding.
            // The source path uses the fill rule supplied by the caller.
            return intersectPreparedPathPrograms(preparedDrawing, fillRule, preparedClip, PathFillRule::NonZero, result);
        }


        virtual bool clip(bool evenOdd = false)
        {
            auto* state = currentState();

            if (!isClippingEnabled())
            {
                state->fCurrentPath.reset();
                return true;
            }

            // Prepare the current drawing path as a filled region.
            PathProgram preparedPath;
            if (!flattenPathForFill(state->fCurrentPath.program(), preparedPath, state->flatness))
                return false;

            // Prepare the existing clipping region.
            PathProgram preparedClip;
            if (!flattenPathForFill(state->fCurrentClipPath.program(), preparedClip, state->flatness))
                return false;

            // The current clip is already a canonical filled region.
            // The incoming path may use nonzero or even-odd winding.
            const PathFillRule rule = evenOdd ? PathFillRule::EvenOdd : PathFillRule::NonZero;

            PathProgram intersection;
            if (!intersectPreparedPathPrograms(preparedClip, PathFillRule::NonZero, preparedPath, rule, intersection))
                return false;

            // Commit only after the complete intersection succeeds.
            PSPath newClip;
            if (!newClip.replaceProgram(intersection))
                return false;

            state->fCurrentClipPath = std::move(newClip);
            state->hasClip = true;

            // PostScript clip/eoclip perform an implicit newpath.
            state->fCurrentPath.reset();

            return true;
        }

        virtual PSPath getClipPath() const 
        {
            return currentState()->fCurrentClipPath; // Return by value (copy)
        }

        // --- Path construction  ---
        


        PSPath& currentPath() const { return currentState()->fCurrentPath; }
        void setCurrentPath(const PSPath& path) const {
            currentState()->fCurrentPath = path;
        }




        // Font handling
        //virtual bool findFont(PSVirtualMachine &vm, const PSName & name, PSObject &outObj)
        //{
        //    printf("PSGraphicsContext::findFont: Not implemented\n");
        //    return false;
        //}

        virtual bool setFont(PSDictionaryHandle fh)
        {
            if (!fh) {
                printf("setFont: null font handle\n");
                return false;
            }
            currentState()->fCurrentFont.resetFromDictionary(fh);
            
            return true;
        }

        virtual PSFontHandle currentFont()
        {
            if (!currentState()->fCurrentFont.isDictionary())
                return {};
            return getNativeFont(currentState()->fCurrentFont);
        }

        virtual const PSObject& fontObject() const {
            return currentState()->fCurrentFont;
        }


        // --- Drawing operations (stubs) ---
        virtual bool stroke() {
            printf("stroke: %zu path segments\n", currentPath().program().ops.size());
            return false;
        }

        virtual bool fillWithRule(PathFillRule rule)
        {
            return false;
        }

        virtual bool fill()
        {
            return fillWithRule(PathFillRule::NonZero);
        }

        virtual bool eofill()
        {
            return fillWithRule(PathFillRule::EvenOdd);
        }



        virtual bool image(PSImage& img, PSFileHandle src)
        {
            printf("PSGraphicsContext::image() [not implemented]\n");
            return false;
        }

        // Render an already-resolved glyph outline without modifying the
        // interpreter's current path.
        virtual bool fillGlyphPath(const PathProgram& path, const PSMatrix& transform)
        {
            printf("PSGraphicsContext::fillGlyphPath() [not implemented]\n");
            return false;
        }

        bool resolveGlyph(uint8_t code, PSResolvedGlyph& glyph) const
        {
            PSDictionaryHandle fontDict = currentState()->getFont();
            PSFontHandle font = getNativeFont(PSObject::fromDictionary(fontDict));
            
            if (!font || !font->valid())
                return false;

            PSObject encodingObj;
            if (!font->get(PSName("Encoding"), encodingObj) || !encodingObj.isArray())
                return false;

            PSArrayHandle encoding = encodingObj.asArray();
            if (!encoding || code >= encoding->size())
                return false;

            PSObject glyphNameObj;
            if (!encoding->get(code, glyphNameObj) || !glyphNameObj.isName())
                return false;

            PSGlyphId glyphId;
            if (!font->resource->glyphForName(glyphNameObj.asName(), glyphId)) {
                if (!font->resource->glyphForName(PSName(".notdef"), glyphId))
                    return false;
            }

            PSGlyphMetrics metrics;
            if (!font->resource->glyphMetrics(glyphId, metrics))
                return false;

            glyph.glyphId = glyphId;
            glyph.metrics = metrics;

            if (font->resource->glyphKind(glyphId) == PSGlyphKind::Outline) {
                glyph.path = font->resource->glyphPath(glyphId);
                if (!glyph.path)
                    return false;
            }

            return true;
        }


        virtual bool showText(const PSMatrix& ctm, const uint8_t* txt, size_t txtSize)
        {
            if (!txt && txtSize != 0) {
                std::printf("showText: invalid string buffer\n");
                return false;
            }

            PSDictionaryHandle fontDict = currentState()->getFont();
            PSFontHandle font = getNativeFont(PSObject::fromDictionary(fontDict));

            if (!font || !font->valid())
            {
                std::printf("showText FAIL: invalid font\n");
                return false;
            }

            double x, y;
            if (!currentPath().getCurrentPoint(ctm, x, y))
            {
                std::printf("showText FAIL: no current point\n");
                return false;
            }

            // Complete effective FontMatrix, including native font normalization.
            const PSMatrix fontMatrix = font->matrix();

            for (size_t i = 0; i < txtSize; ++i) {
                PSResolvedGlyph glyph;

                if (!resolveGlyph(txt[i], glyph)) {
                    std::printf("showText FAIL: resolveGlyph index=%zu char=0x%02X\n", i, static_cast<unsigned>(txt[i]));
                    return false;
                }

                if (!glyph.path) {
                    std::printf("showText FAIL: null glyph path index=%zu char=0x%02X\n", i, static_cast<unsigned>(txt[i]));
                    return false;
                }

                if (font->resource->glyphKind(glyph.glyphId) != PSGlyphKind::Outline) {
                    std::printf("showText FAIL: unexpected glyph kind index=%zu\n", i);
                    return false;
                }

                PSMatrix transform = ctm;
                transform.translate(x, y);
                transform.preMultiply(fontMatrix);

                if (!fillGlyphPath(*glyph.path, transform)) {
                    std::printf("showText FAIL: fillGlyphPath index=%zu char=0x%02X\n", i, static_cast<unsigned>(txt[i]));
                    return false;
                }

                double dx = glyph.metrics.advanceX;
                double dy = glyph.metrics.advanceY;

                fontMatrix.dtransform(dx, dy, dx, dy);

                //std::printf("glyph %zu: pos=(%g,%g) advance=(%g,%g)\n", i, x, y, dx, dy);

                x += dx;
                y += dy;
            }

            if (!currentPath().setCurrentPoint(ctm, x, y)) {
                std::printf("showText FAIL: setCurrentPoint x=%g y=%g\n", x, y);
                return false;
            }

            return true;
        }

        bool showText(const PSMatrix &ctm, const PSString& text) {
            return showText(ctm, text.data(), text.length());
        }
    };

} // namespace waavs
