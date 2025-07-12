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
            if (!index.isInt()) 
                return false;
            
            {
                auto arr = container.asArray();
                int idx = index.asInt();
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->elements.size())
                    return false;
                s.push(arr->elements[idx]);
                return true;
            }

        case PSObjectType::String:
            if (!index.isInt()) 
                return false;
            {
                auto str = container.asString();
                int idx = index.asInt();
                if ( idx < 0 || static_cast<size_t>(idx) >= str.length())
                    return false;
                s.push(PSObject::fromInt(str.data()[idx]));
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

        case PSObjectType::Matrix:
            if (!index.isInt()) 
                return vm.error("op_get:PSObjectType::Matrix, index not an Int");

            {
                int idx = index.asInt();
                const PSMatrix& mat = container.asMatrix();
                if (idx < 0 || idx >= 6)
                    return false;
                s.push(PSObject::fromReal(mat.m[idx]));
                return true;
            }

        default:
            return false;
        }
    }

	// put: a b c -> a b (c = a[b])
    inline bool op_put(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 3)
            return vm.error("op_put: stackunderflow");

        PSObject value;
        PSObject index;
        PSObject container;

        s.pop(value);
        s.pop(index);
        s.pop(container);

        switch (container.type) {
        case PSObjectType::Array:
            if (!index.isInt())
                return vm.error("op_put: typeckeck");

            {
                auto arr = container.asArray();
                int idx = index.asInt();
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->elements.size())
                    return vm.error("op_put: rangecheck");

                arr->elements[idx] = value;
                return true;
            }

        case PSObjectType::String:
            if (!index.isInt())
                return vm.error("op_put: typecheck, string, index not int");
            if (!value.isInt())
                return vm.error("op_put: typecheck, string, value.type != int");
            {
                auto str = container.asString();
                int idx = index.asInt();
                int byte = value.asInt();
                
                if (idx < 0 || byte < 0 || byte > 255 || static_cast<size_t>(idx) >= str.capacity())
                    return vm.error("op_put: rangecheck, string");

				str.put(idx, static_cast<char>(byte));
                return true;
            }

        case PSObjectType::Dictionary:
            if (index.type != PSObjectType::Name) return false;
            {
                auto dict = container.asDictionary();
                if (!dict) 
                    return false;
                return dict->put(index.asName(), value);
            }

        default:
            return vm.error("op_put: typecheck, container");
        }
    }

	// length: container -> length (number of elements in container)
    inline bool op_length(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return vm.error("op_length: stackkunderflow");

        PSObject obj;

        s.pop(obj);

        switch (obj.type) {
        case PSObjectType::Array:
            s.push(PSObject::fromInt(static_cast<int>(obj.asArray()->size())));
            return true;

        case PSObjectType::Matrix:
        {
            return s.push(PSObject::fromInt(6));
        }

        case PSObjectType::String:
            s.push(PSObject::fromInt(static_cast<int>(obj.asString().length())));
            return true;

        case PSObjectType::Dictionary:
            s.push(PSObject::fromInt(static_cast<int>(obj.asDictionary()->size())));
            return true;

        default:
            return vm.error("op_length: typecheck");
        }
    }

    // copy: (n x? ... x? ? x? ... x? x? ... x?) — duplicate top n items
    inline bool op_copy(PSVirtualMachine& vm) {
        auto& s = vm.opStack();

        PSObject top;
        if (!s.top(top))
            return false;


        // 1. Integer form: n copy
        if (top.isInt()) {
            int32_t n = top.asInt();

			s.pop(top); // pop the top item

            if (n < 0 || static_cast<size_t>(n) > s.size() )
                return false; // vm.error("stackunderflow");

            // Copy the top n items (preserve order)
            return s.copy(n);
        }

		// the next two forms MUST have at least two items on the stack
        if (s.size() < 2) 
            return vm.error("op_copy: stackunderflow"); // Need at least two strings

        PSObject destObject;
        PSObject srcObject;

		s.pop(destObject); // pop destination
		s.pop(srcObject); // pop source

        // 2. Array form: array1 array2 copy
        // copy array1 into array2
        if (srcObject.isArray()) {
            if ( !destObject.isArray())
                return vm.error("op_copy:typecheck ARRAY - destination object not array");

            auto dest = destObject.asArray();
            auto src = srcObject.asArray();

            if (!dest || !src) return vm.error("op_copy:invalidaccess");

            size_t maxSize = std::min(dest->size(), src->size());

            for (size_t i = 0; i < maxSize; ++i)
                dest->elements[i] = src->elements[i];

            // push destination back onto stack
			return s.push(destObject); // push updated dest
        }

        // 3. String form: string1 string2 copy
		// copy string1 into string2
        if (srcObject.isString()) 
        {
            // Type check
            if (!destObject.isString())
				return vm.error("op_copy:typecheck");

			auto dest = destObject.asString();
			auto src = srcObject.asString();

            // Make sure neither one is null
            //if (!dest || !src) return  vm.error("op_copy:invalidaccess");

            if (!dest.putInterval(0, src)) 
                return vm.error("op_copy:putInterval failed");

            return s.push(destObject);
        }

        return false; // vm.error("typecheck");
    }


 


    // op_equality
    // eq
    inline bool op_equality(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.size() < 2) 
            return false;

        PSObject b;
        PSObject a;

        s.pop(a);
        s.pop(b);

        // Try as integer first, because one might be int, and one might be real
        if (a.isInt() && b.isInt()) {
            // Both are integers
            s.push(PSObject::fromBool(a.asInt() == b.asInt()));
            return true;
        }
        else if (a.isReal() && b.isReal()) {
            // Both can be treated as reals
            s.push(PSObject::fromBool(a.asReal() == b.asReal()));
            return true;
        }

        bool result = (a.type == b.type);

        if (result) {
            switch (a.type) {
            case PSObjectType::Bool:    result = a.asBool() == b.asBool(); break;
            case PSObjectType::Name:    result = a.asName() == b.asName(); break;
            case PSObjectType::Null:    result = true; break;
            default:
                return vm.error("op_equality: typecheck failed");
                //result = false; 
                //break;
            }
        }
        else {
            //return vm.error("op_equality: typemismatch");
        }

        s.push(PSObject::fromBool(result));
        return true;
    }

    inline bool op_ne(PSVirtualMachine& vm) {
		auto& s = vm.opStack();

        bool ok = op_equality(vm);
        if (!ok)
            return vm.error("op_ne: op_equality false");

        PSObject top;
        
        s.pop(top);

        if (!top.isBool())
            return vm.error("op_ne: typecheck");

        s.push(PSObject::fromBool(!top.asBool()));
        return true;
    }


    inline bool op_type(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) return false;


        PSObject obj;

        s.pop(obj);

        PSName typeName("unknown");

        switch (obj.type) {
        case PSObjectType::Int: typeName = "integertype"; break;
        case PSObjectType::Real: typeName = "realtype"; break;
        case PSObjectType::Bool: typeName = "booleantype"; break;
        case PSObjectType::String: typeName = "stringtype"; break;

        case PSObjectType::Array: 
        case PSObjectType::Matrix: typeName = "arraytype"; break;
        
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
        if (s.empty()) 
            return vm.error("op_cvlit: stackunderflow");

        PSObject obj;
        s.pop(obj);
        obj.setExecutable(false);
        s.push(obj);
        return true;
    }

    inline bool op_cvx(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) 
            return vm.error("op_cvx: stackunderflow");

        PSObject obj;
        s.pop(obj);
        obj.setExecutable(true);
        s.push(obj);
        return true;
    }

    inline bool op_xcheck(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        if (s.empty()) 
            return vm.error("op_xcheck: stackunderflow");

        PSObject obj;
        s.pop(obj);
        s.push(PSObject::fromBool(obj.isExecutable()));
        return true;
    }



    inline bool op_rcheck(PSVirtualMachine& vm) 
    {
        auto& ostk = vm.opStack();

        if (ostk.empty())
            return vm.error("op_rcheck: stackunderflow");

        PSObject obj;
        ostk.pop(obj);

        ostk.push(obj);
        ostk.push(PSObject::fromBool(obj.isAccessReadable()));

        return true;
    }

    inline bool op_wcheck(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();

        if (ostk.empty())
            return vm.error("op_wcheck: stackunderflow");

        PSObject obj;
        ostk.pop(obj);

        ostk.push(obj);
        ostk.push(PSObject::fromBool(obj.isAccessWriteable()));

        return true;
    }

    inline bool op_readonly(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_readonly: stackunderflow");

        PSObject obj;
        ostk.pop(obj);
        obj.setAccessReadable(true);
        obj.setAccessWriteable(false);
        obj.setAccessExecutable(false);

        ostk.push(obj);

        return true;
    }

    inline bool op_writeonly(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_writeonly: stackunderflow");
    
        PSObject obj;
        ostk.pop(obj);
        obj.setAccessReadable(false);
        obj.setAccessWriteable(true);
        obj.setAccessExecutable(false);
        ostk.push(obj);
        
        return true;
    }

    inline bool op_noaccess(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_noaccess: stackunderflow");
    
        PSObject obj;
        ostk.pop(obj);
        obj.setAccessReadable(false);
        obj.setAccessWriteable(false);
        obj.setAccessExecutable(false);
        ostk.push(obj);
        
        return true;
    }

    inline bool op_executeonly(PSVirtualMachine& vm)
    {
        auto& ostk = vm.opStack();
        if (ostk.empty())
            return vm.error("op_executeonly: stackunderflow");
        PSObject obj;
        ostk.pop(obj);
        obj.setAccessReadable(false);
        obj.setAccessWriteable(false);
        obj.setAccessExecutable(true);
        ostk.push(obj);
        return true;
    }

    static const PSOperatorFuncMap& getPolymorphOps()
    {
        static const PSOperatorFuncMap table = {
            { "get",     op_get },
            { "put",     op_put },
            { "length",  op_length },
            { "copy",    op_copy },
            { "eq",      op_equality },
            { "ne",      op_ne },
            { "type",    op_type },
            { "cvlit",   op_cvlit },
            { "cvx",     op_cvx },
            { "xcheck",  op_xcheck },
            { "rcheck",  op_rcheck },
            { "wcheck",  op_wcheck },
            { "readonly", op_readonly },
            { "writeonly", op_writeonly },
            { "executeonly", op_executeonly },
            { "noaccess", op_noaccess },
            { "noaccess", op_noaccess},
        };
        return table;
    }
}
