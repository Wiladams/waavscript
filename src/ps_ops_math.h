#pragma once

#include <cmath>
#include "pscore.h"
#include "psvm.h"
#include "ps_scanner.h"
#include "ps_type_file.h"


namespace waavs {

    // ----- Reusable Templates -----
    template<typename Func>
    inline bool unaryMathOp(PSVirtualMachine& vm, Func func)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("unaryMathOp: stackunderflow");

        PSObject a;
        s.pop(a);

        if (!a.isNumber())
            return vm.error("unaryMathOp: typecheck");

        double result = func(a.asReal());

        if (!std::isfinite(result))
            return vm.error("unaryMathOp: undefinedresult");

        return s.pushReal(result);
    }


    template<typename IntFunc, typename RealFunc>
    inline bool typePreservingUnaryMathOp(PSVirtualMachine& vm, IntFunc intFunc, RealFunc realFunc)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("unaryMathOp: stackunderflow");

        PSObject a;
        s.pop(a);

        if (!a.isNumber())
            return vm.error("unaryMathOp: typecheck");

        if (a.isInt()) {
            int64_t result = intFunc(static_cast<int64_t>(a.asInt()));

            if (result >= INT32_MIN && result <= INT32_MAX)
                return s.pushInt(static_cast<int32_t>(result));
        }

        return s.pushReal(realFunc(a.asReal()));
    }

    template<typename IntFunc, typename RealFunc>
    inline bool typePreservingBinaryMathOp(PSVirtualMachine& vm, IntFunc intFunc, RealFunc realFunc)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("typePreservingBinaryMathOp: stackunderflow");

        PSObject b, a;
        s.pop(b);
        s.pop(a);

        if (!a.isNumber() || !b.isNumber())
        {
            writeObjectDeep(a);
            printf("\n");
            writeObjectDeep(b);
            printf("\n");

            return vm.error("typePreservingBinaryMathOp: typecheck");
        }

        if (a.isInt() && b.isInt()) {
            int64_t result = intFunc(static_cast<int64_t>(a.asInt()), static_cast<int64_t>(b.asInt()));

            if (result >= INT32_MIN && result <= INT32_MAX)
                return s.pushInt(static_cast<int32_t>(result));
        }

        return s.pushReal(realFunc(a.asReal(), b.asReal()));
    }



    // ----- Operator Implementations -----
    inline bool op_add(PSVirtualMachine& vm) {
        return typePreservingBinaryMathOp(vm, [](int64_t a, int64_t b) { return a + b; }, [](double a, double b) { return a + b; });
    }

    inline bool op_sub(PSVirtualMachine& vm) {
        return typePreservingBinaryMathOp(vm, [](int64_t a, int64_t b) { return a - b; }, [](double a, double b) { return a - b; });
    }

    inline bool op_mul(PSVirtualMachine& vm) {
        return typePreservingBinaryMathOp(vm, [](int64_t a, int64_t b) { return a * b; }, [](double a, double b) { return a * b; });
    }

    inline bool op_div(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_div: stackunderflow");

        PSObject b;
        PSObject a;

        s.pop(b);
        s.pop(a);

        if (!a.isNumber() || !b.isNumber())
            return vm.error("op_div: typecheck");

        if (b.asReal() == 0.0)
            return vm.error("op_div: undefinedresult");

        return s.pushReal(a.asReal() / b.asReal());
    }
    
    inline bool op_max(PSVirtualMachine& vm) {
        return typePreservingBinaryMathOp(vm, [](int64_t a, int64_t b) { return std::max(a, b); }, [](double a, double b) { return std::max(a, b); });
    }

    inline bool op_min(PSVirtualMachine& vm) {
        return typePreservingBinaryMathOp(vm, [](int64_t a, int64_t b) { return std::min(a, b); }, [](double a, double b) { return std::min(a, b); });
    }

    inline bool op_idiv(PSVirtualMachine& vm) 
    {
        auto& s = vm.opStack();
        if (s.size() < 2) 
            return vm.error("op_idiv: stackunderflow");

        PSObject b, a;
        s.pop(b); 
        s.pop(a);
        
        if (!a.isInt() || !b.isInt())
            return vm.error("op_idiv: typecheck");

        if (b.asInt() == 0) 
            return vm.error("op_idiv: divisor == 0");

        s.pushInt(a.asInt() / b.asInt());
        return true;
    }

    inline bool op_mod(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) 
            return vm.error("op_mod: stackunderflow");

        PSObject b, a;
        s.pop(b);
        s.pop(a);
        
        if (!a.isInt() || !b.isInt())
            return vm.error("op_mod: typecheck");

        if (b.asInt() == 0) 
            return vm.error("op_mod: dividend == 0");
        
        int modValue = a.asInt() % b.asInt();
        s.push(PSObject::fromInt(modValue));

        return true;
    }

    // Unary math
    inline bool op_neg(PSVirtualMachine& vm) {
        return typePreservingUnaryMathOp(vm, [](int64_t a) { return -a; }, [](double a) { return -a; });
    }

    inline bool op_abs(PSVirtualMachine& vm) {
        return typePreservingUnaryMathOp(vm, [](int64_t a) { return a < 0 ? -a : a; }, [](double a) { return std::abs(a); });
    }

    inline bool op_ceiling(PSVirtualMachine& vm) {
        return typePreservingUnaryMathOp(vm, [](int64_t a) { return a; }, [](double a) { return std::ceil(a); });
    }

    inline bool op_floor(PSVirtualMachine& vm) {
        return typePreservingUnaryMathOp(vm, [](int64_t a) { return a; }, [](double a) { return std::floor(a); });
    }

    inline bool op_round(PSVirtualMachine& vm) {
        return typePreservingUnaryMathOp(vm, [](int64_t a) { return a; }, [](double a) { return std::round(a); });
    }

    inline bool op_truncate(PSVirtualMachine& vm) {
        return typePreservingUnaryMathOp(vm, [](int64_t a) { return a; }, [](double a) { return std::trunc(a); });
    }


    inline bool op_sqrt(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double a) { return std::sqrt(a); }); }

    // Trig
    inline bool op_sin(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double angle) { return std::sin(angle * PI / 180.0); }); }
    inline bool op_cos(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double angle) { return std::cos(angle * PI / 180.0); }); }
    inline bool op_ln(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double x) { return std::log(x); }); }
    inline bool op_log(PSVirtualMachine& vm) { return unaryMathOp(vm, [](double x) { return std::log10(x); }); }


    inline bool op_atan(PSVirtualMachine& vm) 
    {
        auto& s = vm.opStack();
    
        if (s.size() < 2) 
            return vm.error("op_atan: stackunderflow");
        
        double dx, dy;

        if (!s.popReal(dx) || !s.popReal(dy)) 
            return vm.error("op_atan: typecheck, expecting two numbers");

        // Note: atan2 returns angle in radians, we convert to degrees
        double angle = std::atan2(dy, dx) * RAD_TO_DEG;
        s.pushReal(angle);

        return true;
    }

    // Exponentials
    inline bool op_exp(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_exp: stackunderflow");

        PSObject exponent, base;
        s.pop(exponent);
        s.pop(base);

        if (!base.isNumber() || !exponent.isNumber())
            return vm.error("op_exp: typecheck");

        double result = std::pow(base.asReal(), exponent.asReal());

        if (!std::isfinite(result))
            return vm.error("op_exp: undefinedresult");

        return s.pushReal(result);
    }

    

    // Random number generator: 31-bit linear congruential sequence.
    // Do it the same way Adobe and Ghostscript do it
    // so we get the same sequence of random numbers.
    inline bool op_rand(PSVirtualMachine& vm)
    {
        constexpr int64_t modulus = 2147483647;
        constexpr int64_t multiplier = 16807;

        int64_t seed = static_cast<int64_t>(vm.randSeed);
        seed = (seed * multiplier) % modulus;

        if (seed <= 0)
            seed += modulus;

        vm.randSeed = static_cast<int32_t>(seed);
        return vm.opStack().pushInt(vm.randSeed);
    }

    inline bool op_srand(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_srand: stackunderflow");

        PSObject obj;
        s.pop(obj);

        if (!obj.isInt())
            return vm.error("op_srand: typecheck");

        int64_t seed = obj.asInt();

        if (seed < 1)
            seed = -(seed % 2147483646LL) + 1;
        else if (seed > 2147483646LL)
            seed = 2147483646LL;

        vm.randSeed = static_cast<int32_t>(seed);
        return true;
    }

    inline bool op_rrand(PSVirtualMachine& vm)
    {
        return vm.opStack().pushInt(vm.randSeed);
    }

    // ----------------  op_cvi  ----------------------------
    inline bool op_cvi(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_cvi: stackunderflow");

        PSObject value;
        s.pop(value);

        if (value.isInt())
            return s.push(value);

        double number = 0.0;

        if (value.isReal()) {
            number = value.asReal();
        }
        else if (value.isString()) {
            const PSString& str = value.asString();

            if (!value.isAccessReadable())
                return vm.error("op_cvi: invalidaccess");

            OctetCursor cursor(str.data(), str.length());
            auto file = PSMemoryFile::create(cursor);
            PSLexGenState lexState(file);
            PSLexeme lex;

            bool found = false;

            while (nextPSLexeme(lexState, lex)) {
                if (lex.type == PSLexType::Whitespace || lex.type == PSLexType::Comment || lex.type == PSLexType::DSCComment)
                    continue;

                found = true;
                break;
            }

            if (!found || (lex.type != PSLexType::Name && lex.type != PSLexType::Number))
                return vm.error("op_cvi: syntaxerror");

            PSParsedNumber pnum = parsePSNumber(lex.span);

            switch (pnum.kind) {
            case PSParsedNumber::Kind::Integer:
                return s.pushInt(pnum.integer);

            case PSParsedNumber::Kind::Real:
                number = pnum.real;
                break;

            case PSParsedNumber::Kind::LimitCheck:
                return vm.error("op_cvi: limitcheck");

            case PSParsedNumber::Kind::NotNumber:
            default:
                return vm.error("op_cvi: syntaxerror");
            }
        }
        else {
            return vm.error("op_cvi: typecheck");
        }

        if (!std::isfinite(number))
            return vm.error("op_cvi: rangecheck");

        double truncated = std::trunc(number);

        if (truncated < -2147483648.0 || truncated >= 2147483648.0)
            return vm.error("op_cvi: rangecheck");

        return s.pushInt(static_cast<int32_t>(truncated));
    }

    // -----------  op_cvr  ----------------------------
    inline bool op_cvr(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_cvr: stackunderflow");

        PSObject obj;
        s.pop(obj);

        if (obj.isReal())
            return s.push(obj);

        if (obj.isInt())
            return s.pushReal(static_cast<double>(obj.asInt()));

        if (!obj.isString())
            return vm.error("op_cvr: typecheck");

        if (!obj.isAccessReadable())
            return vm.error("op_cvr: invalidaccess");

        const PSString& str = obj.asString();
        OctetCursor cursor(str.data(), str.length());

        auto file = PSMemoryFile::create(cursor);
        PSLexGenState lexState(file);
        PSLexeme lex;

        bool found = false;

        while (nextPSLexeme(lexState, lex)) {
            if (lex.type == PSLexType::Whitespace || lex.type == PSLexType::Comment || lex.type == PSLexType::DSCComment)
                continue;

            found = true;
            break;
        }

        if (!found || (lex.type != PSLexType::Name && lex.type != PSLexType::Number))
            return vm.error("op_cvr: syntaxerror");

        PSParsedNumber pnum = parsePSNumber(lex.span);

        switch (pnum.kind) {
        case PSParsedNumber::Kind::Integer:
            return s.pushReal(static_cast<double>(pnum.integer));

        case PSParsedNumber::Kind::Real:
            return s.pushReal(pnum.real);

        case PSParsedNumber::Kind::LimitCheck:
            return vm.error("op_cvr: limitcheck");

        case PSParsedNumber::Kind::NotNumber:
        default:
            return vm.error("op_cvr: syntaxerror");
        }
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
            //{ "max",      op_max },         // ghostcript extension
            //{ "min",      op_min },         // ghostcript extension
            { ".max",     op_max },         // Common alias
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
            { "cvr",      op_cvr },
        };
        return table;
    }

} // namespace waavs
