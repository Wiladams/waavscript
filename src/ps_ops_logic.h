#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    static const PSOperatorFuncMap logicOps = {
        { "and", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2) return false;

            PSObject b;
            PSObject a;

            s.pop(b);
            s.pop(a);

            if (a.type == PSObjectType::Bool && b.type == PSObjectType::Bool) {
                s.push(PSObject::fromBool(a.data.bVal && b.data.bVal));
                return true;
            }
            if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(a.data.iVal & b.data.iVal));
                return true;
            }
            return false;
        }},

        { "or", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2) return false;

            PSObject b;
            PSObject a;
            
            s.pop(b);
            s.pop(a);

            if (a.type == PSObjectType::Bool && b.type == PSObjectType::Bool) {
                s.push(PSObject::fromBool(a.data.bVal || b.data.bVal));
                return true;
            }
            if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(a.data.iVal | b.data.iVal));
                return true;
            }
            return false;
        }},

        { "xor", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2) return false;

            PSObject b;
            PSObject a;

            s.pop(b);
            s.pop(a);

            if (a.type == PSObjectType::Bool && b.type == PSObjectType::Bool) {
                s.push(PSObject::fromBool(a.data.bVal != b.data.bVal));
                return true;
            }
            if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(a.data.iVal ^ b.data.iVal));
                return true;
            }
            return false;
        }},

        { "not", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject a;
            
            s.pop(a);

            if (a.type == PSObjectType::Bool) {
                s.push(PSObject::fromBool(!a.data.bVal));
                return true;
            }
            if (a.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(~a.data.iVal));
                return true;
            }
            return false;
        }}
    };

} // namespace waavs
