#pragma once

#include <vector>
#include <utility>
#include "pscore.h"

namespace waavs 
{
    template<typename T>
    struct PSStack {
    protected:
        std::vector<T> _data;

    public:
        PSStack() {}

        size_t size() const {
            return _data.size();
        }

        bool clear() {
            _data.clear();
            return true;
        }

        bool empty() const {
            return _data.empty();
		}

        bool peek(T& out) const {
            if (_data.empty()) return false;
            out = _data.back();
            return true;
		}

        // make a bunch of convenience functions for pushing and popping
        bool push(const T& value) 
        {
            _data.push_back(value);
            return true;
        }

        
        template<typename... Args>
        bool pushn(Args&&... args) {
            (_data.push_back(std::forward<Args>(args)), ...);
            return true;
        }

        // convenience functions, assuming size() > 0
        // very risky, since we don't throw exceptions
        //T pop() {

        //    T value = std::move(_data.back());
        //    _data.pop_back();

        //    return value;
        //}

        bool pop(T& out)
        {
            if (_data.empty())
                return false;
            out = _data.back();
            _data.pop_back();
            return true;
        }

        bool popn(size_t n, std::vector<T>& out) 
        {
            if (n > _data.size())
                return false;
            out.resize(n);
            for (size_t i = 0; i < n; ++i) {
                out[n - 1 - i] = _data.back();
                _data.pop_back();
            }
            return true;
        }

        bool dup() {
            if (_data.empty()) return false;
            _data.push_back(_data.back());
            return true;
        }

        bool exch() {
            if (_data.size() < 2) return false;
            std::swap(_data[_data.size() - 1], _data[_data.size() - 2]);
            return true;
        }

        // Copying
        bool copy(size_t n) {
            if (n > _data.size()) 
                return false;
            _data.insert(_data.end(), _data.end() - n, _data.end());
            return true;
        }


        const T& top() const { return _data.back(); }

        bool top(T& out) const {
            if (_data.empty()) return false;
            out = _data.back();
            return true;
        }

        bool nth(size_t n, T& out) const {
            if (n >= _data.size()) return false;
            out = _data[_data.size() - 1 - n];
            return true;
        }

        bool roll(int count, int shift)
        {
            if (count <= 0 || (size_t)count > this->size())
                return false;

            shift = ((shift % count) + count) % count;  // normalize shift to [0, count)
            auto first = this->_data.end() - count;
            std::rotate(first, first + (count - shift), this->_data.end());
            return true;
        }


        typename std::vector<T>::const_iterator begin() const { return _data.begin(); }
        typename std::vector<T>::const_iterator end() const { return _data.end(); }
    };


    //=============================================================================
    // PSObjectStack is a stack specifically for PSObject types, which includes
    // various PostScript data types like integers, strings, arrays, etc.
    // It provides methods to manipulate the stack in a way that is specific to PostScript operations.
    //=============================================================================

    struct PSObjectStack : public PSStack<PSObject>
    {
        // Push a mark object onto the object stack
        bool mark() 
        {
            return push(PSObject::fromMark(PSMark()));
        }
        // Clear the object stack down to the most recent mark object
        bool clearToMark()
        {
            PSObject obj;
            while (!this->empty()) {
                if (!this->pop(obj))  // optional: you could assert here since !empty()
                    return false;
                if (obj.type == PSObjectType::Mark)
                    return true;  // Successfully cleared to the mark
            }
            return false;  // No mark found
        }

        // Count number of objects above the most recent mark (from top of stack downward)
        bool countToMark(int& out) const
        {
            out = 0;
            for (size_t i = size(); i-- > 0;) {
                const auto& obj = _data[i];
                if (obj.type == PSObjectType::Mark)
                    return true;
                ++out;
            }
            return false; // Mark not found
        }



