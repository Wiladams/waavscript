#pragma once

#include "pscore.h"

namespace waavs {
    static void writeObjectShallow(const PSObject& obj, std::ostream& os = std::cout)
    {
        switch (obj.type) {
        case PSObjectType::Int:      os << obj.asInt(); break;
        case PSObjectType::Real:     os << obj.asReal(); break;
        case PSObjectType::Bool:     os << (obj.asBool() ? "true" : "false"); break;

        case PSObjectType::Name:
            if (obj.isLiteralName())
                os << "/";
            os << obj.asName().c_str();
            break;

        case PSObjectType::String:     os << "(" << obj.asString().toString() << ")"; break;
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
        case PSObjectType::FontFace:
            os << "--FONTFACE--"; break;
        case PSObjectType::Font:
            os << "--FONT--"; break;
        case PSObjectType::File:
            os << "--FILE--"; break;
        default:
            os << "--UNKNOWN--";
            break;
        }
    }



    static void writeObjectDeep(const PSObject& obj, std::ostream& os);  // Forward declaration

    static void writeArrayDeep(std::ostream& os, const PSObject& obj)
    {
        PSArrayHandle arr = obj.asArray();

        if (obj.isExecutable())
            os << '{';
        else
            os << "[";

        for (size_t i = 0; i < arr->size(); ++i) {
            const PSObject& element = arr->elements[i];
            writeObjectDeep(element, os);
            if (i + 1 < arr->size())
                std::cout << " ";
        }

        if (obj.isExecutable())
            os << '}';
        else
            os << ']';
    }

    static void writeDictDeep(std::ostream& os, const PSDictionaryHandle dict)
    {
        if (!dict) {
            os << "<<NULLDICT>>";
            return;
        }

        os << "<<";
        bool first = true;

        dict->forEach([&](PSName key, const PSObject& val) {
            if (!first)
                os << " ";
            first = false;

            os << "/" << key.c_str() << " ";
            writeObjectDeep(val, os);
            return true; // continue iteration
            });

        os << ">>";
    }

    static void writeMatrix(std::ostream& os, const PSMatrix& m) {
        os << "[[" << m.m[0] << " " << m.m[1] << "] ["
            << m.m[2] << " " << m.m[3] << "] ["
            << m.m[4] << " " << m.m[5] << "]]";
    }

    static void writeObjectDeep(const PSObject& obj, std::ostream& os = std::cout)
    {
        switch (obj.type) {
        case PSObjectType::Int:
        case PSObjectType::Real:
        case PSObjectType::Bool:
        case PSObjectType::Name:
        case PSObjectType::Mark:
        case PSObjectType::Null:
            writeObjectShallow(obj, os);
            break;

        case PSObjectType::String: {
            auto& strPs = obj.asString();

            os << "(";
            if (strPs.length() == 0) {
                os << strPs.capacity();
            }
            else {
                os << strPs.toString();
            }

                os << ")";
            }
            break;

        case PSObjectType::Array:
            if (obj.asArray())
                writeArrayDeep(os, obj);
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
            os << "--OP:" << (op.name().isValid() ? op.name().c_str() : "UNKNOWN") << "--";
        }
                                   break;

        case PSObjectType::Matrix:
            os << "--MATRIX: ";
            writeMatrix(os, obj.asMatrix());

            break;
        case PSObjectType::FontFace:
            os << "--FONTFACE:" << std::endl;
            writeObjectDeep(PSObject::fromDictionary(obj.asFontFace()->getDictionary()), os);
        default:
            writeObjectShallow(obj, os);
            break;
        }
    }
}
