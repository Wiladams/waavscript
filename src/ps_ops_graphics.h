#pragma once

#include "psvm.h"
#include "ps_type_graphicscontext.h"

namespace waavs {
	//static constexpr double G_PI = 3.14159265358979323846;

    inline bool op_gsave(PSVirtualMachine& vm) {
        vm.graphics()->gsave();
        return true;
    }

    inline bool op_grestore(PSVirtualMachine& vm) {
        vm.graphics()->grestore();
        return true;
    }

    inline bool op_setlinewidth(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        double width;
        if (!s.popReal(width))
            return vm.error("op_setlinewidth: typecheck");

        vm.graphics()->setLineWidth(width);
        return true;
    }

    inline bool op_setstrokeadjust(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto* grph = vm.graphics();

        if (s.empty())
            return vm.error("op_setstrokeadjust: stackunderflow");

        PSObject value;
        if (!s.pop(value) || !value.isBool())
            return vm.error("op_setstrokeadjust: typecheck");

        grph->currentState()->setStrokeAdjust(value.asBool());
        return true;
    }

    inline bool op_setoverprint(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto* grph = vm.graphics();

        if (s.empty())
            return vm.error("op_setoverprint: stackunderflow");

        PSObject value;
        if (!s.pop(value) || !value.isBool())
            return vm.error("op_setoverprint: typecheck");

        grph->currentState()->setOverprint(value.asBool());
        return true;
    }


    inline bool op_setlinecap(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();

        if (ostk.empty())
            return vm.error("op_setlinecap: stackunderflow");

        PSObject obj;
        if (!vm.opStack().pop(obj) || !obj.isIntegralNumber())
            return vm.error("op_setlinecap: typecheck; expected int");

        int val = obj.asInt();
        if (val < 0 || val > 2)
            return vm.error("rangecheck: linecap must be 0, 1, or 2");

        vm.graphics()->setLineCap(static_cast<PSLineCap>(val));
        return true;
    }

    inline bool op_setlinejoin(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_setlinejoin: stackunderflow");

        PSObject obj;
        if (!ostk.pop(obj) || !obj.isIntegralNumber())
            return vm.error("typecheck: expected int");

        int val = obj.asInt();
        if (val < 0 || val > 2)
            return vm.error("rangecheck: linejoin must be 0, 1, or 2");

        vm.graphics()->setLineJoin(static_cast<PSLineJoin>(val));
        return true;
    }

    // setmiterlimit
    inline bool op_setmiterlimit(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_setmiterlimit: stackunderflow");    

        PSObject limit;
        if (!ostk.pop(limit))
            return vm.error("op_setmiterlimit: stackunderflow");

        if (!limit.isNumber())
            return vm.error("typecheck");

        double value = limit.asReal(); // works for int or real

        if (value < 1.0)
            return vm.error("rangecheck");

        vm.graphics()->setMiterLimit(value);
        return true;
    }



    inline bool op_setdash(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        if (ostk.size() < 2)
            return vm.error("op_setdash: stackunderflow");

        PSObject offsetObj, patternObj;

        if (!ostk.pop(offsetObj) || !ostk.pop(patternObj))
            return vm.error("op_setdash: stackunderflow");

        if (!patternObj.isArray() || !offsetObj.isNumber())
            return vm.error("op_setdash: typecheck");

        const auto arr = patternObj.asArray();
        std::vector<double> dashPattern;
        for (const PSObject& elem : *arr) {
            if (!elem.isNumber())
                return vm.error("typecheck");

            double v = elem.asReal();
            if (v < 0.0)
                return vm.error("rangecheck");
            dashPattern.push_back(v);
        }

        double offset = offsetObj.asReal();
        if (offset < 0.0)
            return vm.error("rangecheck");

        vm.graphics()->setDashPattern(std::move(dashPattern), offset);

        return true;
    }

    //=================================================
    // 
    // drawing operations
    //=================================================

