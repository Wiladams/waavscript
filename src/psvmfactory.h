#pragma once

#include <memory>

#include "psvm.h"  
#include "ps_ops_dictstack.h"
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
#include "ps_ops_enviro.h"
#include "ps_ops_file.h"
#include "ps_ops_font.h"
#include "ps_ops_vm.h"
#include "ps_ops_resource.h"
#include "ps_ops_text.h"
#include "ps_ops_path.h"


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

		static inline void registerExtensionOps(PSVirtualMachine* vm)
		{
			PSVMOps extOps;

			//vm->interpret(extOps.op_code_max);
			//vm->interpret(extOps.op_code_min);
		}

		static inline void registerEncodings(PSVirtualMachine* vm)
		{
			PSVMEncodings encodings;

			vm->interpret(encodings.standardEncodingPS);
			vm->interpret(encodings.expertEncodingPS);
			vm->interpret(encodings.isoLatin1EncodingPS);
			vm->interpret(encodings.macRomanEncodingPS);
			vm->interpret(encodings.symbolEncodingPS);
			vm->interpret(encodings.winAnsiEncodingPS);
			vm->interpret(encodings.zapfDingbatsEncodingPS);
        }

		static inline void registerResources(PSVirtualMachine* vm)
		{
            //registerEncodings(vm);

			PSVMEncodings encodings;

			vm->interpret(encodings.fontMapPS);
        }

		static inline void registerCoreOps(PSVirtualMachine *vm) 
		{
			vm->registerOps(getArrayOps());
			vm->registerOps(getDictionaryStackOps());
			vm->registerOps(getControlOps());
			vm->registerOps(getDebugOps());
			vm->registerOps(getDictionaryOps());
			vm->registerOps(getLogicOps());
			vm->registerOps(getMathOps());
			vm->registerOps(getPolymorphOps());
			vm->registerOps(getRelationalOps());
			vm->registerOps(getStackOps());
			vm->registerOps(getStringOps());
			vm->registerOps(getMatrixOps());
			vm->registerOps(getGraphicsOps());
			vm->registerOps(getEnviroOps());
            vm->registerOps(getFileOps());
            vm->registerOps(getFontOps());
			vm->registerOps(getResourceOperators());
			vm->registerOps(getTextOps());
			vm->registerOps(getPathOps());
		}

		// Create a new PSVM instance
		static std::unique_ptr<PSVirtualMachine> createVM()
		{
			auto vm = std::make_unique<PSVirtualMachine>();
			
			// Register built-in operations
			PSVMFactory::registerCoreOps(vm.get());
			PSVMFactory::registerExtensionOps(vm.get());
			PSVMFactory::registerResources(vm.get());

			return vm;
		}

	};
}