        bool pushBool(bool value) { return push(PSObject::fromBool(value)); }
        bool pushInt(int32_t value) { return push(PSObject::fromInt(value)); }
        bool pushReal(double value) { return push(PSObject::fromReal(value)); }
        bool pushLiteralName(const PSName aname) { return push(PSObject::fromName(aname)); }
        bool pushExecName(const PSName aname) { return push(PSObject::fromExecName(aname)); }
        bool pushString(const PSString& str) { return push(PSObject::fromString(str)); }
        bool pushMatrix(const PSMatrix& mat) { return push(PSObject::fromMatrix(mat)); }
        bool pushArray(const PSArrayHandle value) { return push(PSObject::fromArray(value)); }
        bool pushProcedure(const PSArrayHandle value) { PSObject obj = PSObject::fromArray(value); obj.setExecutable(true); return push(obj); }
        bool pushDictionary(const PSDictionaryHandle value) { return push(PSObject::fromDictionary(value)); }
        bool pushFile(const PSFileHandle value) { return push(PSObject::fromFile(value)); }
        bool pushFontFace(const PSFontFaceHandle value) { return push(PSObject::fromFontFace(value)); }
        bool pushFont(const PSFontHandle value) { return push(PSObject::fromFont(value)); }
        bool pushOperator(PSOperator value) { return push(PSObject::fromOperator(value)); }
        bool pushMark(const PSMark& value) { return push(PSObject::fromMark(value)); }

        // Return unboxed values
        // perform basic type check, return false on failure
        bool popBool(bool& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isBool())
                return false;

            out = obj.asBool();
            return true;
        }

        // will coerce real to int, if equivalent
        bool popInt(int32_t& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isInt())
                return false;
            out = obj.asInt();
            return true;
        }

        // will coerce int to real if necessary
        bool popReal(double& out) {
            if (empty())
                return false;

            PSObject obj;
            if (!pop(obj))
                return false;
            
            if (!obj.isNumber())
                return false;
            
            out = obj.asReal();
            
            return true;
        }

        /*
        bool popName(PSName& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isName())
                return false;
            out = obj.asName();
            return true;
        }
        */

        bool blString(PSString& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isString())
                return false;
            out = obj.asString();
            return true;
        }

        bool popArray(PSArrayHandle& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isArray())
                return false;
            out = obj.asArray();
            return true;
        }

        bool popDictionary(PSDictionaryHandle& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isDictionary())
                return false;
            out = obj.asDictionary();
            return true;
        }

        bool popFile(PSFileHandle& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isFile())
                return false;
            out = obj.asFile();
            return true;
        }

        bool popFontFace(PSFontFaceHandle& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isFontFace())
                return false;
            out = obj.asFontFace();
            return true;
        }

        bool popFont(PSFontHandle& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isFont())
                return false;
            out = obj.asFont();
            return true;
        }

        bool popOperator(PSOperator& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isOperator())
                return false;
            out = obj.asOperator();
            return true;
        }

        bool popMark(PSMark& out) {
            PSObject obj;
            if (!pop(obj) || !obj.isMark())
                return false;
            out = obj.asMark();
            return true;
        }
	};

    /*
    struct PSExecutionStack : public PSStack<PSObject>
    {
        // Push a mark object onto the execution stack
        bool mark() 
        {
            return push(PSObject::fromMark(PSMark()));
        }

        // Clear the execution stack down to the most recent mark object
        bool clearToMark()
        {
            PSObject obj;
            while (!this->empty()) {
                if (!this->pop(obj))  // optional: you could assert here since !empty()
                    return false;
                if (obj.type == PSObjectType::Mark)
                    return true;  // Successfully cleared to the mark
            }
            return false;  // No mark found
        }
	};
    */
    /*
    struct PSOperandStack : public PSStack<PSObject>
    {
        bool mark() 
        {
            return push(PSObject::fromMark(PSMark()));
		}

        // cleartomark
        // Removes all objects from the operand stack down to and including
        // the most recent mark object.
        bool clearToMark()
        {
            PSObject obj;
            while (!this->empty()) {
                if (!this->pop(obj))  // optional: you could assert here since !empty()
                    return false;

                if (obj.type == PSObjectType::Mark)
                    return true;  // Successfully cleared to the mark
            }
            return false;  // No mark found
        }

        // Count number of objects above the most recent mark (from top of stack downward)
        bool countToMark(int& out) const
        {
            out = 0;
            for (size_t i = size(); i-- > 0;) {
                const auto& obj = _data[i];
                if (obj.type == PSObjectType::Mark)
                    return true;
                ++out;
            }
            return false; // Mark not found
        }


    };
    */
}
