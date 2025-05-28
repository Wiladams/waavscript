#pragma once

// NOTE:  Something else must include this, and have PSVirtualMachine defined.
#include "pscore.h"
#include "psvm.h"

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
                auto* arr = container.data.arr;
                int idx = index.data.iVal;
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->elements.size())
                    return false;
                s.push(arr->elements[idx]);
                return true;
            }

        case PSObjectType::String:
            if (index.type != PSObjectType::Int) return false;
            {
                auto* str = container.data.str;
                int idx = index.data.iVal;
                if (!str || idx < 0 || static_cast<size_t>(idx) >= str->length)
                    return false;
                s.push(PSObject::fromInt(str->data[idx]));
                return true;
            }

        case PSObjectType::Dictionary:
            if (index.type != PSObjectType::Name) return false;
            {
                auto* dict = container.data.dict;
                PSObject result;
                if (!dict || !dict->get(index.data.name, result)) return false;
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
                auto* arr = container.data.arr;
                int idx = index.data.iVal;
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->elements.size())
                    return false;
                arr->elements[idx] = value;
                return true;
            }

        case PSObjectType::String:
            if (index.type != PSObjectType::Int || value.type != PSObjectType::Int) return false;
            {
                auto* str = container.data.str;
                int idx = index.data.iVal;
                int byte = value.data.iVal;
                if (!str || idx < 0 || byte < 0 || byte > 255 || static_cast<size_t>(idx) >= str->capacity())
                    return false;
				str->data[idx] = static_cast<char>(byte);
                //(*str)[idx] = static_cast<char>(byte);
                return true;
            }

        case PSObjectType::Dictionary:
            if (index.type != PSObjectType::Name) return false;
            {
                auto* dict = container.data.dict;
                if (!dict) return false;
                return dict->put(index.data.name, value);
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
            s.push(PSObject::fromInt(static_cast<int>(obj.data.arr->size())));
            return true;

        case PSObjectType::String:
            s.push(PSObject::fromInt(static_cast<int>(obj.data.str->length)));
            return true;

        case PSObjectType::Dictionary:
            s.push(PSObject::fromInt(static_cast<int>(obj.data.dict->size())));
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

            PSArray* dest = destObject.asArray();
            PSArray* src = srcObject.asArray();

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

			PSString * dest = destObject.asString();
			PSString * src = srcObject.asString();

            // Make sure neither one is null
            if (!dest || !src) return false; //  vm.error("invalidaccess");

            if (!dest->putInterval(0, *src)) return false;

            return s.push(destObject);
        }

        return false; // vm.error("typecheck");
    }


    inline bool op_forall(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject proc;
        PSObject obj;

        s.pop(proc);
        s.pop(obj);

        if (proc.type != PSObjectType::Array || !proc.asArray() || !proc.isExecutable())
            return false;

        switch (obj.type) {
        case PSObjectType::Array: {
            for (const auto& val : obj.data.arr->elements) {
                s.push(val);
                vm.execArray(proc.data.arr);
            }
            return true;
        }

        case PSObjectType::String: {
            for (int i = 0; i < obj.asString()->length; ++i) {
                s.push(PSObject::fromInt(obj.asString()->data[i]));
                vm.execArray(proc.asArray());
            }
            return true;
        }

        case PSObjectType::Dictionary: {
            for (const auto& entry : obj.asDictionary()->entries) {
                s.push(PSObject::fromName(entry.first));
                s.push(entry.second);
                vm.execArray(proc.data.arr);
            }
            return true;
        }

        default: return false;
        }
    }



    inline bool op_assign(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) return false;

        PSObject b;
        PSObject a;

        s.pop(a);
        s.pop(b);

        bool result = (a.type == b.type);

        if (result) {
            switch (a.type) {
            case PSObjectType::Int:     result = a.data.iVal == b.data.iVal; break;
            case PSObjectType::Real:    result = a.data.fVal == b.data.fVal; break;
            case PSObjectType::Bool:    result = a.data.bVal == b.data.bVal; break;
            case PSObjectType::Name:    result = a.data.name == b.data.name; break;
            case PSObjectType::Null:    result = true; break;
            default: result = false; break;
            }
        }

        s.push(PSObject::fromBool(result));
        return true;
    }

    inline bool op_ne(PSVirtualMachine& vm) {
		auto& s = vm.opStack();

        bool ok = op_assign(vm);
        if (!ok) return false;

        PSObject top;
        
        s.pop(top);

        if (top.type != PSObjectType::Bool) return false;

        s.push(PSObject::fromBool(!top.data.bVal));
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


    inline bool op_cvs(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;

        PSObject val;

        s.pop(val);

        char buffer[64] = { 0 };

        switch (val.type) {
        case PSObjectType::Int:
            std::snprintf(buffer, sizeof(buffer), "%d", val.asInt());
            break;
        case PSObjectType::Real:
            std::snprintf(buffer, sizeof(buffer), "%.6g", val.asReal());
            break;
        case PSObjectType::Bool:
            std::snprintf(buffer, sizeof(buffer), val.asBool() ? "true" : "false");
            break;
        case PSObjectType::Name:
            std::snprintf(buffer, sizeof(buffer), "/%s", val.asName());
            break;
        default:
            std::snprintf(buffer, sizeof(buffer), "<object>");
            break;
        }

        auto* str = new PSString(std::strlen(buffer));
        for (size_t i = 0; i < std::strlen(buffer); ++i)
            str->data[i] = buffer[i];

        s.push(PSObject::fromString(str));
        return true;
    }










    // bind: proc -> proc' (resolve names to operators)
    inline bool op_bind(PSVirtualMachine& vm) {
        vm.bind();
        return true;
    }


    static const PSOperatorFuncMap polymorphOps = {
    { "get", op_get },
    { "put", op_put },
    { "length", op_length },
    { "copy", op_copy },
    { "forall", op_forall },
    { "eq", op_assign },  // Polymorphic equality
    { "ne", op_ne },      // Logical negation of `eq`
    { "type", op_type },
    { "cvs", op_cvs },
    { "bind", op_bind },
    };

}
