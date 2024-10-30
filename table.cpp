#include <iostream>
#include "table.h"

void Table::begin(Token* token){
    if (token->getType() == "IDENTIFIER") {
        Entry* tempEntry = nullptr;
        if (token->getSibling() != nullptr) {
            return build(token->getSibling(), token, tempEntry);
        } else if (token->getChild() != nullptr) {
            token = token->getChild();
            return build(token->getChild(), token, tempEntry);
        } else {
                return; 
        }
    } else {
         if (token->getSibling() != nullptr) {
                begin(token->getSibling());
            } else if (token->getChild() != nullptr) {
                begin(token->getChild());
            } else {
                return; 
        }
    }
}

void Table::build(Token* token, Token* prevToken, Entry* prevEntry) {
    if (prevToken == nullptr || token == nullptr) {
        return; // Ensure the tokens are valid before proceeding
    }
    Entry* entry = prevEntry;  // Start with the previous entry

    if (!pause && contains(prevToken->getValue()) && token->getValue() != "{" && token->getValue() != "}" && token->getValue() != "(" && token->getValue() != ")" && token->getValue() != "[" && token->getValue() != "]") {
        // Create a new entry based on the token values
        pause = true;
        if (prevToken->getValue() == "procedure" || prevToken->getValue() == "function") {
            scope++;
            if (prevToken->getValue() == "procedure") {
                exists(token, scope);
                entry = new Entry(token->getValue(), prevToken->getValue(), "NOT APPLICABLE", false, 0, scope);
                if (prevEntry == nullptr ){
                    head = entry;
                }
            } else {
                exists(token, scope);
                entry = new Entry(token->getSibling()->getValue(), prevToken->getValue(), token->getValue(), false, 0, scope);
                if (prevEntry == nullptr ){
                    head = entry;
                } else {
                    prevEntry->setNext(entry);  // Update the next pointer of prevEntry
                }
                return build(token->getSibling()->getSibling(), token->getSibling(), entry );
            }
        } else {
            exists(token, scope);
            entry = new Entry(token->getValue(), "datatype", prevToken->getValue(), false, 0, scope);
             if (prevEntry == nullptr ){
                head = entry;
            } 
            if (token->getSibling() != nullptr) {

                if (token->getSibling()->getValue() == "[") { 
                    if ( prevEntry != nullptr ) {
                        prevEntry->setNext(entry);  // Update the next pointer of prevEntry
                    }
                   return setArray(token->getSibling(), entry);
                } else if (token->getSibling()->getValue() == ",") { 
                    if ( prevEntry != nullptr ) {
                        prevEntry->setNext(entry);  // Update the next pointer of prevEntry
                    }
                    return handleInitList(prevToken->getValue(), token->getSibling()->getSibling(), entry);
                }
            }
        }
        if ( prevEntry != nullptr ) {
        prevEntry->setNext(entry);  // Update the next pointer of prevEntry
        }
    }
    //  Check if it's a function or procedure again and handle parameters
    else if (pause == true) {
        // Process parameters for procedure
            if (contains(prevToken->getValue()) && token->getType() == "IDENTIFIER"){
                exists(token, scope);
                Entry *newEntry = new Entry(token->getValue(), "parameter", prevToken->getValue(), false, 0, scope);
                // Process for array parameters
                if (token->getSibling()->getValue() == "[") {
                    token = token->getSibling()->getSibling();
                    newEntry->setIsArray();
                    newEntry->setArray(stoi(token->getValue()));
                }
                prevEntry->parameters.push_back(newEntry);
            }
    }

    // Recursively traverse siblings or children
    if (token->getSibling() != nullptr) {
        build(token->getSibling(), token, entry);  // Traverse sibling
    } else if (token->getChild() != nullptr) {
        pause = false;
        build(token->getChild(), token, entry);  // Traverse child
    }
}

// Checks if the token is a reserved word
bool Table::contains(std::string token){
    for (int i = 0; i < reserved.size(); i++) {
        if (token == reserved.at(i)){
            return true;
        }
    }
    return false;
}

