#pragma once

#include "psvm.h"
#include "psgraphicscontext.h"
#include "psmatrix.h"

namespace waavs 
{
    static void flattenCubicBezier(
        double x0, double y0,
        double x1, double y1,
        double x2, double y2,
        double x3, double y3,
        double flatness,
        PSPath& path,
        const PSMatrix& ctm)
    {
        auto isFlat = [&](double x0, double y0,
            double x1, double y1,
            double x2, double y2,
            double x3, double y3) -> bool
            {
                double dx = x3 - x0;
                double dy = y3 - y0;
                double len = std::sqrt(dx * dx + dy * dy);
                double d1 = std::abs((dx) * (y1 - y0) - (dy) * (x1 - x0));
                double d2 = std::abs((dx) * (y2 - y0) - (dy) * (x2 - x0));
                return (d1 <= flatness * len && d2 <= flatness * len);
            };

        std::function<void(double, double, double, double, double, double, double, double)> recurse;
        recurse = [&](double x0, double y0,
            double x1, double y1,
            double x2, double y2,
            double x3, double y3)
            {
                if (isFlat(x0, y0, x1, y1, x2, y2, x3, y3)) {
                    path.lineto(ctm, x3, y3);
                }
                else {
                    // Subdivide
                    double x01 = (x0 + x1) * 0.5, y01 = (y0 + y1) * 0.5;
                    double x12 = (x1 + x2) * 0.5, y12 = (y1 + y2) * 0.5;
                    double x23 = (x2 + x3) * 0.5, y23 = (y2 + y3) * 0.5;

                    double x012 = (x01 + x12) * 0.5, y012 = (y01 + y12) * 0.5;
                    double x123 = (x12 + x23) * 0.5, y123 = (y12 + y23) * 0.5;

                    double x0123 = (x012 + x123) * 0.5, y0123 = (y012 + y123) * 0.5;

                    recurse(x0, y0, x01, y01, x012, y012, x0123, y0123);
                    recurse(x0123, y0123, x123, y123, x23, y23, x3, y3);
                }
            };

        recurse(x0, y0, x1, y1, x2, y2, x3, y3);
    }


    static void flattenPath(PSPath& src, double flatness)
    {
        using namespace waavs;

        PSPath dst;
        double cx = 0.0, cy = 0.0; // Current point
        double startX = 0.0, startY = 0.0;
        bool hasCP = false;

        for (const auto& seg : src.segments)
        {
            switch (seg.command)
            {
            case PSPathCommand::MoveTo:
                dst.moveto(seg.fTransform, seg.x1, seg.y1);
                cx = startX = seg.x1;
                cy = startY = seg.y1;
                hasCP = true;
                break;

            case PSPathCommand::LineTo:
                dst.lineto(seg.fTransform, seg.x1, seg.y1);
                cx = seg.x1;
                cy = seg.y1;
                hasCP = true;
                break;

            case PSPathCommand::ClosePath:
                dst.close();
                cx = startX;
                cy = startY;
                hasCP = true;
                break;

            case PSPathCommand::CurveTo:
                if (hasCP) {
                    flattenCubicBezier(cx, cy,
                        seg.x1, seg.y1,
                        seg.x2, seg.y2,
                        seg.x3, seg.y3,
                        flatness, dst, seg.fTransform);

                    cx = seg.x3;
                    cy = seg.y3;
                }
                break;

            //case PSPathCommand::Arc:
            //case PSPathCommand::ArcCCW:

            case PSPathCommand::EllipticArc:
                // Preserve unflattened for now — you can call a separate flattenArc if needed
                dst.segments.push_back(seg);
                cx = src.fCurrentX;
                cy = src.fCurrentY;
                hasCP = true;
                break;

            default:
                break;
            }
        }

        src = std::move(dst);
    }

