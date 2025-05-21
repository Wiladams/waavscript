#pragma once

#include "pscore.h"

namespace waavsps {

    // dup: (x ? x x) — duplicate top item
    inline bool op_dup(waavsps::PSVirtualMachine& vm) {
        if (vm.operandStack.empty()) return false;
        vm.operandStack.push_back(vm.operandStack.back());
        return true;
    }


    // pop: (x ? ) — remove top item from the stack
    inline bool op_pop(waavsps::PSVirtualMachine& vm) {
        if (vm.operandStack.empty()) return false;
        vm.operandStack.pop_back();
        return true;
    }

    // exch: (a b ? b a) — exchange top two items
    inline bool op_exch(waavsps::PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;
        std::swap(s[s.size() - 1], s[s.size() - 2]);
        return true;
    }



    // index: (n x? ... x? ? x? ... x? x?) — duplicate nth-from-top item
    inline bool op_index(waavsps::PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.empty()) return false;

        waavsps::PSObject countObj = s.back(); s.pop_back();
        if (countObj.type != waavsps::PSObjectType::Int) return false;

        int n = countObj.data.iVal;
        if (n < 0 || s.size() <= static_cast<size_t>(n)) return false;

        s.push_back(s[s.size() - 1 - n]);
        return true;
    }

    // roll: (n j x? ... x? ? x_{(1+j)%n} ... x?) — rotate top n items by j
    inline bool op_roll(waavsps::PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        if (s.size() < 2) return false;

        waavsps::PSObject jObj = s.back(); s.pop_back();
        waavsps::PSObject nObj = s.back(); s.pop_back();

        if (jObj.type != waavsps::PSObjectType::Int || nObj.type != waavsps::PSObjectType::Int) return false;

        int n = nObj.data.iVal;
        int j = jObj.data.iVal;
        if (n < 0 || s.size() < static_cast<size_t>(n)) return false;

        std::vector<waavsps::PSObject> temp(s.end() - n, s.end());
        s.erase(s.end() - n, s.end());

        int actualJ = ((j % n) + n) % n;
        std::rotate(temp.rbegin(), temp.rbegin() + actualJ, temp.rend());

        s.insert(s.end(), temp.begin(), temp.end());
        return true;
    }

    // clear: (x? x? ... x? ? ) — remove all items from the stack
    inline bool op_clear(waavsps::PSVirtualMachine& vm) {
        vm.operandStack.clear();
        return true;
    }

    // count: (? n) — push number of items currently on the stack
    inline bool op_count(waavsps::PSVirtualMachine& vm) {
        int count = static_cast<int>(vm.operandStack.size());
        vm.operandStack.push_back(waavsps::PSObject::fromInt(count));
        return true;
    }

    // mark: (? mark) — push a mark object onto the stack
    inline bool op_mark(waavsps::PSVirtualMachine& vm) {
        vm.operandStack.push_back(waavsps::PSObject::mark());
        return true;
    }


    // cleartomark: (x? x? ... mark ? ) — remove items up to and including last mark
    inline bool op_cleartomark(waavsps::PSVirtualMachine& vm) {
        auto& s = vm.operandStack;
        while (!s.empty()) {
            if (s.back().type == waavsps::PSObjectType::Mark) {
                s.pop_back(); // remove the mark too
                return true;
            }
            s.pop_back();
        }
        return false; // no mark found
    }

    // counttomark: (x? x? ... mark ? x? x? ... mark n) — push number of items above top mark
    inline bool op_counttomark(waavsps::PSVirtualMachine& vm) {
        int count = 0;
        for (auto it = vm.operandStack.rbegin(); it != vm.operandStack.rend(); ++it) {
            if (it->type == waavsps::PSObjectType::Mark) {
                vm.operandStack.push_back(waavsps::PSObject::fromInt(count));
                return true;
            }
            ++count;
        }
        return false; // no mark found
    }






}