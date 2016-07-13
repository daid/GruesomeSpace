#include "gss_tokenizer.h"

GssTokenizer::GssTokenizer(string code)
{
    this->code = code;
    line_number = 1;
    indent_amount = 0;
    next.type = GssToken::Type::invalid;
    next_token_position = 0;
}

GssToken& GssTokenizer::peek()
{
    if (next.type == GssToken::Type::invalid)
        updateNext();
    return next;
}

GssToken& GssTokenizer::get()
{
    if (next.type == GssToken::Type::invalid)
        updateNext();
    current = next;
    next.type = GssToken::Type::invalid;
    return current;
}

static bool isWhitespace(char c)
{
    return c == ' ' || c == '\t';
}

static bool isAlpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_';
}

static bool isNum(char c)
{
    return c >= '0' && c <= '9';
}

static bool isAlphaNum(char c)
{
    return isAlpha(c) || isNum(c);
}

void GssTokenizer::updateNext()
{
    next.data = "";
    next.line_number = line_number;
    next.indent_amount = indent_amount;
    while(next_token_position < code.length() && isWhitespace(code[next_token_position]))
        next_token_position++;
    if (next_token_position == code.length())
    {
        //Force and end of line at end of the file before the end of file token.
        // This makes the compiler easier, as it just has to check of end_of_line for the end of statements, and end_of_file will always come at the beginning of a statement.
        next.type = GssToken::Type::end_of_line;
        next_token_position++;
        return;
    }
    if (next_token_position > code.length())
    {
        next.type = GssToken::Type::end_of_file;
        next_token_position++;
        return;
    }
    
    char c = code[next_token_position++];
    if (isAlpha(c))
    {
        next.type = GssToken::Type::name;
        next.data += c;
        while(next_token_position < code.length() && isAlphaNum(code[next_token_position]))
        {
            next.data += code[next_token_position];
            next_token_position++;
        }
        next.data = next.data.lower();
        return;
    }
    
    if (c == '\n')
    {
        next.type = GssToken::Type::end_of_line;
        line_number++;
        indent_amount = 0;
        while(next_token_position + indent_amount < code.length() && isWhitespace(code[next_token_position + indent_amount]))
            indent_amount++;
        return;
    }
    
    if (isNum(c))
    {
        next.type = GssToken::Type::number;
        next.data += c;
        while(next_token_position < code.length() && (isNum(code[next_token_position]) || code[next_token_position] == '.'))
        {
            next.data += code[next_token_position];
            next_token_position++;
        }
        return;
    }

    if (c == '"')
    {
        next.type = GssToken::Type::string;
        while(next_token_position < code.length() && code[next_token_position] != '"')
        {
            if (code[next_token_position] == '\\')
            {
                next.data += code[next_token_position];
                next_token_position++;
                if (next_token_position == code.length())
                    throw GssTokenizerException("Unterminated string constant");
            }
            if (code[next_token_position] == '\n')
                throw GssTokenizerException("Unterminated string constant");
            next.data += code[next_token_position];
            next_token_position++;
        }
        if (next_token_position == code.length())
            throw GssTokenizerException("Unterminated string constant");
        next_token_position++;
        return;
    }
    
    if (c == '(')
    {
        next.type = GssToken::Type::left_bracket;
        return;
    }
    if (c == ')')
    {
        next.type = GssToken::Type::right_bracket;
        return;
    }
    if (c == '[')
    {
        next.type = GssToken::Type::left_square_bracket;
        return;
    }
    if (c == ']')
    {
        next.type = GssToken::Type::right_square_bracket;
        return;
    }
    if (c == ':')
    {
        next.type = GssToken::Type::colon;
        return;
    }
    if (c == ',')
    {
        next.type = GssToken::Type::comma;
        return;
    }
    if (c == ';')
    {
        next.type = GssToken::Type::semi_colon;
        return;
    }
    if (c == '+')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::plus_assign;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::plus;
        return;
    }
    if (c == '-')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::minus_assign;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::minus;
        return;
    }
    if (c == '*')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::star_assign;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::star;
        return;
    }
    if (c == '/')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::slash_assign;
                return;
            }
            if (c == '/')
            {
                //Comment
                while(next_token_position < code.length() && code[next_token_position] != '\n')
                    next_token_position++;
                updateNext();
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::slash;
        return;
    }
    if (c == '&')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::ampersand_assign;
                return;
            }
            if (c == '&')
            {
                next.type = GssToken::Type::logical_and;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::ampersand;
        return;
    }
    if (c == '!')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::notequal;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::exclamation;
        return;
    }
    if (c == '|')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::pipe_assign;
                return;
            }
            if (c == '|')
            {
                next.type = GssToken::Type::logical_or;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::pipe;
        return;
    }
    if (c == '<')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::less_equal;
                return;
            }
            if (c == '<')
            {
                if (next_token_position < code.length())
                {
                    c = code[next_token_position++];
                    if (c == '=')
                    {
                        next.type = GssToken::Type::left_shift_assign;
                        return;
                    }
                    next_token_position--;
                }
                next.type = GssToken::Type::left_shift;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::less;
        return;
    }
    if (c == '>')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::greater_equal;
                return;
            }
            if (c == '>')
            {
                if (next_token_position < code.length())
                {
                    c = code[next_token_position++];
                    if (c == '=')
                    {
                        next.type = GssToken::Type::right_shift_assign;
                        return;
                    }
                    next_token_position--;
                }
                next.type = GssToken::Type::right_shift;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::greater;
        return;
    }
    if (c == '=')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::equal;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::assign;
        return;
    }
    if (c == '.')
    {
        next.type = GssToken::Type::dot;
        return;
    }
    if (c == '%')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::percent_assign;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::percent;
        return;
    }
    if (c == '{')
    {
        next.type = GssToken::Type::left_curly_bracket;
        return;
    }
    if (c == '}')
    {
        next.type = GssToken::Type::right_curly_bracket;
        return;
    }
    if (c == '^')
    {
        if (next_token_position < code.length())
        {
            c = code[next_token_position++];
            if (c == '=')
            {
                next.type = GssToken::Type::circumflex_assign;
                return;
            }
            next_token_position--;
        }
        next.type = GssToken::Type::circumflex;
        return;
    }
    if (c == '@')
    {
        next.type = GssToken::Type::at;
        return;
    }
    if (c == '#')
    {
        //Comment
        while(next_token_position < code.length() && code[next_token_position] != '\n')
            next_token_position++;
        updateNext();
        return;
    }
    throw GssTokenizerException("Unknown token " + string(c));
}

