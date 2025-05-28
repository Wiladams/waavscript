#pragma once

#include <iostream>

#include "pscore.h"
#include "psvm.h"

namespace waavs {
    inline bool ensureExecutableProcedure(PSObject& obj) {
        if (obj.isExecutable()) return true;

        if (obj.isArray() && obj.asArray()->isProcedure()) {
            obj.setExecutable(true); // canonicalize in-place
            return true;
        }

        return false;
    }

    inline bool ensureExecutableProcedureOrError(PSVirtualMachine& vm, PSObject& obj) {
        if (!ensureExecutableProcedure(obj))
            return vm.error("typecheck: expected executable procedure");
        return true;
    }



    static const PSOperatorFuncMap controlOps = {

        { "exec", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return vm.error("stackunderflow");

            PSObject obj;
            s.pop(obj);

        if (!obj.isExecutable()) {
            if (obj.isArray() && obj.asArray()->isProcedure()) {
                // Canonicalize into an executable object
                PSObject execObj;
                execObj.resetFromArray(obj.asArray());
				execObj.setExecutable(true);
                obj = execObj;
            } else {
                return vm.error("typecheck: expected executable object or procedure array");
            }
        }

            vm.execStack().push(obj);
            return vm.run();
        }},

        { "if", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2)
                return vm.error("stackunderflow");

            PSObject proc, cond;
            s.pop(proc);
            s.pop(cond);

            if (!cond.isBool())
                return vm.error("typecheck: expected boolean");

            if (cond.asBool()) {
                if (!ensureExecutableProcedureOrError(vm, proc)) return false;
                vm.execStack().push(proc);
                return vm.run();
            }

            return true;
        }},


        { "ifelse", [](PSVirtualMachine& vm) -> bool {
    auto& s = vm.opStack();
    if (s.size() < 3)
        return vm.error("stackunderflow");

    PSObject procFalse, procTrue, cond;
    s.pop(procFalse);
    s.pop(procTrue);
    s.pop(cond);

    if (!cond.isBool())
        return vm.error("typecheck: expected boolean");

    // Canonicalize true branch
    if (!procTrue.isExecutable()) {
        if (procTrue.isArray() && procTrue.asArray()->isProcedure()) {
            procTrue.setExecutable(true);
        }
 else {
  return vm.error("typecheck: expected procedure for true branch");
}
}

    // Canonicalize false branch
    if (!procFalse.isExecutable()) {
        if (procFalse.isArray() && procFalse.asArray()->isProcedure()) {
            procFalse.setExecutable(true);
        }
 else {
  return vm.error("typecheck: expected procedure for false branch");
}
}

    // Choose and execute
    vm.execStack().push(cond.asBool() ? procTrue : procFalse);
    return vm.run();
}},




        { "repeat", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 2)
                return vm.error("stackunderflow");

            PSObject proc, count;
            s.pop(proc);
            s.pop(count);

            if (!count.isInt())
                return vm.error("typecheck: expected integer");

            if (!ensureExecutableProcedureOrError(vm, proc))
                return false;

            int n = count.asInt();
            for (int i = 0; i < n; ++i) {
                vm.execStack().push(proc);
                if (!vm.run()) return false;

                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }
            }

            return true;
        }},


        { "loop", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 1)
                return vm.error("stackunderflow");

            PSObject proc;
            s.pop(proc);

            if (!ensureExecutableProcedureOrError(vm, proc))
                return false;

            while (true) {
                vm.execStack().push(proc);
                if (!vm.run()) return false;

                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }
            }

            return true;
        } },



        { "exit", [](PSVirtualMachine& vm) -> bool {
            vm.exit();
            return true;
        }},

        { "for", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.size() < 4)
                return vm.error("stackunderflow");

            PSObject proc, limit, increment, initial;
            s.pop(proc);
            s.pop(limit);
            s.pop(increment);
            s.pop(initial);

            if (!initial.isNumber() || !increment.isNumber() || !limit.isNumber())
                return vm.error("typecheck: expected numbers");

            if (!proc.isExecutable()) {
                if (proc.isArray() && proc.asArray()->isProcedure()) {
                    proc.setExecutable(true);
                } else {
                    return vm.error("typecheck: expected procedure");
                }
            }

            double i = initial.asReal();
            double inc = increment.asReal();
            double lim = limit.asReal();

            while ((inc > 0 && i <= lim) || (inc < 0 && i >= lim)) {
                vm.opStack().push(PSObject::fromReal(i));
                vm.execStack().push(proc);
                if (!vm.run()) return false;
                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }
                i += inc;
            }

            return true;
        } },

        { "forall", [](PSVirtualMachine& vm) -> bool {
           auto& s = vm.opStack();
           if (s.size() < 2)
               return vm.error("stackunderflow");

           PSObject proc, container;
           s.pop(proc);
           s.pop(container);

           if (!ensureExecutableProcedureOrError(vm, proc))
               return false;

           if (container.isArray()) {
               auto* arr = container.asArray();
               for (const auto& elem : arr->elements) {
                   vm.opStack().push(elem);
                   vm.execStack().push(proc);
                   if (!vm.run()) return false;
                   if (vm.isExitRequested()) {
                       vm.clearExitRequest();
                       break;
                   }
               }
               return true;
           }

           if (container.isString()) {
               auto* str = container.asString();
               for (size_t i = 0; i < str->size(); ++i) {
                   vm.opStack().push(PSObject::fromInt(str->get(i)));
                   vm.execStack().push(proc);
                   if (!vm.run()) return false;
                   if (vm.isExitRequested()) {
                       vm.clearExitRequest();
                       break;
                   }
               }
               return true;
           }

           return vm.error("typecheck: container not supported by forall");
        } },



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

            // Save prior stop state (if you want nested 'stopped' to work cleanly)
            bool prevStop = vm.isStopRequested();

            // Clear current stop flag before running proc
            vm.clearStopRequest();

            // Execute the procedure
            vm.execArray(proc.asArray());

            // Determine if a stop occurred (flag will already be cleared inside run())
            bool wasStopped = !vm.isStopRequested() && prevStop != vm.isStopRequested();

            // Restore previous stop state
            if (prevStop)
                vm.stop(); // Reassert previous state if it was set

            // Push result
            s.push(PSObject::fromBool(wasStopped));
            return true;
        } }

    };

} // namespace waavs
