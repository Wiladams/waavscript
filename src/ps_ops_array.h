#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

	inline void bindArray(PSVirtualMachine& vm, PSArrayHandle arr) {
		if (!arr || !arr->isProcedure()) return;

		for (auto& obj : arr->elements) {
			if (obj.isName() && obj.isExecutable()) {
				PSObject resolved;
				if (vm.dictionaryStack.load(obj.asName(), resolved)) {
					if (resolved.isOperator()) {
						obj.resetFromOperator(resolved.asOperator());
					}
				}
			}
			// Do NOT recurse into nested procedures
		}
	}



	// ( int -- array )
	inline bool op_array(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.empty()) return false;

		PSObject lenObj;
		s.pop(lenObj);
		if (!lenObj.isInt() || lenObj.asInt() < 0)
			return false;

		auto arr = PSArray::create();
		arr->elements.resize(static_cast<size_t>(lenObj.asInt()));
		return s.push(PSObject::fromArray(arr));
	}

	// ( array -- ... elements ... array )
	inline bool op_aload(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.empty()) return false;

		PSObject arrObj;
		s.pop(arrObj);
		if (!arrObj.isArray()) return false;

		auto arr = arrObj.asArray();
		for (const PSObject& elem : arr->elements)
			s.push(elem);

		return s.push(arrObj); // push original array back
	}

	// ( ... elements ... array -- array )
	inline bool op_astore(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.empty()) return false;

		PSObject arrObj;
		s.pop(arrObj);
		if (!arrObj.isArray()) return false;

		auto arr = arrObj.asArray();
		size_t count = arr->size();
		if (s.size() < count) return false;

		for (size_t i = 0; i < count; ++i) {
			PSObject val;
			s.pop(val);
			arr->elements[count - 1 - i] = val;
		}

		return s.push(arrObj);
	}

	// ( array index count -- subarray )
	inline bool op_getinterval(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.size() < 3) return false;

		PSObject countObj, indexObj, arrObj;
		s.pop(countObj);
		s.pop(indexObj);
		s.pop(arrObj);

		if (!arrObj.isArray() || !indexObj.isInt() || !countObj.isInt())
			return false;

		auto src = arrObj.asArray();
		int start = indexObj.asInt();
		int count = countObj.asInt();

		if (start < 0 || count < 0 || static_cast<size_t>(start + count) > src->elements.size())
			return false;

		auto sub = PSArray::create();
		sub->elements.insert(
			sub->elements.begin(),
			src->elements.begin() + start,
			src->elements.begin() + start + count
		);

		return s.push(PSObject::fromArray(sub));
	}

	// ( destArray index srcArray -- )
	inline bool op_putinterval(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.size() < 3) return false;

		PSObject srcArrObj, indexObj, destArrObj;
		s.pop(srcArrObj);
		s.pop(indexObj);
		s.pop(destArrObj);

		if (!destArrObj.isArray() || !indexObj.isInt() || !srcArrObj.isArray())
			return false;

		auto dest = destArrObj.asArray();
		auto src = srcArrObj.asArray();
		int index = indexObj.asInt();

		if (index < 0 || static_cast<size_t>(index + src->size()) > dest->size())
			return false;

		for (size_t i = 0; i < src->size(); ++i) {
			dest->elements[index + i] = src->elements[i];
		}

		return true;
	}

	inline bool op_bind(PSVirtualMachine& vm) {
		PSObject obj;
		if (!vm.opStack().pop(obj)) return false;

		if (!obj.isArray()) return false;

		auto arr = obj.asArray();
		if (!arr || !arr->isProcedure()) return false;

		bindArray(vm, arr);
		vm.opStack().push(obj);

		return true;
	}

	// Returns the array operator table
	inline const PSOperatorFuncMap& getArrayOps() {
		static const PSOperatorFuncMap table = {
			{ "array",       op_array },
			{ "aload",       op_aload },
			{ "astore",      op_astore },
			{ "getinterval", op_getinterval },
			{ "putinterval", op_putinterval },
			{ "bind",        op_bind  }
		};
		return table;
	}

} // namespace waavs
