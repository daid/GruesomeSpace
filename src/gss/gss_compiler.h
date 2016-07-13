#ifndef GSS_COMPILER_H
#define GSS_COMPILER_H

#include "gss_tokenizer.h"
#include "gss_instructions.h"

/*
    The GssCompiler takes tokens from the GssTokenizer and turns this into
    a list of GssInstructions and a static string table.
*/
class GssCompiler
{
public:
    GssCompiler(GssTokenizer& tokenizer);
    
    void setNativeFunctions(std::vector<GssNativeFunction>& functions);
    void compile();

    std::vector<GssInstruction> instructions;
    std::vector<string> string_table;
private:
    bool global;
    
    GssTokenizer& tokenizer;
    std::vector<string> global_vars;
    std::vector<string> local_vars;
    std::vector< std::vector< GssToken::Type > > binary_operators;
    
    void parseBlock(int minimal_indent);
    
    void parseExpression();
    void parseBinaryOperator(unsigned int precedence);
    void parseSubscript();
    void parseUnary();
    void parseValue();
    
    GssToken expect(GssToken::Type type);
    
    int addToStringTable(string value);
    int addGlobal(GssToken& reference_token, string name);
    int getGlobal(string name);
};

class GssCompilerException : public std::exception
{
public:
    string message;
    
    GssCompilerException(const GssToken& token, string message) : message(message + " at line " + string(token.line_number)) {}
};

#endif//GSS_COMPILER_H
