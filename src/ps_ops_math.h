#pragma once

#include "pscore.h"
#include "psvm.h"
#include <cmath>

namespace waavs {

#ifndef PI
#    define PI 3.14159265358979323846
#endif

    template <typename Func>
    inline bool unaryMathOp(PSVirtualMachine& vm, Func func) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject a = s.back(); s.pop_back();
        if (!a.isNumber()) return false;

        double result = func(a.asReal());
        s.push_back(PSObject::fromReal(result));
        return true;
    }

    template <typename Func>
    inline bool binaryMathOp(PSVirtualMachine& vm, Func func) {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;

        PSObject b = s.back(); s.pop_back();
        PSObject a = s.back(); s.pop_back();
        if (!a.isNumber() || !b.isNumber()) return false;

        double av = a.asReal();
        double bv = b.asReal();
        double result = func(av, bv);

        s.push_back(PSObject::fromReal(result));
        return true;
    }

    static const PSOperatorFuncMap mathOps = {
        // Arithmetic
        { "add", [](PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return a + b; }); }},
        { "sub", [](PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return a - b; }); }},
        { "mul", [](PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return a * b; }); }},
        { "div", [](PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return a / b; }); }},
        { "idiv", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;

            PSObject b = s.back(); s.pop_back();
            PSObject a = s.back(); s.pop_back();
            if (a.type != PSObjectType::Int || b.type != PSObjectType::Int || b.data.iVal == 0) return false;

            s.push_back(PSObject::fromInt(a.data.iVal / b.data.iVal));
            return true;
        }},
        { "mod", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;

            PSObject b = s.back(); s.pop_back();
            PSObject a = s.back(); s.pop_back();
            if (a.type != PSObjectType::Int || b.type != PSObjectType::Int || b.data.iVal == 0) return false;

            s.push_back(PSObject::fromInt(a.data.iVal % b.data.iVal));
            return true;
        }},
        { "max", [](PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return std::max(a, b); }); }},
        { "min", [](PSVirtualMachine& vm) { return binaryMathOp(vm, [](double a, double b) { return std::min(a, b); }); }},

        // Unary math
        { "neg", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return -a; }); }},
        { "abs", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::abs(a); }); }},
        { "sqrt", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::sqrt(a); }); }},
        { "ceiling", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::ceil(a); }); }},
        { "floor", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::floor(a); }); }},
        { "round", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::round(a); }); }},
        { "truncate", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return a < 0 ? std::ceil(a) : std::floor(a); }); }},

        // Trigonometry
        { "sin", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double angle) { return std::sin(angle * PI / 180.0); }); }},
        { "cos", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double angle) { return std::cos(angle * PI / 180.0); }); }},
        { "atan", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;

            PSObject dx = s.back(); s.pop_back();
            PSObject dy = s.back(); s.pop_back();
            if (!dx.isNumber() || !dy.isNumber()) return false;

            double angle = std::atan2(dy.asReal(), dx.asReal()) * 180.0 / PI;
            s.push_back(PSObject::fromReal(angle));
            return true;
        }},

        // Exponential and logarithmic
        { "exp", [](PSVirtualMachine& vm) { return binaryMathOp(vm, [](double base, double exp) { return std::pow(base, exp); }); }},
        { "ln",  [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double x) { return std::log(x); }); }},
        { "log", [](PSVirtualMachine& vm) { return unaryMathOp(vm, [](double x) { return std::log10(x); }); }},

        // Random number generator
        { "rand", [](PSVirtualMachine& vm) -> bool {
            vm.randSeed = (1103515245 * vm.randSeed + 12345) & 0x7FFFFFFF;
            vm.operandStack.push_back(PSObject::fromInt(vm.randSeed));
            return true;
        }},
        { "srand", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.empty()) return false;
            PSObject obj = s.back(); s.pop_back();
            if (obj.type != PSObjectType::Int) return false;
            vm.randSeed = obj.data.iVal & 0x7FFFFFFF;
            return true;
        }},
        { "rrand", [](PSVirtualMachine& vm) -> bool {
            vm.operandStack.push_back(PSObject::fromInt(vm.randSeed));
            return true;
        }}
    };

} // namespace waavs
