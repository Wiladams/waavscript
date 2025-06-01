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
                s.push(PSObject::fromBool(a.asBool() && b.asBool()));
                return true;
            }
            if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(a.asInt()& b.asInt()));
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
                s.push(PSObject::fromBool(a.asBool() || b.asBool()));
                return true;
            }
            if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(a.asInt() | b.asInt()));
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
                s.push(PSObject::fromBool(a.asBool() != b.asBool()));
                return true;
            }
            if (a.type == PSObjectType::Int && b.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(a.asInt() ^ b.asInt()));
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
                s.push(PSObject::fromBool(!a.asBool()));
                return true;
            }
            if (a.type == PSObjectType::Int) {
                s.push(PSObject::fromInt(~a.asInt()));
                return true;
            }
            return false;
        }}
    };

} // namespace waavs
