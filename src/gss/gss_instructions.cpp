#include "gss_instructions.h"

string GssInstruction::toString() const
{
    switch(type)
    {
    case Type::nop:
        return "NOP";
    case Type::push_none:
        return "PUSH NONE";
    case Type::push_int:
        return "PUSH " + string(data.i);
    case Type::push_float:
        return "PUSH " + string(data.f);
    case Type::push_empty_list:
        return "PUSH []";
    case Type::push_string_from_string_table:
        return "PUSH STR[" + string(data.i) + "]";
    case Type::push_script_function:
        return "PUSH FUNC[" + string(data.i) + "]";
    case Type::jump:
        return "JUMP -> " + string(data.i);
    case Type::jump_if_zero:
        return "JUMP ZERO -> " + string(data.i);
    case Type::jump_if_not_zero:
        return "JUMP NOT ZERO -> " + string(data.i);
    case Type::pop:
        return "POP " + string(data.i);
    
    case Type::push_global_by_index:
        return "PUSH GLOBAL [" + string(data.i) + "]";
    case Type::assign_global_by_index:
        return "ASSIGN GLOBAL [" + string(data.i) + "]";

    case Type::push_local_by_index:
        return "PUSH LOCAL [" + string(data.i) + "]";
    case Type::assign_local_by_index:
        return "ASSIGN LOCAL [" + string(data.i) + "]";
    
    case Type::get_from_table:
        return "GET TABLE";
    case Type::assign_to_table:
        return "ASSIGN TABLE";
    case Type::add_to_table:
        return "ADD TO TABLE";
    case Type::get_from_table_by_string_table:
        return "GET MEMBER STR[" + string(data.i) + "]";
    case Type::assign_to_table_by_string_table:
        return "ASSIGN MEMBER STR[" + string(data.i) + "]";
    
    case Type::call_function:
        return "CALL " + string(data.i);
    case Type::ensure_locals:
        return "ENSURE LOCALS " + string(data.i);
    case Type::return_from_function:
        return "RET";
    
    case Type::boolean_not:
        return "!";
    case Type::binary_not:
        return "^";
    case Type::negative:
        return "NEG";
    
    case Type::boolean_or:
        return "||";
    case Type::boolean_and:
        return "&&";
    case Type::binary_or:
        return "|";
    case Type::binary_not_2:
        return "x ^ y";
    case Type::binary_and:
        return "&";
    case Type::boolean_equal:
        return "==";
    case Type::boolean_not_equal:
        return "!=";
    case Type::boolean_less:
        return "<";
    case Type::boolean_less_equal:
        return "<=";
    case Type::boolean_greater:
        return ">";
    case Type::boolean_greater_equal:
        return ">=";
    case Type::left_shift:
        return "LEFT SHIFT";
    case Type::right_shift:
        return "RIGHT SHIFT";
    case Type::add:
        return "ADD";
    case Type::substract:
        return "SUB";
    case Type::multiply:
        return "MUL";
    case Type::division:
        return "DIV";
    case Type::modulo:
        return "MOD";
    }
    return "?";
}
