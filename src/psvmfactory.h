// psvmfactory.h
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

		static inline bool registerExtensionOps(PSVirtualMachine* vm)
		{
            bool success = true;

			PSVMOps extOps;

			vm->interpret(extOps.op_code_max);
			vm->interpret(extOps.op_code_min);

			return success;
		}

		static inline bool registerEncodings(PSVirtualMachine* vm)
		{

			PSVMEncodings encodings;

			vm->interpret(encodings.standardEncodingPS);
			vm->interpret(encodings.expertEncodingPS);
			vm->interpret(encodings.isoLatin1EncodingPS);
			vm->interpret(encodings.macRomanEncodingPS);
			vm->interpret(encodings.symbolEncodingPS);
			vm->interpret(encodings.winAnsiEncodingPS);
			vm->interpret(encodings.zapfDingbatsEncodingPS);

			return true;
        }

		static inline bool registerResources(PSVirtualMachine* vm)
		{

			PSVMEncodings encodings;

			vm->interpret(encodings.fontMapPS);

			return true;
        }

		static inline bool registerCoreOps(PSVirtualMachine *vm) 
		{
			bool success = true;

			success &= vm->registerOps(getArrayOps());
			success &= vm->registerOps(getDictionaryStackOps());
			success &= vm->registerOps(getControlOps());
			success &= vm->registerOps(getDebugOps());
			success &= vm->registerOps(getDictionaryOps());
			success &= vm->registerOps(getLogicOps());
			success &= vm->registerOps(getMathOps());
			success &= vm->registerOps(getPolymorphOps());
			success &= vm->registerOps(getRelationalOps());
			success &= vm->registerOps(getStackOps());
			success &= vm->registerOps(getStringOps());
			success &= vm->registerOps(getMatrixOps());
			success &= vm->registerOps(getGraphicsOps());
			success &= vm->registerOps(getEnviroOps());
            success &= vm->registerOps(getFileOps());
            success &= vm->registerOps(getFontOps());
			success &= vm->registerOps(getResourceOperators());
			success &= vm->registerOps(getTextOps());
			success &= vm->registerOps(getPathOps());
			
			return success;
		}

		// Create a new PSVM instance
		static std::unique_ptr<PSVirtualMachine> createVM()
		{
			auto vm = std::make_unique<PSVirtualMachine>();
			
			// Register built-in operations
			if (!PSVMFactory::registerCoreOps(vm.get()))
				return nullptr
				;
			PSVMFactory::registerExtensionOps(vm.get());
			PSVMFactory::registerResources(vm.get());
			PSVMFactory::registerEncodings(vm.get());

			return vm;
		}

	};
}