#pragma once

// NOTE:  Something else must include this, and have PSVirtualMachine defined.
#include "pscore.h"

//======================================================================
// The operators in here are polymorphic, meaning they can apply
// they can apply to different types of objects.
//======================================================================

namespace waavsps {

	// get: container index -> value
    inline bool op_get(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;

        PSObject index = s.back(); s.pop_back();
        PSObject container = s.back(); s.pop_back();

        switch (container.type) {
        case PSObjectType::Array:
            if (index.type != PSObjectType::Int) return false;
            {
                auto* arr = container.data.arr;
                int idx = index.data.iVal;
                if (!arr || idx < 0 || static_cast<size_t>(idx) >= arr->elements.size())
                    return false;
                s.push_back(arr->elements[idx]);
                return true;
            }

        case PSObjectType::String:
            if (index.type != PSObjectType::Int) return false;
            {
                auto* str = container.data.str;
                int idx = index.data.iVal;
                if (!str || idx < 0 || static_cast<size_t>(idx) >= str->length)
                    return false;
                s.push_back(PSObject::fromInt(str->data[idx]));
                return true;
            }

        case PSObjectType::Dictionary:
            if (index.type != PSObjectType::Name) return false;
            {
                auto* dict = container.data.dict;
                PSObject result;
                if (!dict || !dict->get(index.data.name, result)) return false;
                s.push_back(result);
                return true;
            }

        default:
            return false;
        }
    }

	// put: a b c -> a b (c = a[b])
    inline bool op_put(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.size() < 3) return false;

        PSObject value = s.back(); s.pop_back();
        PSObject index = s.back(); s.pop_back();
        PSObject container = s.back(); s.pop_back();

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
                if (!str || idx < 0 || byte < 0 || byte > 255 || static_cast<size_t>(idx) >= str->capacity)
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
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject obj = s.back(); s.pop_back();

        switch (obj.type) {
        case PSObjectType::Array:
            s.push_back(PSObject::fromInt(static_cast<int>(obj.data.arr->elements.size())));
            return true;

        case PSObjectType::String:
            s.push_back(PSObject::fromInt(static_cast<int>(obj.data.str->length)));
            return true;

        case PSObjectType::Dictionary:
            s.push_back(PSObject::fromInt(static_cast<int>(obj.data.dict->entries.size())));
            return true;

        default:
            return false;
        }
    }

    // copy: (n x? ... x? ? x? ... x? x? ... x?) — duplicate top n items
    inline bool op_copy(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;

        if (s.empty()) return false;
        PSObject top = s.back();

        switch (top.type) {
        case PSObjectType::Array: {
            PSObject countObj = s[s.size() - 1];
            if (countObj.type != PSObjectType::Int) return false;

            int count = countObj.data.iVal;
            if (s.size() < static_cast<size_t>(count + 1)) return false;

            PSArray* arr = new PSArray();
            for (int i = count; i > 0; --i)
                arr->elements.push_back(s[s.size() - i - 1]);

            // remove values + count
            s.erase(s.end() - (count + 1), s.end());

            s.push_back(PSObject::fromArray(arr));
            return true;
        }

        case PSObjectType::String: {
            auto* src = top.data.str;
            if (!src) return false;

            auto* dest = new PSString(src->length);
            for (int i = 0; i < src->length; ++i)
                dest->data[i] = src->data[i];

            s.pop_back();
            s.push_back(PSObject::fromString(dest));
            return true;
        }

        default:
            return false;
        }
    }

    inline bool op_forall(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;

        PSObject proc = s.back(); s.pop_back();
        PSObject obj = s.back(); s.pop_back();

        if (proc.type != PSObjectType::Array || !proc.data.arr || !proc.data.arr->isExecutable())
            return false;

        switch (obj.type) {
        case PSObjectType::Array: {
            for (const auto& val : obj.data.arr->elements) {
                s.push_back(val);
                vm.execArray(proc.data.arr);
            }
            return true;
        }

        case PSObjectType::String: {
            for (int i = 0; i < obj.data.str->length; ++i) {
                s.push_back(PSObject::fromInt(obj.data.str->data[i]));
                vm.execArray(proc.data.arr);
            }
            return true;
        }

        case PSObjectType::Dictionary: {
            for (const auto& entry : obj.data.dict->entries) {
                s.push_back(PSObject::fromName(entry.first.c_str()));
                s.push_back(entry.second);
                vm.execArray(proc.data.arr);
            }
            return true;
        }

        default: return false;
        }
    }



    inline bool op_assign(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;

        PSObject b = s.back(); s.pop_back();
        PSObject a = s.back(); s.pop_back();

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

        s.push_back(PSObject::fromBool(result));
        return true;
    }

    inline bool op_ne(PSVirtualMachine& vm) {
        bool ok = op_assign(vm);
        if (!ok) return false;

        PSObject top = vm.operandStack.back(); vm.operandStack.pop_back();
        if (top.type != PSObjectType::Bool) return false;

        vm.operandStack.push_back(PSObject::fromBool(!top.data.bVal));
        return true;
    }


    inline bool op_type(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject obj = s.back(); s.pop_back();

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

        s.push_back(PSObject::fromName(typeName));
        return true;
    }


    inline bool op_cvs(PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        PSObject val = s.back(); s.pop_back();

        char buffer[64] = { 0 };

        switch (val.type) {
        case PSObjectType::Int:
            std::snprintf(buffer, sizeof(buffer), "%d", val.data.iVal);
            break;
        case PSObjectType::Real:
            std::snprintf(buffer, sizeof(buffer), "%.6g", val.data.fVal);
            break;
        case PSObjectType::Bool:
            std::snprintf(buffer, sizeof(buffer), val.data.bVal ? "true" : "false");
            break;
        case PSObjectType::Name:
            std::snprintf(buffer, sizeof(buffer), "/%s", val.data.name);
            break;
        default:
            std::snprintf(buffer, sizeof(buffer), "<object>");
            break;
        }

        auto* str = new PSString(std::strlen(buffer));
        for (size_t i = 0; i < std::strlen(buffer); ++i)
            str->data[i] = buffer[i];

        s.push_back(PSObject::fromString(str));
        return true;
    }










    // bind: proc -> proc' (resolve names to operators)
    inline bool op_bind(PSVirtualMachine& vm) {
        vm.bind();
        return true;
    }
}
