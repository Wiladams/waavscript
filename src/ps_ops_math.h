#pragma once

#include <cmath>
#include "pscore.h"
#include "psvm.h"

namespace waavs {

#ifndef PI
#  define PI 3.14159265358979323846
#endif

    // ----- Reusable Templates -----

    template <typename Func>
    inline bool unaryMathOp(PSVirtualMachine& vm, Func func) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject a;
        s.pop(a);
        if (!a.isNumber()) return false;

        double result = func(a.asReal());
        s.push(PSObject::fromReal(result));
        return true;
    }

    template <typename Func>
    inline bool binaryMathOp(PSVirtualMachine& vm, Func func) {
        auto& s = vm.opStack();
        if (s.size() < 2) 
            return false;

        PSObject b, a;
        s.pop(b); s.pop(a);

        if (!a.isNumber() || !b.isNumber()) 
            return false;

        double result = func(a.asReal(), b.asReal());
        s.push(PSObject::fromReal(result));

        return true;
    }

    // ----- Operator Implementations -----

    inline bool op_add(PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return a + b; }); }
    inline bool op_sub(PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return a - b; }); }
    inline bool op_mul(PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return a * b; }); }
    inline bool op_div(PSVirtualMachine& vm) 
    { 
        return binaryMathOp(vm, [](double a, double b) { return a / b; }); 
    }
    inline bool op_max(PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return std::max(a, b); }); }
    inline bool op_min(PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return std::min(a, b); }); }

    inline bool op_idiv(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;
        PSObject b, a;
        s.pop(b); s.pop(a);
        if (!a.isInt() || !b.isInt() || b.asInt() == 0) return false;
        s.push(PSObject::fromInt(a.asInt() / b.asInt()));
        return true;
    }

    inline bool op_mod(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;
        PSObject b, a;
        s.pop(b); s.pop(a);
        if (!a.isInt() || !b.isInt() || b.asInt() == 0) 
            return false;
        int modValue = a.asInt() % b.asInt();
        s.push(PSObject::fromInt(modValue));

        return true;
    }

    // Unary math
    inline bool op_neg(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return -a; }); }
    inline bool op_abs(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::abs(a); }); }
    inline bool op_sqrt(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::sqrt(a); }); }
    inline bool op_ceiling(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::ceil(a); }); }
    inline bool op_floor(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::floor(a); }); }
    inline bool op_round(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::round(a); }); }
    inline bool op_truncate(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return a < 0 ? std::ceil(a) : std::floor(a); }); }

    // Trig
    inline bool op_sin(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double angle) { return std::sin(angle * PI / 180.0); }); }
    inline bool op_cos(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double angle) { return std::cos(angle * PI / 180.0); }); }

    inline bool op_atan(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;
        PSObject dx, dy;
        s.pop(dx); s.pop(dy);
        if (!dx.isNumber() || !dy.isNumber()) return false;
        double angle = std::atan2(dy.asReal(), dx.asReal()) * 180.0 / PI;
        s.push(PSObject::fromReal(angle));
        return true;
    }

    // Exponentials
    inline bool op_exp(PSVirtualMachine& vm) { return binaryMathOp(vm, [](double base, double exp) { return std::pow(base, exp); }); }
    inline bool op_ln(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double x) { return std::log(x); }); }
    inline bool op_log(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double x) { return std::log10(x); }); }

    // Random
    inline bool op_rand(PSVirtualMachine& vm) {
        vm.randSeed = (1103515245 * vm.randSeed + 12345) & 0x7FFFFFFF;
        int randomValue = vm.randSeed;
        vm.opStack().push(PSObject::fromInt(randomValue));
        return true;
    }

    inline bool op_srand(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) 
            return vm.error("op_srand: stackunderflow");

        PSObject obj;
        s.pop(obj);
        if (!obj.isInt())
            return vm.error("op_srand: typecheck");
        vm.randSeed = obj.asInt() & 0x7FFFFFFF;

        return true;
    }

    inline bool op_rrand(PSVirtualMachine& vm) {
        vm.opStack().push(PSObject::fromInt(vm.randSeed));
        return true;
    }

    inline bool op_cvi(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty())
            return vm.error("stackunderflow");

        PSObject top;
        s.pop(top);

        if (!top.isNumber())
            return vm.error("typecheck: cvi requires a numeric operand");

        double val = top.asReal();
        int32_t ival = static_cast<int32_t>(val); // truncate toward zero

        s.push(PSObject::fromInt(ival));
        return true;
    }

    // ----- Operator Map -----

    inline const PSOperatorFuncMap& getMathOps() {
        static const PSOperatorFuncMap table = {
            { "add",      op_add },
            { "sub",      op_sub },
            { "mul",      op_mul },
            { "div",      op_div },
            { "idiv",     op_idiv },
            { "mod",      op_mod },
            { "max",      op_max },
            { ".max",     op_max },         // Common alias
            { "min",      op_min },
            { ".min",     op_min},          // Common alias
            { "neg",      op_neg },
            { "abs",      op_abs },
            { "sqrt",     op_sqrt },
            { "ceiling",  op_ceiling },
            { "floor",    op_floor },
            { "round",    op_round },
            { "truncate", op_truncate },
            { "sin",      op_sin },
            { "cos",      op_cos },
            { "atan",     op_atan },
            { "exp",      op_exp },
            { "ln",       op_ln },
            { "log",      op_log },
            { "rand",     op_rand },
            { "srand",    op_srand },
            { "rrand",    op_rrand },

            { "cvi",      op_cvi },
        };
        return table;
    }

} // namespace waavs
