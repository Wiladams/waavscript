#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <unordered_map>

#include "bspan.h"

namespace waavsps {
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
        uint8_t* data = nullptr;
        uint32_t capacity = 0;
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
    using PSOperatorFunc = bool (*)(PSVirtualMachine&);

    struct PSOperator {
        const char* name = nullptr;
        PSOperatorFunc func = nullptr;

        PSOperator() = default;

        PSOperator(const char* n, PSOperatorFunc f)
            : name(n), func(f) {
        }
    };

    // --------------------
    // PSObject
    // --------------------
    struct PSObject {
        PSObjectType type = PSObjectType::Null;

        union {
            int32_t iVal;
            double  fVal;
            bool    bVal;
            const char* name;
            PSString* str;
            PSArray* arr;
            PSDictionary* dict;
            PSOperator* op;
        } data;

        static PSObject fromInt(int32_t v) {
            PSObject o;
            o.type = PSObjectType::Int;
            o.data.iVal = v;
            return o;
        }

        static PSObject fromReal(double v) {
            PSObject o;
            o.type = PSObjectType::Real;
            o.data.fVal = v;
            return o;
        }

        static PSObject fromBool(bool v) {
            PSObject o;
            o.type = PSObjectType::Bool;
            o.data.bVal = v;
            return o;
        }

        static PSObject fromName(const char* n) {
            PSObject o;
            o.type = PSObjectType::Name;
            o.data.name = n;
            return o;
        }

        static PSObject fromName(const waavs::ByteSpan& span) {
            PSObject o;
            o.type = PSObjectType::Name;

            // Allocate and null-terminate
            char* buf = new char[span.size() + 1];
            std::memcpy(buf, span.data(), span.size());
            buf[span.size()] = '\0';

            o.data.name = buf;
            return o;
        }

        static PSObject fromString(PSString* s) {
            PSObject o;
            o.type = PSObjectType::String;
            o.data.str = s;
            return o;
        }

        static PSObject fromArray(PSArray* a) {
            PSObject o;
            o.type = PSObjectType::Array;
            o.data.arr = a;
            return o;
        }

        static PSObject fromOperator(PSOperator* f) {
            PSObject o;
            o.type = PSObjectType::Operator;
            o.data.op = f;
            return o;
        }

        static PSObject fromDictionary(PSDictionary* d) {
            PSObject o;
            o.type = PSObjectType::Dictionary;
            o.data.dict = d;
            return o;
		}

        static PSObject mark() {
            PSObject o;
            o.type = PSObjectType::Mark;
            return o;
        }

        static PSObject null() {
            return PSObject();
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
        std::unordered_map<std::string, PSObject> entries;

        bool put(const std::string& key, const PSObject& value) {
            entries[key] = value;
            return true;
        }

        bool get(const std::string& key, PSObject& out) const {
            auto it = entries.find(key);
            if (it == entries.end()) return false;
            out = it->second;
            return true;
        }

        bool contains(const std::string& key) const {
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
