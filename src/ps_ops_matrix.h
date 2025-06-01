#pragma once


#include "pscore.h"
#include "psvm.h"
#include "psmatrix.h"

namespace waavs {
    
    inline bool matrixFromArray(PSArrayHandle h, PSMatrix& out) {
        if (!h || h->size() != 6 || !h->allNumbers())
            return false;

        for (size_t i = 0; i < 6; ++i) {
            PSObject o;
            if (!h->get(i, o)) return false;
            out.m[i] = o.asReal();
        }

        return true;
    }

    // Helper: Extract matrix from object (matrix or numeric array)
    inline bool extractMatrix(const PSObject& obj, PSMatrix& out) {
        if (obj.isMatrix()) {
            out = obj.asMatrix();
            return true;
        }
        if (obj.isArray()) {
            return matrixFromArray(obj.asArray(), out);
        }
        return false;
    }


    // ( – ? matrix )
    inline bool op_matrix(PSVirtualMachine& vm) {
        return vm.opStack().push(PSObject::fromMatrix(PSMatrix::identity()));
    }

    // ( matrix1 ? matrix2 )
    inline bool op_invertmatrix(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject m1;
        s.pop(m1);

        PSMatrix mat;
        if (!extractMatrix(m1, mat)) return false;

        PSMatrix inv;
        if (!mat.inverse(inv)) return false;

        return s.push(PSObject::fromMatrix(inv));
    }

    // ( m1 m2 ? m3 )
    inline bool op_concatmatrix(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject m2, m1;
        s.pop(m2); s.pop(m1);

        PSMatrix mat1, mat2;
        if (!extractMatrix(m1, mat1) || !extractMatrix(m2, mat2)) return false;

        mat1.preMultiply(mat2);

        return s.push(PSObject::fromMatrix(mat1));
    }

