// ps_ops_path.h
#pragma once

#include "psvm.h"
#include "ps_type_graphicscontext.h"
#include "ps_type_matrix.h"
#include "ps_type_path.h"

#include "pathprogram_flattener.h"

namespace waavs {

    inline bool op_setflat(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        double flatness;
        if (!s.popReal(flatness))
            return vm.error("op_setflat: typecheck");

        if (flatness < 0.0)
            return vm.error("op_setflat: rangecheck");

        vm.graphics()->setFlatness(flatness);
        return true;
    }


    inline bool op_currentflat(PSVirtualMachine& vm)
    {
        return vm.opStack().pushReal(vm.graphics()->getFlatness());
    }


    inline bool op_newpath(PSVirtualMachine& vm)
    {
        vm.graphics()->currentPath().reset();
        return true;
    }


    inline bool op_currentpoint(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double x;
        double y;

        if (!path.getCurrentPoint(ctm, x, y))
            return vm.error("op_currentpoint: nocurrentpoint");

        return s.pushReal(x) && s.pushReal(y);
    }


    inline bool op_moveto(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double x;
        double y;

        if (!s.popReal(y) || !s.popReal(x))
            return vm.error("op_moveto: typecheck");

        if (!path.moveto(ctm, x, y))
            return vm.error("op_moveto: path error");

        return true;
    }


    inline bool op_rmoveto(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double dx;
        double dy;

        if (!s.popReal(dy) || !s.popReal(dx))
            return vm.error("op_rmoveto: typecheck");

        if (!path.rmoveto(ctm, dx, dy))
            return vm.error("op_rmoveto: nocurrentpoint");

        return true;
    }


    inline bool op_lineto(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double x;
        double y;

        if (!s.popReal(y) || !s.popReal(x))
            return vm.error("op_lineto: typecheck");

        if (!path.lineto(ctm, x, y))
            return vm.error("op_lineto: nocurrentpoint");

        return true;
    }


    inline bool op_rlineto(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double dx;
        double dy;

        if (!s.popReal(dy) || !s.popReal(dx))
            return vm.error("op_rlineto: typecheck");

        if (!path.rlineto(ctm, dx, dy))
            return vm.error("op_rlineto: nocurrentpoint");

        return true;
    }


    // Convenience extension: x y width height rectpath -
    inline bool op_rectpath(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double x;
        double y;
        double w;
        double h;

        if (!s.popReal(h) || !s.popReal(w) || !s.popReal(y) || !s.popReal(x))
            return vm.error("op_rectpath: typecheck");

        if (!path.moveto(ctm, x, y) ||
            !path.lineto(ctm, x + w, y) ||
            !path.lineto(ctm, x + w, y + h) ||
            !path.lineto(ctm, x, y + h) ||
            !path.close())
            return vm.error("op_rectpath: path error");

        return true;
    }


    inline bool op_arc(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double cx;
        double cy;
        double radius;
        double startDeg;
        double endDeg;

        if (!s.popReal(endDeg) || !s.popReal(startDeg) || !s.popReal(radius) ||
            !s.popReal(cy) || !s.popReal(cx))
            return vm.error("op_arc: typecheck");

        if (radius < 0.0)
            return vm.error("op_arc: rangecheck");

        if (!path.arc(ctm, cx, cy, radius, startDeg, endDeg))
            return vm.error("op_arc: path error");

        return true;
    }


    inline bool op_arcn(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        double cx;
        double cy;
        double radius;
        double startDeg;
        double endDeg;

        if (!s.popReal(endDeg) || !s.popReal(startDeg) || !s.popReal(radius) ||
            !s.popReal(cy) || !s.popReal(cx))
            return vm.error("op_arcn: typecheck");

        if (radius < 0.0)
            return vm.error("op_arcn: rangecheck");

        if (!path.arcn(ctm, cx, cy, radius, startDeg, endDeg))
            return vm.error("op_arcn: path error");

        return true;
    }

    inline bool op_arct(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        if (s.size() < 5)
            return vm.error("op_arct: stackunderflow");

        double x1;
        double y1;
        double x2;
        double y2;
        double radius;

        if (!s.popReal(radius) || !s.popReal(y2) || !s.popReal(x2) ||
            !s.popReal(y1) || !s.popReal(x1))
            return vm.error("op_arct: typecheck");

        if (radius < 0.0)
            return vm.error("op_arct: rangecheck");

        double xt1;
        double yt1;
        double xt2;
        double yt2;

        if (!path.arcto(ctm, x1, y1, x2, y2, radius, xt1, yt1, xt2, yt2))
            return vm.error("op_arct: path error");

        return true;
    }

