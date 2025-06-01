#pragma once

#include <iostream>

#include "pscore.h"
#include "psvm.h"

namespace waavs {




    static const PSOperatorFuncMap controlOps = {
        { "exec", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) 
                return vm.error("exec::stackunderflow");

            PSObject proc;
            s.pop(proc);

			return runArray(vm, proc);
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

            if (cond.asBool()) 
            {
				return runArray(vm, proc);
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

            // Choose and execute
            auto proc = cond.asBool() ? procTrue : procFalse;
            return runArray(vm, proc);
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

            int n = count.asInt();
            for (int i = 0; i < n; ++i) {
                if (!runArray(vm, proc))
                {
                    return vm.error("repeat:: runArray() failed");
                }

                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }

                if (vm.isStopRequested()) {
                    vm.clearStopRequest();
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

            while (true) {
                //vm.execStack().push(proc);
                if (!pushProcedureToExecStack(vm, proc)) return false;
                if (!vm.run()) return false;

                if (!runArray(vm, proc))
					return false;

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

            //ensureExecutableOrError(vm, proc);

            double i = initial.asReal();
            double inc = increment.asReal();
            double lim = limit.asReal();

            while ((inc > 0 && i <= lim) || (inc < 0 && i >= lim)) {
                vm.opStack().push(PSObject::fromReal(i));
                
                if (!runArray(vm, proc))
                    return false;
                
                if (vm.isExitRequested()) {
                    vm.clearExitRequest();
                    break;
                }
                i += inc;
            }

            return true;
        } },





        { "stop", [](PSVirtualMachine& vm) -> bool {
            vm.stop();
            return true;
        }},

        { "stopped", [](PSVirtualMachine& vm) -> bool {
            auto& s = vm.opStack();
            if (s.empty()) return vm.error("stackunderflow");

            PSObject proc;
            s.pop(proc);

            if (!proc.isArray()) return vm.error("typecheck");

            auto arr = proc.asArray();
            if (!arr || !arr->isProcedure()) return vm.error("typecheck");

            // Save current stop flag
            bool prevStop = vm.isStopRequested();

            // Clear stop for this protected block
            vm.clearStopRequest();

            // Push the procedure to execStack
			if (!runArray(vm, proc)) return false;


            // Capture whether a stop occurred
            bool stopOccurred = vm.isStopRequested();

            // Restore previous stop state (PostScript allows nested `stopped`)
            if (prevStop)
                vm.stop();
            else
                vm.clearStopRequest();  // fully clear it if it wasn’t previously set

            // Push result: true if `stop` was called, false otherwise
            s.push(PSObject::fromBool(stopOccurred));
            return true;
        } }


    };

} // namespace waavs
