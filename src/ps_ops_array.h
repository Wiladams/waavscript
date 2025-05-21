#pragma once

#include "pscore.h"

namespace waavsps {
    inline bool op_array(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject lenObj = s.back(); s.pop_back();
        if (lenObj.type != PSObjectType::Int || lenObj.data.iVal < 0) return false;

        auto* arr = new PSArray();
        arr->elements.resize(static_cast<size_t>(lenObj.data.iVal));
        s.push_back(PSObject::fromArray(arr));
        return true;
    }

	inline bool op_aload(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.empty()) return false;

		PSObject arrObj = s.back(); s.pop_back();
		if (arrObj.type != PSObjectType::Array || !arrObj.data.arr)
			return false;

		for (const PSObject& elem : arrObj.data.arr->elements)
			s.push_back(elem);

		s.push_back(arrObj); // push original array back onto the stack
		return true;
	}

	inline bool op_astore(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.empty()) return false;

		PSObject arrObj = s.back(); s.pop_back();
		if (arrObj.type != PSObjectType::Array || !arrObj.data.arr)
			return false;

		PSArray* arr = arrObj.data.arr;
		size_t count = arr->elements.size();
		if (s.size() < count) return false;

		// Store top stack elements into array, in reverse order
		for (size_t i = 0; i < count; ++i) {
			arr->elements[count - 1 - i] = s.back();
			s.pop_back();
		}

		s.push_back(arrObj); // push array back
		return true;
	}


	inline bool op_getinterval(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 3) return false;

		PSObject countObj = s.back(); s.pop_back();
		PSObject indexObj = s.back(); s.pop_back();
		PSObject arrObj = s.back(); s.pop_back();

		if (arrObj.type != PSObjectType::Array || indexObj.type != PSObjectType::Int || countObj.type != PSObjectType::Int)
			return false;

		auto* src = arrObj.data.arr;
		int start = indexObj.data.iVal;
		int count = countObj.data.iVal;

		if (!src || start < 0 || count < 0 || static_cast<size_t>(start + count) > src->elements.size())
			return false;

		auto* sub = new PSArray();
		sub->elements.insert(
			sub->elements.begin(),
			src->elements.begin() + start,
			src->elements.begin() + start + count
		);

		s.push_back(PSObject::fromArray(sub));
		return true;
	}

	inline bool op_putinterval(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 3) return false;

		PSObject srcArrObj = s.back(); s.pop_back();
		PSObject indexObj = s.back(); s.pop_back();
		PSObject destArrObj = s.back(); s.pop_back();

		if (destArrObj.type != PSObjectType::Array ||
			indexObj.type != PSObjectType::Int ||
			srcArrObj.type != PSObjectType::Array)
			return false;

		auto* dest = destArrObj.data.arr;
		auto* src = srcArrObj.data.arr;
		int index = indexObj.data.iVal;

		if (!dest || !src || index < 0 || static_cast<size_t>(index + src->elements.size()) > dest->elements.size())
			return false;

		for (size_t i = 0; i < src->elements.size(); ++i) {
			dest->elements[index + i] = src->elements[i];
		}

		return true;
	}



}