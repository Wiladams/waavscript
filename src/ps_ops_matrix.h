#pragma once


#include "pscore.h"
#include "psvm.h"

namespace waavs {


    // (  ? matrix )
    inline bool op_matrix(PSVirtualMachine& vm) {
        return vm.opStack().push(PSObject::fromMatrix(PSMatrix::identity()));
    }

	// ( matrix ? matrix' )
	// pop a matrix off the stack, fill it with identity values, and push it back
    inline bool op_identmatrix(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 1)
            return vm.error("op_identmatrix: stackunderflow");

		PSObject obj;
        s.pop(obj);
        if (!obj.isMatrix() && !obj.isArray())
			return vm.error("typecheck: expected matrix or array");

		// we don't care what's currently in the matrix object, just fill it with identity values
        PSMatrix mat = PSMatrix::identity();
		return s.push(PSObject::fromMatrix(mat)); // push back to stack
    }

    // matrix1 matrix2 invertmatrix matrix2
    inline bool op_invertmatrix(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("invertmatrix: stackunderflow");

        PSObject destObj, srcObj;
        s.pop(destObj);
        s.pop(srcObj);

        PSMatrix src;
        if (!extractMatrix(srcObj, src))
            return vm.error("invertmatrix: typecheck");

        PSMatrix inv;
        if (!src.inverse(inv))
            return vm.error("invertmatrix: singular matrix");

        destObj.resetFromMatrix(inv);
        return s.push(destObj);
    }





    // op_transform
    // 
    // x y transform x' y'
	// x y matrix transform x' y'
    inline bool op_transform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 2) return false;

        PSMatrix tmat;

		PSObject top;
		s.top(top);

        if (!extractMatrix(top, tmat)) {

            // Not a matrix, so we expect two numbers
			// which will be transformed by the current CTM
            PSObject y, x;
            s.pop(y);
            s.pop(x);
            if (!x.isNumber() || !y.isNumber()) return false;
            double xOut{ 0 }, yOut{ 0 };
            ctm.transformPoint(x.asReal(), y.asReal(), xOut, yOut);
            //vm.graphics()->transformPoint(x.asReal(), y.asReal(), xOut, yOut);

            return s.push(PSObject::fromReal(xOut)) && s.push(PSObject::fromReal(yOut));
		}

		// We have a matrix, so we expect two numbers
        PSObject m, y, x;
        s.pop(m); 
        s.pop(y); 
        s.pop(x);

        if (!x.isNumber() || !y.isNumber()) return false;

        PSMatrix mat;
        if (!extractMatrix(m, mat)) return vm.error("op_transform: typecheck, could not extract matrix");

        double xOut, yOut;
        mat.transformPoint(x.asReal(), y.asReal(), xOut, yOut);

        return s.push(PSObject::fromReal(xOut)) &&
            s.push(PSObject::fromReal(yOut));
    }


	// op_dtransform
    // 
    // dx dy dtransform dx' dy'
	// dx dy matrix dtransform dx' dy'
    // 
    inline bool op_dtransform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 2) return false;

        PSMatrix tmat;

        PSObject top;
        s.top(top);

        if (!extractMatrix(top, tmat)) {

            // Not a matrix, so we expect two numbers
            // which will be transformed by the current CTM
            PSObject y, x;
            s.pop(y);
            s.pop(x);
            if (!x.isNumber() || !y.isNumber()) return false;
            double xOut{ 0 }, yOut{ 0 };
            ctm.dtransform(x.asReal(), y.asReal(), xOut, yOut);
            //vm.graphics()->dtransformPoint(x.asReal(), y.asReal(), xOut, yOut);

            return s.push(PSObject::fromReal(xOut)) && s.push(PSObject::fromReal(yOut));
        }

        // We have a matrix, so we expect two numbers
        PSObject m, y, x;
        s.pop(m);
        s.pop(y);
        s.pop(x);

        if (!x.isNumber() || !y.isNumber()) return false;

        PSMatrix mat;
        if (!extractMatrix(m, mat)) return vm.error("op_transform: typecheck, could not extract matrix");

        double xOut, yOut;
        mat.dtransform(x.asReal(), y.asReal(), xOut, yOut);

        return s.push(PSObject::fromReal(xOut)) &&
            s.push(PSObject::fromReal(yOut));
    }

    // ( x' y' -- x y )
   // ( x' y' matrix -- x y )
    inline bool op_itransform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto &ctm = vm.graphics()->getCTM();

        if (s.size() < 2)
            return vm.error("itransform: stackunderflow");

        PSMatrix mat;
        PSObject top;
        s.top(top);

        // if the top is not a matrix, we use the current CTM
        if (!extractMatrix(top, mat)) {
            // Use current CTM
            PSObject yObj, xObj;
            s.pop(yObj); 
            s.pop(xObj);

            if (!xObj.isNumber() || !yObj.isNumber())
                return vm.error("itransform: typecheck");

            PSMatrix inv;
            if (!ctm.inverse(inv))
                return vm.error("itransform: singular CTM");

            double xOut, yOut;
            inv.transformPoint(xObj.asReal(), yObj.asReal(), xOut, yOut);

            return s.push(PSObject::fromReal(xOut)) &&
                s.push(PSObject::fromReal(yOut));
        }

        // Matrix was present, so it's 3-arg version
        if (s.size() < 3)
            return vm.error("itransform: stackunderflow");

        PSObject mObj, yObj, xObj;
        s.pop(mObj); s.pop(yObj); s.pop(xObj);

        if (!xObj.isNumber() || !yObj.isNumber())
            return vm.error("itransform: typecheck");

        PSMatrix inv;
        if (!mat.inverse(inv))
            return vm.error("itransform: singular matrix");

        double xOut, yOut;
        inv.transformPoint(xObj.asReal(), yObj.asReal(), xOut, yOut);

        return s.push(PSObject::fromReal(xOut)) &&
            s.push(PSObject::fromReal(yOut));
    }


    // ( dx' dy' -- dx dy )
    // ( dx' dy' matrix -- dx dy )
    inline bool op_idtransform(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("idtransform: stackunderflow");

        PSMatrix mat;
        PSObject top;
        s.top(top);

        if (!extractMatrix(top, mat)) {
            // Use CTM
            PSObject dyObj, dxObj;
            s.pop(dyObj); s.pop(dxObj);

            if (!dxObj.isNumber() || !dyObj.isNumber())
                return vm.error("idtransform: typecheck");

            PSMatrix ctm = vm.graphics()->getCTM();
            PSMatrix inv;
            if (!ctm.inverse(inv))
                return vm.error("idtransform: singular CTM");

            double dxOut, dyOut;
            inv.dtransform(dxObj.asReal(), dyObj.asReal(), dxOut, dyOut);

            return s.push(PSObject::fromReal(dxOut)) &&
                s.push(PSObject::fromReal(dyOut));
        }

        // 3-arg form
        if (s.size() < 3)
            return vm.error("idtransform: stackunderflow");

        PSObject mObj, dyObj, dxObj;
        s.pop(mObj); s.pop(dyObj); s.pop(dxObj);

        if (!dxObj.isNumber() || !dyObj.isNumber())
            return vm.error("idtransform: typecheck");

        PSMatrix inv;
        if (!mat.inverse(inv))
            return vm.error("idtransform: singular matrix");

        double dxOut, dyOut;
        inv.dtransform(dxObj.asReal(), dyObj.asReal(), dxOut, dyOut);

        return s.push(PSObject::fromReal(dxOut)) &&
            s.push(PSObject::fromReal(dyOut));
    }


    // === CTM-RELATED PLACEHOLDERS ===
	// matrix currentmatrix matrix
    inline bool op_currentmatrix(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("stackunderflow");

        PSObject obj;
        s.pop(obj);

        if (!obj.isMatrix() && !obj.isArray())
            return vm.error("typecheck: expected matrix");

        return s.push(PSObject::fromMatrix(vm.graphics()->getCTM()));
    }


    // matrix setmatrix 
	// Replace CTM by matrix on top of stack
    inline bool op_setmatrix(PSVirtualMachine& vm) {
        PSObject obj;
        auto& ctm = vm.graphics()->getCTM();

        if (!vm.opStack().pop(obj))
            return vm.error("op_setmatrix: stackunderflow");

        PSMatrix mat;
        if (!extractMatrix(obj, mat))
            return vm.error("typecheck: expected matrix");

        ctm = mat; // set CTM to the new matrix

        return true;
    }

    // - initmatrix -
	// Reset CTM to identity matrix
    inline bool op_initmatrix(PSVirtualMachine& vm) {
        auto& ctm = vm.graphics()->getCTM();

        ctm.reset(); // identity
        return true;
    }

    // matrix defautlmatrix matrix
    // pop a matrix off the stack.  fill it with 
	// the device's default matrix, and push it back
    inline bool op_defaultmatrix(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("op_defaultmatrix: stackunderflow");

        PSObject obj;
        s.pop(obj);

        if (!obj.isMatrix() && !obj.isArray())
            return vm.error("typecheck: expected matrix");

        PSMatrix mat = vm.graphics()->getDeviceDefaultMatrix();
        return s.push(PSObject::fromMatrix(mat));
    }


    inline bool op_currentscreenmatrix(PSVirtualMachine& vm) {
        // TODO: Rarely used; can stub or implement later
        return false;
    }

    // op_concat
    // 
	// matrix concat -
    inline bool op_concat(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
        auto& ctm = vm.graphics()->getCTM();

        if (s.empty())
			return vm.error("op_concat: stackunderflow");

        PSObject matObj;
        PSMatrix mat;

        s.pop(matObj);

        if (!extractMatrix(matObj, mat))
			return vm.error("op_concat: typecheck, expected matrix or array");

        ctm.preMultiply(mat);

        //vm.graphics()->concat(mat);
        return true;
    }

	// op_concatmatrix
    // 
    // matrix1 matrix2 matrix3 contatmatrix matrix3
    //
    inline bool op_concatmatrix(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        //auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 3) 
            return vm.error("op_concatmatrix: stackunderflow");

        PSObject m3, m2, m1;
		s.pop(m3);
        s.pop(m2); 
        s.pop(m1);

        PSMatrix mat1, mat2, mat3;
        if (!extractMatrix(m1, mat1) || !extractMatrix(m2, mat2)) return false;

        mat2.preMultiply(mat1); // mat2 = mat1 * mat2
        m3.resetFromMatrix(mat2); // copy mat2 to mat3

        //mat1.preMultiply(mat2);
		//m3.resetFromMatrix(mat1); // reset m3 to the result of the multiplication

        return s.push(m3);
    }

    // tx ty translate -        ( translate user space by (tx, ty))
	// tx ty matrix translate matrix' - Define translation by (tx, ty)
    inline bool op_translate(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 2)
            return vm.error("op_translate: stack underflow");

        PSObject top;
        s.top(top);
        PSMatrix mat;

        // First check if it's a matrix.  
        // If it is not a matrix, we should have two numbers
        // which will be use to perform translation on CTM
        if (!extractMatrix(top, mat)) 
        {
            PSObject ty, tx;
            s.pop(ty);
            s.pop(tx);

            if (!tx.isNumber() || !ty.isNumber()) 
                return vm.error("op_translate: typecheck");

            ctm.translate(tx.asReal(), ty.asReal());
            return true;
        }
        else {
            // we need to actually pop the top, because we've only
            // done a peek so far
            PSObject matObj;
            s.pop(matObj);

            // we do have a matrix, so now look for two numbers
            PSObject ty, tx;
            s.pop(ty);
            s.pop(tx);

            if (!tx.isNumber() || !ty.isNumber()) return false;

			// we've already extracted the matrix, so we can use it
            mat.translate(tx.asReal(), ty.asReal());
            return s.push(PSObject::fromMatrix(mat));
        }

        // should never reach here
        return false;
    }

    // sx sy scale
    // sx sy matrix scale
    inline bool op_scale(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& ctm = vm.graphics()->getCTM();

        if (s.size() < 2) 
            return vm.error("op_scale: stackunderflow");

        PSObject top;
        s.top(top);
        PSMatrix mat;

        // First check if it's a matrix.  
        // If it is not a matrix, we should have two numbers
        // which will be use to perform translation on CTM
        if (!extractMatrix(top, mat))
        {
            PSObject sy, sx;
            s.pop(sy);
            s.pop(sx);

            if (!sx.isNumber() || !sy.isNumber()) 
                return vm.error("op_scale: typecheck");

            ctm.scale(sx.asReal(), sy.asReal());
            return true;
        }
        else {
            // we need to actually pop the top, because we've only
            // done a peek so far
            PSObject matObj;
            s.pop(matObj);

            // we do have a matrix, so now look for two numbers
            PSObject sy, sx;
            s.pop(sy);
            s.pop(sx);

            if (!sx.isNumber() || !sy.isNumber()) 
                return vm.error("op_scale: typecheck");

            // we've already extracted the matrix, so we can use it
            mat.scale(sx.asReal(), sy.asReal());
            return s.push(PSObject::fromMatrix(mat));
        }

        // should never reach here
        return false;
    }

    // op_rotate
    // 
    // angle rotate
    // angle matrix rotate
    //
    inline bool op_rotate(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        auto& ctm = vm.graphics()->getCTM();

        if (s.empty()) 
            return vm.error("op_rotate: stackunderflow");


        PSObject top;
        s.pop(top);

        if (top.isNumber()) {
            double angle = top.asReal();
            ctm.rotate(angle);

            return true;
        }
        
        PSMatrix mat;
        if (extractMatrix(top, mat))
        {
            PSObject angObj;
            s.pop(angObj);

            if (!angObj.isNumber()) 
                return vm.error("op_rotate: typecheck, expected number");

            mat.rotate(angObj.asReal());
            return s.push(PSObject::fromMatrix(mat));
        }

        return vm.error("op_rotate: typecheck");
    }


} // namespace waavs

namespace waavs {

    inline const PSOperatorFuncMap& getMatrixOps() {
        static const PSOperatorFuncMap table = {
            { "matrix",              op_matrix },
			{ "identmatrix",         op_identmatrix },
            { "invertmatrix",        op_invertmatrix },
            { "concat",              op_concat },
            { "concatmatrix",        op_concatmatrix },
            { "currentmatrix",       op_currentmatrix },
            { "currentscreenmatrix", op_currentscreenmatrix },
            { "transform",           op_transform },
            { "dtransform",          op_dtransform },
            { "itransform",          op_itransform },
            { "idtransform",         op_idtransform },
            { "setmatrix",           op_setmatrix },
            { "initmatrix",          op_initmatrix },
            { "defaultmatrix",       op_defaultmatrix },
            { "translate",           op_translate },
            { "scale",               op_scale },
            { "rotate",              op_rotate }
        };
        return table;
    }


}
