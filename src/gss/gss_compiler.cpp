#include "gss_compiler.h"

#include "logging.h"

GssCompiler::GssCompiler(GssTokenizer& tokenizer)
: tokenizer(tokenizer)
{
    binary_operators.push_back({GssToken::Type::logical_or});
    binary_operators.push_back({GssToken::Type::logical_and});
    binary_operators.push_back({GssToken::Type::pipe});
    binary_operators.push_back({GssToken::Type::circumflex});
    binary_operators.push_back({GssToken::Type::ampersand});
    binary_operators.push_back({GssToken::Type::equal, GssToken::Type::notequal});
    binary_operators.push_back({GssToken::Type::less, GssToken::Type::less_equal, GssToken::Type::greater, GssToken::Type::greater_equal});
    binary_operators.push_back({GssToken::Type::left_shift, GssToken::Type::right_shift});
    binary_operators.push_back({GssToken::Type::plus, GssToken::Type::minus});
    binary_operators.push_back({GssToken::Type::star, GssToken::Type::slash, GssToken::Type::percent});
}

void GssCompiler::setNativeFunctions(std::vector<GssNativeFunction>& functions)
{
    for(GssNativeFunction& func : functions)
        global_vars.push_back(func.name);
}

void GssCompiler::compile()
{
    global = true;
    parseBlock(0);
}

