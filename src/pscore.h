#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>


#include "ocspan.h"


// global name table for interned strings
namespace waavs {
    struct PSNameTable {
        std::unordered_map<OctetCursor, std::string> pool;

        const char* intern(const OctetCursor & name) 
        {
			// if it's already in the pool, return pointer to the string
            auto it = pool.find(name);
            if (it != pool.end()) return it->second.c_str();

            // if it's not in the pool, create a string to represent it
            // put it in the pool.
            auto inserted = pool.emplace(name, std::string((char *)name.data(), name.size()));
            return inserted.first->second.c_str();
        }

		// We want to support a singleton instance of PSNameTable
        static PSNameTable * getTable() {
			static std::unique_ptr<PSNameTable> gTable = std::make_unique<PSNameTable>();
            return gTable.get();
		}
    };
}

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
    enum struct PSObjectType {
        Null,
        Int,
        Real,
        Bool,
        Name,
        String,
        Array,
        Dictionary,
        Operator,
        Mark
    };



    // --------------------
    // PSString
    // --------------------
    struct PSString
    {
        uint32_t capacity = 0;
        uint8_t* data = nullptr;
        uint32_t length = 0;
        bool isExec = false;

        PSString(size_t cap)
            : capacity(cap), length(0), isExec(false) {
            data = new uint8_t[cap];
        }

        PSString(const char* literal) {
            length = capacity = std::strlen(literal);
            data = new uint8_t[capacity];
            std::memcpy(data, literal, capacity);
            isExec = false;
        }

        ~PSString() {
            delete[] data;
        }

        bool isExecutable() const {
            return isExec;
        }

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
            if (index >= capacity) return false;
            data[index] = value;
            if (index >= length) length = index + 1;
            return true;
        }

        PSString* getInterval(uint32_t index, uint32_t count) const {
            if (index >= capacity) return new PSString((size_t)0);
            if (count > capacity - index) count = capacity - index;

            PSString* out = new PSString(count);
            for (uint32_t i = 0; i < count; ++i)
                out->put(i, data[index + i]);

            return out;
        }

        bool putInterval(uint32_t offset, const PSString& other) {
            if (offset >= capacity) return false;
            uint32_t count = other.length;
            if (offset + count > capacity) count = capacity - offset;

            for (uint32_t i = 0; i < count; ++i)
                put(offset + i, other.data[i]);

            return true;
        }
    };

    // --------------------
    // PSOperator
    // --------------------
    using PSOperatorFunc = std::function<bool(PSVirtualMachine &)>;
    using PSOperatorMap = std::unordered_map<OctetCursor, PSOperatorFunc>;
    
    struct PSOperator {
        OctetCursor name{};
        PSOperatorFunc func = nullptr;

        //PSOperator() = default;

        PSOperator(const OctetCursor&nm, PSOperatorFunc f)
            : name(nm)
            , func(f) {}
    };

    // --------------------
    // PSObject
    // --------------------
    struct PSObject 
    {
        PSObjectType type = PSObjectType::Null;

        union {
            int32_t iVal;
            double  fVal;
            bool    bVal;
            PSString* str;
            PSArray* arr;
            PSDictionary* dict;
            PSOperator* op;
			const char* name;         // strings are interned as char pointers
        } data;

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

        bool resetFromOperator(PSOperator* f) {
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
        bool isExecutable() const;

        bool isNumber() const {
            return type == PSObjectType::Int || type == PSObjectType::Real;
        }

        double asReal() const {
            return (type == PSObjectType::Int) ? static_cast<double>(data.iVal) : data.fVal;
        }
    };

    // --------------------
    // PSDictionary
    // --------------------
    struct PSDictionary {
        std::unordered_map<const char *, PSObject> entries;

        bool put(const char * key, const PSObject& value) {
            entries[key] = value;
            return true;
        }

        bool get(const char * key, PSObject& out) const {
            auto it = entries.find(key);
            if (it == entries.end()) return false;
            out = it->second;
            return true;
        }

        bool contains(const char * key) const {
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
        bool isExec = false;

        PSArray() = default;

        explicit PSArray(size_t initialSize, const PSObject& fill = PSObject()) {
            elements.resize(initialSize, fill);
        }

        size_t length() const {
            return elements.size();
        }

        bool isExecutable() const {
            return isExec;
        }

        void setExecutable(bool flag) {
            isExec = flag;
        }

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
            result->isExec = isExec;
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
            result->isExec = isExec;
            return result;
        }
    };

    // Need to put this here to breeak circular dependency
    inline bool PSObject::isExecutable() const {
        return type == PSObjectType::Operator ||
            (type == PSObjectType::String && data.str && data.str->isExecutable()) ||
            (type == PSObjectType::Array && data.arr && data.arr->isExecutable());
    }
}
