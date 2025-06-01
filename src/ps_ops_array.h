#pragma once

#include "pscore.h"
#include "psvm.h"


namespace waavs {

	static const PSOperatorFuncMap arrayOps = {

		{ "array", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
			if (s.empty()) return false;

			PSObject lenObj;
			s.pop(lenObj);
			if (lenObj.type != PSObjectType::Int || lenObj.asInt() < 0) 
				return false;

			auto arr = PSArray::create();
			arr->elements.resize(static_cast<size_t>(lenObj.asInt()));
			s.push(PSObject::fromArray(arr));
			return true;
		}},

		{ "aload", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
			if (s.empty()) 
				return false;

			PSObject arrObj;
			s.pop(arrObj);
			if (arrObj.type != PSObjectType::Array || !arrObj.asArray())
				return false;

			for (const PSObject& elem : arrObj.asArray()->elements)
				s.push(elem);

			s.push(arrObj); // push original array back onto the stack
			return true;
		}},

		{ "astore", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
			if (s.empty()) return false;

			PSObject arrObj;
			s.pop(arrObj);

			if (!arrObj.isArray() || !arrObj.asArray())
				return false;

			auto arr = arrObj.asArray();
			size_t count = arr->size();
			if (s.size() < count) 
				return false;

			// Store top stack elements into array, in reverse order
			for (size_t i = 0; i < count; ++i) {
				PSObject sObj;
				s.pop(sObj);
				arr->elements[count - 1 - i] = sObj;
			}

			s.push(arrObj); // push array back
			return true;
		}},

		{ "getinterval", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
			if (s.size() < 3) 
				return false;

			PSObject countObj;
			PSObject indexObj;
			PSObject arrObj;

			s.pop(countObj);
			s.pop(indexObj);
			s.pop(arrObj);

			if (!arrObj.isArray() || !indexObj.isInt() || !countObj.isInt())
				return false;

			auto src = arrObj.asArray();
			int start = indexObj.asInt();
			int count = countObj.asInt();

			if (!src || start < 0 || count < 0 || static_cast<size_t>(start + count) > src->elements.size())
				return false;

			auto sub = PSArray::create();
			sub->elements.insert(
				sub->elements.begin(),
				src->elements.begin() + start,
				src->elements.begin() + start + count
			);

			PSObject subObj;
			subObj.resetFromArray(sub);
			s.push(subObj);
			return true;
		}},

		{ "putinterval", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
			if (s.size() < 3) return false;

			PSObject srcArrObj;
			PSObject indexObj;
			PSObject destArrObj;

			s.pop(srcArrObj);
			s.pop(indexObj);
			s.pop(destArrObj);

			if (!destArrObj.isArray() || !indexObj.isInt() || srcArrObj.isArray())
				return false;

			auto dest = destArrObj.asArray();
			auto src = srcArrObj.asArray();
			int index = indexObj.asInt();

			if (!dest || !src || index < 0 || static_cast<size_t>(index + src->size()) > dest->size())
				return false;

			for (size_t i = 0; i < src->size(); ++i) {
				dest->elements[index + i] = src->elements[i];
			}

			return true;
		}}

	}; // end arrayOps

} // namespace waavsps
