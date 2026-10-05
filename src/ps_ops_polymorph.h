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
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->size())
                    return false;
                s.push((*arr)[idx]);
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
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->size())
                    return vm.error("op_put: rangecheck");

                (*arr)[idx] = value;
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
    inline bool op_copy(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject top;
        if (!s.top(top))
            return vm.error("op_copy: stackunderflow");

        // n copy
        if (top.type == PSObjectType::Int)
        {
            int32_t count = top.asInt();
            s.pop(top);

            if (count < 0)
                return vm.error("op_copy: rangecheck");

            if (static_cast<size_t>(count) > s.size())
                return vm.error("op_copy: stackunderflow");

            return s.copy(static_cast<size_t>(count));
        }

        if (s.size() < 2)
            return vm.error("op_copy: stackunderflow");

        PSObject destObj;
        PSObject srcObj;

        s.pop(destObj);
        s.pop(srcObj);


        // array1 array2 copy subarray2
        if (srcObj.isArray())
        {
            if (!destObj.isArray())
                return vm.error("op_copy: typecheck");

            if (!srcObj.isAccessReadable() || !destObj.isAccessWriteable())
                return vm.error("op_copy: invalidaccess");

            auto src = srcObj.asArray();
            auto dest = destObj.asArray();

            if (dest->size() < src->size())
                return vm.error("op_copy: rangecheck");

            for (size_t i = 0; i < src->size(); ++i)
            {
                PSObject value;

                if (!src->get(i, value))
                    return vm.error("op_copy: rangecheck");

                if (!dest->put(i, value))
                    return vm.error("op_copy: rangecheck");
            }

            auto result = dest->subarray(0, src->size());
            if (!result)
                return vm.error("op_copy: rangecheck");

            PSObject resultObj = destObj;
            resultObj.resetFromArray(result);

            return s.push(resultObj);
        }


        // string1 string2 copy substring2
        if (srcObj.isString())
        {
            if (!destObj.isString())
                return vm.error("op_copy: typecheck");

            if (!srcObj.isAccessReadable() || !destObj.isAccessWriteable())
                return vm.error("op_copy: invalidaccess");

            PSString src = srcObj.asString();
            PSString dest = destObj.asString();

            if (dest.length() < src.length())
                return vm.error("op_copy: rangecheck");

            for (uint32_t i = 0; i < src.length(); ++i)
            {
                uint8_t value;

                if (!src.get(i, value))
                    return vm.error("op_copy: rangecheck");

                if (!dest.put(i, value))
                    return vm.error("op_copy: rangecheck");
            }

            PSString result = dest.getInterval(0, static_cast<uint32_t>(src.length()));
            return s.pushString(result);
        }


        // dict1 dict2 copy dict2
        if (srcObj.isDictionary())
        {
            if (!destObj.isDictionary())
                return vm.error("op_copy: typecheck");

            if (!srcObj.isAccessReadable() || !destObj.isAccessWriteable())
                return vm.error("op_copy: invalidaccess");

            auto src = srcObj.asDictionary();
            auto dest = destObj.asDictionary();

            bool success = true;

            src->forEach([&](const PSObject &key, const PSObject& value) {
                if (!dest->put(key, value))
                {
                    success = false;
                    return false;
                }

                return true;
                });

            if (!success)
                return vm.error("op_copy: limitcheck");

            return s.push(destObj);
        }

        return vm.error("op_copy: typecheck");
    }


 


    // op_equality
    // eq
    // A couple of helpers
    inline bool psStringEqual(const PSString& a, const PSString& b)
    {
        if (a.length() != b.length())
            return false;

        return a.length() == 0 || std::memcmp(a.data(), b.data(), a.length()) == 0;
    }

    inline bool psStringNameEqual(const PSString& str, const PSName& name)
    {
        size_t nameLen = std::strlen(name.c_str());

        if (str.length() != nameLen)
            return false;

        return nameLen == 0 || std::memcmp(str.data(), name.c_str(), nameLen) == 0;
    }


    inline bool op_equality(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_equality: stackunderflow");

        PSObject b;
        PSObject a;

        s.pop(b);
        s.pop(a);

        // Numbers compare by mathematical value, regardless of integer/real type.
        if (a.isNumber() && b.isNumber())
            return s.pushBool(a.asReal() == b.asReal());

        // String contents are required for these comparisons.
        if ((a.isString() && b.isString()) ||
            (a.isString() && b.isName()) ||
            (a.isName() && b.isString()))
        {
            if ((a.isString() && !a.isAccessReadable()) ||
                (b.isString() && !b.isAccessReadable()))
                return vm.error("op_equality: invalidaccess");

            if (a.isString() && b.isString())
                return s.pushBool(psStringEqual(a.asString(), b.asString()));

            if (a.isString())
                return s.pushBool(psStringNameEqual(a.asString(), b.asName()));

            return s.pushBool(psStringNameEqual(b.asString(), a.asName()));
        }

        // Different remaining types cannot be equal.
        if (a.type != b.type)
            return s.pushBool(false);

        switch (a.type)
        {
        case PSObjectType::Bool:
            return s.pushBool(a.asBool() == b.asBool());

        case PSObjectType::Name:
            return s.pushBool(a.asName() == b.asName());

        case PSObjectType::String:
            if (!a.isAccessReadable() || !b.isAccessReadable())
                return vm.error("op_equality: invalidaccess");
            return s.pushBool(psStringEqual(a.asString(), b.asString()));

        case PSObjectType::Array:
            return s.pushBool(a.asArray() == b.asArray());

        case PSObjectType::Dictionary:
            return s.pushBool(a.asDictionary() == b.asDictionary());

        case PSObjectType::File:
            return s.pushBool(a.asFile() == b.asFile());

        case PSObjectType::Font:
            return s.pushBool(a.asFont() == b.asFont());

        case PSObjectType::FontFace:
            return s.pushBool(a.asFontFace() == b.asFontFace());

        case PSObjectType::Null:
            return s.pushBool(true);

        default:
            return s.pushBool(false);
        }
    }

    inline bool op_ne(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (!op_equality(vm))
            return false;

        PSObject result;
        if (!s.pop(result))
            return false;

        return s.pushBool(!result.asBool());
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
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_rcheck: stackunderflow");

        PSObject obj;
        s.pop(obj);

        switch (obj.type)
        {
        case PSObjectType::Array:
        case PSObjectType::Matrix:
        case PSObjectType::Dictionary:
        case PSObjectType::File:
        case PSObjectType::String:
            return s.pushBool(obj.isAccessReadable());

        default:
            return vm.error("op_rcheck: typecheck");
        }
    }

    inline bool op_wcheck(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_wcheck: stackunderflow");

        PSObject obj;
        s.pop(obj);

        switch (obj.type)
        {
        case PSObjectType::Array:
        case PSObjectType::Matrix:
        case PSObjectType::Dictionary:
        case PSObjectType::File:
        case PSObjectType::String:
            return s.pushBool(obj.isAccessWriteable());

        default:
            return vm.error("op_wcheck: typecheck");
        }
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
