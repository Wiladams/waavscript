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

		static inline void registerStackOps(PSVirtualMachine* vm) {
			vm->registerBuiltin("pop", op_pop);          // (x → )
			vm->registerBuiltin("dup", op_dup);          // (x → x x)
			vm->registerBuiltin("exch", op_exch);         // (a b → b a)
			vm->registerBuiltin("copy", op_copy);         // (n x₁...xₙ → x₁...xₙ x₁...xₙ)
			vm->registerBuiltin("index", op_index);        // (n x₀...xₙ → x₀...xₙ xₙ)
			vm->registerBuiltin("roll", op_roll);         // (n j x₁...xₙ → x_{(1+j)%n}...)
			vm->registerBuiltin("clear", op_clear);        // (x₁ x₂ ... xₙ → )
			vm->registerBuiltin("count", op_count);        // (→ n)
			vm->registerBuiltin("mark", op_mark);         // (→ mark)
			vm->registerBuiltin("cleartomark", op_cleartomark);  // (x₁ x₂ ... mark → )
			vm->registerBuiltin("counttomark", op_counttomark);  // (x₁ x₂ ... mark → x₁ x₂ ... mark n)
		}

		static inline void registerDictionaryOps(PSVirtualMachine *vm) 
		{
			vm->registerBuiltin("known", op_known);					// checks if a key is in the dict stack
			
			vm->registerBuiltin("def", op_def);						// stores into top of dict stack
			vm->registerBuiltin("load", op_load);
			vm->registerBuiltin("where", op_where);					// looks up the name in the dict stack and returns the dict it was found in
			vm->registerBuiltin("currentdict", op_currentdict);		// returns top of dict stack
			vm->registerBuiltin("dict", op_dict);
			vm->registerBuiltin("begin", op_begin);
			vm->registerBuiltin("end", op_end);
			vm->registerBuiltin("maxlength", op_maxlength);
			vm->registerBuiltin("countdictstack", op_countdictstack);	// returns the number of dicts in the stack
		}

		static inline void registerArrayOps(PSVirtualMachine *vm) 
		{
			vm->registerOps(arrayOps);
			//vm->registerBuiltin("array", op_array);					// creates an array of the given size
			//vm->registerBuiltin("aload", op_aload);
			//vm->registerBuiltin("astore", op_astore);
			//vm->registerBuiltin("getinterval", op_getinterval);
			//vm->registerBuiltin("putinterval", op_putinterval);
		}


		static inline void registerPolymorphicOps(PSVirtualMachine* vm) {
			// Core polymorphic operators
			vm->registerBuiltin("get", op_get);
			vm->registerBuiltin("put", op_put);
			vm->registerBuiltin("length", op_length);
			vm->registerBuiltin("copy", op_copy);
			vm->registerBuiltin("forall", op_forall);
			vm->registerBuiltin("eq", op_assign);
			vm->registerBuiltin("ne", op_ne);
			vm->registerBuiltin("type", op_type);
			vm->registerBuiltin("==", op_print);
			vm->registerBuiltin("cvs", op_cvs);
		}

		// Math operations
		static inline void registerMathOps(PSVirtualMachine* vm) 
		{

			// binary operations
			vm->registerBuiltin("add", op_add);          // (x y -> x + y)
			vm->registerBuiltin("sub", op_sub);          // (x y -> x - y)
			vm->registerBuiltin("mul", op_mul);          // (x y -> x * y)
			vm->registerBuiltin("div", op_div);          // (x y -> x / y)
			vm->registerBuiltin("mod", op_mod);          // (x y -> x % y)

			// unary operations
			vm->registerBuiltin("neg", op_neg);          // (x -> -x)
			vm->registerBuiltin("abs", op_abs);          // (x -> |x|)
			vm->registerBuiltin("sqrt", op_sqrt);        // (x -> √x)

			// Trigonometric functions
			vm->registerBuiltin("sin", op_sin);
			vm->registerBuiltin("cos", op_cos);
			vm->registerBuiltin("atan", op_atan);

			// Exponential and logarithmic functions
			vm->registerBuiltin("exp", op_exp);
			vm->registerBuiltin("ln", op_ln);
			vm->registerBuiltin("log", op_log);

			// Random number generation
			vm->registerBuiltin("rand", op_rand);
			vm->registerBuiltin("srand", op_srand);
			vm->registerBuiltin("rrand", op_rrand);
		}

		static inline void registerRelationalOps(PSVirtualMachine* vm) {
			// eq, ne already registered in polymorphic ops
			vm->registerBuiltin("gt", op_gt);
			vm->registerBuiltin("lt", op_lt);
			vm->registerBuiltin("ge", op_ge);
			vm->registerBuiltin("le", op_le);
		}

		static inline void registerLogicalOps(PSVirtualMachine* vm) {
			vm->registerBuiltin("and", op_and);
			vm->registerBuiltin("or", op_or);
			vm->registerBuiltin("xor", op_xor);
			vm->registerBuiltin("not", op_not);
		}

		static inline void registerControlOps(PSVirtualMachine* vm) {
			vm->registerBuiltin("if", op_if);
			vm->registerBuiltin("ifelse", op_ifelse);
			vm->registerBuiltin("repeat", op_repeat);
			vm->registerBuiltin("loop", op_loop);
			vm->registerBuiltin("exit", op_exit);
			vm->registerBuiltin("for", op_for);
			vm->registerBuiltin("stop", op_stop);
			vm->registerBuiltin("stopped", op_stopped);
		}

		static inline void registerDebugOps(PSVirtualMachine* vm) {
			vm->registerBuiltin("==", op_eqeq);
			vm->registerBuiltin("=", op_eq);
			vm->registerBuiltin("stack", op_stack);
			vm->registerBuiltin("pstack", op_pstack);
			vm->registerBuiltin("print", op_print);
			vm->registerBuiltin("flush", op_flush);
			vm->registerBuiltin("errordict", op_errordict);
			vm->registerBuiltin("handleerror", op_handleerror);
		}

		static inline void registerCoreOps(PSVirtualMachine *vm) 
		{
			registerPolymorphicOps(vm);

			registerDictionaryOps(vm);
			registerStackOps(vm);
			registerArrayOps(vm);
			registerMathOps(vm); 
			registerRelationalOps(vm);
			registerLogicalOps(vm); 
			registerControlOps(vm);
			registerDebugOps(vm);
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