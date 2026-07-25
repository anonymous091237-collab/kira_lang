#include "parser.hpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "../../../include/constants.hpp"
#include "../../../include/debug.h"

Node Parser::parseUnary() {
    if (text == "!" || text == "-" || text == "+") {
        Node node = Tree::createNode(NodeType::UNOP, text);
        move();
        node->child = parseUnary();  // recursive
        return node;
    }

    return parsePrimary();
}
Node Parser::parsePrimary() {
    if (tkType == TokenType::Number) {
        Node node = Tree::createNode(NodeType::NUM, text);
        move();
        return node;
    } else if (tkType == TokenType::String) {
        Node node = Tree::createNode(NodeType::STR, text);
        move();
        return node;
    } else if (tkType == TokenType::Identifier) {
        return parseIdentifiers();
    } else if (text == "(") {
        move();
        Node expr = parseExpression();
        if (text != ")") {
            errorLog("expected ')' after expression!");
        }
        move();
        return expr;
    }

    errorLog("unexpected token in primary expression!");
    return nullptr;
}

//
// ─────────────────────────────────────────────────────────────────────────
//
Node Parser::parseReturn() {
    Node ret = Tree::createNode(NodeType::RETSTMNT, text);

    move();  // consume 'return'

    if (text != ";") {
        std::cout << "before parseExpression\n";
        ret->child = parseExpression();
    }

    return ret;
}

Node Parser::parseParams_helper() {
    if (text.empty() || (text != "def" && text != "var")) {
        errorLog(R"(Expected keytext "def" or "var"!!)");
    }

    Node param = Tree::createNode(NodeType::PARAM, text);

    move();

    // param is not a constant
    if (!text.empty() && text == "var") {
        move();
        // param->flag = (uint8_t)Mutability::Variable;
    }

    if (text.empty() || tkType != TokenType::Identifier) {
        errorLog("invalid param name!");
    }

    param->child = Tree::createNode(NodeType::ID, text);

    move();
    if (text.empty() || text != ":") {
        errorLog("expected ':' after param name!");
    }
    move();
    if (text.empty() ||
        std::find(varTypes.begin(), varTypes.end(), text) == varTypes.end()) {
        errorLog("unknown param type !");
    }
    param->child->sibling = Tree::createNode(NodeType::TYPE, text);

    move();
    if (!text.empty() && text == "=") {
        move();
        param->child->sibling->sibling = parseExpression();
        std::cout << "after expression: " << (text.empty() ? text : "EOF")
                  << '\n';
    }

    return param;
}

//
// ─────────────────────────────────────────────────────────────────────────
//
Node Parser::parseParams() {
    Node parametres = Tree::createNode(NodeType::PARAMS);

    if (!text.empty() && text == ")") {
        DEBUG_PRINT("this function has no params!\n");

    } else {
        int num = 1;
        Node* Child;

        while (!text.empty() && text != ")") {
            Child = Tree::getChild(parametres, num);
            *Child = parseParams_helper();
            num++;
            if (text == ",") move();
        }
    }
    move();
    return parametres;
}

//
// ─────────────────────────────────────────────────────────────────────────
//
Node Parser::ParseWhilestatement() {
    if (text.empty() || text != "while") {
        errorLog("expected 'while'");
    }
    Node while_stmnt = Tree::createNode(NodeType::WHILESTMNT, text);

    move();

    Node* condition = Tree::getChild(while_stmnt, 1);
    (*condition) = parseExpression();

    Node* action = Tree::getChild(while_stmnt, 2);
    *action = parseBlock();

    return while_stmnt;
}
Node Parser::ParseIfstatement() {
    if (text.empty() || text != "if") {
        errorLog("expected 'if'");
    }
    Node if_stmnt = Tree::createNode(NodeType::IFSTMNT, text);
    move();
    Node* condition = Tree::getChild(if_stmnt, 1);
    (*condition) = parseExpression();
    // std::cout << "after condition[" << text << "]\n";
    Node* action = Tree::getChild(if_stmnt, 2);
    *action = parseBlock();

    std::cout << "after block[" << text << "]\n";

    if (text == "else") {
        Node* else_stmnt = Tree::getChild(if_stmnt, 3);
        (*else_stmnt) = Tree::createNode(NodeType::ELSE, text);
        move();
        if (text == "if") {
            Node* counter_action = Tree::getChild(*else_stmnt, 1);
            *counter_action = ParseIfstatement();
        } else if (text.empty() || text != "{") {
            errorLog("expected '{' after else statement");
        } else {
            Node* counter_action = Tree::getChild(*else_stmnt, 1);
            *counter_action = parseBlock();
        }
    }
    return if_stmnt;
}
Node Parser::parseFunDeclaration() {
    if (text.empty() || text != "fn") {
        errorLog(
            "Any function declaration should begin with the keytext \"fn\"");
    }
    Node myFunction = Tree::createNode(NodeType::FUNDEC, text);

    move();

    if (text.empty() || tkType != TokenType::Identifier) {
        errorLog("invalid function name!");
    }

    Node* Child = Tree::getChild(myFunction, 1);
    (*Child) = Tree::createNode(NodeType::ID, text);

    move();

    if (text.empty() || text != "(") {
        throw std::runtime_error("expected '(' after function name!\n");
    }
    move();

    Child = Tree::getChild(myFunction, 2);

    *Child = parseParams();

    if (text.empty() || text != ":") {
        errorLog("expected ':' after function parameters!");
    }

    move();

    if (text.empty() ||
        std::find(varTypes.begin(), varTypes.end(), text) == varTypes.end()) {
        errorLog("unknown function type !");
    }
    Child = Tree::getChild(myFunction, 3);
    *Child = Tree::createNode(NodeType::TYPE, text);

    move();
    Child = Tree::getChild(myFunction, 4);
    *Child = parseBlock();

    return myFunction;
}

//
// ─────────────────────────────────────────────────────────────────────────
//
Node Parser::parseVarDeclaration() {
    // assuming we came from a text let
    if (text.empty() || (text != "def" && text != "var")) {
        errorLog(R"(Expected keytext "def" or "var"!!)");
    }

    Node variable = Tree::createNode(NodeType::VARDEC, text);

    // variable is not a constant

    if (text == "var")
        variable->is_constant = false;

    else
        variable->is_constant = true;

    move();

    if (text.empty() || tkType != TokenType::Identifier) {
        errorLog("Invalid variable name!");
    }
    Node* Child;

    Child = Tree::getChild(variable, 1);
    (*Child) = Tree::createNode(NodeType::ID, text);

    move();
    if (text.empty() || (text != ":" && text != "->")) {
        errorLog("expected ':' or '->' after variable name!");
    }

    if (text == ":")
        variable->is_ptr = false;

    else
        variable->is_ptr = true;

    move();

    if (text.empty() || std::ranges::find(varTypes, text) == varTypes.end()) {
        errorLog("unknown variable type !");
    }

    Child = Tree::getChild(variable, 2);
    (*Child) = Tree::createNode(NodeType::TYPE, text);

    move();

    if (!text.empty() && text == "=") {
        move();
        Child = Tree::getChild(variable, 3);
        (*Child) = parseExpression();
    }

    return variable;
}
