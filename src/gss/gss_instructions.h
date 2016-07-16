#ifndef GSS_INSTRUCTIONS_H
#define GSS_INSTRUCTIONS_H

#include <functional>
#include <stdint.h>
#include "stringImproved.h"

class GssInstruction
{
public:
    enum class Type
    {
        nop,
        push_none,
        push_int,
        push_float,
        push_empty_list,
        push_string_from_string_table,
        push_script_function,
        jump,
        jump_if_zero,
        jump_if_not_zero,
        pop,
        
        push_global_by_index,
        assign_global_by_index,
        
        push_local_by_index,
        assign_local_by_index,
        
        get_from_table,
        assign_to_table,
        add_to_table,
        get_from_table_by_string_table,
        assign_to_table_by_string_table,
        
        call_function,
        ensure_locals,
        return_from_function,
        
        boolean_not,
        binary_not,
        negative,
        
        boolean_or,
        boolean_and,
        binary_or,
        binary_not_2,
        binary_and,
        boolean_equal,
        boolean_not_equal,
        boolean_less,
        boolean_less_equal,
        boolean_greater,
        boolean_greater_equal,
        left_shift,
        right_shift,
        add,
        substract,
        multiply,
        division,
        modulo,
    };
    union Data {
        float f;
        int32_t i;
    };
    
    Type type;
    Data data;
    
    GssInstruction(Type type) : type(type) { data.i = 0; }
    GssInstruction(Type type, float value) : type(type) { data.f = value; }
    GssInstruction(Type type, int value) : type(type) { data.i = value; }
    GssInstruction(Type type, unsigned int value) : type(type) { data.i = value; }
    
    string toString() const;
};

#endif//GSS_INSTRUCTIONS_H