    inline bool op_clippath(PSVirtualMachine& vm) 
    {
        PSPath clipPath = vm.graphics()->getClipPath();  // Retrieves a copy
        vm.opStack().push(PSObject::fromPath(std::move(clipPath)));
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
    
    inline bool op_eofill(PSVirtualMachine& vm) {
        vm.graphics()->eofill();

        return true;
    }

    // ( x y width height -- )
    inline bool op_rectfill(PSVirtualMachine& vm) {
        auto& ctm = vm.graphics()->getCTM();

        PSObject hObj, wObj, yObj, xObj;
        if (!vm.opStack().pop(hObj)) return vm.error("rectfill: missing height");
        if (!vm.opStack().pop(wObj)) return vm.error("rectfill: missing width");
        if (!vm.opStack().pop(yObj)) return vm.error("rectfill: missing y");
        if (!vm.opStack().pop(xObj)) return vm.error("rectfill: missing x");

        if (!hObj.isNumber() || !wObj.isNumber() || !yObj.isNumber() || !xObj.isNumber())
            return vm.error("rectfill: all operands must be numbers");

        double x = xObj.asReal();
        double y = yObj.asReal();
        double w = wObj.asReal();
        double h = hObj.asReal();

        auto& path = vm.graphics()->currentPath();
        path.moveto(ctm, x, y);
        path.lineto(ctm, x + w, y);
        path.lineto(ctm, x + w, y + h);
        path.lineto(ctm, x, y + h);
        path.close();

        vm.graphics()->fill();

        return true;
    }

    // ( x y width height -- )
    inline bool op_rectstroke(PSVirtualMachine& vm) {
        auto& ctm = vm.graphics()->getCTM();

        PSObject hObj, wObj, yObj, xObj;
        if (!vm.opStack().pop(hObj)) return vm.error("rectstroke: missing height");
        if (!vm.opStack().pop(wObj)) return vm.error("rectstroke: missing width");
        if (!vm.opStack().pop(yObj)) return vm.error("rectstroke: missing y");
        if (!vm.opStack().pop(xObj)) return vm.error("rectstroke: missing x");

        if (!hObj.isNumber() || !wObj.isNumber() || !yObj.isNumber() || !xObj.isNumber())
            return vm.error("rectstroke: all operands must be numbers");

        double x = xObj.asReal();
        double y = yObj.asReal();
        double w = wObj.asReal();
        double h = hObj.asReal();

        auto& path = vm.graphics()->currentPath();
        path.moveto(ctm, x, y);
        path.lineto(ctm, x + w, y);
        path.lineto(ctm, x + w, y + h);
        path.lineto(ctm, x, y + h);
        path.close();


        vm.graphics()->stroke();

        return true;
    }

    // Color setting operations
    inline bool op_setgray(PSVirtualMachine& vm) {
        PSObject obj;
        if (!vm.opStack().pop(obj) || !obj.isNumber())
            return vm.error("typecheck: expected number");

        const double gv = std::clamp(obj.asReal(), 0.0, 1.0);

        vm.graphics()->setGray(gv);

        return true;
    }

    inline bool op_currentrgbcolor(PSVirtualMachine& vm) {
        double r, g, b;
        if (!vm.graphics()->getCurrentRgb(r, g, b))
            return vm.error("currentrgbcolor: no current color set");
        vm.opStack().push(PSObject::fromReal(r));
        vm.opStack().push(PSObject::fromReal(g));
        vm.opStack().push(PSObject::fromReal(b));

        return true;
    }

    inline bool op_setrgbcolor(PSVirtualMachine& vm)
    {
        PSObject b, g, r;

        if (!vm.opStack().pop(b) || !vm.opStack().pop(g) || !vm.opStack().pop(r))
            return vm.error("op_setrgbcolor: stackunderflow");

        if (!r.isNumber() || !g.isNumber() || !b.isNumber())
            return vm.error("op_setrgbcolor: typecheck; expected 3 numbers");

        const double rv = std::clamp(r.asReal(), 0.0, 1.0);
        const double gv = std::clamp(g.asReal(), 0.0, 1.0);
        const double bv = std::clamp(b.asReal(), 0.0, 1.0);

        vm.graphics()->setRGB(rv, gv, bv);
        return true;
    }

    inline bool op_setrgbacolor(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();
        auto* grph = vm.graphics();

        if (ostk.size() < 4)
            return vm.error("op_setrgbacolor: stackunderflow; expected 4 numbers");

        PSObject aObj, bObj, gObj, rObj;

        vm.opStack().pop(aObj);
        vm.opStack().pop(bObj);
        vm.opStack().pop(gObj);
        vm.opStack().pop(rObj);



        if (!rObj.isNumber() || !gObj.isNumber() || !bObj.isNumber() || !aObj.isNumber())
            return vm.error("op_setrgbacolor: typecheck; expected 4 numbers");

        double a = aObj.asReal();
        double b = bObj.asReal();
        double g = gObj.asReal();
        double r = rObj.asReal();

        grph->currentState()->strokePaint = PSPaint::fromRGBA(r, g, b, a);
        grph->currentState()->fillPaint = PSPaint::fromRGBA(r, g, b, a);


        return true;
    }

    inline bool op_setcmykcolor(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();

        if (ostk.size() < 4)
            return vm.error("op_setcmykcolor: stackunderflow; expected 4 numbers");

        double k, y, m, c;

        if (!ostk.popReal(k) || !ostk.popReal(y) || !ostk.popReal(m) || !ostk.popReal(c))
            return vm.error("op_setcmykcolor: typecheck");

        //if (!vm.opStack().pop(k) || !vm.opStack().pop(y) || !vm.opStack().pop(m) || !vm.opStack().pop(c) ||
        //    !c.isNumber() || !m.isNumber() || !y.isNumber() || !k.isNumber())
        //    return vm.error("op_setcmykcolor:typecheck: expected 4 numbers");

        vm.graphics()->setCMYK(c, m, y, k);

        return true;
    }

    // Reference implementation of sethsbcolor
    bool op_sethsbcolor(PSVirtualMachine& vm)
    {
        PSObject bObj, sObj, hObj;
        if (!vm.opStack().pop(bObj) || !vm.opStack().pop(sObj) || !vm.opStack().pop(hObj))
            return vm.error("sethsbcolor: stack underflow");

        if (!hObj.isNumber() || !sObj.isNumber() || !bObj.isNumber())
            return vm.error("sethsbcolor: all operands must be real");

        double h = hObj.asReal();
        double s = sObj.asReal();
        double v = bObj.asReal();

        if (h < 0.0) h = 0.0;
        if (s < 0.0) s = 0.0;
        if (v < 0.0) v = 0.0;
        if (h > 1.0) h = 1.0;
        if (s > 1.0) s = 1.0;
        if (v > 1.0) v = 1.0;

        double r, g, b;

        if (s == 0.0) {
            // Grayscale
            r = g = b = v;
        }
        else {
            h *= 6.0; // Scale hue to [0,6)
            int i = static_cast<int>(std::floor(h));
            double f = h - i;
            double p = v * (1.0 - s);
            double q = v * (1.0 - s * f);
            double t = v * (1.0 - s * (1.0 - f));

            switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
            default: r = g = b = 0.0; break; // unreachable
            }
        }

        vm.graphics()->setRGB(r, g, b);
        return true;
    }