void GssCompiler::parseBlock(int minimal_indent)
{
    int start_indent = -1;
    while(tokenizer.peek().type != GssToken::Type::end_of_file)
    {
        GssToken token = tokenizer.peek();
        if (token.type == GssToken::Type::end_of_line)
        {
            tokenizer.get();
            continue;
        }
        if (start_indent == -1)
        {
            start_indent = token.indent_amount;
            if (start_indent < minimal_indent)
                throw GssCompilerException(tokenizer.peek(), "No proper indentation (got: " + string(tokenizer.peek().indent_amount) + " expected: " + string(minimal_indent) + ")");
        }
        if (token.indent_amount > start_indent)
            throw GssCompilerException(token, "Inconsistent indentation (got: " + string(token.indent_amount) + " expected: " + string(start_indent) + ")");
        if (token.indent_amount < start_indent)
            break;
        if (token.type == GssToken::Type::name)
        {
            if (token.data == "if")
            {
                tokenizer.get();
                parseExpression();
                instructions.emplace_back(GssInstruction::Type::jump_if_zero);
                int jump_instruction_index = instructions.size() - 1;
                expect(GssToken::Type::colon);
                expect(GssToken::Type::end_of_line);
                parseBlock(start_indent + 1);
                instructions[jump_instruction_index].data.i = instructions.size();
            }else if (token.data == "while")
            {
                tokenizer.get();
                int while_jump_location = instructions.size();
                parseExpression();
                instructions.emplace_back(GssInstruction::Type::jump_if_zero);
                int jump_instruction_index = instructions.size() - 1;
                expect(GssToken::Type::colon);
                expect(GssToken::Type::end_of_line);
                parseBlock(start_indent + 1);
                instructions.emplace_back(GssInstruction::Type::jump, while_jump_location);
                instructions[jump_instruction_index].data.i = instructions.size();
            }else if (token.data == "for")
            {
                tokenizer.get();
                parseStatement(false);
                expect(GssToken::Type::semi_colon);
                int jump_condition_target = instructions.size();
                parseExpression();
                instructions.emplace_back(GssInstruction::Type::jump_if_zero, 0);
                int jump_to_end_instruction_index = instructions.size() - 1;
                instructions.emplace_back(GssInstruction::Type::jump, 0);
                int jump_past_condition_index = instructions.size() - 1;
                instructions.emplace_back(GssInstruction::Type::jump, 0);
                int jump_increment_target = instructions.size();
                expect(GssToken::Type::semi_colon);
                parseStatement(false);
                expect(GssToken::Type::colon);
                instructions.emplace_back(GssInstruction::Type::jump, jump_condition_target);
                instructions[jump_past_condition_index].data.i = instructions.size();
                expect(GssToken::Type::end_of_line);
                parseBlock(start_indent + 1);
                instructions.emplace_back(GssInstruction::Type::jump, jump_increment_target);
                instructions[jump_to_end_instruction_index].data.i = instructions.size();
            }else if (token.data == "var")
            {
                tokenizer.get();
                token = expect(GssToken::Type::name);
                string var_name = token.data;
                token = tokenizer.peek();
                if (global)
                {
                    addGlobal(token, var_name);
                }else{
                    for(const string& v : local_vars)
                        if (v == var_name)
                            throw GssCompilerException(token, "Duplicate local variable definition.");
                    local_vars.push_back(var_name);
                    if (instructions.size() > 0 && instructions.back().type == GssInstruction::Type::ensure_locals)
                    {
                        instructions.back().data.i = local_vars.size();
                    }else{
                        instructions.emplace_back(GssInstruction::Type::ensure_locals, local_vars.size());
                    }
                }
                if (token.type == GssToken::Type::assign)
                {
                    token = tokenizer.get();
                    parseExpression();
                    if (global)
                    {
                        instructions.emplace_back(GssInstruction::Type::assign_global_by_index, getGlobal(var_name));
                    }else{
                        instructions.emplace_back(GssInstruction::Type::assign_local_by_index, local_vars.size() - 1);
                    }
                }
                expect(GssToken::Type::end_of_line);
            }else if (token.data == "function")
            {
                if (!global)
                    throw GssCompilerException(token, "Function definition inside function definition not allowed.");
                tokenizer.get();
                token = expect(GssToken::Type::name);
                string function_name = token.data;
                local_vars.clear();
                expect(GssToken::Type::left_bracket);
                token = tokenizer.peek();
                if (token.type == GssToken::Type::name)
                {
                    while(true)
                    {
                        string parameter_name = expect(GssToken::Type::name).data;
                        local_vars.push_back(parameter_name);
                        if (tokenizer.peek().type == GssToken::Type::right_bracket)
                            break;
                        expect(GssToken::Type::comma);
                    }
                }
                expect(GssToken::Type::right_bracket);
                expect(GssToken::Type::colon);
                expect(GssToken::Type::end_of_line);
                int jump_instruction_location = instructions.size();
                int function_index = addGlobal(token, function_name);
                instructions.emplace_back(GssInstruction::Type::jump, 0);
                global = false;
                instructions.emplace_back(GssInstruction::Type::ensure_locals, local_vars.size());
                parseBlock(start_indent + 1);
                if (instructions.back().type != GssInstruction::Type::return_from_function)
                {
                    instructions.emplace_back(GssInstruction::Type::push_none);
                    instructions.emplace_back(GssInstruction::Type::return_from_function);
                }
                instructions[jump_instruction_location].data.i = instructions.size();
                instructions.emplace_back(GssInstruction::Type::push_script_function, jump_instruction_location + 1);
                instructions.emplace_back(GssInstruction::Type::assign_global_by_index, function_index);
                global = true;
            }else if (token.data == "return")
            {
                if (global)
                    throw GssCompilerException(token, "Return outside of a function");
                tokenizer.get();
                parseExpression();
                expect(GssToken::Type::end_of_line);
                instructions.emplace_back(GssInstruction::Type::return_from_function);
            }else{
                parseStatement(true);
            }
        }else{
            throw GssCompilerException(token, "Unexpected: " + token.toString());
        }
    }
}

