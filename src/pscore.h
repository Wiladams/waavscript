#pragma once

#include <cstdint>
#include <cstring>

#include <string>
#include <string_view>
#include <unordered_map>
#include <map>

#include <vector>
#include <functional>
#include <memory>


#include "ocspan.h"
#include "nametable.h"





namespace waavs {
    // --------------------
    // Forward Declarations
    // --------------------
    struct PSObject;
    struct PSString;
    struct PSArray;
    struct PSOperator;
    struct PSDictionary;
    struct PSVirtualMachine;

    // --------------------
    // PSObject Type Enum
    // --------------------
    enum struct PSObjectType : char {
		Null    = 'z'      // Null type, represents a null object
        ,Int     = 'i'      // Integer type, represents a 32-bit signed integer
        ,Real    = 'f'      // Real type, represents a double-precision floating-point number
		,Bool    = 'b'      // Boolean type, represents a true or false value
		,Name    = 'n'      // Name type, represents an interned name (string)
		,String = 's'      // String type, represents a PSString object
        ,Array   = 'a'
        ,Dictionary = 'd'
        ,Operator    = 'O'
        ,Mark    = 'm'
        ,Invalid = '?'      // Invalid type, used for uninitialized objects
		,Any = '*'          // Any type, used for generic operations
    };



    // --------------------
    // PSString
    // --------------------
    struct PSString
    {
        uint32_t fCapacity = 0;
        uint8_t* data = nullptr;
        uint32_t length = 0;
        //bool isExec = false;

        PSString(size_t cap)
            : fCapacity(cap)
            , length(0)
            //, isExec(false) 
        {
            data = new uint8_t[cap];
        }

        PSString(const char* literal) 
        {
            length = fCapacity = std::strlen(literal);
            data = new uint8_t[fCapacity];
            std::memcpy(data, literal, fCapacity);
            //isExec = false;
        }

        ~PSString() { delete[] data;}

        size_t size() const {return length;}
		size_t capacity() const { return fCapacity; }

        //bool isExecutable() const {
        //    return isExec;
        //}

        void reset() {
            length = 0;
        }

        std::string toString() const {
            return std::string(reinterpret_cast<const char*>(data), length);
        }

        bool get(uint32_t index, uint8_t& out) const {
            if (index >= length) return false;
            out = data[index];
            return true;
        }

        bool put(uint32_t index, uint8_t value) {
            if (index >= fCapacity) return false;
            data[index] = value;
            if (index >= length) length = index + 1;
            return true;
        }

        PSString* getInterval(uint32_t index, uint32_t count) const {
            if (index >= fCapacity) return new PSString((size_t)0);
            if (count > fCapacity - index) count = fCapacity - index;

            PSString* out = new PSString(count);
            for (uint32_t i = 0; i < count; ++i)
                out->put(i, data[index + i]);

            return out;
        }

        bool putInterval(uint32_t offset, const PSString& other) {
            if (offset >= fCapacity) return false;
            uint32_t count = other.length;
            if (offset + count > fCapacity) count = fCapacity - offset;

            for (uint32_t i = 0; i < count; ++i)
                put(offset + i, other.data[i]);

            return true;
        }
    };

    // --------------------
    // PSOperator
    // --------------------
    // These definitions are used for builtin operators that are known at compile time
    using PSOperatorFunc = std::function<bool(PSVirtualMachine&)>;
    using PSOperatorFuncMap = std::unordered_map<const char*, PSOperatorFunc>;

    struct PSOperator {
        const char* name = nullptr;       // Always interned and stable
        PSOperatorFunc func = nullptr;

        PSOperator() = default;

        PSOperator(const char* internedName, PSOperatorFunc f) noexcept
            : name(internedName), func(f) {
        }
    };



    // --------------------
    // PSObject
    // --------------------
    struct PSObject
    {
        union {
            int32_t iVal;
            double  fVal;
            bool    bVal;
            PSString* str;
            PSArray* arr;
            PSDictionary* dict;
            const PSOperator* op;
            const char* name;         // strings are interned as char pointers
        } data;

        PSObjectType type = PSObjectType::Null;
		bool fIsExec = false; // Is this object executable?



        // Reset the current instance to a specific type
        bool resetFromInt(int32_t v) {
            type = PSObjectType::Int;
            data.iVal = v;
            return true;
        }
        bool resetFromReal(double v) {
            type = PSObjectType::Real;
            data.fVal = v;
            return true;
        }
        bool resetFromBool(bool v) {
            type = PSObjectType::Bool;
            data.bVal = v;
            return true;
        }

        bool resetFromName(const OctetCursor& span)
        {
            const char* internedName = PSNameTable::getTable()->intern(span);

            type = PSObjectType::Name;
            data.name = internedName;

            return true;
        }

        bool resetFromString(PSString* s) {
            type = PSObjectType::String;
            data.str = s;
            return true;
        }

        bool resetFromArray(PSArray* a) {
            type = PSObjectType::Array;
            data.arr = a;
            return true;
        }

        bool resetFromOperator(const PSOperator* f) {
            type = PSObjectType::Operator;
            data.op = f;
            return true;
        }

        bool resetFromDictionary(PSDictionary* d) {
            type = PSObjectType::Dictionary;
            data.dict = d;
            return true;
        }

        bool resetFromMark() {
            type = PSObjectType::Mark;
            return true;
        }

        bool reset() {
            *this = PSObject();
            return true;
        }

        // Some attributes
        inline bool isExecutable() const {return fIsExec;}
		void setExecutable(bool flag) { fIsExec = flag; }