    inline bool op_image(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 5)
            return vm.error("op_image: stackunderflow");

        PSObject sourceObj;
        PSObject matrixObj;
        int32_t bpc;
        int32_t height;
        int32_t width;

        s.pop(sourceObj);
        s.pop(matrixObj);

        PSMatrix matrix;
        if (!extractMatrix(matrixObj, matrix))
            return vm.error("op_image: typecheck; expected matrix");

        if (!s.popInt(bpc) || !s.popInt(height) || !s.popInt(width))
            return vm.error("op_image: typecheck; width, height, and bpc must be integers");

        if (width <= 0 || height <= 0)
            return vm.error("op_image: rangecheck; invalid image dimensions");

        if (bpc != 8)
            return vm.error("op_image: rangecheck; only 8-bit grayscale supported");

        if (sourceObj.isArray() && sourceObj.isExecutable())
            return vm.scheduleImage(width, height, bpc, matrixObj, sourceObj);

        if (sourceObj.isFile())
        {
            PSImage img;
            img.width = width;
            img.height = height;
            img.bitsPerComponent = bpc;
            img.transform = matrix;

            return vm.graphics()->image(img, sourceObj.asFile());
        }

        return vm.error("op_image: typecheck; expected procedure or file");
    }

    bool op_setscreen(PSVirtualMachine& vm)
    {
        PSObject proc, angle, freq;

        // Pop in reverse order: proc, angle, freq
        if (!vm.opStack().pop(proc) || !proc.isExecutable()) {
            return vm.error("op_setscreen: invalidOperand");
        }

        if (!vm.opStack().pop(angle) || !angle.isNumber()) {
            return vm.error("op_setscreen: invalidOperand()");
        }

        if (!vm.opStack().pop(freq) || !freq.isNumber()) {
            return vm.error("op_setscreen: invalidOperand()");
        }

        // At this point, we have valid frequency, angle, and proc.
        // Stub: Do nothing for now.
        return true;
    }



    // Add other graphics operators here...
    inline const PSOperatorFuncMap& getGraphicsOps() {
        static const PSOperatorFuncMap table = {
            // Graphics state management
            { "gsave",         op_gsave },
            { "grestore",      op_grestore },

            // Color attributes
            { "setgray",       op_setgray },
            { "setrgbcolor",   op_setrgbcolor },
            { "setrgbacolor",  op_setrgbacolor },
            { "setcmykcolor",  op_setcmykcolor },
            { "sethsbcolor",   op_sethsbcolor },
            { "currentrgbcolor", op_currentrgbcolor },
            
            // Drawing attributes
            { "setlinewidth",  op_setlinewidth },
            { "setlinecap",    op_setlinecap },
            { "setlinejoin",   op_setlinejoin },
            { "setmiterlimit", op_setmiterlimit },
            { "setdash",       op_setdash },

            // Path operations
            { "clippath",      op_clippath },

            { "rectfill",      op_rectfill },
			{ "rectstroke",    op_rectstroke },


            // Path rendering
            { "stroke",        op_stroke },
            { "fill",          op_fill },
            { "eofill",        op_eofill },

            // Images
            {"image", op_image },

            {"setscreen", op_setscreen },

            // Future state operations
            {"setstrokeadjust", op_setstrokeadjust },
            {"setoverprint", op_setoverprint },
        };
        return table;
    }



} // namespace waavs
