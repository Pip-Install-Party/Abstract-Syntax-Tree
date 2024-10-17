#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include <vector>
#include <string>
#include "tokenizer.h"
#include <queue>

namespace TokenTypes {
    constexpr const char* IDENTIFIER            = "IDENTIFIER";
    constexpr const char* INTEGER               = "INTEGER";
    constexpr const char* DOUBLE_QUOTE          = "DOUBLE_QUOTE";
    constexpr const char* SINGLE_QUOTE          = "SINGLE_QUOTE";
    constexpr const char* COMMA                 = "COMMA";
    constexpr const char* SEMICOLON             = "SEMICOLON";
    constexpr const char* ASSIGNMENT_OPERATOR   = "ASSIGNMENT_OPERATOR";
    constexpr const char* L_BRACE               = "L_BRACE";
    constexpr const char* R_BRACE               = "R_BRACE";
    constexpr const char* L_BRACKET             = "L_BRACKET";
    constexpr const char* R_BRACKET             = "R_BRACKET";
    constexpr const char* STRING                = "STRING";
    constexpr const char* PLUS                  = "PLUS";
    constexpr const char* MINUS                 = "MINUS";
    constexpr const char* LT_EQUAL              = "LT_EQUAL";
    constexpr const char* GT_EQUAL              = "GT_EQUAL";
    constexpr const char* GT                    = "GT";
    constexpr const char* LT                    = "LT";
    constexpr const char* BOOLEAN_EQUAL         = "BOOLEAN_EQUAL";
    constexpr const char* BOOLEAN_AND           = "BOOLEAN_AND";
    constexpr const char* ASTERISK              = "ASTERISK";
    constexpr const char* CHARACTER             = "CHARACTER";
}

using namespace TokenTypes;

// Usage example
extern std::string tokenType; //= TokenTypes::IDENTIFIER; // or TokenTypes::INTEGER

// The Parser class is responsible for utilizing a vector of tokens created with the tokenizer
// and converting it into a concrete syntax tree using a recursive descent parsing technique.
// This class utilizes a LCRS binary tree to store the CST.
class Parser 
{
private:
    Token *head;
    // Vector holding all of the tokens in order from the tokenizer.
    std::vector<Token> tokenList;

    // Variable to track the current position in the token list.
    int index;

    std::queue<Token*> tokenQueue;

    void state0();
    void state1(Token *lastToken);
    void state2(Token *lastToken);
    

public:
    // Parser construtor that is passed a vector of tokens from tokenizer.
    Parser(std::vector<Token>& tokenList);

    // Begin the parsing process.
    void begin(){ state0(); };
    void printTree(std::ofstream&);
};

#endif // PARSER_H