        // Validating the type of the object
        bool is(PSObjectType t) const { return (type == t) || (t == PSObjectType::Any); }
        bool isNumber() const { return type == PSObjectType::Int || type == PSObjectType::Real; }
        bool isInt() const { return type == PSObjectType::Int; }
        bool isReal() const { return type == PSObjectType::Real; }
        bool isArray() const { return type == PSObjectType::Array; }
        bool isString() const { return type == PSObjectType::String; }
        bool isDictionary() const { return type == PSObjectType::Dictionary; }
        bool isOperator() const { return type == PSObjectType::Operator; }
        bool isName() const { return type == PSObjectType::Name; }
        bool isBool() const { return type == PSObjectType::Bool; }
        bool isMark() const { return type == PSObjectType::Mark; }

        // Return the object as a specific type
        int asInt() const { return data.iVal; }
        double asReal() const { return (type == PSObjectType::Int) ? static_cast<double>(data.iVal) : data.fVal; }
        bool asBool() const { return data.bVal; }
        PSArray* asArray() const { return (type == PSObjectType::Array) ? data.arr : nullptr; }
        PSString* asString() const { return (type == PSObjectType::String) ? data.str : nullptr; }
        const char* asName() const { return (type == PSObjectType::Name) ? data.name : nullptr; }
        PSDictionary* asDictionary() const { return (type == PSObjectType::Dictionary) ? data.dict : nullptr; }
        const PSOperator* asOperator() const { return (type == PSObjectType::Operator) ? data.op : nullptr; }

        // Convenience static functions of the 'fromXXX()' variety
        static PSObject fromInt(int32_t v) { PSObject obj; obj.resetFromInt(v); return obj; }
        static PSObject fromReal(double v) { PSObject obj; obj.resetFromReal(v); return obj; }
        static PSObject fromBool(bool v) { PSObject obj; obj.resetFromBool(v); return obj; }
        static PSObject fromName(const OctetCursor& span) { PSObject obj;  obj.resetFromName(span); return obj; }
        static PSObject fromString(PSString* s) { PSObject obj; obj.resetFromString(s);  return obj; }
        static PSObject fromArray(PSArray* a) { PSObject obj;  obj.resetFromArray(a);  return obj; }
        static PSObject fromOperator(const PSOperator* f) { PSObject obj;  obj.resetFromOperator(f);  return obj; }
        static PSObject fromDictionary(PSDictionary* d) { PSObject obj;  obj.resetFromDictionary(d);   return obj; }
        static PSObject fromMark() { PSObject obj;  obj.resetFromMark();  return obj; }
    };

    struct PSOperatorSignature
    {
        const char* name;       // Name of the operator
        PSObjectType kinds[8];  // Types of arguments
        uint8_t arity;          // Number of arguments

        PSOperatorSignature(const char* opName, const char* typeString)
        {
            name = PSNameTable::INTERN(opName);
            arity = 0;

            // Assign argument types
            while (*typeString && arity < 8) {
                kinds[arity++] = static_cast<PSObjectType>(*typeString++);
            }
        }
    };

    struct PSOperatorArgs {
        const PSOperatorSignature& signature;
        PSObject values[8];   // fixed-size array
        size_t count;

        bool isValid() const { return count == signature.arity; }

        PSObject& operator[](size_t i) { return values[i]; }
        const PSObject& operator[](size_t i) const { return values[i]; }

        const char* operatorName() const { return signature.name; }
        PSObjectType operandType(size_t i) const { return (i < count) ? signature.kinds[i] : PSObjectType::Invalid; }
    };


    // --------------------
    // PSDictionary
    // --------------------
    struct PSDictionary {
        std::unordered_map<const char*, PSObject> entries;

        size_t size() const {
            return entries.size();
		}

        bool put(const char* key, const PSObject& value) {
            entries[key] = value;
            return true;
        }

        bool get(const char* key, PSObject& out) const {
            auto it = entries.find(key);
            if (it == entries.end()) return false;
            out = it->second;
            return true;
        }

        bool contains(const char* key) const {
            return entries.find(key) != entries.end();
        }

        void clear() {
            entries.clear();
        }
    };

    // --------------------
    // PSArray
    // --------------------
    struct PSArray {
        std::vector<PSObject> elements;
       // bool isExec = false;

        PSArray() = default;

        explicit PSArray(size_t initialSize, const PSObject& fill = PSObject()) {
            elements.resize(initialSize, fill);
        }

        size_t size() const {
            return elements.size();
        }

        //bool isExecutable() const {
        //    return isExec;
        //}

        //void setExecutable(bool flag) {
        //    isExec = flag;
        //}

        bool get(size_t index, PSObject& out) const {
            if (index >= elements.size()) return false;
            out = elements[index];
            return true;
        }

        bool put(size_t index, const PSObject& val) {
            if (index >= elements.size()) return false;
            elements[index] = val;
            return true;
        }

        bool append(const PSObject& val) {
            elements.push_back(val);
            return true;
        }

        void reset() {
            elements.clear();
        }

        PSArray* copy() const {
            PSArray* result = new PSArray();
            result->elements = elements;
            //result->isExec = isExec;
            return result;
        }

        PSArray* subarray(size_t index, size_t count) const {
            if (index >= elements.size()) return new PSArray(0);
            count = std::min(count, elements.size() - index);
            PSArray* result = new PSArray();
            result->elements.insert(
                result->elements.end(),
                elements.begin() + index,
                elements.begin() + index + count
            );
            //result->isExec = isExec;
            return result;
        }
    };


}

// Help the operand processing
namespace waavs {
	// An operand signature is a tuple of name and types of arguments
    struct OperandSignature {
        const char* name;       // name of the operator
        PSObjectType kinds[8];  // Max 8 arguments
        uint8_t arity;          // how many arguments are there
    };
}