    inline bool op_arcto(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        if (s.size() < 5)
            return vm.error("op_arcto: stackunderflow");

        double x1;
        double y1;
        double x2;
        double y2;
        double radius;

        if (!s.popReal(radius) || !s.popReal(y2) || !s.popReal(x2) ||
            !s.popReal(y1) || !s.popReal(x1))
            return vm.error("op_arcto: typecheck");

        if (radius < 0.0)
            return vm.error("op_arcto: rangecheck");

        double xt1;
        double yt1;
        double xt2;
        double yt2;

        if (!path.arcto(ctm, x1, y1, x2, y2, radius, xt1, yt1, xt2, yt2))
            return vm.error("op_arcto: path error");

        return s.pushReal(xt1) && s.pushReal(yt1) &&
            s.pushReal(xt2) && s.pushReal(yt2);
    }


    inline bool op_curveto(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        if (s.size() < 6)
            return vm.error("op_curveto: stackunderflow");

        double x1;
        double y1;
        double x2;
        double y2;
        double x3;
        double y3;

        if (!s.popReal(y3) || !s.popReal(x3) ||
            !s.popReal(y2) || !s.popReal(x2) ||
            !s.popReal(y1) || !s.popReal(x1))
            return vm.error("op_curveto: typecheck");

        if (!path.curveto(ctm, x1, y1, x2, y2, x3, y3))
            return vm.error("op_curveto: nocurrentpoint");

        return true;
    }


    inline bool op_rcurveto(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();
        auto& path = vm.graphics()->currentPath();
        const PSMatrix& ctm = vm.graphics()->getCTM();

        if (s.size() < 6)
            return vm.error("op_rcurveto: stackunderflow");

        double dx1;
        double dy1;
        double dx2;
        double dy2;
        double dx3;
        double dy3;

        if (!s.popReal(dy3) || !s.popReal(dx3) ||
            !s.popReal(dy2) || !s.popReal(dx2) ||
            !s.popReal(dy1) || !s.popReal(dx1))
            return vm.error("op_rcurveto: typecheck");

        if (!path.rcurveto(ctm, dx1, dy1, dx2, dy2, dx3, dy3))
            return vm.error("op_rcurveto: nocurrentpoint");

        return true;
    }


    inline bool op_closepath(PSVirtualMachine& vm)
    {
        auto& path = vm.graphics()->currentPath();

        if (!path.hasCurrentPoint())
            return vm.error("op_closepath: nocurrentpoint");

        if (!path.close())
            return vm.error("op_closepath: path error");

        return true;
    }


    inline bool op_flattenpath(PSVirtualMachine& vm)
    {
        auto* grph = vm.graphics();
        PSPath& path = grph->currentPath();

        if (path.empty())
            return true;

        PathProgram flat;

        if (!flattenPathProgram(path.program(), flat, grph->getFlatness()))
            return vm.error("op_flattenpath: flatten failed");

        if (!path.replaceProgram(flat))
            return vm.error("op_flattenpath: invalid flattened path");

        return true;
    }


    inline bool op_pathforall(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 4)
            return vm.error("op_pathforall: stackunderflow");

        PSObject procClose;
        PSObject procCurve;
        PSObject procLine;
        PSObject procMove;

        s.pop(procClose);
        s.pop(procCurve);
        s.pop(procLine);
        s.pop(procMove);

        if (!procMove.isArray() || !procMove.isExecutable())
            return vm.error("op_pathforall: typecheck");

        if (!procLine.isArray() || !procLine.isExecutable())
            return vm.error("op_pathforall: typecheck");

        if (!procCurve.isArray() || !procCurve.isExecutable())
            return vm.error("op_pathforall: typecheck");

        if (!procClose.isArray() || !procClose.isExecutable())
            return vm.error("op_pathforall: typecheck");

        PSObject path = PSObject::fromPath(vm.graphics()->currentPath());

        return vm.schedulePathForAll(path, procMove, procLine, procCurve, procClose);
    }


    inline bool op_setpath(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject pathObj;
        if (!s.pop(pathObj))
            return vm.error("op_setpath: stackunderflow");

        if (!pathObj.isPath())
            return vm.error("op_setpath: typecheck");

        vm.graphics()->setCurrentPath(pathObj.asPath());
        return true;
    }


    inline bool op_currentpath(PSVirtualMachine& vm)
    {
        return vm.opStack().push(PSObject::fromPath(vm.graphics()->currentPath()));
    }


    inline const PSOperatorFuncMap& getPathOps()
    {
        static const PSOperatorFuncMap table = {
            { "setflat",       op_setflat },
            { "currentflat",   op_currentflat },

            { "newpath",       op_newpath },
            { "currentpoint",  op_currentpoint },
            { "moveto",        op_moveto },
            { "rmoveto",       op_rmoveto },
            { "lineto",        op_lineto },
            { "rlineto",       op_rlineto },
            { "arc",           op_arc },
            { "arcn",          op_arcn },
            { "arct",          op_arct },
            { "arcto",         op_arcto },
            { "rectpath",      op_rectpath },
            { "curveto",       op_curveto },
            { "rcurveto",      op_rcurveto },
            { "closepath",     op_closepath },

            { "flattenpath",   op_flattenpath },
            { "pathforall",    op_pathforall },
            { "setpath",       op_setpath },
            { "currentpath",   op_currentpath }
        };

        return table;
    }

} // namespace waavs