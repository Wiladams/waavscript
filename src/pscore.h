#pragma once

#include <cstdint>
#include <cstring>

#include <string>
#include <string_view>
#include <unordered_map>
#include <map>

#include <vector>
#include <functional>
#include <memory>
#include <variant>

#include "ocspan.h"
#include "psstring.h"
#include "psmatrix.h"



// global name table for interned strings.  Anything that is to be a name used
// in a table as a key, should be interned here.
namespace waavs {

    struct PSNameTable {
    private:
        std::map<std::string, const char*> pool;

        const char* intern(std::string_view sv) {
            // Attempt to insert a new entry; try_emplace does nothing if key already exists
            auto [it, inserted] = pool.try_emplace(std::string(sv), nullptr);

            if (inserted) {
                // Assign the stable c_str() from the key (std::string stored in the map)
                it->second = it->first.c_str();
            }

            return it->second;
        }

        const char* intern(const OctetCursor& span) { return intern(std::string_view(reinterpret_cast<const char*>(span.data()), span.size())); }
        const char* intern(const char* cstr) { return intern(std::string_view(cstr)); }

        static PSNameTable* getTable() {
            static std::unique_ptr<PSNameTable> gTable = std::make_unique<PSNameTable>();
            return gTable.get();
        }

    public:
        // NOTE::
        // These should only be used by things inside pscore.h
        // there might ba couple of exceptions, like the cvn operator
        // but for the most part, sting interning should be an internal thing
        static const char* INTERN(const OctetCursor& span) { return getTable()->intern(span); }
        static const char* INTERN(const char* cstr) { return getTable()->intern(cstr); }
    };
}

namespace waavs {
    // --------------------
    // Forward Declarations
    // --------------------
    struct PSObject;
    struct PSString;
    struct PSArray;
    struct PSOperator;
    struct PSDictionary;
    struct PSVirtualMachine;



    // Handle aliases for clarity
    using PSArrayHandle = std::shared_ptr<PSArray>;
    using PSDictionaryHandle = std::shared_ptr<PSDictionary>;

    // --------------------
    // PSOperator
    // --------------------
    // These definitions are used for builtin operators that are known at compile time
    //using PSOperatorFunc = std::function<bool(PSVirtualMachine&)>;
    using PSOperatorFunc = bool(*)(PSVirtualMachine&);
    using PSOperatorFuncMap = std::unordered_map<const char*, PSOperatorFunc>;

    struct PSOperator {
        const char* name = nullptr;       // Always interned and stable
        PSOperatorFunc func = nullptr;

        PSOperator() = default;

        PSOperator(const char* opName, PSOperatorFunc f) noexcept
            : name(nullptr), func(f) {
			name = PSNameTable::INTERN(opName);
        }

		// an operator() overload to call the function
        bool operator()(PSVirtualMachine& vm) {
            if (func) {
                return func(vm);
            }
            return false; // or throw an error
        }

        // Check if the operator is valid
		bool isValid() const noexcept { 
            return (name != nullptr) && (func != nullptr); 
        }


    };


    // --------------------
// PSObject Type Enum
// --------------------
    enum struct PSObjectType : char {
        Null = 'z'      // Null type, represents a null object
        , Int = 'i'      // Integer type, represents a 32-bit signed integer
        , Real = 'f'      // Real type, represents a double-precision floating-point number
        , Bool = 'b'      // Boolean type, represents a true or false value
        , Name = 'n'      // Name type, represents an interned name (string)
        , String = 's'      // String type, represents a PSString object
        , Array = 'a'
        , Dictionary = 'd'
        , Operator = 'O'
        , Mark = 'm'
        , Matrix = 'x'
        , Invalid = '?'      // Invalid type, used for uninitialized objects
        , Any = '*'          // Any type, used for generic operations
    };


struct PSObject {
private:
    using Variant = std::variant<
        std::monostate,                      // Null
        int32_t,                             // Int
        double,                              // Real
        bool,                                // Bool
        const char*,                         // Name (interned)
        PSOperator,                   // Operator
		PSMatrix,                            // Matrix
        PSString,                      // String
        PSArrayHandle,                       // Array
        PSDictionaryHandle,                  // Dictionary
        std::nullptr_t                       // Mark
    >;

    bool fIsExec = false;
    Variant fValue;

public:
    PSObjectType type = PSObjectType::Null;

    // Reset state
    bool reset() {
        fValue = std::monostate{};
        type = PSObjectType::Null;
        fIsExec = false;
        return true;
    }

    bool resetFromInt(int32_t v) {
        reset(); type = PSObjectType::Int; fValue = v; return true;
    }
    bool resetFromReal(double v) {
        reset(); type = PSObjectType::Real; fValue = v; return true;
    }
    bool resetFromBool(bool v) {
        reset(); type = PSObjectType::Bool; fValue = v; return true;
    }