    // Turn a portion of an arc into a cubic bezier representation
    static inline void emitArcSegmentAsBezier(PSPath& out, double cx, double cy, double r, double t0, double t1, const PSMatrix& ctm) {
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

        //out.curveto(ctm, x1, y1, x2, y2, x3, y3);
        flattenCubicBezier(x0, y0, x1, y1, x2, y2, x3, y3, 0.01, out, ctm);
    }

    // setflat
    // 
    inline bool op_setflat(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();

        if (ostk.empty())
            return vm.error("op_setflat: stackunderflow");

        double flatness = 0.0;
        if (!ostk.popReal(flatness))
            return vm.error("typecheck; expected number");

        if (flatness < 0.0)
            return vm.error("rangecheck");

        grph->setFlatness(flatness);
        return true;
    }

    // currentflat
    // Returns the current flatness setting
    inline bool op_currentflat(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();

        double flatness = vm.graphics()->getFlatness();
        ostk.pushReal(flatness);

        return true;
    }

    inline bool op_newpath(PSVirtualMachine& vm) {
        auto& path = vm.graphics()->currentPath();
        path.reset();

        return true;
    }

    inline bool op_currentpoint(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        double x = 0.0, y = 0.0;
        if (!path.getCurrentPoint(x, y))
            return vm.error("op_currentpoint:currentpoint, none available");

        return ostk.pushReal(x) && ostk.pushReal(y);
    }

    inline bool op_moveto(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        if (ostk.size() < 2)
            return vm.error("op_moveto: stackunderflow; expected two numbers");

        double x0, y0;
        if (!ostk.popReal(y0) || !ostk.popReal(x0))
            return vm.error("op_moveto:typecheck; expected two numbers");

        if (!path.moveto(ctm, x0, y0))
            return vm.error("op_moveto: path.moveto() error");

        return true;
    }

    inline bool op_rmoveto(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        PSObject objdy, objdx;

        if (!ostk.pop(objdy) || !ostk.pop(objdx) || !objdx.isNumber() || !objdy.isNumber())
            return vm.error("op_moveto:typecheck; expected two numbers");


        double x0{ 0 }, y0{ 0 };
        if (!path.getCurrentPoint(x0, y0))
            return vm.error("op_rmoveto:nocurrentpoint");

        double dx = objdx.asReal();
        double dy = objdy.asReal();

        return path.moveto(ctm, x0 + dx, y0 + dy);
    }

    inline bool op_lineto(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        if (ostk.size() < 2)
            return vm.error("op_lineto: stackunderflow; expected two numbers");

        double x, y;
        if (!ostk.popReal(y) || !ostk.popReal(x))
            return vm.error("op_lineto:typecheck; expected two numbers");

        return path.lineto(ctm, x, y);
    }

    inline bool op_rlineto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        PSObject dyObj, dxObj;
        if (!s.pop(dyObj) || !s.pop(dxObj) || !dxObj.isNumber() || !dyObj.isNumber())
            return vm.error("op_rlineto:typecheck; expected two numbers");

        double x0, y0;
        if (!path.getCurrentPoint(x0, y0)) 
            return vm.error("op_rlineto:nocurrentpoint");

        double dx = dxObj.asReal();
        double dy = dyObj.asReal();

