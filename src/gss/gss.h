#ifndef GSS_H
#define GSS_H

#include "stringImproved.h"
#include "gss_instructions.h"
#include "gss_memory.h"
#include "gss_native_function_call_data.h"

class GssEngine : sf::NonCopyable
{
public:
    void compile(string code);
    void addNativeFunction(string name, std::function<void(GssNativeFunctionCallData&)> function);
    
    void step();
private:
    GssMemory* memory;

    std::vector<GssNativeFunction> native_functions;

    std::vector<GssInstruction> instructions;
    std::vector<string> string_table;
    
    unsigned int instruction_pointer;
    unsigned int locals_stack_position;
};

class GssRuntimeException : public std::exception
{
public:
    string message;
    
    GssRuntimeException(string message) : message(message) {}
};

#endif//GSS_H
