#pragma once

#include <memory>
#include <vector>
#include <cstdio>

#include "psgraphicstate.h"
#include "psmatrix.h"
#include "psimage.h"
#include "psfont.h"
//#include "fontmonger.h"

namespace waavs {

    class PSGraphicsContext {
    protected:
        PSGraphicsStack stateStack;
		double pageWidth = 612; // Default A4 width in points
		double pageHeight = 792; // Default A4 height in points

    public:
        virtual ~PSGraphicsContext() = default;

        virtual PSMatrix getDeviceDefaultMatrix() const {
			return PSMatrix::identity(); // Default to identity matrix
        }

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

        virtual void initClipPath() {
            PSPath& clip = currentState()->fCurrentClipPath;
            auto& ctm = currentState()->ctm;

            clip.reset();
            clip.moveto(ctm, 0, 0);
            clip.lineto(ctm, pageWidth, 0);
            clip.lineto(ctm, pageWidth, pageHeight);
            clip.lineto(ctm, 0, pageHeight);
            clip.close();
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



        // --- Path construction  ---
        virtual PSPath getClipPath() const {
            return currentState()->fCurrentClipPath; // Return by value (copy)
        }

        PSPath& currentPath() const { return currentState()->fCurrentPath; }


        //virtual bool newpath() {
        //    return currentPath().reset();
        //}


        virtual bool closepath() {
            return currentPath().close();
        }

        // Font handling
        virtual bool findFont(PSVirtualMachine &vm, const PSName & name, PSObject &outObj)
        {
            printf("PSGraphicsContext::findFont: Not implemented\n");
            return false;
        }

        virtual bool setFont(PSFontHandle fh)
        {
            if (!fh) {
                printf("setFont: null font handle\n");
                return false;
            }
            currentState()->fCurrentFont.resetFromFont(fh);
            
            return true;
        }

        virtual PSFontHandle currentFont()
        {
            return currentState()->fCurrentFont.asFont();
        }

        virtual const PSObject& fontObject() const {
            return currentState()->fCurrentFont;
        }

        virtual bool getStringWidth(PSFontHandle fontHandle, const PSString &stringObj, double &dx, double &dy) const
        {
            dx = 0;
            dy = 0;

            printf("PSGraphicsContext::getStringWidth: Not implemented\n");

            return false;
        }

        virtual bool getCharPath(PSFontHandle fontHandle, const PSMatrix &ctm, const PSString& str, PSPath& outPSPath) const
        {
            printf("PSGraphicsContext::getCharPath: Not implemented\n");
            outPSPath.reset(); // Clear output path
            return false;
        }

        // --- Drawing operations (stubs) ---
        virtual bool stroke() {
            printf("stroke: %zu path segments\n", currentPath().segments.size());
            return false;
        }


        virtual bool fill() {
            printf("PSGraphicsContext::fill() called [not implemented]\n");
            return false;
        }

        virtual bool eofill() {
            printf("PSGraphicsContext::eofill() called [not implemented]\n");
            return false;
        }

        virtual bool image(PSImage& img)
        {
            printf("PSGraphicsContext::image() [not implemented]\n");
            return false;
        }

        virtual bool showText(const PSString& text) {
            printf("PSGraphicsContext::showText() [not implemented]\n");
            return false;
        }
    };

} // namespace waavs
