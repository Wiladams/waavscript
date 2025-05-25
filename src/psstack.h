#pragma once

#pragma once
#include <vector>
#include <utility>

namespace waavs {
    template<typename T>
    struct PSStack {
    private:
        std::vector<T> _data;

    public:
        PSStack() {}

        size_t length() const {
            return _data.size();
        }

        void clear() {
            _data.clear();
        }

        bool push(const T& value) {
            _data.push_back(value);
            return true;
        }

        template<typename... Args>
        bool pushn(Args&&... args) {
            (_data.push_back(std::forward<Args>(args)), ...);
            return true;
        }

        bool pop(T& out) {
            if (_data.empty())
                return false;
            out = _data.back();
            _data.pop_back();
            return true;
        }

        bool popn(size_t n, std::vector<T>& out) {
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

        bool copy(size_t n) {
            if (n > _data.size()) return false;
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

}