string GssToken::toString()
{
    switch(type)
    {
    case Type::invalid:
        return "[invalid]";
    case Type::end_of_file:
        return "[EOF]";
    case Type::end_of_line:
        return "[EOL]";
    case Type::name:
        return "Identifier:" + data;
    case Type::number:
        return "Number:" + data;
    case Type::string:
        return "\"" + data + "\"";
    case Type::left_bracket:
        return "(";
    case Type::right_bracket:
        return ")";
    case Type::left_square_bracket:
        return "[";
    case Type::right_square_bracket:
        return "]";
    case Type::colon:
        return ":";
    case Type::comma:
        return ",";
    case Type::semi_colon:
        return ";";
    case Type::plus:
        return "+";
    case Type::minus:
        return "-";
    case Type::star:
        return "*";
    case Type::slash:
        return "/";
    case Type::ampersand:
        return "&";
    case Type::exclamation:
        return "!";
    case Type::pipe:
        return "|";
    case Type::less:
        return "<";
    case Type::greater:
        return ">";
    case Type::assign:
        return "=";
    case Type::dot:
        return ".";
    case Type::percent:
        return "%";
    case Type::left_curly_bracket:
        return "{";
    case Type::right_curly_bracket:
        return "}";
    case Type::equal:
        return "==";
    case Type::notequal:
        return "!=";
    case Type::less_equal:
        return "<=";
    case Type::greater_equal:
        return ">=";
    case Type::circumflex:
        return "^";
    case Type::left_shift:
        return "<<";
    case Type::right_shift:
        return ">>";
    case Type::plus_assign:
        return "+=";
    case Type::minus_assign:
        return "-=";
    case Type::star_assign:
        return "*=";
    case Type::slash_assign:
        return "/=";
    case Type::percent_assign:
        return "%=";
    case Type::ampersand_assign:
        return "&=";
    case Type::pipe_assign:
        return "|=";
    case Type::circumflex_assign:
        return "^=";
    case Type::left_shift_assign:
        return "<<=";
    case Type::right_shift_assign:
        return ">>=";
    case Type::logical_and:
        return "&&";
    case Type::logical_or:
        return "||";
    case Type::at:
        return "@";
    }
    return "?";
}