        return path.lineto(ctm, x0 + dx, y0 + dy);
    }

    // convenience for creating a rectangle path
    // x y width height -- path
    //
    inline bool op_rectpath(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        if (ostk.size() < 4)
            return vm.error("rectpath: stackunderflow");

        double x;
        double y;
        double w;
        double h;

        if (!ostk.popReal(h) || !ostk.popReal(w) ||
            !ostk.popReal(y) || !ostk.popReal(x))
            return vm.error("op_rectpath: typecheck; expected four numbers");


        return path.moveto(ctm, x, y)
            && path.lineto(ctm, x + w, y)
            && path.lineto(ctm, x + w, y + h)
            && path.lineto(ctm, x, y + h)
            && path.lineto(ctm, x, y); // Optional final lineto to starting point (not closepath)
    }

    static bool emitArc(PSPath& path,
        const PSMatrix& ctm,
        double cx, double cy,
        double radius,
        double startDeg,
        double endDeg,
        bool clockwise)
    {
        if (radius < 0.0)
            return false; // rangecheck

        double startRad = startDeg * DEG_TO_RAD;
        double endRad = endDeg * DEG_TO_RAD;
        double sweep = endRad - startRad;

        if (clockwise) {
            if (sweep >= 0.0)
                sweep -= 2 * PI;
        }
        else {
            if (sweep <= 0.0)
                sweep += 2 * PI;
        }

        int steps = std::ceil(std::abs(sweep) / QUARTER_ARC);
        if (steps < 1) steps = 1;
        double delta = sweep / steps;

        double startX = cx + radius * std::cos(startRad);
        double startY = cy + radius * std::sin(startRad);

        // Ensure valid start point
        if (path.hasCurrentPoint())
        {
            // draw line segment from current point to our start point
            // if they are different
            double curX, curY;
            path.getCurrentPoint(curX, curY);
            constexpr double EPS = 1e-10;

            if (std::abs(curX-startX) > EPS || std::abs(curY-startY) > EPS) {
                path.lineto(ctm, startX, startY);
            }
        }else
        {
            // there is no current point, so we need to initiate things 
            // by moving to the start point
            path.moveto(ctm, startX, startY);
        }

        for (int i = 0; i < steps; ++i) {
            double t0 = startRad + i * delta;
            double t1 = t0 + delta;
            emitArcSegmentAsBezier(path, cx, cy, radius, t0, t1, ctm);
        }

        return true;
    }


    bool op_arc(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();
        auto& path = grph->currentPath();
        const PSMatrix& ctm = grph->getCTM();

        double radius, startDeg, endDeg, cy, cx;
        if (!ostk.popReal(endDeg)) return vm.error("op_arc: invalid angle2");
        if (!ostk.popReal(startDeg)) return vm.error("op_arc: invalid angle1");
        if (!ostk.popReal(radius)) return vm.error("op_arc: invalid radius");
        if (!ostk.popReal(cy)) return vm.error("op_arc: invalid y");
        if (!ostk.popReal(cx)) return vm.error("op_arc: invalid x");

        if (!emitArc(path, ctm, cx, cy, radius, startDeg, endDeg, false))
            return vm.error("op_arc: emitArc failed");

        return true;
    }


    // arcn - arc counter-clockwise
    static bool op_arcn(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();
        auto& path = grph->currentPath();
        const PSMatrix& ctm = grph->getCTM();

        double radius, startDeg, endDeg, cy, cx;
        if (!ostk.popReal(endDeg)) return vm.error("op_arcn: invalid angle2");
        if (!ostk.popReal(startDeg)) return vm.error("op_arcn: invalid angle1");
        if (!ostk.popReal(radius)) return vm.error("op_arcn: invalid radius");
        if (!ostk.popReal(cy)) return vm.error("op_arcn: invalid y");
        if (!ostk.popReal(cx)) return vm.error("op_arcn: invalid x");

        if (!emitArc(path, ctm, cx, cy, radius, startDeg, endDeg, true))
            return vm.error("op_arcn: emitArc failed");

        return true;
    }



    static inline bool op_arcto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 5)
            return vm.error("arcto: stackunderflow");

        double r, y, y2, x2, y1, x1;
        //PSObject rObj, x1Obj, y1Obj, x2Obj, y2Obj;
        if (!s.popReal(r) || !s.popReal(y2) || !s.popReal(x2) ||
            !s.popReal(y1) || !s.popReal(x1))
            return vm.error("arcto: typecheck; expected five numbers");

        double x0, y0;
        if (!path.getCurrentPoint(x0, y0))
            return vm.error("arcto: no currentpoint");

        // Transform all points
        //double x1, y1, x2, y2, r, dummy;
        //ctm.transformPoint(x1Obj.asReal(), y1Obj.asReal(), x1, y1);
        //ctm.transformPoint(x2Obj.asReal(), y2Obj.asReal(), x2, y2);
        //ctm.dtransform(rObj.asReal(), 0.0, r, dummy);  // only x-direction used for circle

        // Execute the arc and retrieve tangent points
        double xt1, yt1, xt2, yt2;
        if (!path.arcto(ctm, x0, y0, x1, y1, x2, y2, r, xt1, yt1, xt2, yt2))
            return vm.error("arcto: unable to compute arc");

        // Push the tangent points as per spec
        s.pushReal(xt1);
        s.pushReal(yt1);
        s.pushReal(xt2);
        s.pushReal(yt2);

        return true;
    }





    static inline bool op_curveto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        // Ensure 6 items are available
        if (s.size() < 6)
            return vm.error("curveto: stackunderflow");

        PSObject y3Obj, x3Obj, y2Obj, x2Obj, y1Obj, x1Obj;
        s.pop(y3Obj); s.pop(x3Obj);
        s.pop(y2Obj); s.pop(x2Obj);
        s.pop(y1Obj); s.pop(x1Obj);

        if (!x1Obj.isNumber() || !y1Obj.isNumber() ||
            !x2Obj.isNumber() || !y2Obj.isNumber() ||
            !x3Obj.isNumber() || !y3Obj.isNumber()) {
            return vm.error("curveto: typecheck");
        }

        // Validate there's a currentpoint
        if (!path.hasCurrentPoint())
            return vm.error("curveto: no currentpoint");

        double x1 = x1Obj.asReal();
        double y1 = y1Obj.asReal();
        double x2 = x2Obj.asReal();
        double y2 = y2Obj.asReal();
        double x3 = x3Obj.asReal();
        double y3 = y3Obj.asReal();

        return path.curveto(ctm, x1, y1, x2, y2, x3, y3);
    }


    static inline bool op_rcurveto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 6)
            return vm.error("rcurveto: stackunderflow");

        PSObject dy3Obj, dx3Obj, dy2Obj, dx2Obj, dy1Obj, dx1Obj;
        s.pop(dy3Obj); s.pop(dx3Obj);
        s.pop(dy2Obj); s.pop(dx2Obj);
        s.pop(dy1Obj); s.pop(dx1Obj);

        if (!dx1Obj.isNumber() || !dy1Obj.isNumber() ||
            !dx2Obj.isNumber() || !dy2Obj.isNumber() ||
            !dx3Obj.isNumber() || !dy3Obj.isNumber()) {
            return vm.error("rcurveto: typecheck");
        }

        double cx, cy;
        if (!path.getCurrentPoint(cx, cy))
            return vm.error("rcurveto: no currentpoint");

        double dx1 = dx1Obj.asReal(), dy1 = dy1Obj.asReal();
        double dx2 = dx2Obj.asReal(), dy2 = dy2Obj.asReal();
        double dx3 = dx3Obj.asReal(), dy3 = dy3Obj.asReal();


        double x1 = cx + dx1;
        double y1 = cy + dy1;
        double x2 = x1 + dx2;
        double y2 = y1 + dy2;
        double x3 = x2 + dx3;
        double y3 = y2 + dy3;

        return path.curveto(ctm, x1, y1, x2, y2, x3, y3);
    }


    static inline bool op_closepath(PSVirtualMachine& vm) {
        vm.graphics()->closepath();
        return true;
    }

    static inline bool op_pathbbox(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        PSPath path;

        PSObject topper;
        if (ostk.top(topper) && topper.isPath())
        {
            ostk.pop(topper); // Remove the path from the stack
            path = topper.asPath();
        }
        else {
            path = vm.graphics()->currentPath();
        }

        double minX, minY, maxX, maxY;
        if (!path.getBoundingBox(minX, minY, maxX, maxY)) {
            // Empty path — spec is vague here; use 0s or raise an error
            minX = minY = maxX = maxY = 0.0;
        }

        ostk.push(PSObject::fromReal(minX));
        ostk.push(PSObject::fromReal(minY));
        ostk.push(PSObject::fromReal(maxX));
        ostk.push(PSObject::fromReal(maxY));

        return true;
    }

    inline bool op_flattenpath(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();

        if (!grph) {
            vm.error("op_flattenpath: Graphics context is not available for flattenpath");
            return false;
        }

        PSPath &currentPath = grph->currentPath(); // Ensure we have a current path

        double flatness = grph->getFlatness();

        flattenPath(currentPath, flatness);

        return true;
    }

    bool op_pathforall(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();

        if (ostk.size() < 4) {
            return vm.error("op_pathforall: requires 4 procedures on the stack");
        }


        PSObject procClose, procCurve, procLine, procMove;

        // Pop in reverse order: closepath, curveto, lineto, moveto
        if (!vm.opStack().pop(procClose) || !procClose.isExecutable())
            return vm.error("op_pathforall: invalid operand (closepath proc)");

        if (!vm.opStack().pop(procCurve) || !procCurve.isExecutable())
            return vm.error("op_pathforall: invalid operand (curveto proc)");

        if (!vm.opStack().pop(procLine) || !procLine.isExecutable())
            return vm.error("op_pathforall: invalid operand (lineto proc)");

        if (!vm.opStack().pop(procMove) || !procMove.isExecutable())
            return vm.error("op_pathforall: invalid operand (moveto proc)");

        const PSPath& path = grph->currentPath();

        for (const PSPathSegment& seg : path.segments)
        {
            switch (seg.command)
            {
            case PSPathCommand::MoveTo:
                vm.opStack().pushReal(seg.x1);
                vm.opStack().pushReal(seg.y1);
                if (!vm.runProc(procMove))
                    return false;
                break;

            case PSPathCommand::LineTo:
                vm.opStack().pushReal(seg.x1);
                vm.opStack().pushReal(seg.y1);
                if (!vm.runProc(procLine))
                    return false;
                break;

            case PSPathCommand::CurveTo:
                vm.opStack().pushReal(seg.x1);
                vm.opStack().pushReal(seg.y1);
                vm.opStack().pushReal(seg.x2);
                vm.opStack().pushReal(seg.y2);
                vm.opStack().pushReal(seg.x3);
                vm.opStack().pushReal(seg.y3);
                if (!vm.runProc(procCurve))
                    return false;
                break;

            case PSPathCommand::ClosePath:
                if (!vm.runProc(procClose))
                    return false;
                break;

            default:
                printf("op_pathforall: skipping: %d\n", seg.command);
                // pathforall ignores arcs and non-standard segment types
                break;
            }
        }

        return true;
    }




    // Add other graphics operators here...
    inline const PSOperatorFuncMap& getPathOps() {
        static const PSOperatorFuncMap table = {
            
            // path construction attributes
            { "setflat",       op_setflat },
            { "currentflat",   op_currentflat },

            // path construction
            { "newpath",       op_newpath },
            { "currentpoint",  op_currentpoint },
            { "moveto",        op_moveto },
            { "rmoveto",       op_rmoveto },
            { "lineto",        op_lineto },
            { "rlineto",       op_rlineto },
            { "arc",           op_arc },
            { "arcn",          op_arcn },
            { "arcto",         op_arcto },
            { "rectpath",      op_rectpath },

            { "curveto",       op_curveto },
            { "rcurveto",      op_rcurveto },
            { "closepath",     op_closepath },

            // path management
            { "flattenpath",   op_flattenpath },
            { "pathbbox",      op_pathbbox },
            { "pathforall",     op_pathforall},
        };
        return table;
    }

}