#pragma once

#include <cstdint>
#include <cstring>

#include <string>
#include <string_view>
#include <unordered_map>
#include <iostream>

#include <vector>
#include <functional>
#include <memory>
#include <variant>

#include "ocspan.h"
#include "ps_type_name.h"
#include "ps_type_string.h"
#include "ps_type_matrix.h"
//#include "ps_type_image.h"
#include "ps_type_path.h"




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
    struct PSFont;
    struct PSFontFace;
    struct PSFile;

    // Handle aliases for clarity
    using PSArrayHandle = std::shared_ptr<PSArray>;
    using PSDictionaryHandle = std::shared_ptr<PSDictionary>;
    using PSFileHandle = std::shared_ptr<PSFile>;
    using PSFontFaceHandle = std::shared_ptr<PSFontFace>;
    using PSFontHandle = std::shared_ptr<PSFont>;
    using PSMatrixHandle = std::shared_ptr<PSMatrix>;
}

// Define this custom hash function here, because we need it
// to easily use PSName as a key in unordered_map, like PSDictionary
namespace std {
    template<> struct hash<waavs::PSName> {
        size_t operator()(const waavs::PSName& name) const noexcept {
            return std::hash<const char*>()(name.c_str());
        }
    };
}

namespace waavs
{
    // --------------------
    // PSMark
    // Used to mark positions on a stack or in a procedure
    // --------------------
    struct PSMark {
    private:
        PSName fName = nullptr; // Name of the marker, always interned

    public:
        PSMark(const char* name = nullptr) noexcept
            : fName(PSNameTable::INTERN(name)) {
        }

        const PSName& name() const noexcept {
            return fName;
        }
    };


    // --------------------
    // PSOperator
    // --------------------
    // These definitions are used for builtin operators that are known at compile time
    using PSOperatorFunc = bool(*)(PSVirtualMachine&);
    using PSOperatorFuncMap = std::unordered_map<PSName, PSOperatorFunc>;

    struct PSOperator 
    {
    private:
        PSName fName;       // Always interned and stable
        PSOperatorFunc fFunc = nullptr;

    public:
        PSOperator() = default;

        constexpr PSOperator(const PSName& opName, PSOperatorFunc f) noexcept
            : fName(opName)
            , fFunc(f) {
        }

        const PSName & name() const noexcept { return fName; }

        bool exec(PSVirtualMachine& vm) const
        {
            if (fFunc != nullptr) {
                return fFunc(vm);
            }
            return false; // No function to execute
        }

        constexpr bool isValid() const noexcept { return fFunc != nullptr;}

    };
}

namespace waavs {

    // --------------------
    // PSObject Type Enum
    // --------------------
    enum struct PSObjectType : char {
          Null      = 'z'       // Null type, represents a null object
        , Invalid   = '?'       // Invalid type, used for uninitialized objects
        , Any       = '*'       // Any type, used for generic operations
        , Int       = 'i'       // Integer type, represents a 32-bit signed integer
        , Float     = 'r'       // single precision floating-point number
        , Real      = 'R'        // double precision floating-point number
        , Bool      = 'b'       // Boolean type, represents a true or false value
        , Pointer   = 'V'       // Pointer type, represents a pointer to an object
        , Name      = 'n'       // Name type, represents an interned name (string)
        , String    = 's'       // String type, represents a PSString object
        , Array     = 'a'       // Sequence of entries
        , Dictionary = 'd'  // Associative array (key-value pairs)
        , Operator  = 'O'    // Operator type, represents a function or procedure
        , Path      = 'p'        // Path type, represents a drawing path
        , File      = 'L'        // File type, represents a file object
        , Font      = 'f'        // Font type, represents a font object
        , FontFace  = 'F'    // FontFace type, represents a font face object
        , Mark      = 'm'
        , Matrix    = 'x'
        , Save      = 'S'       // VM Save state
    };

    enum PSObjectFlags : uint32_t {
        PS_OBJ_FLAG_NONE        = 0,
        PS_OBJ_FLAG_EXECUTABLE  = 1 << 0,
        PS_OBJ_FLAG_SYSTEM_OP = 1 << 1,