    bool resetFromInternedName(const char* interned) {
        reset(); type = PSObjectType::Name; fValue =interned; return true;
	}
    bool resetFromName(const char* cstr) {
		return resetFromInternedName(PSNameTable::INTERN(cstr));
    }
    bool resetFromName(const OctetCursor& oc) {
        return resetFromInternedName(PSNameTable::INTERN(oc));
    }
    bool resetFromString(PSString s) {
        reset(); type = PSObjectType::String; fValue = s; return true;
    }
    bool resetFromArray(PSArrayHandle a) {
        reset(); type = PSObjectType::Array; fValue = a; return true;
    }
    bool resetFromDictionary(PSDictionaryHandle d) {
        reset(); type = PSObjectType::Dictionary; fValue = d; return true;
    }
    bool resetFromOperator(const PSOperator& f) {
        reset(); type = PSObjectType::Operator; fValue = f; fIsExec = true; return true;
    }
    bool resetFromMatrix(const PSMatrix& m) {
        reset(); type = PSObjectType::Matrix; fValue = m; return true;
	}
    bool resetFromMark() {
        reset(); type = PSObjectType::Mark; fValue = nullptr; return true;
    }


    // Static constructors
    static PSObject fromInt(int32_t v) { PSObject o; o.resetFromInt(v); return o; }
    static PSObject fromReal(double v) { PSObject o; o.resetFromReal(v); return o; }
    static PSObject fromBool(bool v) { PSObject o; o.resetFromBool(v); return o; }
    static PSObject fromName(const char* n) { PSObject o; o.resetFromName(n); return o; }
	static PSObject fromName(const OctetCursor& oc) { PSObject o; o.resetFromName(oc); return o; }
	static PSObject fromInternedName(const char* interned) { PSObject o; o.resetFromInternedName(interned); return o; }
    static PSObject fromString(PSString s) { PSObject o; o.resetFromString(s); return o; }
    static PSObject fromArray(PSArrayHandle a) { PSObject o; o.resetFromArray(a); return o; }
    static PSObject fromDictionary(PSDictionaryHandle d) { PSObject o; o.resetFromDictionary(d); return o; }
    static PSObject fromOperator(const PSOperator& f) { PSObject o; o.resetFromOperator(f); return o; }
	static PSObject fromMatrix(const PSMatrix& m) { PSObject o; o.resetFromMatrix(m); return o; }
    static PSObject fromMark() { PSObject o; o.resetFromMark(); return o; }


    // Accessors using std::get
    template<typename T>
    constexpr bool get(T& out) const {
        if (const T* ptr = std::get_if<T>(&fValue)) {
            out = *ptr;
            return true;
        }
        return false;
    }

    template<typename T>
    constexpr T as() const {
        return std::get<T>(fValue);
    }

    template<typename T>
    constexpr T* try_as() {
        return std::get_if<T>(&fValue);
    }

    // Legacy-style accessors
    int asInt() const { return as<int32_t>(); }
    double asReal() const { return (type == PSObjectType::Int) ? static_cast<double>(as<int32_t>()) : as<double>(); }
    bool asBool() const { return as<bool>(); }
    const char* asName() const { return as<const char*>(); }
    PSString asString() const { return as<PSString>(); }
    PSArrayHandle asArray() const { return as<PSArrayHandle>(); }
    PSDictionaryHandle asDictionary() const { return as<PSDictionaryHandle>(); }
    PSOperator asOperator() const { return as<PSOperator>(); }
	PSMatrix asMatrix() const { return as<PSMatrix>(); }

    // Type checks
    bool isExecutable() const { return fIsExec; }
    void setExecutable(bool flag) { fIsExec = flag; }

    inline constexpr bool is(PSObjectType t) const { return (type == t) || (t == PSObjectType::Any); }
    inline bool isNumber() const { return isInt() || isReal(); }
    inline bool isInt() const { return type == PSObjectType::Int; }
    inline bool isReal() const { return type == PSObjectType::Real; }
    inline bool isBool() const { return type == PSObjectType::Bool; }
    inline bool isName() const { return type == PSObjectType::Name; }
    inline bool isLiteralName() const { return isName() && !fIsExec; }
    inline bool isString() const { return type == PSObjectType::String; }
    inline bool isArray() const { return type == PSObjectType::Array; }
    inline bool isDictionary() const { return is(PSObjectType::Dictionary); }
    inline bool isOperator() const { return is(PSObjectType::Operator); }
    inline bool isMark() const { return type == PSObjectType::Mark; }
    inline bool isMatrix() const { return is(PSObjectType::Matrix); }
    inline bool isNull() const { return type == PSObjectType::Null; }

    // Signature helper
    char typeChar() const { return static_cast<char>(type); }
};


    struct PSOperatorSignature
    {
        const char* name;       // Name of the operator
        PSObjectType kinds[8];  // Types of arguments
        uint8_t arity;          // Number of arguments

