#pragma once

// NOTE:  Something else must include this, and have PSVirtualMachine defined.
#include "pscore.h"
#include "psvm.h"
#include "ps_object_helpers.h"

//======================================================================
// The operators in here are polymorphic, meaning they can apply
// they can apply to different types of objects.
//======================================================================

namespace waavs 
{

	// get: container index -> value
    inline bool op_get(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 2)
            return vm.error("op_get: stackunderflow");

        PSObject index;
        PSObject container;

        if (!s.pop(index) || !s.pop(container))
            return vm.error("op_get: stackunderflow");

        switch (container.type)
        {
        case PSObjectType::Array:
        {
            if (!index.isInt())
                return vm.error("op_get: typecheck");

            auto arr = container.asArray();
            if (!arr)
                return vm.error("op_get: invalidaccess");

            int32_t idx = index.asInt();
            if (idx < 0 || static_cast<size_t>(idx) >= arr->size())
                return vm.error("op_get: rangecheck");

            return s.push((*arr)[static_cast<size_t>(idx)]);
        }

        case PSObjectType::String:
        {
            if (!index.isInt())
                return vm.error("op_get: typecheck");

            auto str = container.asString();
            int32_t idx = index.asInt();

            if (idx < 0 || static_cast<size_t>(idx) >= str.length())
                return vm.error("op_get: rangecheck");

            uint8_t value;
            if (!str.get(static_cast<uint32_t>(idx), value))
                return vm.error("op_get: rangecheck");

            return s.pushInt(value);
        }

        case PSObjectType::Matrix:
        {
            if (!index.isInt())
                return vm.error("op_get: typecheck");

            int32_t idx = index.asInt();
            if (idx < 0 || idx >= 6)
                return vm.error("op_get: rangecheck");

            const PSMatrix& mat = container.asMatrix();
            return s.pushReal(mat.m[idx]);
        }

        case PSObjectType::Dictionary:
        {
            auto dict = container.asDictionary();
            if (!dict)
                return vm.error("op_get: typecheck, expected dictionary");
            PSObject result;
            if (!dict->get(index, result))
                return vm.error("op_get: dictionary lookup failed");
            return s.push(result);
        }

        default:
            return vm.error("op_get: typecheck");
        }
    }

	// put: a b c -> a b (c = a[b])
    inline bool op_put(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.size() < 3)
            return vm.error("op_put: stackunderflow");

        PSObject value;
        PSObject index;
        PSObject container;

        if (!s.pop(value) || !s.pop(index) || !s.pop(container))
            return vm.error("op_put: stackunderflow");

        switch (container.type)
        {
        case PSObjectType::Array:
        {
            if (!index.isInt())
                return vm.error("op_put: typecheck");

            auto arr = container.asArray();
            if (!arr)
                return vm.error("op_put: invalidaccess");

            int32_t idx = index.asInt();
            if (idx < 0 || static_cast<size_t>(idx) >= arr->size())
                return vm.error("op_put: rangecheck");

            (*arr)[static_cast<size_t>(idx)] = value;
            return true;
        }

        case PSObjectType::String:
        {
            if (!index.isInt())
                return vm.error("op_put: typecheck");

            if (!value.isInt())
                return vm.error("op_put: typecheck");

            auto str = container.asString();

            int32_t idx = index.asInt();
            int32_t byte = value.asInt();

            if (idx < 0 || byte < 0 || byte > 255 || static_cast<size_t>(idx) >= str.length())
                return vm.error("op_put: rangecheck");

            if (!str.put(static_cast<uint32_t>(idx), static_cast<uint8_t>(byte)))
                return vm.error("op_put: rangecheck");

            return true;
        }

        case PSObjectType::Dictionary:
        {
            auto dict = container.asDictionary();
            if (!dict)
                return vm.error("op_put: invalidaccess");

            if (!dict->put(index, value))
                return vm.error("op_put: typecheck");

            return true;
        }

        default:
            return vm.error("op_put: typecheck");
        }
    }



    // length: container -> length (number of elements in container)
    inline bool op_length(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_length: stackunderflow");

        PSObject obj;
        s.pop(obj);

        switch (obj.type)
        {
        case PSObjectType::Name:
            return s.push(PSObject::fromInt(static_cast<int>(obj.asName().size())));

        case PSObjectType::Array:
            return s.push(PSObject::fromInt(static_cast<int>(obj.asArray()->size())));

        case PSObjectType::Matrix:
            return s.push(PSObject::fromInt(6));

        case PSObjectType::String:
            return s.push(PSObject::fromInt(static_cast<int>(obj.asString().length())));

        case PSObjectType::Dictionary:
            return s.push(PSObject::fromInt(static_cast<int>(obj.asDictionary()->size())));

        default:
            break;
        }

        /*
        // DIAGNOSTIC
        printf("=== op_length typecheck ===\n");
        printf("type: %d\n", static_cast<int>(obj.type));
        printf("object: ");
        writeObjectDeep(obj);
        printf("\n");
        */

        return vm.error("op_length: typecheck");
    }

    // copy: (n x? ... x? ? x? ... x? x? ... x?) — duplicate top n items
    inline bool op_copy(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        PSObject top;
        if (!s.top(top))
            return vm.error("op_copy: stackunderflow");

        // ------------------------------------------------------------
        // 1. Stack form:
        //
        // n copy
        //
        // Duplicate the top n objects on the operand stack.
        // ------------------------------------------------------------
        if (top.isInt())
        {
            int32_t count = top.asInt();

            if (count < 0)
                return vm.error("op_copy: rangecheck");

            s.pop(top);

            if (static_cast<size_t>(count) > s.size())
                return vm.error("op_copy: stackunderflow");

            return s.copy(count);
        }

        // All composite forms require:
        //
        // source destination copy
        //
        if (s.size() < 2)
            return vm.error("op_copy: stackunderflow");

        PSObject destObject;
        PSObject srcObject;

        if (!s.pop(destObject) || !s.pop(srcObject))
            return vm.error("op_copy: stackunderflow");

        // ------------------------------------------------------------
        // 2. Array form:
        //
        // array1 array2 copy -> subarray2
        //
        // Destination must be at least as large as source.
        // The returned array is the interval of the destination that
        // received the copied elements.
        // ------------------------------------------------------------
        if (srcObject.isArray())
        {
            if (!destObject.isArray())
                return vm.error("op_copy: typecheck");

            auto src = srcObject.asArray();
            auto dest = destObject.asArray();

            if (!src || !dest)
                return vm.error("op_copy: invalidaccess");

            if (dest->size() < src->size())
                return vm.error("op_copy: rangecheck");

            for (size_t i = 0; i < src->size(); ++i)
            {
                if (!dest->put(i, (*src)[i]))
                    return vm.error("op_copy: invalidaccess");
            }

            auto result = dest->subarray(0, src->size());
            if (!result)
                return vm.error("op_copy: rangecheck");

            return s.pushArray(result);
        }

        // ------------------------------------------------------------
        // 3. String form:
        //
        // string1 string2 copy -> substring2
        //
        // Destination must be at least as large as source.
        // The returned string is the interval of the destination that
        // received the copied bytes.
        // ------------------------------------------------------------
        if (srcObject.isString())
        {
            if (!destObject.isString())
                return vm.error("op_copy: typecheck");

            const PSString& src = srcObject.asString();
            PSString dest = destObject.asString();

            if (dest.length() < src.length())
                return vm.error("op_copy: rangecheck");

            if (!dest.putInterval(0, src))
                return vm.error("op_copy: invalidaccess");

            PSString result = dest.getInterval(0, static_cast<uint32_t>(src.length()));
            return s.pushString(result);
        }

        // ------------------------------------------------------------
        // 4. Dictionary form:
        //
        // dict1 dict2 copy -> dict2
        //
        // Copy all entries from source into destination. Existing
        // destination entries with matching keys are replaced.
        // ------------------------------------------------------------
        if (srcObject.isDictionary())
        {
            if (!destObject.isDictionary())
                return vm.error("op_copy: typecheck");

            auto src = srcObject.asDictionary();
            auto dest = destObject.asDictionary();

            if (!src || !dest)
                return vm.error("op_copy: invalidaccess");

            bool success = true;

            src->forEachConst([&](const PSObject& key, const PSObject& value) -> bool
                {
                    if (!dest->put(key, value))
                    {
                        success = false;
                        return false;
                    }

                    return true;
                });

            if (!success)
                return vm.error("op_copy: invalidaccess");

            return s.push(destObject);
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
            return vm.error("op_eq: stackunderflow");

        PSObject a, b;
        s.pop(b);
        s.pop(a);

        bool result = false;

        switch (a.type)
        {
        case PSObjectType::Int:
        case PSObjectType::Real:
            if (b.isNumber())
                result = a.asReal() == b.asReal();
            break;

        case PSObjectType::Bool:
            if (b.isBool())
                result = a.asBool() == b.asBool();
            break;

        case PSObjectType::Null:
            result = b.type == PSObjectType::Null;
            break;

        case PSObjectType::Name:
            if (b.isName())
                result = a.asName() == b.asName();
            else if (b.isString()) {
                const auto& name = a.asName();
                const auto& str = b.asString();
                result = name.size() == str.length() &&
                    (str.length() == 0 || std::memcmp(name.c_str(), str.data(), str.length()) == 0);
            }
            break;

        case PSObjectType::String:
            if (b.isString()) {
                const auto& sa = a.asString();
                const auto& sb = b.asString();
                result = sa.length() == sb.length() &&
                    (sa.length() == 0 || std::memcmp(sa.data(), sb.data(), sa.length()) == 0);
            }
            else if (b.isName()) {
                const auto& str = a.asString();
                const auto& name = b.asName();
                result = name.size() == str.length() &&
                    (str.length() == 0 || std::memcmp(str.data(), name.c_str(), str.length()) == 0);
            }
            break;

        case PSObjectType::Array:
            if (b.isArray())
                result = a.asArray() == b.asArray();
            break;

        case PSObjectType::Dictionary:
            if (b.isDictionary())
                result = a.asDictionary() == b.asDictionary();
            break;

        case PSObjectType::File:
            if (b.isFile())
                result = a.asFile() == b.asFile();
            break;

        case PSObjectType::FontFace:
            if (b.isFontFace())
                result = a.asFontFace() == b.asFontFace();
            break;

        case PSObjectType::Operator:
            if (b.isOperator())
                result = a.asOperator().name() == b.asOperator().name();
            break;

        default:
            break;
        }

        return s.pushBool(result);
    }

    inline bool op_ne(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (!op_equality(vm))
            return false;

        PSObject result;
        if (!s.pop(result) || !result.isBool())
            return vm.error("op_ne: typecheck");

        return s.pushBool(!result.asBool());
    }


    inline bool op_type(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_type: stackunderflow");

        PSObject obj;
        if (!s.pop(obj))
            return vm.error("op_type: stackunderflow");

        PSName typeName;

        switch (obj.type)
        {
        case PSObjectType::Null:       typeName = PSName("nulltype"); break;
        case PSObjectType::Int:        typeName = PSName("integertype"); break;
        case PSObjectType::Float:
        case PSObjectType::Real:       typeName = PSName("realtype"); break;
        case PSObjectType::Bool:       typeName = PSName("booleantype"); break;
        case PSObjectType::Name:       typeName = PSName("nametype"); break;
        case PSObjectType::String:     typeName = PSName("stringtype"); break;
        case PSObjectType::Array:
        case PSObjectType::Matrix:     typeName = PSName("arraytype"); break;
        case PSObjectType::Dictionary:typeName = PSName("dicttype"); break;
        case PSObjectType::File:       typeName = PSName("filetype"); break;
        case PSObjectType::Operator:   typeName = PSName("operatortype"); break;
        case PSObjectType::Mark:       typeName = PSName("marktype"); break;
        case PSObjectType::Save:       typeName = PSName("savetype"); break;

        // WaavScript internal types exposed as PostScript dictionaries.
        //case PSObjectType::Font:
        case PSObjectType::FontFace:   typeName = PSName("dicttype"); break;

        default:
            return vm.error("op_type: typecheck");
        }

        return s.pushLiteralName(typeName);
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
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_readonly: stackunderflow");

        PSObject obj;
        if (!s.pop(obj))
            return vm.error("op_readonly: stackunderflow");

        obj.setAccessReadable(true);
        obj.setAccessWriteable(false);
        obj.setAccessExecutable(true);

        return s.push(obj);
    }

    inline bool op_writeonly(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_writeonly: stackunderflow");

        PSObject obj;
        if (!s.pop(obj))
            return vm.error("op_writeonly: stackunderflow");

        obj.setAccessReadable(false);
        obj.setAccessWriteable(true);
        obj.setAccessExecutable(false);

        return s.push(obj);
    }

    inline bool op_noaccess(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_noaccess: stackunderflow");

        PSObject obj;
        if (!s.pop(obj))
            return vm.error("op_noaccess: stackunderflow");

        obj.setAccessReadable(false);
        obj.setAccessWriteable(false);
        obj.setAccessExecutable(false);

        return s.push(obj);
    }

    inline bool op_executeonly(PSVirtualMachine& vm)
    {
        auto& s = vm.opStack();

        if (s.empty())
            return vm.error("op_executeonly: stackunderflow");

        PSObject obj;
        if (!s.pop(obj))
            return vm.error("op_executeonly: stackunderflow");

        obj.setAccessReadable(false);
        obj.setAccessWriteable(false);
        obj.setAccessExecutable(true);

        return s.push(obj);
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
