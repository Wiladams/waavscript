#pragma once


#include "psvm.h"
#include "ps_lexer.h"
#include "ps_scanner.h"


namespace waavs 
{
	// The PSInterpreter takes a strea of tokens and runs them through the PSVirtualMachine.
    // This is the main entry point when you're running a PS script
    // or running an interactive interface.
    struct PSInterpreter 
    {
    private:
        PSVirtualMachine& fVM;

    public:
        PSInterpreter(PSVirtualMachine& vm)
            : fVM(vm) 
        {
        }

        /*
            enum struct PSObjectType : char {
		Null    = 'z'      // Null type, represents a null object
        ,Int     = 'i'      // Integer type, represents a 32-bit signed integer
        ,Real    = 'f'      // Real type, represents a double-precision floating-point number
		,Bool    = 'b'      // Boolean type, represents a true or false value
		,Name    = 'n'      // Name type, represents an interned name (string)
		,String = 's'      // String type, represents a PSString object
        ,Array   = 'a'
        ,Dictionary = 'd'
        ,Operator    = 'O'
        ,Mark    = 'm'
        ,Invalid = '?'      // Invalid type, used for uninitialized objects
		,Any = '*'          // Any type, used for generic operations
    };
        */
        
        /*
        // Let's try a brute force approach
        bool interpret(PSTokenGenerator& tokGen)
        {
            PSObject obj;

            while (parseObject(tokGen, obj))
            {
                switch (obj.type)
                {
                case PSObjectType::Int:
                case PSObjectType::Real:
                case PSObjectType::Bool:
                case PSObjectType::String:
                case PSObjectType::Array:
                case PSObjectType::Dictionary:
                    fVM.opStack().push(obj);
                    break;

				case PSObjectType::Name:
                    if (obj.isExecutable())
                        fVM.executeName(obj.asName());
                    else
						fVM.opStack().push(obj);
                    break;

                default:
					return fVM.error("Error: interpreter::interpret ==> Unsupported object type encountered: " );
                }
            }
        }
        */
        
        
        bool interpret(PSTokenGenerator &tokGen) 
        {
            PSObject obj;

            while (parseObject(tokGen, obj))
            {
                if (obj.isExecutable()) {
                    // push to execution stack then run
                    fVM.execStack().push(obj);
                    fVM.run();

                    if (fVM.isExitRequested()) {
                        break;
                    }

                    if (fVM.isStopRequested()) {
                        fVM.clearStopRequest();
                        break;
                    }
                }
                else {
                    fVM.opStack().push(obj);
                }
            }

            return true;
        }
        

        bool interpret(const OctetCursor input) {
            PSTokenGenerator tokGen(input);
        
			return interpret(tokGen);
        }

    };
}