        PS_OBJ_FLAG_ACCESS_READABLE    = 1 << 2,
        PS_OBJ_FLAG_ACCESS_WRITABLE    = 1 << 3,
        PS_OBJ_FLAG_ACCESS_EXECUTABLE = 1 << 4,

        // Future bits here (like `CONST`, `PROTECTED`, `FROM_ROM`, etc.)
    };

    struct PSObject {
    private:
        using Variant = std::variant<
            std::monostate,                     // INVALID
            int32_t,                            // Int
            float,                              // Float (single precision)
            double,                             // Real (double precision)
            bool,                               // Bool
            void*,                              // Pointer
            PSName,                             // Name (interned)
            PSOperator,                         // Operator
            PSMatrix,                           // Matrix
            PSPath,                             // Path
            PSString,                           // String
            PSArrayHandle,                      // Array
            PSDictionaryHandle,                 // Dictionary
            PSFileHandle,                       // File
            PSFontFaceHandle,                   // FontFace
            PSFontHandle,                       // Font
            PSMark                              // Mark
        >;

        uint32_t fFlags{ PS_OBJ_FLAG_NONE };
        Variant fValue;

    public:
        PSObjectType type = PSObjectType::Null;



        // Reset state
        bool reset() {
            fValue = std::monostate{};
            type = PSObjectType::Null;
            fFlags = PS_OBJ_FLAG_NONE;
            setAccessReadable(true);
            setAccessWriteable(true);
            setAccessExecutable(true);

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

        bool resetFromName(const PSName& n) {
            reset(); type = PSObjectType::Name; fValue = n; return true;
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

        bool resetFromFile(PSFileHandle f) {
            reset(); type = PSObjectType::File; fValue = f; return true;
        }

        bool resetFromFontFace(PSFontFaceHandle v) {
            reset(); type = PSObjectType::FontFace; fValue = v; return true;
        }

        bool resetFromFont(PSFontHandle v) {
            reset(); type = PSObjectType::Font; fValue = v; return true;
        }

        bool resetFromOperator(const PSOperator& f) {
            reset(); type = PSObjectType::Operator; setExecutable(true); fValue = f; return true;
        }
        bool resetFromMatrix(const PSMatrix& m) {
            reset(); type = PSObjectType::Matrix; fValue = m; return true;
        }
        bool resetFromPath(const PSPath& p) {
            reset(); type = PSObjectType::Path; fValue = p; return true;
        }
        bool resetFromPath(PSPath&& p) {
            reset(); type = PSObjectType::Path; fValue = std::move(p); return true;
        }
        bool resetFromMark(const PSMark& m) {
            reset(); type = PSObjectType::Mark; fValue = m; return true;
        }

        bool resetFromSave() { reset(); type = PSObjectType::Save; return true; }


        // Static constructors
        static PSObject fromInt(int32_t v) { PSObject o; o.resetFromInt(v); return o; }
        static PSObject fromReal(double v) { PSObject o; o.resetFromReal(v); return o; }
        static PSObject fromBool(bool v) { PSObject o; o.resetFromBool(v); return o; }
        static PSObject fromName(const PSName& n) { PSObject o; o.resetFromName(n); return o; }
        static PSObject fromExecName(const PSName& n) { PSObject o; o.resetFromName(n); o.setExecutable(true); return o; }
        //static PSObject fromName(const OctetCursor& oc) { PSObject o; o.resetFromName(oc); return o; }
        static PSObject fromString(PSString s) { PSObject o; o.resetFromString(s); return o; }
        static PSObject fromArray(PSArrayHandle a) { PSObject o; o.resetFromArray(a); return o; }
        static PSObject fromDictionary(PSDictionaryHandle d) { PSObject o; o.resetFromDictionary(d); return o; }
        static PSObject fromFile(PSFileHandle f) { PSObject o; o.resetFromFile(f); return o; }
        static PSObject fromFontFace(PSFontFaceHandle v) { PSObject o; o.resetFromFontFace(v); return o; }
        static PSObject fromFont(PSFontHandle v) { PSObject o; o.resetFromFont(v); return o; }
        static PSObject fromOperator(const PSOperator& f) { PSObject o; o.resetFromOperator(f); return o; }
        static PSObject fromMatrix(const PSMatrix& m) { PSObject o; o.resetFromMatrix(m); return o; }
        static PSObject fromPath(const PSPath& p) { PSObject o; o.resetFromPath(p); return o; }
        static PSObject fromPath(PSPath&& p) { PSObject o; o.resetFromPath(p); return o; }
        static PSObject fromMark(const PSMark& m) { PSObject o; o.resetFromMark(m); return o; }
        static PSObject fromSave() { PSObject o; o.resetFromSave(); return o; }

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
        int32_t asInt() const { return (type == PSObjectType::Int) ? as<int32_t>() : static_cast<int32_t>(as<double>()); }
        double asReal() const { return (type == PSObjectType::Int) ? static_cast<double>(as<int32_t>()) : as<double>(); }
        bool asBool() const { return as<bool>(); }
        PSName asName() const { return as<PSName>(); }
        const char* asNameCStr() const { return as<PSName>().c_str(); }
        
        const PSString& asString() const { return std::get<PSString>(fValue); }
        PSString& asMutableString() {
            return std::get<PSString>(fValue);
        }


        PSArrayHandle asArray() const { return as<PSArrayHandle>(); }
        PSDictionaryHandle asDictionary() const { return as<PSDictionaryHandle>(); }
        PSFileHandle asFile() const { return as<PSFileHandle>(); }
        PSFontFaceHandle asFontFace() const { return as<PSFontFaceHandle>(); }
        PSFontHandle asFont() const { return as<PSFontHandle>(); }
        PSOperator asOperator() const { return as<PSOperator>(); }
        PSMatrix asMatrix() const { return as<PSMatrix>(); }
        PSPath asPath() const { return as<PSPath>(); }
        PSMark asMark() const { return as<PSMark>(); }

        // Type and flag checks
        // Checking and setting object attributes
        inline void setFlag(uint32_t flag) noexcept { fFlags |= flag; }
        inline void clearFlag(uint32_t flag) noexcept { fFlags &= ~flag; }
        inline bool hasFlag(uint32_t flag) const noexcept { return (fFlags & flag) != 0; }

        inline bool isSystemOp() const noexcept { return hasFlag(PS_OBJ_FLAG_SYSTEM_OP); }
        inline bool isExecutable() const noexcept { return hasFlag(PS_OBJ_FLAG_EXECUTABLE); }

        inline bool isAccessReadable() const noexcept { return hasFlag(PS_OBJ_FLAG_ACCESS_READABLE); }
        inline bool isAccessWriteable() const noexcept { return hasFlag(PS_OBJ_FLAG_ACCESS_WRITABLE); }
        inline bool isAccessExecutable() const noexcept { return hasFlag(PS_OBJ_FLAG_ACCESS_EXECUTABLE); }

        inline void setExecutable(bool val) noexcept {val ? setFlag(PS_OBJ_FLAG_EXECUTABLE) : clearFlag(PS_OBJ_FLAG_EXECUTABLE);}
        inline void setSystemOp(bool val) noexcept { val ? setFlag(PS_OBJ_FLAG_SYSTEM_OP) : clearFlag(PS_OBJ_FLAG_SYSTEM_OP); }
        
        inline void setAccessReadable(bool val) noexcept { val ? setFlag(PS_OBJ_FLAG_ACCESS_READABLE) : clearFlag(PS_OBJ_FLAG_ACCESS_READABLE); }
        inline void setAccessWriteable(bool val) noexcept { val ? setFlag(PS_OBJ_FLAG_ACCESS_WRITABLE) : clearFlag(PS_OBJ_FLAG_ACCESS_WRITABLE); }
        inline void setAccessExecutable(bool val) noexcept { val ? setFlag(PS_OBJ_FLAG_ACCESS_EXECUTABLE) : clearFlag(PS_OBJ_FLAG_ACCESS_EXECUTABLE); }


        // Checking object type
        inline constexpr bool is(PSObjectType t) const { return (type == t) || (t == PSObjectType::Any); }
        inline bool isNumber() const { return isInt() || isReal(); }
        inline bool isInt() const {return is(PSObjectType::Int) || (isReal() && (as<double>() == static_cast<int64_t>(as<double>()))); }
        inline bool isReal() const { return is(PSObjectType::Real); }
        inline bool isBool() const { return is(PSObjectType::Bool); }
        inline bool isName() const { return is(PSObjectType::Name); }
        inline bool isLiteralName() const { return isName() && !isExecutable(); }
        inline bool isExecutableName() const { return isName() && isExecutable(); }
        inline bool isString() const { return is(PSObjectType::String); }
        inline bool isArray() const { return is(PSObjectType::Array); }
        inline bool isExecutableArray() const { return isArray() && isExecutable(); }
        inline bool isDictionary() const { return is(PSObjectType::Dictionary); }
        inline bool isFile() const { return is(PSObjectType::File); }
        inline bool isFontFace() const { return is(PSObjectType::FontFace); }
        inline bool isFont() const { return is(PSObjectType::Font); }
        inline bool isOperator() const { return is(PSObjectType::Operator); }
        inline bool isMark() const { return is(PSObjectType::Mark); }
        inline bool isMatrix() const { return is(PSObjectType::Matrix); }
        inline bool isPath() const { return is(PSObjectType::Path); }
        inline bool isNull() const { return is(PSObjectType::Null); }
        inline bool isSave() const { return is(PSObjectType::Save); }

        // Signature helper
        char typeChar() const { return static_cast<char>(type); }
    };
}





namespace waavs {
    // --------------------
    // PSArray
    // --------------------
    struct PSArray {
    public:
        std::vector<PSObject> elements;

