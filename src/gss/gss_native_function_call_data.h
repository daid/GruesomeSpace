#ifndef GSS_NATIVE_FUNCTION_CALL_DATA_H
#define GSS_NATIVE_FUNCTION_CALL_DATA_H

#include <SFML/System.hpp>
#include "stringImproved.h"

class GssMemory;
class GssNativeFunctionCallData : sf::NonCopyable
{
public:
    GssNativeFunctionCallData(unsigned int parameter_stack_position, unsigned int parameter_count, GssMemory* memory);

    int getParameterCount();

    bool isNone(unsigned int index);
    bool isInt(unsigned int index);
    bool isFloat(unsigned int index);
    bool isNumber(unsigned int index);
    bool isString(unsigned int index);
    
    int getInt(unsigned int index);
    float getFloat(unsigned int index);
    float getNumber(unsigned int index);
    string getString(unsigned int index);
    
    void returnNone();
    void returnInt(int i);
    void returnFloat(float f);
    void returnString(string s);
private:
    unsigned int parameter_stack_position;
    unsigned int parameter_count;
    GssMemory* memory;
};

class GssNativeFunction
{
public:
    GssNativeFunction(string name, std::function<void(GssNativeFunctionCallData& engine)> function);

    string name;
    std::function<void(GssNativeFunctionCallData& engine)> function;
};

#endif//GSS_NATIVE_FUNCTION_CALL_DATA_H
