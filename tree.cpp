#include "tree.h"
#include <stack>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <sstream>

 // Make an output filestream
std::string filename = "AST_test_file_output.txt";
std::ofstream ASTOutput(filename);

// Prints an arrow for our output
void printArrow(int spaces){
    std::vector<char> arrowCharacters = {'|','|','|','|','v'};
    ASTOutput << '\n';
    for (int i =0; i < arrowCharacters.size(); i++){
        for (int j = 0; j < spaces; j++) {
            ASTOutput << ' ' ;
        }
            ASTOutput << arrowCharacters.at(i) << '\n';
    }
    for (int k = 0; k < spaces; k++) {
	    ASTOutput << ' ';
    }
} 

// Prints the abstract syntax tree to the provided output stream
void Tree::printTree(Token* head, Token* prevToken){
    
    int lineNumber = 1;
    bool ignore = true;

    if (head == nullptr){
        return;
    }
    if (head->getValue() == "{") {
        ignore = false;
        ASTOutput << "BEGIN BLOCK";
	spaceCount += 11;
    } else if (head->getValue() == "}") {
        ignore = false;
        ASTOutput << "END BLOCK";
	spaceCount += 9;
    } else if(head->getValue() == "else"){
        ignore = false;
        ASTOutput << "ELSE";
	spaceCount += 4;
    } else if(head->getValue() == "return"){
        ignore = false;
        ASTOutput << "RETURN";
	spaceCount += 6;
        head = head->getSibling();
        while(head->getValue() == "("){
            head = head->getSibling();
        }
        if (head->getSibling() != nullptr && contains(equationOperators, head->getSibling()->getValue())) {
            ASTOutput << " ----> ";
	    spaceCount += 7;
            head = handleAssignment(head); 
            prevToken = nullptr;
        }
    } else if (head->getValue() == "procedure" || head->getValue() == "function") {
        lineNumber += 6;
        ASTOutput << "DECLARATION";
	spaceCount += 11;
	printArrow(spaceCount);
        while(head->getSibling() != nullptr) {
            head = head->getSibling();
        }
    } else if (head->getValue() == "if"){
        ignore = false;
        ASTOutput << "IF ----> ";
	spaceCount += 9;
        // This needs to break out to handleAssignment();
        head = head->getSibling();
        head = handleAssignment(head); 
        prevToken = nullptr;
        
    } else if (head->getValue() == "while"){
        ignore = false;
        ASTOutput << "WHILE ----> ";
	spaceCount += 12;
        // This needs to break out to handleAssignment();
        head = head->getSibling();
        while(head->getSibling()->getValue() == "("){
            head = head->getSibling();
        }
        head = handleAssignment(head); 
        prevToken = nullptr;
        
    }else if (head->getValue() == "printf"){
        ignore = true;
        ASTOutput << "PRINTF";
	spaceCount += 6;
        while (head->getSibling() != nullptr) {
            head = head->getSibling();
            if (head->getType() != "L_PAREN" && head->getType() != "R_PAREN" && head->getType() != "DOUBLE_QUOTE" && head->getType() != "COMMA" && head->getType() != "SEMICOLON") {
                ASTOutput << " ----> ";
		spaceCount += 7;
                ASTOutput << head->getValue();
		spaceCount += head->getValue().length();
            }
        }
        lineNumber += 6;
        printArrow(spaceCount);
        prevToken = nullptr;
        
    } else if (head->getValue() == "for"){
        short forCount = 0;
        ignore = false;
        while (forCount < 3){
            ASTOutput << "FOR EXPRESSION " << forCount+1 << " ----> ";
	    spaceCount += 23;
            head = head->getSibling();
            head = handleAssignment(head); 
            prevToken = nullptr;
            if (forCount+1 < 3) {
                lineNumber += 6;
                printArrow(spaceCount);
	    }
            forCount++;
        }
       
    } else if (prevToken != nullptr && contains(varTypes, prevToken->getValue())) {
        ignore = false; // this was false but i think should be true 
	ASTOutput << "DECLARATION";
	spaceCount += 11;
        if (head->getSibling() != nullptr && head->getSibling()->getValue() == ",") {
            while(head->getSibling() != nullptr) {
                head = head->getSibling();
                if (head->getType() == "IDENTIFIER") {
        	    lineNumber += 6;
        	    printArrow(spaceCount);
		    ASTOutput << "DECLARATION";
		    spaceCount += 11;
                }
            } 
        } else {
            ignore = true; 
        }
    } else if (head->getType() == "IDENTIFIER") {  
        if (head->getSibling() != nullptr && head->getSibling()->getValue() == "=") {
            ignore = false;
            ASTOutput << "ASSIGNMENT" << " ----> ";
	    spaceCount += 17;
            // This needs to break out to handleAssignment();
            head = handleAssignment(head); 
            prevToken = nullptr;
        } else if (head->getSibling() != nullptr && isIndex(head->getSibling()->getType())){ // checks for "L_BRACKET" to see if assigning an index... if so print extra characters and resume as normal
            ASTOutput << "ASSIGNMENT" << " ----> ";
	    spaceCount += 17;
            head = handleAssignment(head);
            lineNumber += 6;
            printArrow(spaceCount);
        } else if (isFunction(head->getValue())){
            ignore = false;
            isCall = true;
            ASTOutput << "CALL";
	    spaceCount += 4;
            while(head->getSibling() != nullptr) {
                if (head->getType() == "IDENTIFIER" || head->getType() == "L_PAREN" || head->getType() == "R_PAREN"){
                    ASTOutput << " ----> " << head->getValue();
		    spaceCount += 7 + head->getValue().length();
                }
                head = head->getSibling();
            }
        }
    } else if (head->getSibling() == nullptr && head->getChild() != nullptr && head->getValue() == ";"){
        lineNumber += 6;
        printArrow(spaceCount);
    }
    if (head->getSibling() != nullptr) {
        if (!ignore){
            ASTOutput << " ----> ";
            ASTOutput << head->getValue();
	    spaceCount += 7 + head->getValue().length();
        }
        return printTree(head->getSibling(), head);
    } else if (head->getChild() != nullptr) {
        if (!ignore) {
        lineNumber += 6;
        printArrow(spaceCount);
	}
        isCall = false;
        return printTree(head->getChild(), head);
    } 
    return;
}

