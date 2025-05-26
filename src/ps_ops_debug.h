#pragma once

#include "pscore.h"
#include "psvm.h"
#include <iostream>

namespace waavs {

    static const PSOperatorFuncMap debugOps = {

        { "==", [](PSVirtualMachine& vm) -> bool {
            if (vm.operandStack.empty()) return false;
            const PSObject& obj = vm.operandStack.back();
            std::cout << "[==] ";

            switch (obj.type) {
            case PSObjectType::Int:      std::cout << obj.data.iVal; break;
            case PSObjectType::Real:     std::cout << obj.data.fVal; break;
            case PSObjectType::Bool:     std::cout << (obj.data.bVal ? "true" : "false"); break;
            case PSObjectType::Null:     std::cout << "null"; break;
            case PSObjectType::Mark:     std::cout << "-mark-"; break;
            case PSObjectType::Name:     std::cout << "/" << obj.data.name; break;
            case PSObjectType::String:
                if (obj.data.str) std::cout << "(" << obj.data.str->toString() << ")";
                else std::cout << "(null)";
                break;
            case PSObjectType::Array:    std::cout << "[...]"; break;
            case PSObjectType::Dictionary: std::cout << "<<...>>"; break;
            case PSObjectType::Operator: std::cout << "--op--"; break;
            default: std::cout << "--unknown--"; break;
            }

            std::cout << std::endl;
            return true;
        }},

        { "=", [](PSVirtualMachine& vm) -> bool {
            if (vm.operandStack.empty()) return false;
            const PSObject& obj = vm.operandStack.back();

            switch (obj.type) {
            case PSObjectType::Int:      std::cout << obj.data.iVal; break;
            case PSObjectType::Real:     std::cout << obj.data.fVal; break;
            case PSObjectType::Bool:     std::cout << (obj.data.bVal ? "true" : "false"); break;
            case PSObjectType::Null:     std::cout << "null"; break;
            case PSObjectType::Mark:     std::cout << "-mark-"; break;
            case PSObjectType::Name:     std::cout << "/" << obj.data.name; break;
            case PSObjectType::String:
                if (obj.data.str) std::cout << obj.data.str->toString();
                break;
            default:
                std::cout << "?";
                break;
            }

            std::cout << std::endl;
            return true;
        }},

        { "print", [](PSVirtualMachine& vm) -> bool {
            if (vm.operandStack.empty()) return false;
            PSObject obj = vm.operandStack.back(); vm.operandStack.pop_back();

            if (obj.type != PSObjectType::String || !obj.data.str) return false;
            std::cout << obj.data.str->toString();
            return true;
        }},

        { "flush", [](PSVirtualMachine&) -> bool {
            std::cout.flush();
            return true;
        }},

        { "stack", [](PSVirtualMachine& vm) -> bool {
            for (const auto& obj : vm.operandStack) {
                switch (obj.type) {
                case PSObjectType::Int:      std::cout << obj.data.iVal << std::endl; break;
                case PSObjectType::Real:     std::cout << obj.data.fVal << std::endl; break;
                case PSObjectType::Bool:     std::cout << (obj.data.bVal ? "true" : "false") << std::endl; break;
                case PSObjectType::Name:     std::cout << "/" << obj.data.name << std::endl; break;
                case PSObjectType::String:
                    if (obj.data.str) std::cout << "(" << obj.data.str->toString() << ")" << std::endl;
                    break;
                case PSObjectType::Null:     std::cout << "null" << std::endl; break;
                case PSObjectType::Mark:     std::cout << "-mark-" << std::endl; break;
                case PSObjectType::Array:    std::cout << "[...]" << std::endl; break;
                case PSObjectType::Dictionary: std::cout << "<<...>>" << std::endl; break;
                case PSObjectType::Operator: std::cout << "--op--" << std::endl; break;
                default: std::cout << "--unknown--" << std::endl; break;
                }
            }
            return true;
        }},

        { "pstack", [](PSVirtualMachine& vm) -> bool {
            for (const auto& obj : vm.operandStack) {
                std::cout << "[==] ";
                switch (obj.type) {
                case PSObjectType::Int:      std::cout << obj.data.iVal; break;
                case PSObjectType::Real:     std::cout << obj.data.fVal; break;
                case PSObjectType::Bool:     std::cout << (obj.data.bVal ? "true" : "false"); break;
                case PSObjectType::Name:     std::cout << "/" << obj.data.name; break;
                case PSObjectType::String:
                    if (obj.data.str) std::cout << "(" << obj.data.str->toString() << ")";
                    break;
                case PSObjectType::Null:     std::cout << "null"; break;
                case PSObjectType::Mark:     std::cout << "-mark-"; break;
                case PSObjectType::Array:    std::cout << "[...]" << std::endl; break;
                case PSObjectType::Dictionary: std::cout << "<<...>>"; break;
                case PSObjectType::Operator: std::cout << "--op--"; break;
                default: std::cout << "--unknown--"; break;
                }
                std::cout << std::endl;
            }
            return true;
        }},

        { "errordict", [](PSVirtualMachine& vm) -> bool {
            PSDictionary* dict = new PSDictionary();
            vm.operandStack.push_back(PSObject::fromDictionary(dict));
            return true;
        }},

        { "handleerror", [](PSVirtualMachine&) -> bool {
            std::cerr << "An error occurred." << std::endl;
            return true;
        }}
    };

} // namespace waavs
