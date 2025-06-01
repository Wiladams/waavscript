#pragma once

// NOTE:  Something else must include this, and have PSVirtualMachine defined.
#include "pscore.h"
#include "psvm.h"
#include "nametable.h"

//======================================================================
// The operators in here are polymorphic, meaning they can apply
// they can apply to different types of objects.
//======================================================================

namespace waavs 
{

	// get: container index -> value
    inline bool op_get(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject index;
        PSObject container;

        s.pop(index);
        s.pop(container);

        switch (container.type) {
        case PSObjectType::Array:
            if (index.type != PSObjectType::Int) return false;
            {
                auto arr = container.asArray();
                int idx = index.asInt();
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->elements.size())
                    return false;
                s.push(arr->elements[idx]);
                return true;
            }

        case PSObjectType::String:
            if (index.type != PSObjectType::Int) return false;
            {
                auto str = container.asString();
                int idx = index.asInt();
                if (!str || idx < 0 || static_cast<size_t>(idx) >= str->length())
                    return false;
                s.push(PSObject::fromInt(str->data()[idx]));
                return true;
            }

        case PSObjectType::Dictionary:
            if (index.type != PSObjectType::Name) return false;
            {
                auto dict = container.asDictionary();
                PSObject result;
                if (!dict || !dict->get(index.asName(), result)) return false;
                s.push(result);
                return true;
            }

        default:
            return false;
        }
    }

	// put: a b c -> a b (c = a[b])
    inline bool op_put(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 3) return false;

        PSObject value;
        PSObject index;
        PSObject container;

        s.pop(value);
        s.pop(index);
        s.pop(container);

        switch (container.type) {
        case PSObjectType::Array:
            if (index.type != PSObjectType::Int) return false;
            {
                auto arr = container.asArray();
                int idx = index.asInt();
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->elements.size())
                    return false;
                arr->elements[idx] = value;
                return true;
            }

        case PSObjectType::String:
            if (index.type != PSObjectType::Int || value.type != PSObjectType::Int) return false;
            {
                auto str = container.asString();
                int idx = index.asInt();
                int byte = value.asInt();
                if (!str || idx < 0 || byte < 0 || byte > 255 || static_cast<size_t>(idx) >= str->capacity())
                    return false;
				str->put(idx, static_cast<char>(byte));
                return true;
            }

        case PSObjectType::Dictionary:
            if (index.type != PSObjectType::Name) return false;
            {
                auto dict = container.asDictionary();
                if (!dict) return false;
                return dict->put(index.asName(), value);
            }

        default:
            return false;
        }
    }

	// length: container -> length (number of elements in container)
    inline bool op_length(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject obj;

        s.pop(obj);

        switch (obj.type) {
        case PSObjectType::Array:
            s.push(PSObject::fromInt(static_cast<int>(obj.asArray()->size())));
            return true;

        case PSObjectType::String:
            s.push(PSObject::fromInt(static_cast<int>(obj.asString()->length())));
            return true;

        case PSObjectType::Dictionary:
            s.push(PSObject::fromInt(static_cast<int>(obj.asDictionary()->size())));
            return true;

        default:
            return false;
        }
    }

    // copy: (n x? ... x? ? x? ... x? x? ... x?) — duplicate top n items
    inline bool op_copy(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        PSObject top;
        if (!s.peek(top))
            return false;


        // 1. Integer form: n copy
        if (top.isInt()) {
            int32_t n = top.asInt();

			s.pop(); // pop the top item

            if (n < 0 || static_cast<size_t>(n) > s.size() )
                return false; // vm.error("stackunderflow");

            // Copy the top n items (preserve order)
            return s.copy(n);
        }

		// the next two forms MUST have at least two items on the stack
        if (s.size() < 2) return false; // Need at least two strings

        PSObject destObject;
        PSObject srcObject;

		s.pop(destObject); // pop destination
		s.pop(srcObject); // pop source

        // 2. Array form: array1 array2 copy
        // copy array1 into array2
        if (top.isArray()) {
            if (!srcObject.isArray() || !destObject.isArray())
                return false; // vm.error("typecheck");

            auto dest = destObject.asArray();
            auto src = srcObject.asArray();

            if (!dest || !src) return false; // vm.error("invalidaccess");

            size_t maxSize = std::min(dest->size(), src->size());

            for (size_t i = 0; i < maxSize; ++i)
                dest->elements[i] = src->elements[i];

            // push destination back onto stack
			s.push(destObject); // push updated dest
            
            return true;
        }

        // 3. String form: string1 string2 copy
		// copy string1 into string2
        if (top.isString()) 
        {
            // Type check
            if (!srcObject.isString() || !destObject.isString())
				return false; // vm.error("typecheck");

			auto dest = destObject.asString();
			auto src = srcObject.asString();

            // Make sure neither one is null
            if (!dest || !src) return  vm.error("op_copy:invalidaccess");

            if (!dest->putInterval(0, *src)) return false;

            return s.push(destObject);
        }

        return false; // vm.error("typecheck");
    }


    inline bool op_forall(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2)
            return vm.error("forall: stackunderflow");

        PSObject proc, container;
        s.pop(proc);
        s.pop(container);

        //if (!ensureExecutableOrError(vm, proc))
        //    return vm.error("forall: procedure is not executable");

        auto apply = [&](const PSObject& val1, const PSObject* val2 = nullptr) -> bool {
            if (val2) s.push(*val2);
            s.push(val1);
            
            if (!runArray(vm, proc)) {
                return vm.error("forall: failed to run procedure");
			}
            
            if (vm.isExitRequested()) {
                vm.clearExitRequest();
                return false; // exit terminates loop early
            }
            return true;
            };

        switch (container.type) {
        case PSObjectType::Array: {
            for (const auto& val : container.asArray()->elements) {
                if (!apply(val)) break;
            }
            return true;
        }

        case PSObjectType::String: {
            auto str = container.asString();
            for (int i = 0; i < str->length(); ++i) {
                PSObject obj;
                uint8_t byte;
				str->get(i, byte);
                obj = PSObject::fromInt(static_cast<unsigned char>(byte));
                if (!apply(obj)) break;
            }
            return true;
        }

        case PSObjectType::Dictionary: {
            for (const auto& kv : container.asDictionary()->entries()) {
                PSObject key = PSObject::fromName(kv.first);
                PSObject val = kv.second;
                if (!apply(key, &val)) break;
            }
            return true;
        }

        default:
            return vm.error("forall: unsupported container type");
        }
    }



    inline bool op_equality(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b;
        PSObject a;

        s.pop(a);
        s.pop(b);

        bool result = (a.type == b.type);

        if (result) {
            switch (a.type) {
            case PSObjectType::Int:     result = a.asInt() == b.asInt(); break;
            case PSObjectType::Real:    result = a.asReal() == b.asReal(); break;
            case PSObjectType::Bool:    result = a.asBool() == b.asBool(); break;
            case PSObjectType::Name:    result = a.asName() == b.asName(); break;
            case PSObjectType::Null:    result = true; break;
            default: result = false; break;
            }
        }

        s.push(PSObject::fromBool(result));
        return true;
    }

    inline bool op_ne(PSVirtualMachine& vm) {
		auto& s = vm.opStack();

        bool ok = op_equality(vm);
        if (!ok) return false;

        PSObject top;
        
        s.pop(top);

        if (!top.isBool()) return false;

        s.push(PSObject::fromBool(!top.asBool()));
        return true;
    }


    inline bool op_type(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;


        PSObject obj;

        s.pop(obj);

        const char* typeName = "unknown";

        switch (obj.type) {
        case PSObjectType::Int: typeName = "integertype"; break;
        case PSObjectType::Real: typeName = "realtype"; break;
        case PSObjectType::Bool: typeName = "booleantype"; break;
        case PSObjectType::String: typeName = "stringtype"; break;
        case PSObjectType::Array: typeName = "arraytype"; break;
        case PSObjectType::Dictionary: typeName = "dicttype"; break;
        case PSObjectType::Name: typeName = "nametype"; break;
        case PSObjectType::Null: typeName = "nulltype"; break;
        default: break;
        }

        s.push(PSObject::fromName(typeName));
        return true;
    }






    inline bool op_cvlit(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return vm.error("stackunderflow");

        PSObject obj;
        s.pop(obj);
        obj.setExecutable(false);
        s.push(obj);
        return true;
    }

    inline bool op_cvx(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return vm.error("stackunderflow");

        PSObject obj;
        s.pop(obj);
        obj.setExecutable(true);
        s.push(obj);
        return true;
    }

    inline bool op_xcheck(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return vm.error("stackunderflow");

        PSObject obj;
        s.pop(obj);
        s.push(PSObject::fromBool(obj.isExecutable()));
        return true;
    }








    // bind: proc -> proc' (resolve names to operators)
    inline bool op_bind(PSVirtualMachine& vm) {
        vm.bind();
        return true;
    }


    static const PSOperatorFuncMap polymorphOps = {
	{ "get", op_get },          // get: container index -> value (get value from container at index)
	{ "put", op_put },          // put: a b c -> a b (c = a[b])
	{ "length", op_length },    // length: container -> length (number of elements in container)
	{ "copy", op_copy },        // copy: (n x? ... x? ? x? ... x? x? ... x?) — duplicate top n items
	{ "forall", op_forall },    // forall: proc container -> (apply proc to each element in container)
    { "eq", op_equality },      // Polymorphic equality
    { "ne", op_ne },            // Logical negation of `eq`
	{ "type", op_type },        // type: object -> type (get the type of an object as a name)
	{ "cvlit",  op_cvlit },     // mark an object as literal (not executable)
	{ "cvx", op_cvx },          // mark an object as executable
	{ "xcheck", op_xcheck },    // xcheck: object -> bool (check if object is executable)
    { "bind", op_bind },
    };

}
