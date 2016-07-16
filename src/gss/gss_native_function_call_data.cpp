#include "gss_native_function_call_data.h"
#include "gss_variant.h"
#include "gss_memory.h"

GssNativeFunction::GssNativeFunction(string name, std::function<void(GssNativeFunctionCallData& engine)> function)
: name(name), function(function)
{
}

GssNativeFunctionCallData::GssNativeFunctionCallData(unsigned int parameter_stack_position, unsigned int parameter_count, GssMemory* memory)
: parameter_stack_position(parameter_stack_position), parameter_count(parameter_count), memory(memory)
{
}

int GssNativeFunctionCallData::getParameterCount()
{
    return parameter_count;
}

bool GssNativeFunctionCallData::isNone(unsigned int index)
{
    if (index >= parameter_count)
        return true;
    GssVariant* v = memory->getStack(parameter_stack_position + index);
    if (v->type == GssVariant::Type::none)
        return true;
    return false;
}

bool GssNativeFunctionCallData::isInt(unsigned int index)
{
    if (index >= parameter_count)
        return false;
    GssVariant* v = memory->getStack(parameter_stack_position + index);
    if (v->type == GssVariant::Type::integer)
        return true;
    return false;
}

bool GssNativeFunctionCallData::isFloat(unsigned int index)
{
    if (index >= parameter_count)
        return false;
    GssVariant* v = memory->getStack(parameter_stack_position + index);
    if (v->type == GssVariant::Type::float_value)
        return true;
    return false;
}

bool GssNativeFunctionCallData::isNumber(unsigned int index)
{
    return isInt(index) || isFloat(index);
}

bool GssNativeFunctionCallData::isString(unsigned int index)
{
    if (index >= parameter_count)
        return false;
    GssVariant* v = memory->getStack(parameter_stack_position + index);
    if (v->type == GssVariant::Type::string)
        return true;
    return false;
}

int GssNativeFunctionCallData::getInt(unsigned int index)
{
    if (index >= parameter_count)
        return 0;
    GssVariant* v = memory->getStack(parameter_stack_position + index);
    if (v->type == GssVariant::Type::integer)
        return v->data.i;
    return 0;
}

float GssNativeFunctionCallData::getFloat(unsigned int index)
{
    return getNumber(index);
}

float GssNativeFunctionCallData::getNumber(unsigned int index)
{
    if (index >= parameter_count)
        return 0.0;
    GssVariant* v = memory->getStack(parameter_stack_position + index);
    if (v->type == GssVariant::Type::integer)
        return v->data.i;
    if (v->type == GssVariant::Type::float_value)
        return v->data.f;
    return 0.0;
}

string GssNativeFunctionCallData::getString(unsigned int index)
{
    if (index >= parameter_count)
        return "";
    GssVariant* v = memory->getStack(parameter_stack_position + index);
    if (v->type == GssVariant::Type::string)
        return memory->getString(v->data.i);
    return "";
}

void GssNativeFunctionCallData::returnNone()
{
    GssVariant* v = memory->getStack(parameter_stack_position - 1);
    v->type = GssVariant::Type::none;
}

void GssNativeFunctionCallData::returnInt(int i)
{
    GssVariant* v = memory->getStack(parameter_stack_position - 1);
    v->type = GssVariant::Type::integer;
    v->data.i = i;
}

void GssNativeFunctionCallData::returnFloat(float f)
{
    GssVariant* v = memory->getStack(parameter_stack_position - 1);
    v->type = GssVariant::Type::float_value;
    v->data.f = f;
}

void GssNativeFunctionCallData::returnString(string s)
{
    GssVariant* v = memory->getStack(parameter_stack_position - 1);
    v->type = GssVariant::Type::string;
    v->data.i = memory->createString(s);
}
