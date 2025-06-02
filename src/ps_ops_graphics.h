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

    inline bool op_moveto(PSVirtualMachine& vm) {
        PSObject y, x;
        if (!vm.opStack().pop(y) || !vm.opStack().pop(x) || !x.isNumber() || !y.isNumber())
            return vm.error("typecheck: expected two numbers");

        vm.graphics()->moveto(x.asReal(), y.asReal());
        return true;
    }

    inline bool op_lineto(PSVirtualMachine& vm) {
        PSObject y, x;
        if (!vm.opStack().pop(y) || !vm.opStack().pop(x) || !x.isNumber() || !y.isNumber())
            return vm.error("typecheck: expected two numbers");

        vm.graphics()->lineto(x.asReal(), y.asReal());
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

        double startRad = startAngle * (3.141592653589793 / 180.0);
        double sweepRad = (endAngle - startAngle) * (3.141592653589793 / 180.0);

        vm.graphics()->arcTo(cx, cy, radius, startRad, sweepRad);

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
            return vm.error("typecheck: expected 4 numbers");

        vm.graphics()->setCMYK(c.asReal(), m.asReal(), y.asReal(), k.asReal());
        return true;
    }



    // Add other graphics operators here...

    static const PSOperatorFuncMap graphicsOps = {
        // Graphics state management
        { "gsave",       op_gsave },
        { "grestore",    op_grestore },

        // Drawing attributes
        { "setlinewidth", op_setlinewidth },
        { "setlinecap",   op_setlinecap },
        { "setlinejoin",  op_setlinejoin },

        // Path building
        { "newpath",     op_newpath },
        { "moveto",      op_moveto },
        { "lineto",      op_lineto },
		{ "arc",         op_arc },
        { "closepath",   op_closepath },
        { "stroke",      op_stroke },
		{ "fill",        op_fill },

        // Color setting
        { "setgray",       op_setgray },
        { "setrgbcolor",   op_setrgbcolor },
        { "setcmykcolor",  op_setcmykcolor }
    };


} // namespace waavs