    // ( x y matrix ? x' y' )
    inline bool op_transform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 3) return false;

        PSObject m, y, x;
        s.pop(m); s.pop(y); s.pop(x);

        if (!x.isNumber() || !y.isNumber()) return false;

        PSMatrix mat;
        if (!extractMatrix(m, mat)) return false;

        double xOut, yOut;
        mat.transformPoint(x.asReal(), y.asReal(), xOut, yOut);

        return s.push(PSObject::fromReal(xOut)) &&
            s.push(PSObject::fromReal(yOut));
    }


    // ( dx dy matrix ? dx' dy' )
    inline bool op_dtransform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 3) return false;

        PSObject m, dy, dx;
        s.pop(m); s.pop(dy); s.pop(dx);

        if (!dx.isNumber() || !dy.isNumber()) return false;

        PSMatrix mat;
        if (!extractMatrix(m, mat)) return false;

        double dxOut, dyOut;
        mat.dtransform(dx.asReal(), dy.asReal(), dxOut, dyOut);

        return s.push(PSObject::fromReal(dxOut)) &&
            s.push(PSObject::fromReal(dyOut));
    }

    // ( x' y' matrix ? x y )
    inline bool op_itransform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 3) return false;

        PSObject m, y, x;
        s.pop(m); s.pop(y); s.pop(x);

        if (!x.isNumber() || !y.isNumber()) return false;

        PSMatrix mat;
        if (!extractMatrix(m, mat)) return false;

        PSMatrix inv;
        if (!mat.inverse(inv)) return false;

        double xOut, yOut;
        inv.transformPoint(x.asReal(), y.asReal(), xOut, yOut);

        return s.push(PSObject::fromReal(xOut)) &&
            s.push(PSObject::fromReal(yOut));
    }

    // ( dx' dy' matrix ? dx dy )
    inline bool op_idtransform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 3) return false;

        PSObject m, dy, dx;
        s.pop(m); s.pop(dy); s.pop(dx);

        if (!dx.isNumber() || !dy.isNumber()) return false;

        PSMatrix mat;
        if (!extractMatrix(m, mat)) return false;

        PSMatrix inv;
        if (!mat.inverse(inv)) return false;

        double dxOut, dyOut;
        inv.dtransform(dx.asReal(), dy.asReal(), dxOut, dyOut);

        return s.push(PSObject::fromReal(dxOut)) &&
            s.push(PSObject::fromReal(dyOut));
    }

    // === CTM-RELATED PLACEHOLDERS ===

    inline bool op_currentmatrix(PSVirtualMachine& vm) {
        // TODO: Push current CTM onto the stack
        return false;
    }

    inline bool op_setmatrix(PSVirtualMachine& vm) {
        // TODO: Replace current CTM with matrix on stack
        return false;
    }

    inline bool op_initmatrix(PSVirtualMachine& vm) {
        // TODO: Reset CTM to default (device space)
        return false;
    }

    inline bool op_defaultmatrix(PSVirtualMachine& vm) {
        // TODO: Push default device-space matrix
        return false;
    }

    inline bool op_currentscreenmatrix(PSVirtualMachine& vm) {
        // TODO: Rarely used; can stub or implement later
        return false;
    }

    inline bool op_concat(PSVirtualMachine& vm) {
        // TODO: CTM := matrix × CTM
        return false;
    }


    // ( tx ty matrix ? matrix' ) or ( tx ty ? )
    inline bool op_translate(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject top;
        s.pop(top);

        if (top.isMatrix() || top.isArray()) {
            if (s.size() < 2) return false;

            PSObject ty, tx;
            s.pop(ty); s.pop(tx);

            if (!tx.isNumber() || !ty.isNumber()) return false;

            PSMatrix mat;
            if (!extractMatrix(top, mat)) return false;

            mat.translate(tx.asReal(), ty.asReal());
            return s.push(PSObject::fromMatrix(mat));
        }
        else if (top.isNumber()) {
            if (s.empty()) return false;

            PSObject ty = top;
            PSObject tx;
            s.pop(tx);

            if (!tx.isNumber()) return false;

            // TODO: apply to CTM
            return false; // stub
        }

        return false;
    }

    // ( sx sy matrix ? matrix' ) or ( sx sy ? )
    inline bool op_scale(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject top;
        s.pop(top);

        if (top.isMatrix() || top.isArray()) {
            if (s.size() < 2) return false;

            PSObject sy, sx;
            s.pop(sy); s.pop(sx);

            if (!sx.isNumber() || !sy.isNumber()) return false;

            PSMatrix mat;
            if (!extractMatrix(top, mat)) return false;

            mat.scale(sx.asReal(), sy.asReal());
            return s.push(PSObject::fromMatrix(mat));
        }
        else if (top.isNumber()) {
            if (s.empty()) return false;

            PSObject sy = top;
            PSObject sx;
            s.pop(sx);

            if (!sx.isNumber()) return false;

            // TODO: apply to CTM
            return false; // stub
        }

        return false;
    }

    // ( angle matrix ? matrix' ) or ( angle ? )
    inline bool op_rotate(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject top;
        s.pop(top);

        if (top.isMatrix() || top.isArray()) {
            if (s.empty()) return false;

            PSObject angle;
            s.pop(angle);

            if (!angle.isNumber()) return false;

            PSMatrix mat;
            if (!extractMatrix(top, mat)) return false;

            mat.rotate(angle.asReal());
            return s.push(PSObject::fromMatrix(mat));
        }
        else if (top.isNumber()) {
            double angle = top.asReal();
            // TODO: apply to CTM
            return false; // stub
        }

        return false;
    }


} // namespace waavs

namespace waavs {

    static const PSOperatorFuncMap opsMatrix = {
        { "matrix",              op_matrix },
        { "invertmatrix",        op_invertmatrix },
        { "concatmatrix",        op_concatmatrix },
        { "transform",           op_transform },
        { "dtransform",          op_dtransform },
        { "itransform",          op_itransform },
        { "idtransform",         op_idtransform },

        { "currentmatrix",       op_currentmatrix },
        { "setmatrix",           op_setmatrix },
        { "initmatrix",          op_initmatrix },
        { "defaultmatrix",       op_defaultmatrix },
        { "currentscreenmatrix", op_currentscreenmatrix },
        { "concat",              op_concat },
        { "translate",           op_translate },
        { "scale",               op_scale },
        { "rotate",              op_rotate },
    };

}