// Checks if the token exists in the symbol table
void Table::exists(Token* token, int scope){
    auto temp = head;
    while(temp != nullptr) {
        if (temp->getIDName() == token->getValue() && !contains(token->getValue()) ){
            if (temp->getScope() == 0) {
                std::cerr << "Error on line: " << token->getLineNumber() << " variable \"" << token->getValue() << "\" already defined globally\n";
                exit(1);
            }
            if (temp->getScope() == scope) {
                std::cerr << "Error on line: " << token->getLineNumber() << " variable \"" << token->getValue() << "\" already defined locally\n";
                exit(1);
            }
        }
        for (int i = 0; i < temp->parameters.size(); i++) {
            if (temp->parameters.at(i)->getIDName() == token->getValue() && !contains(token->getValue()) ){
                if (temp->parameters.at(i)->getScope() == 0) {
                    std::cerr << "Error on line: " << token->getLineNumber() << " variable \"" << token->getValue() << "\" already defined globally\n";
                    exit(1);
                }
                  if (temp->parameters.at(i)->getScope() == scope) {
                    std::cerr << "Error on line: " << token->getLineNumber() << " variable \"" << token->getValue() << "\" already defined locally\n";
                    exit(1);
                }
            }
        }
        temp = temp->getNext();
    }
}


void Table::setArray(Token* token, Entry* entry){
    entry->setArray(std::stoi(token->getSibling()->getValue()));
    build(token->getSibling()->getSibling(), token, entry); 
}


void Table::handleInitList(std::string type, Token* token, Entry* prevEntry){
    Entry *entry = new Entry(token->getValue(), "datatype", type, false, 0, scope);
    prevEntry->setNext(entry);  // Update the next pointer of prevEntry
    if (token->getSibling()->getValue() == ";") {
        return build(token->getSibling()->getChild(), token->getSibling(), entry);
    }
    return handleInitList(type, token->getSibling()->getSibling(), entry);
}

void Table::printTable(){

    //make temp head pointer
    Entry* tempHead = this->head;

    while(tempHead != nullptr) {
        std::cout << "IDENTIFIER_NAME: " << tempHead->getIDName() << std::endl;
        std::cout << "IDENTIFIER_TYPE: " << tempHead->getIDType() << std::endl;
        std::cout << "DATATYPE: " << tempHead->getDType() << std::endl;
        std::cout << "DATATYPE_IS_ARRAY: ";
        tempHead->getIsArray() ?  std::cout << "yes" :  std::cout << "no";
        std::cout << "\nDATATYPE_ARRAY_SIZE: " << tempHead->getArraySize() << std::endl;
        std::cout << "SCOPE: " << tempHead->getScope() << std::endl << std::endl;
        tempHead = tempHead->getNext();
    }
}

// Print function for any Entries that have parameters
void Table::printParameters(){

    // make local head pointer
    Entry* tempHead = this->head;

    // Cycle through all entries in the table
    while(tempHead != nullptr) {
        if (tempHead->parameters.size() > 0) {  // If current entry has parameters...
            std::cout << "PARAMETER LIST FOR: " << tempHead->getIDName() << std::endl;
            for (int i = 0; i < tempHead->parameters.size(); i++) { // Print each parameter
                std::cout << "IDENTIFIER_NAME: " << tempHead->parameters.at(i)->getIDName() << std::endl;
                std::cout << "DATATYPE: " << tempHead->parameters.at(i)->getDType() << std::endl;
                std::cout << "DATATYPE_IS_ARRAY: ";
                tempHead->parameters.at(i)->getIsArray() ? std::cout << "yes" : std::cout << "no";
                std::cout << "\nDATATYPE_ARRAY_SIZE: " << tempHead->parameters.at(i)->getArraySize() << std::endl;
                std::cout << "SCOPE: " << tempHead->parameters.at(i)->getScope() << std::endl << std::endl;
            }
        }
        tempHead = tempHead->getNext();
    }
}