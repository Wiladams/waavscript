#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

	// ( int -- array )
	inline bool op_array(PSVirtualMachine& vm) 
	{
		auto& ostk = vm.opStack();
		if (ostk.empty()) 
			return vm.error("op_array: stackunderflow");

		int32_t len;
        if (!ostk.popInt(len))
			return vm.error("op_array: typecheck");

        if (len < 0)
            return vm.error("op_array: rangecheck");

		return ostk.pushArray(PSArray::create(static_cast<size_t>(len)));
	}

	// ( array -- ... elements ... array )
	inline bool op_aload(PSVirtualMachine& vm) 
	{
		auto& s = vm.opStack();
		if (s.empty()) 
			return vm.error("op_aload: stackunderflow");

		PSArrayHandle arr;
		if (!s.popArray(arr))
			return vm.error("op_load: typecheck");

		for (const auto& elem : *arr)
			s.push(elem);

		return s.pushArray(arr); // push original array back
	}

	// ( ... elements ... array -- array )
	inline bool op_astore(PSVirtualMachine& vm) {
		auto& s = vm.opStack();
		if (s.empty()) 
			return vm.error("op_astore: stackunderflow");

		PSArrayHandle arr;
        if (!s.popArray(arr))
			return vm.error("op_astore: typeckeck");

		size_t count = arr->size();
		if (s.size() < count) 
			return false;

		for (size_t i = 0; i < count; ++i) {
			PSObject val;
			s.pop(val);
			(*arr)[count - 1 - i] = val;
		}

		return s.pushArray(arr);
	}

	inline bool op_getinterval(PSVirtualMachine& vm)
	{
		auto& s = vm.opStack();

		if (s.size() < 3)
			return vm.error("op_getinterval: stackunderflow");

		int32_t start;
		int32_t count;
		PSObject containerObj;

		if (!s.popInt(count) || !s.popInt(start))
			return vm.error("op_getinterval: typecheck");

		if (!s.pop(containerObj))
			return vm.error("op_getinterval: stackunderflow");

		if (!containerObj.isArray() && !containerObj.isString())
			return vm.error("op_getinterval: typecheck");

		if (start < 0 || count < 0)
			return vm.error("op_getinterval: rangecheck");

		size_t index = static_cast<size_t>(start);
		size_t length = static_cast<size_t>(count);

		if (containerObj.isArray())
		{
			auto arr = containerObj.asArray();

			if (index > arr->size() || length > arr->size() - index)
				return vm.error("op_getinterval: rangecheck");

			auto sub = arr->subarray(index, length);
			if (!sub)
				return vm.error("op_getinterval: rangecheck");

			return s.pushArray(sub);
		}

		auto str = containerObj.asString();

		if (index > str.length() || length > str.length() - index)
			return vm.error("op_getinterval: rangecheck");

		auto subStr = str.getInterval(start, count);
		return s.pushString(subStr);
	}

	// ( destArray index srcArray -- )
	inline bool op_putinterval(PSVirtualMachine& vm)
	{
		auto& s = vm.opStack();

		if (s.size() < 3)
			return vm.error("op_putinterval: stackunderflow");

		PSObject srcObj;
		PSObject indexObj;
		PSObject destObj;

		s.pop(srcObj);
		s.pop(indexObj);
		s.pop(destObj);

		if (indexObj.type != PSObjectType::Int)
			return vm.error("op_putinterval: typecheck");

		int32_t indexValue = indexObj.asInt();

		if (indexValue < 0)
			return vm.error("op_putinterval: rangecheck");

		size_t index = static_cast<size_t>(indexValue);

		if (destObj.isArray())
		{
			if (!srcObj.isArray())
				return vm.error("op_putinterval: typecheck");

			if (!destObj.isAccessWriteable() || !srcObj.isAccessReadable())
				return vm.error("op_putinterval: invalidaccess");

			auto dest = destObj.asArray();
			auto src = srcObj.asArray();

			if (index > dest->size() || src->size() > dest->size() - index)
				return vm.error("op_putinterval: rangecheck");

			for (size_t i = 0; i < src->size(); ++i)
			{
				PSObject value;

				if (!src->get(i, value))
					return vm.error("op_putinterval: rangecheck");

				if (!dest->put(index + i, value))
					return vm.error("op_putinterval: rangecheck");
			}

			return true;
		}

		if (destObj.isString())
		{
			if (!srcObj.isString())
				return vm.error("op_putinterval: typecheck");

			if (!destObj.isAccessWriteable() || !srcObj.isAccessReadable())
				return vm.error("op_putinterval: invalidaccess");

			PSString dest = destObj.asString();
			PSString src = srcObj.asString();

			if (index > dest.length() || src.length() > dest.length() - index)
				return vm.error("op_putinterval: rangecheck");

			for (size_t i = 0; i < src.length(); ++i)
			{
				uint8_t value;

				if (!src.get(static_cast<uint32_t>(i), value))
					return vm.error("op_putinterval: rangecheck");

				if (!dest.put(static_cast<uint32_t>(index + i), value))
					return vm.error("op_putinterval: rangecheck");
			}

			return true;
		}

		return vm.error("op_putinterval: typecheck");
	}


	inline bool op_bind(PSVirtualMachine& vm) {
        auto& ostk = vm.opStack();

        if (ostk.empty())
            return vm.error("op_bind: stackunderflow");
		
		PSArrayHandle arr;
		if (!ostk.popArray(arr))
			return vm.error("op_bind: typecheck; not array");;

		for (const auto& elem : *arr) {
			if (elem.isExecutableName()) {
				// If the name resolves to an operator, then replace it in 
				// the array with the actual operator object.
				PSObject resolved;
				if (vm.dictionaryStack.load(elem.asName(), resolved)) {
					if (resolved.isOperator()) {
						const_cast<PSObject&>(elem).resetFromOperator(resolved.asOperator());
					}
				} 
			}
			// Do NOT recurse into nested procedures
		}

		ostk.pushProcedure(arr);

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
