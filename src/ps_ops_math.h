#pragma once

#include "pscore.h"
#include <cmath>

namespace waavsps {
#ifndef PI
#    define PI 3.14159265358979323846
#endif

	template <typename Func>
	inline bool unaryMathOp(waavsps::PSVirtualMachine& vm, Func func) {
		auto& s = vm.operandStack;
		if (s.empty()) return false;

		waavsps::PSObject a = s.back(); s.pop_back();
		if (!a.isNumber()) return false;

		double result = func(a.asReal());
		s.push_back(waavsps::PSObject::fromReal(result));
		return true;
	}

    template <typename Func>
    inline bool binaryMathOp(waavsps::PSVirtualMachine& vm, Func func) 
    {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;

        waavsps::PSObject b = s.back(); s.pop_back();
        waavsps::PSObject a = s.back(); s.pop_back();
        if (!a.isNumber() || !b.isNumber()) return false;

        double av = a.asReal();
        double bv = b.asReal();
        double result = func(av, bv);

        s.push_back(waavsps::PSObject::fromReal(result));
        return true;
    }


	//===========================
	// Binary operators
	//===========================
	inline bool op_add(waavsps::PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a + b; });
	}

	inline bool op_sub(waavsps::PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a - b; });
	}

	inline bool op_mul(waavsps::PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a * b; });
	}

	inline bool op_div(waavsps::PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return a / b; });
	}

	inline bool op_idiv(waavsps::PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		waavsps::PSObject b = s.back(); s.pop_back();
		waavsps::PSObject a = s.back(); s.pop_back();
		if (a.type != waavsps::PSObjectType::Int || b.type != waavsps::PSObjectType::Int) return false;

		if (b.data.iVal == 0) return false;
		s.push_back(waavsps::PSObject::fromInt(a.data.iVal / b.data.iVal));
		return true;
	}

	inline bool op_mod(waavsps::PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		waavsps::PSObject b = s.back(); s.pop_back();
		waavsps::PSObject a = s.back(); s.pop_back();
		if (a.type != waavsps::PSObjectType::Int || b.type != waavsps::PSObjectType::Int) return false;

		if (b.data.iVal == 0) return false;
		s.push_back(waavsps::PSObject::fromInt(a.data.iVal % b.data.iVal));
		return true;
	}

	inline bool op_max(waavsps::PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return std::max(a, b); });
	}

	inline bool op_min(waavsps::PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double a, double b) { return std::min(a, b); });
	}


	//===========================
	// Unary operators
	//===========================
	inline bool op_neg(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return -a; });
	}

	inline bool op_abs(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::abs(a); });
	}

	inline bool op_sqrt(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::sqrt(a); });
	}

	inline bool op_ceiling(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::ceil(a); });
	}

	inline bool op_floor(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::floor(a); });
	}

	inline bool op_round(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return std::round(a); });
	}

	inline bool op_truncate(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double a) { return a < 0 ? std::ceil(a) : std::floor(a); });
	}


	//===========================
	// Trigonometric functions
	//===========================
	inline bool op_sin(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double angle) {
			return std::sin(angle * PI / 180.0); // deg ? rad
			});
	}

	inline bool op_cos(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double angle) {
			return std::cos(angle * PI / 180.0); // deg ? rad
			});
	}

	inline bool op_atan(waavsps::PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		auto b = s.back(); s.pop_back(); // dx
		auto a = s.back(); s.pop_back(); // dy
		if (!a.isNumber() || !b.isNumber()) return false;

		double dy = a.asReal();
		double dx = b.asReal();

		double angle = std::atan2(dy, dx) * 180.0 / PI; // rad ? deg
		s.push_back(waavsps::PSObject::fromReal(angle));
		return true;
	}

	//===========================
	// Exponential and logarithmic functions
	//===========================
	inline bool op_exp(waavsps::PSVirtualMachine& vm) {
		return binaryMathOp(vm, [](double base, double exponent) {
			return std::pow(base, exponent);
			});
	}

	inline bool op_ln(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double x) {
			return std::log(x); // natural log
			});
	}

	inline bool op_log(waavsps::PSVirtualMachine& vm) {
		return unaryMathOp(vm, [](double x) {
			return std::log10(x); // base-10
			});
	}


	//===========================
	// Random number generation
	//===========================
	inline bool op_rand(waavsps::PSVirtualMachine& vm) {
		vm.randSeed = (1103515245 * vm.randSeed + 12345) & 0x7FFFFFFF;
		vm.operandStack.push_back(waavsps::PSObject::fromInt(vm.randSeed));
		return true;
	}

	inline bool op_srand(waavsps::PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.empty()) return false;
		auto obj = s.back(); s.pop_back();
		if (obj.type != waavsps::PSObjectType::Int) return false;
		vm.randSeed = obj.data.iVal & 0x7FFFFFFF;
		return true;
	}

	inline bool op_rrand(waavsps::PSVirtualMachine& vm) {
		vm.operandStack.push_back(waavsps::PSObject::fromInt(vm.randSeed));
		return true;
	}


}