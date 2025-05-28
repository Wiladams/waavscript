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
        T& pop() {
            //if (_data.empty())
            //  return T(); // throw std::out_of_range("Stack is empty");
            T& value = _data.back();
            _data.pop_back();
            return value;
        }

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

        bool roll(int n, int j) {
            if ((size_t)n > _data.size()) return false;
            if (n <= 0 || j == 0) return true;

            j = ((j % n) + n) % n; // normalize
            auto first = _data.end() - n;
            std::rotate(first, first + (n - j), _data.end());
            return true;
        }

        T& top() { return _data.back(); }
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





        typename std::vector<T>::const_iterator begin() const { return _data.begin(); }
        typename std::vector<T>::const_iterator end() const { return _data.end(); }
    };


    struct PSOperandStack : public PSStack<PSObject>
    {
        bool mark() 
        {
            return push(PSObject::fromMark());
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

        bool roll(int count, int shift) 
        {
            if (count <= 0 || (size_t)count > this->size())
                return false;

            shift = ((shift % count) + count) % count;  // normalize shift to [0, count)
            auto first = this->_data.end() - count;
            std::rotate(first, first + (count - shift), this->_data.end());
            return true;
        }
    };

}
