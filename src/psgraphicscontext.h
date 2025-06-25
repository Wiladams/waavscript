#pragma once

#include <memory>
#include <vector>
#include <cstdio>

#include "psgraphicstate.h"
#include "psmatrix.h"
#include "psimage.h"
#include "psfont.h"
#include "fontmonger.h"

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
        virtual void onShowPage() {
            //printf("onShowPage: show the current page\n", pageWidth, pageHeight);
        }

        virtual void showPage() {
            onShowPage();
            printf("showpage: flush and start new page (%gx%g)\n", pageWidth, pageHeight);
            newpath();            // Clear current path
        }

        virtual void erasePage() {
            printf("erasepage: clear page content (%gx%g)\n", pageWidth, pageHeight);
            newpath();            // Simulate clearing
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
            clip.reset();
            clip.moveto(0, 0);
            clip.lineto(pageWidth, 0);
            clip.lineto(pageWidth, pageHeight);
            clip.lineto(0, pageHeight);
            clip.close();
        }

        virtual void initGraphics() {
            reset();              // Clear state stack
            resetCTM();           // Identity CTM
            newpath();            // Clear current path
            setRGB(0, 0, 0);      // Default black color
            setLineWidth(1);      // Default line width
            initClipPath();      // Default clip path
        }


        // --- CTM operations ---
        virtual void setCTM(const PSMatrix& m) {
            currentState()->ctm = m;
        }

        virtual void concat(const PSMatrix& m) {
            currentState()->ctm.preMultiply(m);
        }


        virtual PSMatrix & getCTM()  {
            return currentState()->ctm;
        }

        virtual void resetCTM() {
            currentState()->ctm.reset();
        }

        virtual void translate(double tx, double ty) {
            currentState()->ctm.translate(tx, ty);
        }

        virtual void scale(double sx, double sy) {
            currentState()->ctm.scale(sx, sy);
        }

        virtual void rotate(double angle) {
            currentState()->ctm.rotate(angle, 0, 0);
        }

        virtual void transformPoint(double x, double y, double &outX, double &outY) {
            currentState()->ctm.transformPoint(x, y, outX, outY);
		}

        //virtual void itransformPoint(double x, double y, double &outX, double &outY) {
        //    currentState()->ctm.itransform(x, y, outX, outY);
		//}

        virtual void dtransformPoint(double x, double y, double &outX, double &outY) {
            currentState()->ctm.dtransform(x, y, outX, outY);
        }

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


        virtual bool newpath() {
            return currentPath().reset();
        }


        virtual bool closepath() {
            return currentPath().close();
        }

        // Font handling
        virtual bool findFont(const PSName & name, PSObject &outObj)
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
        }

        virtual PSFontHandle currentFont()
        {
            return currentState()->fCurrentFont.asFont();
        }

        virtual const PSObject& fontObject() const {
            return currentState()->fCurrentFont;
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
    };

} // namespace waavs