        // Constructors
        PSArray() = default;

        explicit PSArray(size_t size, const PSObject& fill = PSObject())
            : elements(size, fill) {
        }

        // Copy/move support
        PSArray(const PSArray&) = default;
        PSArray(PSArray&&) noexcept = default;
        PSArray& operator=(const PSArray&) = default;
        PSArray& operator=(PSArray&&) noexcept = default;

        // Size and flags
        size_t size() const { return elements.size(); }

        // Element access
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

        // Make a deep copy of this array
        std::shared_ptr<PSArray> copy() const {
            auto result = std::make_shared<PSArray>(*this);
            return result;
        }

        // Return a literal subarray from this array
        std::shared_ptr<PSArray> subarray(size_t index, size_t count) const {
            auto result = std::make_shared<PSArray>();
            if (index >= elements.size()) return result;

            count = std::min(count, elements.size() - index);
            result->elements.insert(
                result->elements.end(),
                elements.begin() + index,
                elements.begin() + index + count
            );
            return result;
        }

        // Predicate-based validation
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
            return allOf([=](const PSObject& o) { return o.isNumber(); });

            //return allOfType(PSObjectType::Int) || allOfType(PSObjectType::Real);
        }

        // Factory method
        static std::shared_ptr<PSArray> create(size_t size = 0, const PSObject& fill = PSObject()) {
            return std::make_shared<PSArray>(size, fill);
        }
    };


}


// Some helper functions
namespace waavs {
    inline bool matrixFromArray(const PSArrayHandle h, PSMatrix& out) 
    {
        if ( h->size() != 6 || !h->allNumbers())
            return false;

        for (size_t i = 0; i < 6; ++i) {
            PSObject o;
            if (!h->get(i, o)) return false;
            out.m[i] = o.asReal();
        }

        return true;
    }

    // Helper: Extract matrix from object (matrix or numeric array)
    inline bool extractMatrix(const PSObject& obj, PSMatrix& out) 
    {
        if (obj.isMatrix()) {
            out = obj.asMatrix();
            return true;
        }
        if (obj.isArray()) {
            return matrixFromArray(obj.asArray(), out);
        }
        return false;
    }
}


