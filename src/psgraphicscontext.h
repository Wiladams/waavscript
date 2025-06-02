#pragma once

#include <memory>
#include <vector>
#include <cstdio>

#include "psgraphicstate.h"
#include "psmatrix.h"
#include "pspath.h"


namespace waavs {

    class PSGraphicsContext {
    protected:
        PSGraphicsStack stateStack;
        PSPath currentPath;

    public:
        virtual ~PSGraphicsContext() = default;

        // --- State access ---
        PSGraphicsState* currentState() { return stateStack.get(); }
        PSGraphicsStack& states() { return stateStack; }

        // --- Stack operations ---
        virtual void gsave() {
            stateStack.gsave();
        }

        virtual void grestore() {
            stateStack.grestore();
        }

        virtual void reset() {
            stateStack.reset();
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
            printf("stroke: %zu path segments\n", currentPath.segments.size());
            currentPath.clear();
        }


        virtual void fill() {
            printf("PSGraphicsContext::fill() called [not implemented]\n");
        }



        // --- Path construction  ---
        virtual void newpath() {
            currentPath.clear();
        }

        virtual void moveto(double x, double y) {
            currentPath.moveto(x, y);
        }

        virtual void lineto(double x, double y) {
            currentPath.lineto(x, y);
        }

        virtual void curveto(double x1, double y1, double x2, double y2, double x3, double y3) {
            currentPath.curveto(x1, y1, x2, y2, x3, y3);
        }

        virtual void arcTo(double cx, double cy, double radius, double angle1Deg, double angle2Deg) {
            double startRad = angle1Deg * (PSMatrix::PI / 180.0);
            double sweepRad = (angle2Deg - angle1Deg) * (PSMatrix::PI / 180.0);
            currentPath.arcTo(cx, cy, radius, startRad, sweepRad);
        }


        virtual void closepath() {
            currentPath.closepath();
        }

    };

} // namespace waavs
