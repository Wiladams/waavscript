#pragma once

#include "pscore.h"
#include "psvm.h"

namespace waavs {

    static const PSOperatorFuncMap controlOps = {
        { "if", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2)
                return vm.error("stackunderflow");

            PSObject proc;
            PSObject cond;

            s.pop(proc);
            s.pop(cond);

            if (!cond.isBool())
                return vm.error("typecheck: expected boolean");
            if (!proc.isArray() || !proc.isExecutable())
                return vm.error("typecheck: expected executable array");

            if (cond.asBool()) {
                if (!vm.execArray(proc.asArray()))
                    return vm.error("execution failed in if");
            }

            return true;
        }},


        { "ifelse", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 3)
                return false;

            PSObject proc2;
            PSObject proc1;
            PSObject cond;

            s.pop(proc2);
            s.pop(proc1);
            s.pop(cond);

            if (!cond.isBool() ||
                !proc1.isArray() || !proc1.isExecutable() ||
                !proc2.isArray() || !proc2.isExecutable())
                return false;

            vm.execArray(cond.asBool() ? proc1.asArray() : proc2.asArray());
            return true;
        }},

        { "repeat", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2) return false;

            PSObject proc;
            PSObject count;

            s.pop(proc);
            s.pop(count);

            if (!count.isInt() || !proc.isArray() || !proc.isExecutable())
                return false;

            int n = count.asInt();
            for (int i = 0; i < n; ++i) {
                vm.execArray(proc.asArray());
                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }
            }
            return true;
        }},

        { "loop", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject proc;

            s.pop(proc);

            if (!proc.isArray() || !proc.isExecutable()) return false;

            while (true) {
                vm.execArray(proc.asArray());
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
            auto& s = vm.opStack();
            if (s.size() < 4) return false;

            PSObject proc;
            PSObject limit;
            PSObject inc;
            PSObject init;

            s.pop(proc);
            s.pop(limit);
            s.pop(inc);
            s.pop(init);

            if (!init.isNumber() || !inc.isNumber() || !limit.isNumber() ||
                !proc.isArray() || !proc.isExecutable())
                return false;

            double i = init.asReal();
            double step = inc.asReal();
            double end = limit.asReal();

            if (step == 0.0) return false;

            if (step > 0) {
                for (; i <= end; i += step) {
                    s.push(PSObject::fromReal(i));
                    vm.execArray(proc.asArray());
                    if (vm.isExitRequested()) {
                        vm.clearExitRequest();
                        break;
                    }
                }
            }
            else {
                for (; i >= end; i += step) {
                    s.push(PSObject::fromReal(i));
                    vm.execArray(proc.asArray());
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
            auto& s = vm.opStack();
            if (s.empty()) return false;

            PSObject proc;

            s.pop(proc);

            if (!proc.isArray() || !proc.isExecutable()) return false;

            bool previous = vm.stopRequested;
            vm.stopRequested = false;

            vm.execArray(proc.asArray());
            bool wasStopped = vm.stopRequested;

            vm.stopRequested = previous;
            s.push(PSObject::fromBool(wasStopped));
            return true;
        }}
    };

} // namespace waavs
