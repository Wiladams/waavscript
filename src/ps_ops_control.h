#pragma once
#include "pscore.h"

namespace waavsps {

	inline bool op_if(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 2) return false;

		PSObject proc = s.back(); s.pop_back();
		PSObject cond = s.back(); s.pop_back();

		if (cond.type != PSObjectType::Bool || proc.type != PSObjectType::Array || !proc.data.arr->isExecutable())
			return false;

		if (cond.data.bVal) {
			vm.execArray(proc.data.arr);
		}
		return true;
	}

	inline bool op_ifelse(PSVirtualMachine& vm) {
		auto& s = vm.operandStack;
		if (s.size() < 3) return false;

		PSObject proc2 = s.back(); s.pop_back();
		PSObject proc1 = s.back(); s.pop_back();
		PSObject cond = s.back(); s.pop_back();

		if (cond.type != PSObjectType::Bool ||
			proc1.type != PSObjectType::Array || !proc1.data.arr->isExecutable() ||
			proc2.type != PSObjectType::Array || !proc2.data.arr->isExecutable())
			return false;

		if (cond.data.bVal)
			vm.execArray(proc1.data.arr);
		else
			vm.execArray(proc2.data.arr);

		return true;
	}

	inline bool op_repeat(PSVirtualMachine& vm) {
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
	}

	inline bool op_loop(PSVirtualMachine& vm) {
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
	}

	inline bool op_exit(PSVirtualMachine& vm) {
		vm.exit();
		return true;
	}

	inline bool op_for(PSVirtualMachine& vm) {
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
				if (vm.exitRequested) {
					vm.exitRequested = false;
					break;
				}
			}
		}
		else {
			for (; i >= end; i += step) {
				s.push_back(PSObject::fromReal(i));
				vm.execArray(proc.data.arr);
				if (vm.exitRequested) {
					vm.exitRequested = false;
					break;
				}
			}
		}
		return true;
	}

	inline bool op_stop(PSVirtualMachine& vm) {
		vm.stop();
		return true;
	}

	inline bool op_stopped(PSVirtualMachine& vm) {
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
	}



}
