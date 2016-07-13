#include "gss_variant.h"

bool GssVariant::isZero()
{
    switch(type)
    {
    case Type::none:
        return true;
    case Type::integer:
        return data.i == 0;
    case Type::float_value:
        return data.f == 0;
    case Type::string:
        //TODO: Zero length strings are zero.
        return false;
    case Type::list:
        //TODO: Empty lists are zero.
        return false;
    case Type::dictionary:
        //TODO: Empty dictionaries are zero.
        return false;
    case Type::script_function:
        return false;
    case Type::native_function:
        return false;
    case Type::function_return_data:
        return false;
    }
    return false;
}

string GssVariant::toString()
{
    switch(type)
    {
    case Type::none:
        return "[NONE]";
    case Type::integer:
        return string(data.i);
    case Type::float_value:
        return string(data.f);
    case Type::string:
        return "[STR]";
    case Type::list:
        return "[LIST]";
    case Type::dictionary:
        return "[DICT]";
    case Type::script_function:
        return "[FUNC:"+string(data.i)+"]";
    case Type::native_function:
        return "[CFUNC:"+string(data.i)+"]";
    case Type::function_return_data:
        return "[RET]";
    }
    return "?";
}

bool GssVariant::canBeFloat()
{
    return type == Type::integer || type == Type::float_value;
}

float GssVariant::toFloat()
{
    if (type == Type::integer)
        return data.i;
    else if (type == Type::float_value)
        return data.f;
    return 0.0;
}
