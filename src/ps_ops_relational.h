#pragma once

#include "pscore.h"

namespace waavsps {
	template <typename Func>
	inline bool binaryCompareOp(PSVirtualMachine& vm, Func func) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		PSObject b = s.back(); s.pop_back();
		PSObject a = s.back(); s.pop_back();

		if (!a.isNumber() || !b.isNumber()) return false;

		double av = a.asReal();
		double bv = b.asReal();
		s.push_back(PSObject::fromBool(func(av, bv)));
		return true;
	}

	inline bool op_gt(PSVirtualMachine& vm) {
		return binaryCompareOp(vm, [](double a, double b) { return a > b; });
	}

	inline bool op_lt(PSVirtualMachine& vm) {
		return binaryCompareOp(vm, [](double a, double b) { return a < b; });
	}

	inline bool op_ge(PSVirtualMachine& vm) {
		return binaryCompareOp(vm, [](double a, double b) { return a >= b; });
	}

	inline bool op_le(PSVirtualMachine& vm) {
		return binaryCompareOp(vm, [](double a, double b) { return a <= b; });
	}


}