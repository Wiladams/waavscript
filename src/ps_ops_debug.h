#pragma once

#include "pscore.h"
#include "psvm.h"
#include <iostream>

namespace waavs {
    static void writeObjectShallow(const PSObject& obj)
    {
        switch (obj.type) {
        case PSObjectType::Int:      std::cout << obj.asInt(); break;
        case PSObjectType::Real:     std::cout << obj.asReal(); break;
        case PSObjectType::Bool:     std::cout << (obj.asBool() ? "true" : "false"); break;
        case PSObjectType::Name:     std::cout << "/" << obj.asName(); break;
        case PSObjectType::String:
            if (obj.asString()) std::cout << "(" << obj.asString()->toString() << ")";
            break;
        case PSObjectType::Null:     std::cout << "NULL"; break;
        case PSObjectType::Mark:     std::cout << "-MARK-"; break;
        case PSObjectType::Array:
            if (obj.asArray())
                std::cout << "[...(" << obj.asArray()->size() << ")]";
            else
				std::cout << "[NULLPTR]";
            break;
        case PSObjectType::Dictionary: std::cout << "<<...>>"; break;
        case PSObjectType::Operator: std::cout << "--OP--"; break;
        default: std::cout << "--UNKNOWN--"; break;
        }
    }



    static void writeObjectDeep(const PSObject& obj);  // Forward declaration

    static void writeArrayDeep(const PSArray* arr) 
    {
        std::cout << "[";
        for (size_t i = 0; i < arr->size(); ++i) {
            const PSObject& element = arr->elements[i];
            writeObjectDeep(element);
            if (i + 1 < arr->size())
                std::cout << " ";
        }
        std::cout << "]";
    }

    static void writeDictDeep(const PSDictionary* dict) 
    {
        std::cout << "<<";
        bool first = true;
        for (const auto& pair : dict->entries) {
            if (!first) 
                std::cout << " ";
            
            first = false;
            std::cout << "/" << pair.first << " ";
            writeObjectDeep(pair.second);
        }
        std::cout << ">>";
    }

    static void writeObjectDeep(const PSObject& obj)
    {
        switch (obj.type) {
        case PSObjectType::Int:
        case PSObjectType::Real:
        case PSObjectType::Bool:
        case PSObjectType::Name:
        case PSObjectType::Mark:
        case PSObjectType::Null:
            writeObjectShallow(obj);
            break;

        case PSObjectType::String:
            if (obj.asString())
                std::cout << "(" << obj.asString()->toString() << ")";
            else
                std::cout << "()";
            break;

        case PSObjectType::Array:
            if (obj.asArray())
                writeArrayDeep(obj.asArray());
            else
                std::cout << "[]";
            break;

        case PSObjectType::Dictionary:
            if (obj.asDictionary())
                writeDictDeep(obj.asDictionary());
            else
                std::cout << "<<>>";
            break;

        case PSObjectType::Operator:
            std::cout << "--OP:" << (obj.asOperator() && obj.asOperator()->name ? obj.asOperator()->name : "unknown") << "--";
            break;


        default:
            std::cout << "--UNKNOWN--";
            break;
        }
    }

    static const PSOperatorFuncMap debugOps = {

        { "==", [](PSVirtualMachine& vm) -> bool {
            if (vm.opStack().empty()) return false;
            PSObject obj;
            vm.opStack().pop(obj);

			writeObjectDeep(obj);

            std::cout << std::endl;
            return true;
        }},

        { "=", [](PSVirtualMachine& vm) -> bool {
            if (vm.opStack().empty()) return false;

            PSObject obj;
            if (!vm.opStack().pop(obj))
                return false;

			writeObjectShallow(obj);

            std::cout << std::endl;

            return true;
        }},

        { "print", [](PSVirtualMachine& vm) -> bool {
            if (vm.opStack().empty()) return false;
            PSObject obj;
			vm.opStack().pop(obj);

            if (!obj.isString() || !obj.asString()) return false;
            std::cout << obj.asString()->toString();
            return true;
        }},

        { "flush", [](PSVirtualMachine&) -> bool {
            std::cout.flush();
            return true;
        }},

        { "stack", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
            for (const auto& obj : s) {
                writeObjectShallow(obj);
				std::cout << " ";
            }
			std::cout << std::endl;

            return true;
        }},

        { "pstack", [](PSVirtualMachine& vm) -> bool {
			auto& s = vm.opStack();
            for (const auto& obj : s) {
				writeObjectDeep(obj);
                std::cout << std::endl;
            }
            return true;
        }},

        { "errordict", [](PSVirtualMachine& vm) -> bool {
            PSDictionary* dict = new PSDictionary();
            vm.opStack().push(PSObject::fromDictionary(dict));
            return true;
        }},

        { "handleerror", [](PSVirtualMachine&) -> bool {
            std::cerr << "An error occurred." << std::endl;
            return true;
        }}
    };

} // namespace waavs
