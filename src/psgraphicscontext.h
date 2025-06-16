#pragma once

#include <memory>
#include <vector>
#include <cstdio>

#include "psgraphicstate.h"
#include "psmatrix.h"
#include "psimage.h"

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
        PSGraphicsState* currentState() { return stateStack.get(); }
        PSGraphicsStack& states() { return stateStack; }

		PSPath& currentPath()  { return currentState()->fCurrentPath; }
        bool currentPoint(double& x, double& y)  {
            if (currentPath().segments.empty()) {
                return false; // No current point
            }
			currentPath().getCurrentPoint(x, y);
            return true;
		}

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

        virtual void showPage() {
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


        virtual void initGraphics() {
            reset();              // Clear state stack
            resetCTM();           // Identity CTM
            newpath();            // Clear current path
            setRGB(0, 0, 0);      // Default black color
            setLineWidth(1);      // Default line width
        }


        // --- CTM operations ---
        virtual void setCTM(const PSMatrix& m) {
            currentState()->ctm = m;
        }

        virtual void concat(const PSMatrix& m) {
            currentState()->ctm.preMultiply(m);
        }


        virtual PSMatrix getCTM()  {
            return currentState()->ctm;
        }

        virtual void resetCTM() {
            currentState()->ctm = PSMatrix::identity();
        }

        virtual void translate(double tx, double ty) {
            currentState()->ctm.preMultiply(PSMatrix::translation(tx, ty));
        }

        virtual void scale(double sx, double sy) {
            currentState()->ctm.preMultiply(PSMatrix::scaling(sx, sy));
        }

        virtual void rotate(double angle) {
            currentState()->ctm.preMultiply(PSMatrix::rotation(angle));
        }

        virtual void transformPoint(double x, double y, double outX, double outY) {
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

        virtual void setMiterLimit(double limit) {
            currentState()->miterLimit = limit;
        }

        virtual void setFlatness(double f) {
            currentState()->flatness = f;
        }

        // --- Paint (color) ---
        virtual void setStrokePaint(const PSPaint& paint) {
            currentState()->strokePaint = paint;
        }

        virtual void setFillPaint(const PSPaint& paint) {
            currentState()->fillPaint = paint;
        }

        // --- Drawing operations (stubs) ---
        virtual void stroke() {
            printf("stroke: %zu path segments\n", currentPath().segments.size());
        }


        virtual void fill() {
            printf("PSGraphicsContext::fill() called [not implemented]\n");
        }

        virtual bool image(PSImage& img)
        {
            printf("PSGraphicsContext::image() [not implemented]\n");
            return false;
        }

        // --- Path construction  ---
        virtual void newpath() {
            currentPath().reset();
        }

        virtual void moveto(double x, double y) {
            currentPath().moveto(x, y);
        }

        virtual void lineto(double x, double y) {
            currentPath().lineto(x, y);
        }

        virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3) {
            currentPath().curveto(x1, y1, x2, y2, x3, y3);
        }

        virtual void arcTo(double cx, double cy, double radius, double angle1Deg, double angle2Deg) {
            double startRad = angle1Deg * (PSMatrix::PI / 180.0);
            double sweepRad = (angle2Deg - angle1Deg) * (PSMatrix::PI / 180.0);
            currentPath().arcTo(cx, cy, radius, startRad, sweepRad);
        }


        virtual void closepath() {
            currentPath().close();
        }

    };

} // namespace waavs
