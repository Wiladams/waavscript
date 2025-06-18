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

    // setmiterlimit
    inline bool op_setmiterlimit(PSVirtualMachine& vm) {
        PSObject limit;
        if (!vm.opStack().pop(limit))
            return vm.error("op_setmiterlimit: stackunderflow");

        if (!limit.isNumber())
            return vm.error("typecheck");

        double value = limit.asReal(); // works for int or real

        if (value < 1.0)
            return vm.error("rangecheck");

        vm.graphics()->setMiterLimit(value);
        return true;
    }

    // setflat
    // 
    inline bool op_setflat(PSVirtualMachine& vm) {
        PSObject obj;
        if (!vm.opStack().pop(obj))
            return vm.error("stackunderflow");

        if (!obj.isNumber())
            return vm.error("typecheck");

        double flatness = obj.asReal();
        if (flatness < 0.0)
            return vm.error("rangecheck");

        vm.graphics()->setFlatness(flatness);
        return true;
    }

    // currentflat
    // Returns the current flatness setting
    inline bool op_currentflat(PSVirtualMachine& vm) {
        double flatness = vm.graphics()->getFlatness();
        vm.opStack().push(PSObject::fromReal(flatness));
        return true;
    }

    inline bool op_setdash(PSVirtualMachine& vm) {
        PSObject offsetObj, patternObj;

        if (!vm.opStack().pop(offsetObj) || !vm.opStack().pop(patternObj))
            return vm.error("stackunderflow");

        if (!patternObj.isArray() || !offsetObj.isNumber())
            return vm.error("typecheck");

        const auto arr = patternObj.asArray();
        std::vector<double> dashPattern;
        for (const PSObject& elem : arr->elements) {
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


    // Path building
    inline bool op_clippath(PSVirtualMachine& vm) {
        PSPath clipPath = vm.graphics()->getClipPath();  // Retrieves a copy
        vm.opStack().push(PSObject::fromPath(std::move(clipPath)));
        return true;
    }

    //inline bool op_setclippath(PSVirtualMachine& vm) {
    //    PSObject pathObj;
    //    if (!vm.opStack().pop(pathObj) || !pathObj.isPath())
    //        return vm.error("typecheck: expected path object");
    //    const PSPath& path = pathObj.asPath();
    //    vm.graphics()->setClipPath(path);
    //    return true;
    //}

    inline bool op_newpath(PSVirtualMachine& vm) {
        vm.graphics()->newpath();
        return true;
    }

    inline bool op_currentpoint(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        
        double x = 0.0, y = 0.0;
        if (!path.getCurrentPoint(x, y))
            return vm.error("currentpoint:nocurrentpoint");

        return vm.opStack().push(PSObject::fromReal(x)) && vm.opStack().push(PSObject::fromReal(y));
    }

    inline bool op_moveto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();


        PSObject y, x;
        if (!s.pop(y) || !s.pop(x) || !x.isNumber() || !y.isNumber())
            return vm.error("op_moveto:typecheck; expected two numbers");

        if (!path.moveto(x.asReal(), y.asReal()))
            return vm.error("op_moveto:currentpatherror");

        return true;
    }

    inline bool op_rmoveto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        PSObject dy, dx;

        if (!s.pop(dy) || !s.pop(dx) || !dx.isNumber() || !dy.isNumber())
            return vm.error("op_moveto:typecheck; expected two numbers");

        auto& path = vm.graphics()->currentPath();

        double x0{ 0 }, y0{ 0 };
        if (!path.getCurrentPoint(x0, y0))
            return vm.error("op_rmoveto:nocurrentpoint");

        return path.moveto(x0 + dx.asReal(), y0 + dy.asReal());
    }

    inline bool op_lineto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        PSObject y, x;
        if (!s.pop(y) || !s.pop(x) || !x.isNumber() || !y.isNumber())
            return vm.error("typecheck: expected two numbers");

        return path.lineto(x.asReal(), y.asReal());
    }

    inline bool op_rlineto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        PSObject dy, dx;
        if (!s.pop(dy) || !s.pop(dx) || !dx.isNumber() || !dy.isNumber())
            return vm.error("op_rlineto:typecheck; expected two numbers");

        double x0, y0;
        if (!path.getCurrentPoint(x0, y0)) return vm.error("op_rlineto:nocurrentpoint");


        return path.lineto(x0 + dx.asReal(), y0 + dy.asReal());
    }

    inline bool op_rectpath(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        if (s.size() < 4)
            return vm.error("rectpath: stackunderflow");

        PSObject heightObj, widthObj, yObj, xObj;
        s.pop(heightObj);
        s.pop(widthObj);
        s.pop(yObj);
        s.pop(xObj);

        if (!xObj.isNumber() || !yObj.isNumber() ||
            !widthObj.isNumber() || !heightObj.isNumber())
            return vm.error("rectpath: typecheck");

        double x = xObj.asReal();
        double y = yObj.asReal();
        double w = widthObj.asReal();
        double h = heightObj.asReal();

        // Rectangles can have negative width/height, which should still work
        return path.moveto(x, y)
            && path.lineto(x + w, y)
            && path.lineto(x + w, y + h)
            && path.lineto(x, y + h)
            && path.lineto(x, y); // Optional final lineto to starting point (not closepath)
    }
    
    // arc - draws an arc from startAngle to endAngle in degrees
    inline bool op_arc(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        PSObject endAngleObj, startAngleObj, radiusObj, yObj, xObj;
        if (!s.pop(endAngleObj) || !s.pop(startAngleObj) ||
            !s.pop(radiusObj) || !s.pop(yObj) || !s.pop(xObj))
            return vm.error("stackunderflow");

        if (!xObj.isNumber() || !yObj.isNumber() ||
            !radiusObj.isNumber() || !startAngleObj.isNumber() || !endAngleObj.isNumber())
            return vm.error("typecheck: expected 5 numbers");

        double cx = xObj.asReal();
        double cy = yObj.asReal();
        double radius = radiusObj.asReal();
        double startAngle = startAngleObj.asReal();
        double endAngle = endAngleObj.asReal();

        return path.arc(cx, cy, radius, startAngle, endAngle);
    }

    // arcn - arc counter-clockwise
    inline bool op_arcn(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        if (s.size() < 5)
            return vm.error("arcn: stackunderflow");

        PSObject endAngleObj, startAngleObj, radiusObj, yObj, xObj;
        if (!s.pop(endAngleObj) || !s.pop(startAngleObj) || !s.pop(radiusObj) ||
            !s.pop(yObj) || !s.pop(xObj))
            return vm.error("arcn: stackunderflow");

        if (!xObj.isNumber() || !yObj.isNumber() || !radiusObj.isNumber() || 
            !startAngleObj.isNumber() || !endAngleObj.isNumber())
            return vm.error("arcn: typecheck");

        double cx = xObj.asReal();
        double cy = yObj.asReal();
        double radius = radiusObj.asReal();
        double startDeg = startAngleObj.asReal();
        double endDeg = endAngleObj.asReal();

        return path.arcCCW(cx, cy, radius, startDeg, endDeg);
    }

    // arcto - draws an arc to a point (x1, y1) with a radius r, and then to (x2, y2)
    inline bool op_arcto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        if (s.size() < 5)
            return vm.error("arcto: stackunderflow");

        PSObject y2Obj, x2Obj, y1Obj, x1Obj, rObj;
        s.pop(y2Obj); s.pop(x2Obj);
        s.pop(y1Obj); s.pop(x1Obj);
        s.pop(rObj);

        if (!x1Obj.isNumber() || !y1Obj.isNumber() ||
            !x2Obj.isNumber() || !y2Obj.isNumber() || !rObj.isNumber())
            return vm.error("arcto: typecheck");

        double x1 = x1Obj.asReal();
        double y1 = y1Obj.asReal();
        double x2 = x2Obj.asReal();
        double y2 = y2Obj.asReal();
        double r = rObj.asReal();

        double cx, cy;
        if (!path.getCurrentPoint(cx, cy))
            return vm.error("arcto: no currentpoint");

        // Use a helper function that returns the tangent points
        double xt1, yt1, xt2, yt2;
        if (!path.arcto(cx, cy, x1, y1, x2, y2, r, xt1, yt1, xt2, yt2))
            return vm.error("arcto: unable to compute arc");

        // Push output values (start and end of arc)
        s.push(PSObject::fromReal(xt1));
        s.push(PSObject::fromReal(yt1));
        s.push(PSObject::fromReal(xt2));
        s.push(PSObject::fromReal(yt2));

        return true;
    }



 

    inline bool op_curveto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        // Ensure 6 items are available
        if (s.size() < 6)
            return vm.error("curveto: stackunderflow");

        PSObject y3, x3, y2, x2, y1, x1;
        s.pop(y3); s.pop(x3);
        s.pop(y2); s.pop(x2);
        s.pop(y1); s.pop(x1);

        if (!x1.isNumber() || !y1.isNumber() ||
            !x2.isNumber() || !y2.isNumber() ||
            !x3.isNumber() || !y3.isNumber()) {
            return vm.error("curveto: typecheck");
        }

        // Validate there's a currentpoint
        if (!path.hasCurrentPoint())
            return vm.error("curveto: no currentpoint");

        return path.curveto(
            x1.asReal(), y1.asReal(),
            x2.asReal(), y2.asReal(),
            x3.asReal(), y3.asReal());
    }


    inline bool op_rcurveto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();

        if (s.size() < 6)
            return vm.error("rcurveto: stackunderflow");

        PSObject dy3, dx3, dy2, dx2, dy1, dx1;
        s.pop(dy3); s.pop(dx3);
        s.pop(dy2); s.pop(dx2);
        s.pop(dy1); s.pop(dx1);

        if (!dx1.isNumber() || !dy1.isNumber() ||
            !dx2.isNumber() || !dy2.isNumber() ||
            !dx3.isNumber() || !dy3.isNumber()) {
            return vm.error("rcurveto: typecheck");
        }

        double cx, cy;
        if (!path.getCurrentPoint(cx, cy))
            return vm.error("rcurveto: no currentpoint");

        double x1 = cx + dx1.asReal();
        double y1 = cy + dy1.asReal();
        double x2 = x1 + dx2.asReal();
        double y2 = y1 + dy2.asReal();
        double x3 = x2 + dx3.asReal();
        double y3 = y2 + dy3.asReal();

        return path.curveto(x1, y1, x2, y2, x3, y3);
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

    // ( x y width height -- )
    inline bool op_rectstroke(PSVirtualMachine& vm) {
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

        vm.graphics()->stroke();

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

    inline bool op_image(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 5)
            return vm.error("stackunderflow");

        // Pop operands in reverse order
        PSObject procObj, matrixObj, bpcObj, heightObj, widthObj;
        s.pop(procObj);
        s.pop(matrixObj);
        s.pop(bpcObj);
        s.pop(heightObj);
        s.pop(widthObj);

        // Validate types
        if (!procObj.isExecutableArray())
            return vm.error("typecheck: data source must be a procedure");
        if (!bpcObj.isInt() || !heightObj.isInt() || !widthObj.isInt())
            return vm.error("typecheck: width, height, and bpc must be integers");

        int width = widthObj.asInt();
        int height = heightObj.asInt();
        int bpc = bpcObj.asInt();

        if (width <= 0 || height <= 0)
            return vm.error("rangecheck: invalid width or height");
        if (bpc != 8)
            return vm.error("rangecheck: only 8-bit grayscale images supported");

        // Extract matrix
        PSMatrix matrix;
        if (!extractMatrix(matrixObj, matrix))
            return vm.error("typecheck: expected array or matrix object");

        // Execute the data source procedure
        if (!vm.execProc(procObj))
            return vm.error("exec: failed to execute image data procedure");

        if (s.empty())
            return vm.error("stackunderflow: no result from image procedure");

        PSObject result;
        s.pop(result);

        if (!result.isString())
            return vm.error("typecheck: image procedure must return a string");

        const PSString& str = result.asString();
        size_t expectedBytes = static_cast<size_t>(width) * height;

        if (str.length() < expectedBytes)
            return vm.error("rangecheck: insufficient image data");

        // Copy only the required bytes
        std::vector<uint8_t> imageData;
        imageData.insert(imageData.end(), str.data(), str.data() + expectedBytes);

        // Create PSImage and delegate to graphics backend
        PSImage img;
        img.width = width;
        img.height = height;
        img.bitsPerComponent = bpc;
        img.transform = matrix;
        img.data = std::move(imageData);

        return vm.graphics()->image(img);
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
            { "setmiterlimit", op_setmiterlimit },
            { "setflat",       op_setflat },
            { "currentflat",   op_currentflat },
            { "setdash",       op_setdash },

            // Path operations
            { "clippath",      op_clippath },
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
			{ "rectstroke",    op_rectstroke },

            // Curves
            { "curveto",       op_curveto },
            { "rcurveto",      op_rcurveto },
            { "closepath",     op_closepath },

            // Path rendering
            { "stroke",        op_stroke },
            { "fill",          op_fill },

            // Images
            {"image", op_image }
        };
        return table;
    }



} // namespace waavs
