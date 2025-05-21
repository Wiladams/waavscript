#pragma once

#include "pscore.h"

namespace waavsps {
	inline bool op_and(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		PSObject b = s.back(); s.pop_back();
		PSObject a = s.back(); s.pop_back();

		if (a.type == PSObjectType::Bool && b.type == PSObjectType::Bool) {
			s.push_back(PSObject::fromBool(a.data.bVal && b.data.bVal));
			return true;
		}
		if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
			s.push_back(PSObject::fromInt(a.data.iVal & b.data.iVal));
			return true;
		}
		return false;
	}

	inline bool op_or(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		PSObject b = s.back(); s.pop_back();
		PSObject a = s.back(); s.pop_back();

		if (a.type == PSObjectType::Bool && b.type == PSObjectType::Bool) {
			s.push_back(PSObject::fromBool(a.data.bVal || b.data.bVal));
			return true;
		}
		if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
			s.push_back(PSObject::fromInt(a.data.iVal | b.data.iVal));
			return true;
		}
		return false;
	}

	inline bool op_xor(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		PSObject b = s.back(); s.pop_back();
		PSObject a = s.back(); s.pop_back();

		if (a.type == PSObjectType::Bool && b.type == PSObjectType::Bool) {
			s.push_back(PSObject::fromBool(a.data.bVal != b.data.bVal));
			return true;
		}
		if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
			s.push_back(PSObject::fromInt(a.data.iVal ^ b.data.iVal));
			return true;
		}
		return false;
	}

	inline bool op_not(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.empty()) return false;

		PSObject a = s.back(); s.pop_back();

		if (a.type == PSObjectType::Bool) {
			s.push_back(PSObject::fromBool(!a.data.bVal));
			return true;
		}
		if (a.type == PSObjectType::Int) {
			s.push_back(PSObject::fromInt(~a.data.iVal));
			return true;
		}
		return false;
	}
}