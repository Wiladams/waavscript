#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    static const PSOperatorFuncMap controlOps = {
        { "if", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;

            PSObject proc = s.back(); s.pop_back();
            PSObject cond = s.back(); s.pop_back();

            if (cond.type != PSObjectType::Bool || proc.type != PSObjectType::Array || !proc.data.arr->isExecutable())
                return false;

            if (cond.data.bVal)
                vm.execArray(proc.data.arr);

            return true;
        }},

        { "ifelse", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 3) return false;

            PSObject proc2 = s.back(); s.pop_back();
            PSObject proc1 = s.back(); s.pop_back();
            PSObject cond = s.back(); s.pop_back();

            if (cond.type != PSObjectType::Bool ||
                proc1.type != PSObjectType::Array || !proc1.data.arr->isExecutable() ||
                proc2.type != PSObjectType::Array || !proc2.data.arr->isExecutable())
                return false;

            vm.execArray(cond.data.bVal ? proc1.data.arr : proc2.data.arr);
            return true;
        }},

        { "repeat", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 2) return false;

            PSObject proc = s.back(); s.pop_back();
            PSObject count = s.back(); s.pop_back();

            if (count.type != PSObjectType::Int || proc.type != PSObjectType::Array || !proc.data.arr->isExecutable())
                return false;

            int n = count.data.iVal;
            for (int i = 0; i < n; ++i) {
                vm.execArray(proc.data.arr);
                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }
            }
            return true;
        }},

        { "loop", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.empty()) return false;

            PSObject proc = s.back(); s.pop_back();
            if (proc.type != PSObjectType::Array || !proc.data.arr->isExecutable()) return false;

            while (true) {
                vm.execArray(proc.data.arr);
                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }
            }
            return true;
        }},

        { "exit", [](PSVirtualMachine& vm) -> bool {
            vm.exit();
            return true;
        }},

        { "for", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.operandStack;
            if (s.size() < 4) return false;

            PSObject proc = s.back(); s.pop_back();
            PSObject limit = s.back(); s.pop_back();
            PSObject inc = s.back(); s.pop_back();
            PSObject init = s.back(); s.pop_back();

            if (!init.isNumber() || !inc.isNumber() || !limit.isNumber() ||
                proc.type != PSObjectType::Array || !proc.data.arr->isExecutable())
                return false;

            double i = init.asReal();
            double step = inc.asReal();
            double end = limit.asReal();

            if (step == 0.0) return false;

            if (step > 0) {
                for (; i <= end; i += step) {
                    s.push_back(PSObject::fromReal(i));
                    vm.execArray(proc.data.arr);
                    if (vm.isExitRequested()) {
                        vm.clearExitRequest();
                        break;
                    }
                }
            }
     else {
      for (; i >= end; i += step) {
          s.push_back(PSObject::fromReal(i));
          vm.execArray(proc.data.arr);
          if (vm.isExitRequested()) {
              vm.clearExitRequest();
              break;
          }
      }
  }
  return true;
}},

{ "stop", [](PSVirtualMachine& vm) -> bool {
    vm.stop();
    return true;
}},

{ "stopped", [](PSVirtualMachine& vm) -> bool {
    auto& s = vm.operandStack;
    if (s.empty()) return false;

    PSObject proc = s.back(); s.pop_back();
    if (proc.type != PSObjectType::Array || !proc.data.arr->isExecutable()) return false;

    bool previous = vm.stopRequested;
    vm.stopRequested = false;

    vm.execArray(proc.data.arr);
    bool wasStopped = vm.stopRequested;

    vm.stopRequested = previous;
    s.push_back(PSObject::fromBool(wasStopped));
    return true;
}}
    };

} // namespace waavs