bool Tree::contains(const std::vector<std::string> reserved, std::string type){
    for (const auto& word : reserved) {
        if (type == word) {
            return true;
        }
    }
    return false;
}

// Function that handles functions and single normal and single array parameters
Token* Tree::handleFunction(Token *head, std::vector<Token*>& equationAsVec, bool &isFunctionCall) {
    head = head->getSibling(); // Should be getting L_PAREN of the function
    equationAsVec.push_back(head);
    isFunctionCall = true;  // For infixToPostfix to know we want to eventually output () or []

    while (head->getValue() != ")") {
        head = head->getSibling();
        // Array declaration check here, []
        if (head->getType() == "IDENTIFIER" && head->getSibling() != nullptr && head->getSibling()->getValue() == "[") {
            equationAsVec.push_back(head); // Identifier pushed back
            head = head->getSibling(); 
            
            equationAsVec.push_back(head); // Starting '[' pushed back
            head = head->getSibling();
            
            equationAsVec.push_back(head); // Array size or index here
            head = head->getSibling();
            
            equationAsVec.push_back(head); // Ending ']' pusehd back
        } 
        else if (head->getValue() != ")" && head->getValue() != "(") {
            equationAsVec.push_back(head); // Normal function parameter added ex.func(param)
        }
    }
    equationAsVec.push_back(head); 
    return head->getSibling(); 
}

Token* Tree::handleIndex(Token * head, std::vector<Token*>& equationAsVec) {
    while (head->getSibling() != nullptr && head->getSibling()->getType() != "R_BRACKET") {
        head = head->getSibling(); // Should be getting L_BRACKET of the function first time through
        equationAsVec.push_back(head); 
        head = head->getSibling();
    } 
    if (head->getSibling()->getType() == "R_BRACKET") {
        head = head->getSibling(); 
        equationAsVec.push_back(head); 
    } else {
	std::cerr << "error incorrectly formatted index" << std::endl;
	exit (1);
    }
    equationAsVec.push_back(head);  
    return head->getSibling(); 
}

Token* Tree::handleAssignment(Token* head) {
    std::vector<Token*> equationAsVec;
    Token* prev = nullptr;
    isCall = isFunction(head->getValue());
    bool isIndex = Tree::isIndex(head->getType());
    if (head->getValue() != "(") {
        equationAsVec.push_back(head);
    }

    head = head->getSibling();
    
    
    //while(head != nullptr && head->getValue() != ";" && (contains(equationOperators, head->getValue()) || head->getValue() != ";" || head->getValue() != "[" || head->getValue() != "]" || head->getType() == "IDENTIFIER" || head->getType() == "INTEGER" || head->getType() == "CHARACTER" || head->getType() == "STRING" || head->getType() == "DOUBLE_QUOTE")) {
    // Tokens processed until semicolon is reached
    while(head != nullptr && head->getValue() != ";") {
        
        // Process head as a function, since it is a function name followed by L_PAREN
        if (isFunction(head->getValue()) && head->getSibling() && head->getSibling()->getValue() == "("){
            equationAsVec.push_back(head); // Function's name
            head = handleFunction(head, equationAsVec, isCall); // Process function parameters
            
            // Checking if function has a '!'
            if (prev != nullptr && prev->getValue() == "!") {
                equationAsVec.push_back(prev);
                prev = nullptr; 
            }
        
        } 
        else if (isIndex) {
            equationAsVec.push_back(head); // Function's name
            head = handleIndex(head, equationAsVec);
        }
        // Process head as an identifier followed by '('
        else if (head->getType() == "IDENTIFIER" && head->getSibling() != nullptr && head->getSibling()->getValue() == "(" && isCall) {
            auto temp = prev;
            equationAsVec.push_back(head); // Add identifier
            prev = head;
            head = handleFunction(head, equationAsVec, isCall); // Process function

            // Checking if function has a '!'
            if (prev != nullptr && prev->getValue() == "!") {
                equationAsVec.push_back(prev);
                prev = nullptr;  
            }
        }
        // Checking for a '!' before function or expressions and set to prev until we process function or identifier
        else if (head->getValue() == "!") {
            prev = head;
        }
        // Checking for parentheses
        else if (head->getValue() == "(" || head->getValue() == ")") {
            equationAsVec.push_back(head);  
        }
        // Any other operand pushed here
        else if (head->getValue() != "(" && head->getValue() != "!"){  
            equationAsVec.push_back(head); 

            // Any '!' before operand
            if (prev != nullptr && prev->getValue() == "!"){
                equationAsVec.push_back(prev); 
                prev = nullptr;  
            } 
        }

        // Go to next token or break if none left
        if (head ->getSibling() != nullptr) {
            prev = head;
            head = head->getSibling();
        } 
        else {
            break;
        }
        
    } 
    
    // Convert infix to postfix
    std::vector<Token*> postFix = infixToPostfix(equationAsVec, isCall);
    for (int i = 0; i < postFix.size(); i++) {
        std::string tokenValue = postFix.at(i)->getValue();

        // Print token value directly and mark that a space is needed
        if (!tokenValue.empty()) {
            ASTOutput << tokenValue;
	    spaceCount += tokenValue.length();
            if (i != postFix.size() - 1) {
                ASTOutput << " ----> ";
		spaceCount += 7;
            }
        }
    }

    return head;
}

