#pragma once

#include "pscore.h"
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
    inline bool binaryMathOp(PSVirtualMachine& vm, Func func) 
    {
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


	//===========================
	// Binary operators
	//===========================
	inline bool op_add(PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a + b; });
	}

	inline bool op_sub(PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a - b; });
	}

	inline bool op_mul(PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a * b; });
	}

	inline bool op_div(PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a / b; });
	}

	inline bool op_idiv(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		PSObject b = s.back(); s.pop_back();
		PSObject a = s.back(); s.pop_back();
		if (a.type != PSObjectType::Int || b.type != PSObjectType::Int) return false;

		if (b.data.iVal == 0) return false;
		s.push_back(PSObject::fromInt(a.data.iVal / b.data.iVal));
		return true;
	}

	inline bool op_mod(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		PSObject b = s.back(); s.pop_back();
		PSObject a = s.back(); s.pop_back();
		if (a.type != PSObjectType::Int || b.type != PSObjectType::Int) return false;

		if (b.data.iVal == 0) return false;
		s.push_back(PSObject::fromInt(a.data.iVal % b.data.iVal));
		return true;
	}

	inline bool op_max(PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return std::max(a, b); });
	}

	inline bool op_min(PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return std::min(a, b); });
	}


	//===========================
	// Unary operators
	//===========================
	inline bool op_neg(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return -a; });
	}

	inline bool op_abs(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::abs(a); });
	}

	inline bool op_sqrt(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::sqrt(a); });
	}

	inline bool op_ceiling(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::ceil(a); });
	}

	inline bool op_floor(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::floor(a); });
	}

	inline bool op_round(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::round(a); });
	}

	inline bool op_truncate(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return a < 0 ? std::ceil(a) : std::floor(a); });
	}


	//===========================
	// Trigonometric functions
	//===========================
	inline bool op_sin(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double angle) {
			return std::sin(angle * PI / 180.0); // deg ? rad
			});
	}

	inline bool op_cos(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double angle) {
			return std::cos(angle * PI / 180.0); // deg ? rad
			});
	}

	inline bool op_atan(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		auto b = s.back(); s.pop_back(); // dx
		auto a = s.back(); s.pop_back(); // dy
		if (!a.isNumber() || !b.isNumber()) return false;

		double dy = a.asReal();
		double dx = b.asReal();

		double angle = std::atan2(dy, dx) * 180.0 / PI; // rad ? deg
		s.push_back(PSObject::fromReal(angle));
		return true;
	}

	//===========================
	// Exponential and logarithmic functions
	//===========================
	inline bool op_exp(PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double base, double exponent) {
			return std::pow(base, exponent);
			});
	}

	inline bool op_ln(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double x) {
			return std::log(x); // natural log
			});
	}

	inline bool op_log(PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double x) {
			return std::log10(x); // base-10
			});
	}


	//===========================
	// Random number generation
	//===========================
	inline bool op_rand(PSVirtualMachine& vm) {
		vm.randSeed = (1103515245 * vm.randSeed + 12345) & 0x7FFFFFFF;
		vm.operandStack.push_back(PSObject::fromInt(vm.randSeed));
		return true;
	}

	inline bool op_srand(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.empty()) return false;
		auto obj = s.back(); s.pop_back();
		if (obj.type != PSObjectType::Int) return false;
		vm.randSeed = obj.data.iVal & 0x7FFFFFFF;
		return true;
	}

	inline bool op_rrand(PSVirtualMachine& vm) {
		vm.operandStack.push_back(PSObject::fromInt(vm.randSeed));
		return true;
	}


}