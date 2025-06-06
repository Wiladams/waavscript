#pragma once

#include <memory>

#include "psvm.h"  
#include "ps_ops_polymorph.h"
#include "ps_ops_dictionary.h"
#include "ps_ops_array.h"
#include "ps_ops_math.h"
#include "ps_ops_stack.h"
#include "ps_ops_relational.h"
#include "ps_ops_logic.h"
#include "ps_ops_control.h"
#include "ps_ops_debug.h"
#include "ps_ops_string.h"
#include "ps_ops_matrix.h"
#include "ps_ops_graphics.h"

namespace waavs
{
	// We have this factory because there is a separation of the virtual machine
	// and the operations that come with it by default.  This factory allows
	// us to create a new virtual machine and register the built-in operations
	// 
	class PSVMFactory
	{
	public:
		PSVMFactory() = default;
		~PSVMFactory() = default;

		static inline void registerCoreOps(PSVirtualMachine *vm) 
		{
			vm->registerOps(getArrayOps());
			vm->registerOps(getControlOps());
			vm->registerOps(getDebugOps());
			vm->registerOps(getDictionaryOps());
			vm->registerOps(getLogicOps());
			vm->registerOps(mathOps);
			vm->registerOps(getPolymorphOps());
			vm->registerOps(relationalOps);
			vm->registerOps(stackOps);
			vm->registerOps(getStringOps());
			vm->registerOps(opsMatrix);
			vm->registerOps(graphicsOps);
		}

		// Create a new PSVM instance
		static std::unique_ptr<PSVirtualMachine> createVM()
		{
			auto vm = std::make_unique<PSVirtualMachine>();
			
			// Register built-in operations
			PSVMFactory::registerCoreOps(vm.get());

			return vm;
		}

	};
}