// Function to get precedence of operators
int Tree::getPrecedence(std::string op) {
    if (op == "+" || op == "-" || op == ">=" || op == "<=" || op == "==" || op == ">" || op == "<" || op == "==" || op == "&&" || op == "!" || op == "||") return 1;
    if (op == "*" || op == "/" || op == "%") return 2;
    return 0;
}

// Function to check if a character is an operator
bool Tree::isOperator(std::string c) {
    return c == "+" || c == "-" || c == "*" || c == "/" || c == "=" || c == "%";
}

std::vector<Token*> Tree::infixToPostfix(const std::vector<Token*> infix, bool isFunctionCall) {
    std::stack<Token*> operators;
    std::vector<Token*> postfix;

    for (Token* t : infix) {
        std::string tokenType = t->getType();
        std::string tokenValue = t->getValue();
        
        // If it's an operand, add it to the postfix output
        if (tokenType == "INTEGER" || tokenType == "STRING" || tokenType == "CHARACTER" || tokenType == "IDENTIFIER" || tokenType == "SINGLE_QUOTE" || tokenType == "DOUBLE_QUOTE" || tokenValue == "!"){
            postfix.push_back(t);
        }
        // If it's a left parenthesis, push it onto the stack
        else if (tokenType == "L_PAREN") {
            if (isFunctionCall) {
                postfix.push_back(t);  // Push '(' if it's part of a function call to output later
            } else {
                operators.push(t);  // Processing a subexpression here (not a function)
            }
        }
        // If it's a right parenthesis, pop until the left parenthesis
        else if (tokenType == "R_PAREN") {
            if (isFunctionCall) {
                postfix.push_back(t);  // Push ')' if it's part of a function call to output later
            } else {
                // Processing it as a regular subexpression, pop operators until '(' is found
                while (!operators.empty() && operators.top()->getValue() != "(") {
                    postfix.push_back(operators.top());
                    operators.pop();
                }
                if (!operators.empty()) {
                    operators.pop(); 
                }
            }
        }
        // Add array brackets here if it is a function call, if not, it is a normal operator.
        else if (tokenType == "L_BRACKET" || tokenType == "R_BRACKET") {
                postfix.push_back(t);  
        }
        // If it's an operator
        else if (isOperator(tokenValue) || tokenType == "GT_EQUAL" || tokenType == "LT_EQUAL" || tokenType == "GT" || tokenType == "LT" || tokenType == "BOOLEAN_EQUAL" || tokenType == "BOOLEAN_AND" || tokenType == "BOOLEAN_NOT") { // can code this to check for all explicit tokens like prev if checks above... can add other tokens to operators too 
            // Pop all operators with higher or equal precedence from the stack
            while (!operators.empty() && operators.top()->getType() != "L_PAREN" &&
                    getPrecedence(operators.top()->getValue()) >= getPrecedence(tokenValue)) {
                postfix.push_back(operators.top());
                operators.pop();
            }
            // Push the current operator onto the stack
            operators.push(t);
        }
    }

    // Pop any remaining operators in the stack
    while (!operators.empty()) {
        postfix.push_back(operators.top());
        operators.pop();
    }

    return postfix;
}


bool Tree::isFunction(std::string tokenName){
    Entry* tableHead = symbolTable->getHead();
    while(tableHead != nullptr) {
        if ((tableHead->getIDType() == "procedure" || tableHead->getIDType() == "function") && tableHead->getIDName() == tokenName) {
            return true; 
        }
        tableHead = tableHead->getNext();
    }
    return false;
}

bool Tree::isIndex(std::string headType) {
    if(headType == "L_BRACKET") {
        return true;
    }
    return false;
}