void GssCompiler::parseValue()
{
    GssToken& token = tokenizer.get();
    if (token.type == GssToken::Type::left_bracket)
    {
        parseExpression();
        token = tokenizer.get();
        if (token.type != GssToken::Type::right_bracket)
            throw GssCompilerException(token, "Unexpected: " + token.toString() + " expected: ')'");
    }else if (token.type == GssToken::Type::left_square_bracket)
    {
        instructions.emplace_back(GssInstruction::Type::push_empty_list, token.data.toFloat());
        token = tokenizer.peek();
        if (token.type != GssToken::Type::right_square_bracket)
        {
            int index = 0;
            while(true)
            {
                parseExpression();
                instructions.emplace_back(GssInstruction::Type::add_to_table, token.data.toFloat());
                token = tokenizer.peek();
                if (token.type != GssToken::Type::comma)
                    break;
                tokenizer.get();
                index++;
            }
        }
        expect(GssToken::Type::right_square_bracket);
    }else if (token.type == GssToken::Type::number)
    {
        if (token.data.find(".") > -1)
        {
            instructions.emplace_back(GssInstruction::Type::push_float, token.data.toFloat());
        }else{
            instructions.emplace_back(GssInstruction::Type::push_int, token.data.toInt());
        }
    }else if (token.type == GssToken::Type::string)
    {
        instructions.emplace_back(GssInstruction::Type::push_string_from_string_table, addToStringTable(token.data));
    }else if (token.type == GssToken::Type::name)
    {
        bool done = false;
        if (token.data == "none")
        {
            instructions.emplace_back(GssInstruction::Type::push_none);
            done = true;
        }
        if (token.data == "true")
        {
            instructions.emplace_back(GssInstruction::Type::push_int, 1);
            done = true;
        }
        if (token.data == "false")
        {
            instructions.emplace_back(GssInstruction::Type::push_int, 0);
            done = true;
        }
        if (!global)
        {
            int index = 0;
            for(const string& v : local_vars)
            {
                if (v == token.data)
                {
                    instructions.emplace_back(GssInstruction::Type::push_local_by_index, index);
                    done = true;
                    break;
                }
                index++;
            }
        }
        if (!done)
        {
            int global_var_index = getGlobal(token.data);
            if (global_var_index > -1)
            {
                instructions.emplace_back(GssInstruction::Type::push_global_by_index, global_var_index);
                done = true;
            }
        }
        if (!done)
            throw GssCompilerException(token, "Failed to find variable: " + token.data);
    }else{
        throw GssCompilerException(token, "Unexpected: " + token.toString());
    }
}

void GssCompiler::parseUnary()
{
    GssToken token = tokenizer.peek();
    if (token.type == GssToken::Type::exclamation)
    {
        tokenizer.get();
        parseValue();
        instructions.emplace_back(GssInstruction::Type::boolean_not);
        return;
    }
    else if (token.type == GssToken::Type::circumflex)
    {
        tokenizer.get();
        parseValue();
        instructions.emplace_back(GssInstruction::Type::binary_not);
        return;
    }
    else if (token.type == GssToken::Type::minus)
    {
        tokenizer.get();
        parseValue();
        if (instructions.back().type == GssInstruction::Type::push_int)
            instructions.back().data.i = -instructions.back().data.i;
        else if (instructions.back().type == GssInstruction::Type::push_float)
            instructions.back().data.f = -instructions.back().data.f;
        else
            instructions.emplace_back(GssInstruction::Type::negative);
        return;
    }
    parseValue();
}

void GssCompiler::parseSubscript()
{
    parseUnary();
    while(true)
    {
        GssToken token = tokenizer.peek();
        if (token.type == GssToken::Type::left_square_bracket)
        {
            tokenizer.get();
            parseExpression();
            expect(GssToken::Type::right_square_bracket);
            instructions.emplace_back(GssInstruction::Type::get_from_table);
            continue;
        }
        if (token.type == GssToken::Type::dot)
        {
            tokenizer.get();
            string member = expect(GssToken::Type::name).data;
            instructions.emplace_back(GssInstruction::Type::get_from_table_by_string_table, addToStringTable(member));
            continue;
        }
        if (token.type == GssToken::Type::left_bracket)
        {
            tokenizer.get();
            int arg_count = 0;
            token = tokenizer.peek();
            if (token.type != GssToken::Type::right_bracket)
            {
                while(true)
                {
                    parseExpression();
                    arg_count++;
                    token = tokenizer.peek();
                    if (token.type == GssToken::Type::comma)
                    {
                        tokenizer.get();
                        continue;
                    }
                    break;
                }
            }
            instructions.emplace_back(GssInstruction::Type::call_function, arg_count);
            expect(GssToken::Type::right_bracket);
            continue;
        }
        break;
    }
}

