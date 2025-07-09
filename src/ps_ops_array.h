#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

	// ( int -- array )
	inline bool op_array(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.empty()) 
			return vm.error("op_array: stackunderflow");

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
		if (s.empty()) 
			return vm.error("op_aload: stackunderflow");


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
		if (s.empty()) 
			return vm.error("op_astore: stackunderflow");

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
		if (s.size() < 3) 
			return vm.error("op_getinterval: stackunderflow");

		PSObject countObj, indexObj, containerObj;
		s.pop(countObj);
		s.pop(indexObj);
		s.pop(containerObj);

		if (!containerObj.isArray() && !containerObj.isString())
            return vm.error("op_getinterval: typecheck; container not array ir strubg");
			
		if (!indexObj.isInt() || !countObj.isInt())
			return vm.error("op_getinterval: typecheck; index or count not an int");

		int start = indexObj.asInt();
		int count = countObj.asInt();
		PSObject intervalObj;

		if (containerObj.isArray())
		{
			auto arr = containerObj.asArray();

			if (start < 0 || count < 0 || static_cast<size_t>(start + count) > arr->elements.size())
				return vm.error("op_getinterval: rangecheck; array");

			auto sub = PSArray::create();
			sub->elements.insert(
				sub->elements.begin(),
				arr->elements.begin() + start,
				arr->elements.begin() + start + count
			);

            intervalObj.resetFromArray(sub);
		} else if (containerObj.isString())
		{
			auto &str = containerObj.asString();
            auto subStr = str.getInterval(start, count);
			intervalObj.resetFromString(subStr);
        }

		return s.push(intervalObj);
	}

	// ( destArray index srcArray -- )
	inline bool op_putinterval(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.size() < 3) 
			return vm.error("op_putinterval: stackunderflow");


		PSObject srcArrObj, indexObj, destArrObj;
		s.pop(srcArrObj);
		s.pop(indexObj);
		s.pop(destArrObj);

		if (!destArrObj.isArray() ||  !srcArrObj.isArray())
			return vm.error("op_putinterval: typecheck; dest or src not array");

		if (!indexObj.isInt())
            return vm.error("op_putinterval: typecheck; index not int");

		auto dest = destArrObj.asArray();
		auto src = srcArrObj.asArray();
		int index = indexObj.asInt();

		if (index < 0 || static_cast<size_t>(index + src->size()) > dest->size())
			return vm.error("op_putinterval: rangecheck");

		for (size_t i = 0; i < src->size(); ++i) {
			dest->elements[index + i] = src->elements[i];
		}

		return true;
	}


	inline bool op_bind(PSVirtualMachine& vm) {
		PSObject obj;
		if (!vm.opStack().pop(obj)) return false;

		if (!obj.isArray())
			return vm.error("op_bind: typecheck");

		auto arr = obj.asArray();
		if (!arr)
			return vm.error("op_bind: valuecheck");

		for (auto& elem : arr->elements) {
			if (elem.isExecutableName()) {
				// If the name resolves to an operator, then replace it in 
				// the array with the actual operator object.
				PSObject resolved;
				if (vm.dictionaryStack.load(elem.asName(), resolved)) {
					if (resolved.isOperator()) {
						elem.resetFromOperator(resolved.asOperator());
					}
				} 

			}
			// Do NOT recurse into nested procedures
		}

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
