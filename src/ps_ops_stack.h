#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    static const PSOperatorFuncMap stackOps = {

        { "dup", [](PSVirtualMachine& vm) {
            if (vm.operandStack.empty()) return false;
            vm.operandStack.push_back(vm.operandStack.back());
            return true;
        }},

        { "pop", [](PSVirtualMachine& vm) {
            if (vm.operandStack.empty()) return false;
            vm.operandStack.pop_back();
            return true;
        }},

        { "exch", [](PSVirtualMachine& vm) {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;
            std::swap(s[s.size() - 1], s[s.size() - 2]);
            return true;
        }},

        { "index", [](PSVirtualMachine& vm) {
            auto& s = vm.operandStack;
            if (s.empty()) return false;
            PSObject countObj = s.back(); s.pop_back();
            if (countObj.type != PSObjectType::Int) return false;
            int n = countObj.data.iVal;
            if (n < 0 || s.size() <= static_cast<size_t>(n)) return false;
            s.push_back(s[s.size() - 1 - n]);
            return true;
        }},

        { "roll", [](PSVirtualMachine& vm) {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;
            PSObject jObj = s.back(); s.pop_back();
            PSObject nObj = s.back(); s.pop_back();
            if (jObj.type != PSObjectType::Int || nObj.type != PSObjectType::Int) return false;
            int n = nObj.data.iVal;
            int j = jObj.data.iVal;
            if (n < 0 || s.size() < static_cast<size_t>(n)) return false;

            std::vector<PSObject> temp(s.end() - n, s.end());
            s.erase(s.end() - n, s.end());

            int actualJ = ((j % n) + n) % n;
            std::rotate(temp.rbegin(), temp.rbegin() + actualJ, temp.rend());

            s.insert(s.end(), temp.begin(), temp.end());
            return true;
        }},

        { "clear", [](PSVirtualMachine& vm) {
            vm.operandStack.clear();
            return true;
        }},

        { "count", [](PSVirtualMachine& vm) {
            int count = static_cast<int>(vm.operandStack.size());
            vm.operandStack.push_back(PSObject::fromInt(count));
            return true;
        }},

        { "mark", [](PSVirtualMachine& vm) {
            vm.operandStack.push_back(PSObject::fromMark());
            return true;
        }},

        { "cleartomark", [](PSVirtualMachine& vm) {
            auto& s = vm.operandStack;
            while (!s.empty()) {
                if (s.back().type == PSObjectType::Mark) {
                    s.pop_back();
                    return true;
                }
                s.pop_back();
            }
            return false;
        }},

        { "counttomark", [](PSVirtualMachine& vm) {
            int count = 0;
            for (auto it = vm.operandStack.rbegin(); it != vm.operandStack.rend(); ++it) {
                if (it->type == PSObjectType::Mark) {
                    vm.operandStack.push_back(PSObject::fromInt(count));
                    return true;
                }
                ++count;
            }
            return false;
        }},
    };

} // namespace waavs
