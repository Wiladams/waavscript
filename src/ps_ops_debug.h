#pragma once

#include "pscore.h"
#include "psvm.h"
#include <iostream>

namespace waavs {
    static void writeObjectShallow(std::ostream& os, const PSObject& obj)
    {
        switch (obj.type) {
        case PSObjectType::Int:      os << obj.asInt(); break;
        case PSObjectType::Real:     os << obj.asReal(); break;
        case PSObjectType::Bool:     os << (obj.asBool() ? "true" : "false"); break;
        case PSObjectType::Name:     os << "/" << obj.asName(); break;
        case PSObjectType::String:
            if (obj.asString()) os << "(" << obj.asString()->toString() << ")";
            break;
        case PSObjectType::Null:     os << "NULL"; break;
        case PSObjectType::Mark:     os << "-MARK-"; break;
        case PSObjectType::Array:
            if (obj.asArray())
                os << "[...(" << obj.asArray()->size() << ")]";
            else
                os << "[NULLPTR]";
            break;
        case PSObjectType::Dictionary: os << "<<...>>"; break;
        case PSObjectType::Operator: os << "--OP--"; break;
        case PSObjectType::Matrix: {
            auto m = obj.asMatrix();
            os << "[" << m.m[0] << " " << m.m[1] << " " << m.m[2] << " " << m.m[3] << " " << m.m[4] << " " << m.m[5] << "]";
        }
            break;

        default:
            os << "--UNKNOWN--";
            break;
        }
    }



    static void writeObjectDeep(std::ostream& os, const PSObject& obj);  // Forward declaration

    static void writeArrayDeep(std::ostream& os, const PSArrayHandle arr)
    {
        std::cout << "[";
        for (size_t i = 0; i < arr->size(); ++i) {
            const PSObject& element = arr->elements[i];
            writeObjectDeep(os, element);
            if (i + 1 < arr->size())
                std::cout << " ";
        }
        std::cout << "]";
    }

    static void writeDictDeep(std::ostream& os, const PSDictionaryHandle dict)
    {
        if (!dict) {
            os << "<<NULLDICT>>";
            return;
        }

        os << "<<";
        bool first = true;
        for (const auto& pair : dict->entries()) {
            if (!first) 
                os << " ";
            
            first = false;
            os << "/" << pair.first << " ";
            writeObjectDeep(os, pair.second);
        }
        os << ">>";
    }

    static void writeMatrix(std::ostream& os, const PSMatrix& m) {
        os << "[[" << m.m[0] << " " << m.m[1] << "] ["
            << m.m[2] << " " << m.m[3] << "] ["
            << m.m[4] << " " << m.m[5] << "]]";
    }

    static void writeObjectDeep(std::ostream& os, const PSObject& obj)
    {
        switch (obj.type) {
        case PSObjectType::Int:
        case PSObjectType::Real:
        case PSObjectType::Bool:
        case PSObjectType::Name:
        case PSObjectType::Mark:
        case PSObjectType::Null:
            writeObjectShallow(os, obj);
            break;

        case PSObjectType::String:
            if (obj.asString())
                os << "(" << obj.asString()->toString() << ")";
            else
                os << "()";
            break;

        case PSObjectType::Array:
            if (obj.asArray())
                writeArrayDeep(os, obj.asArray());
            else
                os << "[]";
            break;

        case PSObjectType::Dictionary:
            if (obj.asDictionary())
                writeDictDeep(os, obj.asDictionary());
            else
                os << "<<>>";
            break;

        case PSObjectType::Operator: {
                auto op = obj.asOperator();
                os << "--OP:" << (op.name ? op.name : "UNKNOWN") << "--";
            }
            break;

		case PSObjectType::Matrix:
            os << "--MATRIX: ";
		    writeMatrix(os, obj.asMatrix());

			break;

        default:
            os << "--UNKNOWN--";
            break;
        }
    }

    static bool op_eqeq(PSVirtualMachine& vm) {
        if (vm.opStack().empty()) return false;
        PSObject obj;
        vm.opStack().pop(obj);
        writeObjectDeep(std::cout, obj);
        std::cout << std::endl;
        return true;
    }

    static bool op_eq(PSVirtualMachine& vm) {
        if (vm.opStack().empty()) return false;
        PSObject obj;
        if (!vm.opStack().pop(obj)) return false;
        writeObjectShallow(std::cout, obj);
        std::cout << std::endl;
        return true;
    }

    static bool op_print(PSVirtualMachine& vm) {
        if (vm.opStack().empty()) return false;
        PSObject obj;
        vm.opStack().pop(obj);

        if (!obj.isString() || !obj.asString()) return false;
        std::cout << obj.asString()->toString();
        return true;
    }

    static bool op_flush(PSVirtualMachine&) {
        std::cout.flush();
        return true;
    }

    static bool op_stack(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        for (const auto& obj : s) {
            writeObjectShallow(std::cout, obj);
            std::cout << " ";
        }
        std::cout << std::endl;
        return true;
    }

    static bool op_pstack(PSVirtualMachine& vm) {
        auto& s = vm.opStack();
        std::printf("<< pstack BEGIN <<\n");
        for (const auto& obj : s) {
            writeObjectDeep(std::cout, obj);
            std::cout << std::endl;
        }
        std::printf(">> pstack END >>\n");
        return true;
    }

    static bool op_errordict(PSVirtualMachine& vm) {
        auto dict = PSDictionary::create();
        vm.opStack().push(PSObject::fromDictionary(dict));
        return true;
    }

    static bool op_handleerror(PSVirtualMachine&) {
        std::cerr << "An error occurred." << std::endl;
        return true;
    }

    static bool op_showpage(PSVirtualMachine&) {
        std::cerr << "== SHOWPAGE ==" << std::endl;
        return true;
    }

    // --- Debug Operator Table ---

    inline const PSOperatorFuncMap& getDebugOps() {
        static const PSOperatorFuncMap table = {
            { "==",         op_eqeq },
            { "=",          op_eq },
            { "print",      op_print },
            { "flush",      op_flush },
            { "stack",      op_stack },
            { "pstack",     op_pstack },
            { "errordict",  op_errordict },
            { "handleerror",op_handleerror },
            { "showpage",   op_showpage }
        };
        return table;
    }

} // namespace waavs