        PSOperatorSignature(const char* opName, const char* typeString)
        {
            name = PSNameTable::INTERN(opName);
            arity = 0;

            // Assign argument types
            while (*typeString && arity < 8) {
                kinds[arity++] = static_cast<PSObjectType>(*typeString++);
            }
        }
    };

    struct PSOperatorArgs {
        const PSOperatorSignature& signature;
        PSObject values[8];   // fixed-size array
        size_t count;

        bool isValid() const { return count == signature.arity; }

        PSObject& operator[](size_t i) { return values[i]; }
        const PSObject& operator[](size_t i) const { return values[i]; }

        const char* operatorName() const { return signature.name; }
        PSObjectType operandType(size_t i) const { return (i < count) ? signature.kinds[i] : PSObjectType::Invalid; }
    };


    // --------------------
    // PSDictionary
    // --------------------
    struct PSDictionary {
    private:
        PSDictionary() = default;

        PSDictionary(size_t initialSize) {
            fEntries.reserve(initialSize);
        }

        std::unordered_map<const char*, PSObject> fEntries;

    public:
        
        static PSDictionaryHandle create(size_t initialSize = 0) {
            auto ptr = std::shared_ptr<PSDictionary>(new PSDictionary(initialSize));

            return ptr;
        }

        const std::unordered_map<const char*, PSObject> entries() const {
            return fEntries;
		}

        size_t size() const {
            return fEntries.size();
		}

        bool put(const char* key, const PSObject& value) {
			const char* internedKey = PSNameTable::INTERN(key);
            fEntries[internedKey] = value;
            return true;
        }

        bool get(const char* key, PSObject& out) const {
            const char* internedKey = PSNameTable::INTERN(key);

            auto it = fEntries.find(internedKey);
            if (it == fEntries.end()) return false;
            out = it->second;
            return true;
        }

        bool contains(const char* key) const {
            const char* internedKey = PSNameTable::INTERN(key);

            return fEntries.find(internedKey) != fEntries.end();
        }

        void clear() {
            fEntries.clear();
        }
    };

    // --------------------
    // PSArray
    // --------------------
    struct PSArray {
    private:
        // We want this constructor to be private so that we can
		// totally control how PSArray objects are created
		// you MUST use the static create() method
        PSArray() = default;

    public:
        std::vector<PSObject> elements;
		bool fIsProcedure = false; // Is this array a procedure?

        //explicit PSArray(size_t initialSize, const PSObject& fill = PSObject()) {
        //    elements.resize(initialSize, fill);
        //}

        size_t size() const {return elements.size();}

        // Is this array a procedure or not?
        constexpr bool isProcedure() const noexcept {return fIsProcedure;}
        void setIsProcedure(bool flag) noexcept {fIsProcedure = flag;}


        bool get(size_t index, PSObject& out) const {
            if (index >= elements.size()) return false;
            out = elements[index];
            return true;
        }

        bool put(size_t index, const PSObject& val) {
            if (index >= elements.size()) return false;
            elements[index] = val;
            return true;
        }

        bool append(const PSObject& val) {
            elements.push_back(val);
            return true;
        }

        void reset() {
            elements.clear();
        }

        PSArrayHandle copy() const {
            auto result = PSArray::create();
            result->setIsProcedure(fIsProcedure);
            result->elements = elements;
            return result;
        }

        // A subarray is a literal array, because we don't know if they
        // are getting a whole program or not
        PSArrayHandle subarray(size_t index, size_t count) const {
            if (index >= elements.size()) 
                return PSArray::create(0);
            
            count = std::min(count, elements.size() - index);
            auto result = PSArray::create();
            //result->setIsProcedure(fIsProcedure);

            result->elements.insert(
                result->elements.end(),
                elements.begin() + index,
                elements.begin() + index + count
            );

            return result;
        }

        // Functional programming
        // // Predicate-based validation
        template <typename Pred>
        bool allOf(Pred pred) const {
            for (const auto& obj : elements)
                if (!pred(obj)) return false;
            return true;
        }

        bool allOfType(PSObjectType t) const {
            return allOf([=](const PSObject& o) { return o.is(t); });
        }

        bool allNumbers() const {
			return allOfType(PSObjectType::Int) || allOfType(PSObjectType::Real);
        }


        // factory constructor
        static std::shared_ptr<PSArray> create(size_t initialSize = 0, const PSObject& fill = PSObject()) {
            auto ptr = std::shared_ptr<PSArray>(new PSArray());
            ptr->elements.resize(initialSize, fill);
            return ptr;
		}
    };


}

// Help the operand processing
namespace waavs {
	// An operand signature is a tuple of name and types of arguments
    struct OperandSignature {
        const char* name;       // name of the operator
        PSObjectType kinds[8];  // Max 8 arguments
        uint8_t arity;          // how many arguments are there
    };
}