void GssCompiler::parseBinaryOperator(unsigned int precedence)
{
    if (precedence >= binary_operators.size())
    {
        parseSubscript();
        return;
    }
    parseBinaryOperator(precedence + 1);
    GssToken token = tokenizer.peek();
    for(GssToken::Type type : binary_operators[precedence])
    {
        if (token.type == type)
        {
            tokenizer.get();
            parseBinaryOperator(precedence);
            switch(type)
            {
            case GssToken::Type::logical_or:
                instructions.emplace_back(GssInstruction::Type::boolean_or);
                break;
            case GssToken::Type::logical_and:
                instructions.emplace_back(GssInstruction::Type::boolean_and);
                break;
            case GssToken::Type::pipe:
                instructions.emplace_back(GssInstruction::Type::binary_or);
                break;
            case GssToken::Type::circumflex:
                instructions.emplace_back(GssInstruction::Type::binary_not_2);
                break;
            case GssToken::Type::ampersand:
                instructions.emplace_back(GssInstruction::Type::binary_and);
                break;
            case GssToken::Type::equal:
                instructions.emplace_back(GssInstruction::Type::boolean_equal);
                break;
            case GssToken::Type::notequal:
                instructions.emplace_back(GssInstruction::Type::boolean_not_equal);
                break;
            case GssToken::Type::less:
                instructions.emplace_back(GssInstruction::Type::boolean_less);
                break;
            case GssToken::Type::less_equal:
                instructions.emplace_back(GssInstruction::Type::boolean_less_equal);
                break;
            case GssToken::Type::greater:
                instructions.emplace_back(GssInstruction::Type::boolean_greater);
                break;
            case GssToken::Type::greater_equal:
                instructions.emplace_back(GssInstruction::Type::boolean_greater_equal);
                break;
            case GssToken::Type::left_shift:
                instructions.emplace_back(GssInstruction::Type::left_shift);
                break;
            case GssToken::Type::right_shift:
                instructions.emplace_back(GssInstruction::Type::right_shift);
                break;
            case GssToken::Type::plus:
                instructions.emplace_back(GssInstruction::Type::add);
                break;
            case GssToken::Type::minus:
                instructions.emplace_back(GssInstruction::Type::substract);
                break;
            case GssToken::Type::star:
                instructions.emplace_back(GssInstruction::Type::multiply);
                break;
            case GssToken::Type::slash:
                instructions.emplace_back(GssInstruction::Type::division);
                break;
            case GssToken::Type::percent:
                instructions.emplace_back(GssInstruction::Type::modulo);
                break;
            default:
                throw GssCompilerException(token, "Unknown operator: " + token.toString());
            }
        }
    }
}

void GssCompiler::parseExpression()
{
    parseBinaryOperator(0);
}

void GssCompiler::parseStatement(bool with_end_of_line)
{
    parseExpression();
    GssToken& token = tokenizer.get();
    if (token.type == GssToken::Type::assign)
    {
        GssInstruction last_instruction = instructions.back();
        instructions.pop_back();
        parseExpression();
        switch(last_instruction.type)
        {
        case GssInstruction::Type::push_global_by_index:
            instructions.emplace_back(GssInstruction::Type::assign_global_by_index, last_instruction.data.i);
            break;
        case GssInstruction::Type::get_from_table:
            instructions.emplace_back(GssInstruction::Type::assign_to_table, last_instruction.data.i);
            break;
        case GssInstruction::Type::get_from_table_by_string_table:
            instructions.emplace_back(GssInstruction::Type::assign_to_table_by_string_table, last_instruction.data.i);
            break;
        case GssInstruction::Type::push_local_by_index:
            instructions.emplace_back(GssInstruction::Type::assign_local_by_index, last_instruction.data.i);
            break;
        default:
            throw GssCompilerException(token, "Parsing error, impossible assignment (" + last_instruction.toString() + ")");
        }
        if (with_end_of_line)
            expect(GssToken::Type::end_of_line);
    }else if (with_end_of_line && token.type == GssToken::Type::end_of_line)
    {
        instructions.emplace_back(GssInstruction::Type::pop, 1);
    }else{
        throw GssCompilerException(token, "Unexpected: " + token.toString());
    }
}

GssToken GssCompiler::expect(GssToken::Type type)
{
    GssToken token = tokenizer.get();
    if (token.type != type)
    {
        GssToken expect_token;
        expect_token.type = type;
        throw GssCompilerException(token, "Unexpected: " + token.toString() + " expected: " + expect_token.toString());
    }
    return token;
}

int GssCompiler::addToStringTable(string value)
{
    for(unsigned int n=0; n<string_table.size(); n++)
    {
        if (string_table[n] == value)
            return n;
    }
    string_table.push_back(value);
    return string_table.size() - 1;
}

int GssCompiler::addGlobal(GssToken& reference_token, string name)
{
    for(const string& v : global_vars)
        if (v == name)
            throw GssCompilerException(reference_token, "Duplicate global variable definition: " + name);
    global_vars.push_back(name);
    return global_vars.size() - 1;
}

int GssCompiler::getGlobal(string name)
{
    int index = 0;
    for(const string& v : global_vars)
    {
        if (v == name)
            return index;
        index++;
    }
    return -1;
}
