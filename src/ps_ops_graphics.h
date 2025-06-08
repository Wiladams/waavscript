#pragma once

#include "psvm.h"
#include "psgraphicscontext.h"

namespace waavs {
	static constexpr double G_PI = 3.14159265358979323846;

    inline bool op_gsave(PSVirtualMachine& vm) {
        vm.graphics()->gsave();
        return true;
    }

    inline bool op_grestore(PSVirtualMachine& vm) {
        vm.graphics()->grestore();
        return true;
    }

    inline bool op_setlinewidth(PSVirtualMachine& vm) {
        PSObject obj;
        if (!vm.opStack().pop(obj) || !obj.isNumber())
            return vm.error("typecheck: expected number");

        vm.graphics()->setLineWidth(obj.asReal());
        return true;
    }

    inline bool op_setlinecap(PSVirtualMachine& vm) {
        PSObject obj;
        if (!vm.opStack().pop(obj) || !obj.isInt())
            return vm.error("typecheck: expected int");

        int val = obj.asInt();
        if (val < 0 || val > 2)
            return vm.error("rangecheck: linecap must be 0, 1, or 2");

        vm.graphics()->setLineCap(static_cast<PSLineCap>(val));
        return true;
    }

    inline bool op_setlinejoin(PSVirtualMachine& vm) {
        PSObject obj;
        if (!vm.opStack().pop(obj) || !obj.isInt())
            return vm.error("typecheck: expected int");

        int val = obj.asInt();
        if (val < 0 || val > 2)
            return vm.error("rangecheck: linejoin must be 0, 1, or 2");

        vm.graphics()->setLineJoin(static_cast<PSLineJoin>(val));
        return true;
    }



    inline bool op_newpath(PSVirtualMachine& vm) {
        vm.graphics()->newpath();
        return true;
    }

    inline bool op_currentpoint(PSVirtualMachine& vm) {
        double x = 0.0, y = 0.0;

        if (!vm.graphics()->currentPoint(x, y))
            return vm.error("nocurrentpoint");

        return vm.opStack().push(PSObject::fromReal(x)) && vm.opStack().push(PSObject::fromReal(y));
    }

    inline bool op_moveto(PSVirtualMachine& vm) {
        PSObject y, x;
        if (!vm.opStack().pop(y) || !vm.opStack().pop(x) || !x.isNumber() || !y.isNumber())
            return vm.error("typecheck: expected two numbers");

        vm.graphics()->moveto(x.asReal(), y.asReal());
        return true;
    }

    inline bool op_rmoveto(PSVirtualMachine& vm) {
        auto& stack = vm.opStack();
        if (stack.size() < 2) return vm.error("rmoveto: stack underflow");

        PSObject dy, dx;
        stack.pop(dy);
        stack.pop(dx);
        if (!dx.isNumber() || !dy.isNumber()) return vm.error("rmoveto: expected two numbers");

        auto* g = vm.graphics();
        double x0{ 0 }, y0{ 0 };
        g->currentPoint(x0, y0);
        g->moveto(x0 + dx.asReal(), y0 + dy.asReal());
        return true;
    }

    inline bool op_lineto(PSVirtualMachine& vm) {
        PSObject y, x;
        if (!vm.opStack().pop(y) || !vm.opStack().pop(x) || !x.isNumber() || !y.isNumber())
            return vm.error("typecheck: expected two numbers");

        vm.graphics()->lineto(x.asReal(), y.asReal());
        return true;
    }

    inline bool op_rlineto(PSVirtualMachine& vm) {
        auto& stack = vm.opStack();
        if (stack.size() < 2) return vm.error("rlineto: stack underflow");

        PSObject dy, dx;
        stack.pop(dy);
        stack.pop(dx);
        if (!dx.isNumber() || !dy.isNumber()) return vm.error("rlineto: expected two numbers");

        auto* g = vm.graphics();
        double x0, y0;
        g->currentPoint(x0, y0);
        g->lineto(x0 + dx.asReal(), y0 + dy.asReal());
        return true;
    }

    inline bool op_arc(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 5) return vm.error("arc: stackunderflow");

