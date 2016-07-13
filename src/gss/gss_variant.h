#ifndef GSS_VARIANT_H
#define GSS_VARIANT_H

#include <stdint.h>
#include "stringImproved.h"

class GssVariant
{
public:
    enum class Type
    {
        none,
        integer,
        float_value,
        string,
        list,
        dictionary,
        script_function,
        native_function,
        
        function_return_data
    };
    union Data
    {
        int32_t i;
        float f;
        uint16_t s[2];
    };
    
    Type type;
    Data data;
    
    bool isZero();
    string toString();
    bool canBeFloat();
    float toFloat();
};

#endif//GSS_VARIANT_H
