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
        auto& ostk = vm.opStack();
        auto& ctm = vm.graphics()->getCTM();

        if (ostk.empty())
            return vm.error("op_setlinewidth: stackunderflow");    

        PSObject widthObj;
        ostk.pop(widthObj);

        if (!widthObj.isNumber())
            return vm.error("op_setlinewidth:typecheck; expected number");

        double width, dwidth;

        ctm.dtransform(widthObj.asReal(), 0.0, width, dwidth);

        vm.graphics()->setLineWidth(width);

        //vm.graphics()->setLineWidth(widthObj.asReal());

        return true;
    }

    inline bool op_setlinecap(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();

        if (ostk.empty())
            return vm.error("op_setlinecap: stackunderflow");

        PSObject obj;
        if (!vm.opStack().pop(obj) || !obj.isInt())
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
        if (!ostk.pop(obj) || !obj.isInt())
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

    // setflat
    // 
    inline bool op_setflat(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_setflat: stackunderflow");

        PSObject obj;
        if (!ostk.pop(obj))
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
    inline bool op_currentflat(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();

        double flatness = vm.graphics()->getFlatness();
        ostk.pushReal(flatness);

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

    //=================================================
    // 
    // Path building operations
    //=================================================

    inline bool op_clippath(PSVirtualMachine& vm) 
    {
        PSPath clipPath = vm.graphics()->getClipPath();  // Retrieves a copy
        vm.opStack().push(PSObject::fromPath(std::move(clipPath)));
        return true;
    }

    inline bool op_pathbbox(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();
        PSPath path;

        PSObject topper;
        if (ostk.top(topper) && topper.isPath())
        {
            path = topper.asPath();
            //s.pop();
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

        PSObject objy, objx;
        if (!ostk.pop(objy) || !ostk.pop(objx) || !objx.isNumber() || !objy.isNumber())
            return vm.error("op_moveto:typecheck; expected two numbers");

        if (!path.moveto(ctm, objx.asReal(), objy.asReal()))
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

        PSObject objy, objx;
        if (!ostk.pop(objy) || !ostk.pop(objx) || !objx.isNumber() || !objy.isNumber())
            return vm.error("typecheck: expected two numbers");

        return path.lineto(ctm, objx.asReal(), objy.asReal());
    }

    inline bool op_rlineto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        PSObject dyObj, dxObj;
        if (!s.pop(dyObj) || !s.pop(dxObj) || !dxObj.isNumber() || !dyObj.isNumber())
            return vm.error("op_rlineto:typecheck; expected two numbers");

        double x0, y0;
        if (!path.getCurrentPoint(x0, y0)) return vm.error("op_rlineto:nocurrentpoint");

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

        PSObject heightObj, widthObj, yObj, xObj;
        ostk.pop(heightObj);
        ostk.pop(widthObj);
        ostk.pop(yObj);
        ostk.pop(xObj);

        if (!xObj.isNumber() || !yObj.isNumber() ||
            !widthObj.isNumber() || !heightObj.isNumber())
            return vm.error("op_rectpath: typecheck");

        double x = xObj.asReal();
        double y = yObj.asReal();
        double w = widthObj.asReal();
        double h = heightObj.asReal();


        return path.moveto(ctm, x, y)
            && path.lineto(ctm, x + w, y)
            && path.lineto(ctm, x + w, y + h)
            && path.lineto(ctm, x, y + h)
            && path.lineto(ctm, x, y); // Optional final lineto to starting point (not closepath)
    }
    
    // arc 
    // x y radius startAngle endAngle -- path
    // draws an arc from startAngle to endAngle in degrees
    inline bool op_arc(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

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

        return path.arc(ctm, cx, cy, radius, startAngle, endAngle);
    }

    // arcn - arc counter-clockwise
    inline bool op_arcn(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

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

        return path.arcCCW(ctm, cx, cy, radius, endDeg, startDeg);
    }

    inline bool op_arcto(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 5)
            return vm.error("arcto: stackunderflow");

        PSObject rObj, x1Obj, y1Obj, x2Obj, y2Obj;
        s.pop(rObj);
        s.pop(y2Obj); s.pop(x2Obj);
        s.pop(y1Obj); s.pop(x1Obj);


        if (!x1Obj.isNumber() || !y1Obj.isNumber() ||
            !x2Obj.isNumber() || !y2Obj.isNumber() || !rObj.isNumber())
            return vm.error("arcto: typecheck");

        double x0, y0;
        if (!path.getCurrentPoint(x0, y0))
            return vm.error("arcto: no currentpoint");

        // Transform all points
        double x1, y1, x2, y2, r, dummy;
        ctm.transformPoint(x1Obj.asReal(), y1Obj.asReal(), x1, y1);
        ctm.transformPoint(x2Obj.asReal(), y2Obj.asReal(), x2, y2);
        ctm.dtransform(rObj.asReal(), 0.0, r, dummy);  // only x-direction used for circle

        // Execute the arc and retrieve tangent points
        double xt1, yt1, xt2, yt2;
        if (!path.arcto(ctm, x0, y0, x1, y1, x2, y2, r, xt1, yt1, xt2, yt2))
            return vm.error("arcto: unable to compute arc");

        // Push the tangent points as per spec
        s.push(PSObject::fromReal(xt1));
        s.push(PSObject::fromReal(yt1));
        s.push(PSObject::fromReal(xt2));
        s.push(PSObject::fromReal(yt2));

        return true;
    }



 

    inline bool op_curveto(PSVirtualMachine& vm) {
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

        return path.curveto( ctm, x1, y1,  x2, y2,  x3, y3);
    }


    inline bool op_rcurveto(PSVirtualMachine& vm) {
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

        vm.graphics()->setGray(obj.asReal());
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
        if (!vm.runProc(procObj))
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
            { "sethsbcolor",   op_sethsbcolor },
            { "currentrgbcolor", op_currentrgbcolor },
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
            { "pathbbox",      op_pathbbox },
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
            { "eofill",        op_eofill },

            // Images
            {"image", op_image }
        };
        return table;
    }



} // namespace waavs
