#ifndef GSS_TOKENIZER_H
#define GSS_TOKENIZER_H

#include <exception>

#include "stringImproved.h"

class GssToken
{
public:
    enum class Type
    {
        invalid,
        end_of_file,
        end_of_line,
        name,
        number,
        string,
        left_bracket,
        right_bracket,
        left_square_bracket,
        right_square_bracket,
        colon,
        comma,
        semi_colon,
        plus,
        minus,
        star,
        slash,
        ampersand,
        exclamation,
        pipe,
        less,
        greater,
        assign,
        dot,
        percent,
        left_curly_bracket,
        right_curly_bracket,
        equal,
        notequal,
        less_equal,
        greater_equal,
        circumflex,
        left_shift,
        right_shift,
        plus_assign,
        minus_assign,
        star_assign,
        slash_assign,
        percent_assign,
        ampersand_assign,
        //exclamation_assign, same as "notequal"
        pipe_assign,
        circumflex_assign,
        left_shift_assign,
        right_shift_assign,
        logical_or,
        logical_and,
        at
    };
    Type type;
    string data;
    int line_number;
    int indent_amount;
    
    string toString();
};

class GssTokenizer
{
public:
    GssTokenizer(string code);
    
    GssToken& peek();
    GssToken& get();

private:
    int line_number;
    int indent_amount;
    unsigned int next_token_position;
    string code;
    GssToken current;
    GssToken next;
    
    void updateNext();
};

class GssTokenizerException : public std::exception
{
public:
    string message;
    
    GssTokenizerException(string message) : message(message) {}
};

#endif//GSS_TOKENIZER_H
