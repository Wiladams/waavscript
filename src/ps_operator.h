#pragma once

#include "pscore.h"

namespace waavs {

    // PSOperatorSignature
    // Represents a signature for a PostScript operator, including its name and argument types.
    // In some cases, the signature is either variadic, or dependent on the first
    /// argument on the stack.  In those cases, it's up to the operator implementation 
    // to figure out which operators are valid, and what to pull off the stack.
    // In those cases, the signature might just be a single "*"
    // this case is different than when there are no operands expected to come off the stack
    // in those cases, the typeString should be empty.  Or better yet, the signature should
    // indicate 'invalid' with a type signature of "?"
    struct PSOperatorSignature
    {
        PSName name;       // Name of the operator
        PSObjectType kinds[8];  // Types of arguments
        uint8_t arity;          // Number of arguments

        PSOperatorSignature(const PSName & opName, const char* typeString)
        {
            name = opName;
            arity = 0;

            // Assign argument types
            while (*typeString && arity < 8) {
                kinds[arity++] = static_cast<PSObjectType>(*typeString++);
            }
        }
    };

    struct PSOperatorArgs {
    private:
        const PSOperatorSignature& fSignature;
        PSObject fValues[8];   // fixed-size array
        size_t fCount;

    public:
        bool isValid() const { return fCount == fSignature.arity; }

        PSObject& operator[](size_t i) { return fValues[i]; }
        const PSObject& operator[](size_t i) const { return fValues[i]; }

        const PSName & operatorName() const { return fSignature.name; }
        PSObjectType operandType(size_t i) const { return (i < fCount) ? fSignature.kinds[i] : PSObjectType::Invalid; }
    };
}