#pragma once

namespace waavsps {


    enum class PSObjectType {
        Null,
        Int,
        Real,
        Bool,
        Name,
        String,
        Array,
        Dictionary,
        Operator,
        Mark
    };

    // Forward declarations
    struct PSObject;
    struct PSString; 
    struct PSOperator;
    struct PSArray;
    struct PSDictionary;
}