        PSObject endAngleObj, startAngleObj, radiusObj, yObj, xObj;
        s.pop(endAngleObj);
        s.pop(startAngleObj);
        s.pop(radiusObj);
        s.pop(yObj);
        s.pop(xObj);

        if (!xObj.isNumber() || !yObj.isNumber() ||
            !radiusObj.isNumber() || !startAngleObj.isNumber() || !endAngleObj.isNumber())
            return vm.error("arc: typecheck");

        double cx = xObj.asReal();
        double cy = yObj.asReal();
        double radius = radiusObj.asReal();
        double startAngle = startAngleObj.asReal();
        double endAngle = endAngleObj.asReal();

        //double startRad = startAngle * (3.141592653589793 / 180.0);
        //double sweepRad = (endAngle - startAngle) * (3.141592653589793 / 180.0);

        //vm.graphics()->arcTo(cx, cy, radius, startRad, sweepRad);
        vm.graphics()->arcTo(cx, cy, radius, startAngle, endAngle);

        return true;
    }

    inline bool op_arcto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 5) return vm.error("arcto: stack underflow");

        PSObject rObj, y2Obj, x2Obj, y1Obj, x1Obj;
        s.pop(rObj); s.pop(y2Obj); s.pop(x2Obj); s.pop(y1Obj); s.pop(x1Obj);

        if (!x1Obj.isNumber() || !y1Obj.isNumber() ||
            !x2Obj.isNumber() || !y2Obj.isNumber() || !rObj.isNumber()) {
            return vm.error("arcto: expected five numbers");
        }

        double x1 = x1Obj.asReal();
        double y1 = y1Obj.asReal();
        double x2 = x2Obj.asReal();
        double y2 = y2Obj.asReal();
        double r = rObj.asReal();

        double x0, y0;
        if (!vm.graphics()->currentPoint(x0, y0))
            return vm.error("arcto: no current point");

        // Vectors
        double dx1 = x1 - x0, dy1 = y1 - y0;
        double dx2 = x2 - x1, dy2 = y2 - y1;

        double len1 = std::hypot(dx1, dy1);
        double len2 = std::hypot(dx2, dy2);

        if (len1 == 0.0 || len2 == 0.0 || r <= 0.0)
            return vm.error("arcto: degenerate input");

        dx1 /= len1; dy1 /= len1;
        dx2 /= len2; dy2 /= len2;

        double cosA = dx1 * dx2 + dy1 * dy2;
        double sinA = dx1 * dy2 - dy1 * dx2;

        double angle = std::atan2(sinA, cosA) / 2.0;
        double tanHalf = std::tan(angle);

        if (std::abs(tanHalf) < 1e-8)
            return vm.error("arcto: lines nearly collinear");

        double dist = r / tanHalf;

        if (dist > len1 || dist > len2)
            return vm.error("arcto: radius too large");

        // Points A and B on the lines before and after the corner
        double ax = x1 - dx1 * dist;
        double ay = y1 - dy1 * dist;
        double bx = x1 - dx2 * dist;
        double by = y1 - dy2 * dist;

        // Add line to arc start
        vm.graphics()->lineto(ax, ay);

        // Compute arc center
        double ux = dx1 + dx2;
        double uy = dy1 + dy2;
        double ulen = std::hypot(ux, uy);
        if (ulen == 0.0)
            return vm.error("arcto: undefined arc center");

        ux /= ulen; uy /= ulen;
        double cx = x1 - ux * r / std::sin(angle);
        double cy = y1 - uy * r / std::sin(angle);

        // Convert arc A -> B into cubic Bezier approximation
        double theta = angle * 2.0;
        double alpha = std::sin(theta) * (std::sqrt(4 + 3 * std::pow(std::tan(theta / 2), 2)) - 1) / 3;

        // Tangents at A and B
        double tx1 = ay - cy;
        double ty1 = cx - ax;
        double tlen1 = std::hypot(tx1, ty1);
        tx1 = tx1 / tlen1;
        ty1 = ty1 / tlen1;

        double tx2 = by - cy;
        double ty2 = cx - bx;
        double tlen2 = std::hypot(tx2, ty2);
        tx2 = tx2 / tlen2;
        ty2 = ty2 / tlen2;

        // Control points
        double cp1x = ax + alpha * tx1 * r;
        double cp1y = ay + alpha * ty1 * r;

        double cp2x = bx + alpha * tx2 * r;
        double cp2y = by + alpha * ty2 * r;

        // Emit curve
        vm.graphics()->curveto(cp1x, cp1y, cp2x, cp2y, bx, by);

        // Return the arc tangents
        return s.push(PSObject::fromReal(ax)) &&
            s.push(PSObject::fromReal(ay)) &&
            s.push(PSObject::fromReal(bx)) &&
            s.push(PSObject::fromReal(by));
    }


    inline bool op_arcn(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 5) return vm.error("arcn: stackunderflow");

        PSObject endAngleObj, startAngleObj, radiusObj, yObj, xObj;
        s.pop(endAngleObj); s.pop(startAngleObj);
        s.pop(radiusObj); s.pop(yObj); s.pop(xObj);

        if (!xObj.isNumber() || !yObj.isNumber() ||
            !radiusObj.isNumber() || !startAngleObj.isNumber() || !endAngleObj.isNumber())
            return vm.error("arcn: typecheck");

        double cx = xObj.asReal();
        double cy = yObj.asReal();
        double radius = radiusObj.asReal();
        double startAngle = startAngleObj.asReal();
        double endAngle = endAngleObj.asReal();

        // Note: arcTo uses clockwise sweep, so invert the angle direction
        double startRad = startAngle * (G_PI / 180.0);
        double sweepRad = (endAngle - startAngle) * (G_PI / 180.0); // sweep is negative for counter-clockwise

        vm.graphics()->arcTo(cx, cy, radius, startRad, sweepRad);

        return true;
    }

    inline bool op_rectpath(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 4) return vm.error("rectpath: stack underflow");

        PSObject hObj, wObj, yObj, xObj;
        s.pop(hObj); s.pop(wObj); s.pop(yObj); s.pop(xObj);

        if (!xObj.isNumber() || !yObj.isNumber() || !wObj.isNumber() || !hObj.isNumber())
            return vm.error("rectpath: expected four numbers");

        double x = xObj.asReal();
        double y = yObj.asReal();
        double w = wObj.asReal();
        double h = hObj.asReal();

        vm.graphics()->moveto(x, y);
        vm.graphics()->lineto(x + w, y);
        vm.graphics()->lineto(x + w, y + h);
        vm.graphics()->lineto(x, y + h);
        vm.graphics()->closepath();

        return true;
    }

    // ( x y width height -- )
    inline bool op_rectfill(PSVirtualMachine& vm) {
        PSObject h, w, y, x;
        if (!vm.opStack().pop(h)) return vm.error("rectfill: missing height");
        if (!vm.opStack().pop(w)) return vm.error("rectfill: missing width");
        if (!vm.opStack().pop(y)) return vm.error("rectfill: missing y");
        if (!vm.opStack().pop(x)) return vm.error("rectfill: missing x");

        if (!h.isNumber() || !w.isNumber() || !y.isNumber() || !x.isNumber())
            return vm.error("rectfill: all operands must be numbers");

        double dx = x.asReal();
        double dy = y.asReal();
        double dw = w.asReal();
        double dh = h.asReal();

        auto& path = vm.graphics()->currentPath();
        path.moveto(dx, dy);
        path.lineto(dx + dw, dy);
        path.lineto(dx + dw, dy + dh);
        path.lineto(dx, dy + dh);
        path.close();

        vm.graphics()->fill();

        return true;
    }


    inline bool op_curveto(PSVirtualMachine& vm) {
        auto& stack = vm.opStack();
        if (stack.size() < 6)
            return vm.error("curveto: stack underflow");

        PSObject y3, x3, y2, x2, y1, x1;
        stack.pop(y3); stack.pop(x3);
        stack.pop(y2); stack.pop(x2);
        stack.pop(y1); stack.pop(x1);

        if (!x1.isNumber() || !y1.isNumber() ||
            !x2.isNumber() || !y2.isNumber() ||
            !x3.isNumber() || !y3.isNumber()) {
            return vm.error("curveto: expected six numbers");
        }

        vm.graphics()->curveto(
            x1.asReal(), y1.asReal(),
            x2.asReal(), y2.asReal(),
            x3.asReal(), y3.asReal()
        );

        return true;
    }

    inline bool op_rcurveto(PSVirtualMachine& vm) {
        auto& stack = vm.opStack();
        if (stack.size() < 6) return vm.error("rcurveto: stack underflow");

        PSObject dy3, dx3, dy2, dx2, dy1, dx1;
        stack.pop(dy3); stack.pop(dx3);
        stack.pop(dy2); stack.pop(dx2);
        stack.pop(dy1); stack.pop(dx1);

        if (!dx1.isNumber() || !dy1.isNumber() ||
            !dx2.isNumber() || !dy2.isNumber() ||
            !dx3.isNumber() || !dy3.isNumber()) {
            return vm.error("rcurveto: expected six numbers");
        }

        auto* g = vm.graphics();
        double x0, y0;
        g->currentPoint(x0, y0);

        double x1 = x0 + dx1.asReal();
        double y1 = y0 + dy1.asReal();
        double x2 = x1 + dx2.asReal();
        double y2 = y1 + dy2.asReal();
        double x3 = x2 + dx3.asReal();
        double y3 = y2 + dy3.asReal();

        g->curveto(x1, y1, x2, y2, x3, y3);
        return true;
    }

    inline bool op_closepath(PSVirtualMachine& vm) {
        vm.graphics()->closepath();
        return true;
    }

    inline bool op_stroke(PSVirtualMachine& vm) {
        vm.graphics()->stroke();
        return true;
    }

    inline bool op_fill(PSVirtualMachine& vm) {
        vm.graphics()->fill();

        return true;
    }

    // Color setting operations
    inline bool op_setgray(PSVirtualMachine& vm) {
        PSObject obj;
        if (!vm.opStack().pop(obj) || !obj.isNumber())
            return vm.error("typecheck: expected number");

        vm.graphics()->setGray(obj.asReal());
        return true;
    }

    inline bool op_setrgbcolor(PSVirtualMachine& vm) {
        PSObject b, g, r;

		vm.opStack().pop(b);
		vm.opStack().pop(g);
		vm.opStack().pop(r);

        if (!r.isNumber() || !g.isNumber() || !b.isNumber())
            return vm.error("typecheck: expected 3 numbers");

        vm.graphics()->setRGB(r.asReal(), g.asReal(), b.asReal());
        return true;
    }

    inline bool op_setcmykcolor(PSVirtualMachine& vm) {
        PSObject k, y, m, c;
        if (!vm.opStack().pop(k) || !vm.opStack().pop(y) || !vm.opStack().pop(m) || !vm.opStack().pop(c) ||
            !c.isNumber() || !m.isNumber() || !y.isNumber() || !k.isNumber())
            return vm.error("op_setcmykcolor:typecheck: expected 4 numbers");

        vm.graphics()->setCMYK(c.asReal(), m.asReal(), y.asReal(), k.asReal());
        return true;
    }



    // Add other graphics operators here...
    inline const PSOperatorFuncMap& getGraphicsOps() {
        static const PSOperatorFuncMap table = {
            // Graphics state management
            { "gsave",         op_gsave },
            { "grestore",      op_grestore },

            // Color setting
            { "setgray",       op_setgray },
            { "setrgbcolor",   op_setrgbcolor },
            { "setcmykcolor",  op_setcmykcolor },

            // Drawing attributes
            { "setlinewidth",  op_setlinewidth },
            { "setlinecap",    op_setlinecap },
            { "setlinejoin",   op_setlinejoin },

            // Path operations
            { "newpath",       op_newpath },
            { "currentpoint",  op_currentpoint },
            { "moveto",        op_moveto },
            { "rmoveto",       op_rmoveto },
            { "lineto",        op_lineto },
            { "rlineto",       op_rlineto },
            { "arc",           op_arc },
            { "arcto",         op_arcto },
            { "arcn",          op_arcn },
            { "rectpath",      op_rectpath },
            { "rectfill",      op_rectfill },

            // Curves
            { "curveto",       op_curveto },
            { "rcurveto",      op_rcurveto },
            { "closepath",     op_closepath },

            // Path rendering
            { "stroke",        op_stroke },
            { "fill",          op_fill }
        };
        return table;
    }



} // namespace waavs
