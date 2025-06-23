#ifndef PARSER_PARSE_CPP
#define PARSER_PARSE_CPP

#include <vector>
#include <exception>
#include <iostream>
#include <string>
#include <sstream>

#include "token.cpp"

namespace Parser {
    using Tokenizer::Token;

    struct FormulaFactory;

    enum FormulaType {
        Atom,
        Unary,
        Binary
    };

    struct Formula {
        friend FormulaFactory;
        const FormulaType type;
        Token token;

    protected:
        Formula(Token token, FormulaType type) : token(token), type(type) {}

    private:
        Formula(Token token) : Formula(token, FormulaType::Atom) {}
    };

    struct UnaryFormula : Formula {
        friend FormulaFactory;
        Formula* right;

    private:
        UnaryFormula(Token token) : Formula(token, FormulaType::Unary), right(nullptr) {}
    };

    struct BinaryFormula : Formula {
        friend FormulaFactory;
        Formula* right;
        Formula* left;

    private:
        BinaryFormula(Token token) : Formula(token, FormulaType::Binary), right(nullptr), left(nullptr) {}
    };

    struct FormulaFactory {
        static Formula* makeFormula(Token token) {
            return new Formula(token);
        }

        static UnaryFormula* makeUnaryFormula(Token token) {
            return new UnaryFormula(token);
        }

        static BinaryFormula* makeBinaryFormula(Token token) {
            return new BinaryFormula(token);
        }
    };

    std::string __to_str_formula(Parser::Formula* formula, Token parent) {
        std::stringstream ss;
        switch (formula->type) {
            case Parser::FormulaType::Atom:
                ss << formula->token.name();
                break;
            case Parser::FormulaType::Unary: {
                auto op = (Parser::UnaryFormula*)formula;
                ss << op->token.name();
                bool bigterm = op->right->type != Parser::FormulaType::Atom; 
                if (bigterm)
                    ss << '(';
                ss << __to_str_formula(op->right, op->token);
                if (bigterm)
                    ss << ')';
                break;
            }
            case Parser::FormulaType::Binary: {
                auto op = (Parser::BinaryFormula*)formula;
                bool unwrapable = op->token == Token::Or or op->token == Token::And;
                bool unwrap = unwrapable and op->token == parent;
                if (!unwrap)
                    ss << '(';
                ss << __to_str_formula(op->left, op->token);
                ss << op->token.name();
                ss << __to_str_formula(op->right, op->token);
                if (!unwrap)
                    ss << ')';
                break;
            }
        }
        return ss.str();
    }

    std::string to_str(Parser::Formula* formula) {
        if (formula == nullptr)
            return "";
        std::stringstream ss;
        switch (formula->type) {
            case Parser::FormulaType::Atom:
                ss << formula->token.name();
                break;
            case Parser::FormulaType::Unary: {
                auto op = (Parser::UnaryFormula*)formula;
                ss << op->token.name();
                ss << __to_str_formula(op->right, op->token);
                break;
            }
            case Parser::FormulaType::Binary: {
                auto op = (Parser::BinaryFormula*)formula;
                ss << __to_str_formula(op->left, op->token);
                ss << op->token.name();
                ss << __to_str_formula(op->right, op->token);
                break;
            }
        }
        return ss.str();
    }

    std::string to_str(std::vector<Formula*> formulas) {
        std::stringstream ss;
        ss << '[';
        if (formulas.size() > 0) {
            ss << to_str(formulas[0]);
            for (int i = 1; i < formulas.size(); i++) {
                ss << ", ";
                ss << to_str(formulas[i]);
            }
        }
        ss << ']';
        return ss.str();
    }

    std::string to_str(std::list<Formula*> formulas) {
        std::stringstream ss;
        ss << '[';
        auto it = formulas.begin();
        if (it != formulas.end()) {
            ss << to_str(*it);
            while (++it != formulas.end()) {
                ss << ", ";
                ss << to_str(*it);
            }
        }
        ss << ']';
        return ss.str();
    }

    Formula* parse(std::vector<Token> tokens) {
        std::vector<Token> symbol_stack;
        std::vector<Formula*> formula_stack;
        for (auto token : tokens) {
            if (token.is_variable()) {
                formula_stack.push_back(FormulaFactory::makeFormula(token)); 
                continue;
            }
            if (token == Token::RParen) {
                while (symbol_stack.size() > 0 and symbol_stack.back() != Token::LParen) {
                    Token symbol = symbol_stack.back();
                    symbol_stack.pop_back();

                    int operands = (symbol == Token::Not) ? 1 : 2;
                    if (formula_stack.size() < operands)
                        throw std::runtime_error("Syntax Error: Not Enough Operands");

                    Formula* right = formula_stack.back();
                    formula_stack.pop_back();
                    if (operands == 1) {
                        UnaryFormula* op = FormulaFactory::makeUnaryFormula(symbol);
                        op->right = right;
                        formula_stack.push_back(op);
                    } else {
                        Formula* left = formula_stack.back();
                        formula_stack.pop_back();
                        BinaryFormula* op = FormulaFactory::makeBinaryFormula(symbol);
                        op->right = right;
                        op->left = left;
                        formula_stack.push_back(op);
                    }
                }
                if (symbol_stack.size() == 0)
                    throw std::runtime_error("Syntax Error: Extra Right Parenthesis");
                symbol_stack.pop_back();
                continue;
            }
            int precedence = token.precedence();
            // Deal with precedence
            while(symbol_stack.size() > 0 
                    and symbol_stack.back() != Token::LParen 
                    and precedence < symbol_stack.back().precedence()){
                Token symbol = symbol_stack.back();
                symbol_stack.pop_back();

                int operands = (symbol == Token::Not) ? 1 : 2;
                if (formula_stack.size() < operands)
                    throw std::runtime_error("Syntax Error: Not Enough Operands");

                Formula* right = formula_stack.back();
                formula_stack.pop_back();
                if (operands == 1) {
                    UnaryFormula* op = FormulaFactory::makeUnaryFormula(symbol);
                    op->right = right;
                    formula_stack.push_back(op);
                } else {
                    Formula* left = formula_stack.back();
                    formula_stack.pop_back();
                    BinaryFormula* op = FormulaFactory::makeBinaryFormula(symbol);
                    op->right = right;
                    op->left = left;
                    formula_stack.push_back(op);
                }
            }
            symbol_stack.push_back(token);
        }
        while (symbol_stack.size() > 0 and symbol_stack.back() != Token::LParen) {
            Token symbol = symbol_stack.back();
            symbol_stack.pop_back();

            int operands = (symbol == Token::Not) ? 1 : 2;
            if (formula_stack.size() < operands)
                throw std::runtime_error("Syntax Error: Not Enough Operands");

            Formula* right = formula_stack.back();
            formula_stack.pop_back();
            if (operands == 1) {
                UnaryFormula* op = FormulaFactory::makeUnaryFormula(symbol);
                op->right = right;
                formula_stack.push_back(op);
            } else {
                Formula* left = formula_stack.back();
                formula_stack.pop_back();
                BinaryFormula* op = FormulaFactory::makeBinaryFormula(symbol);
                op->right = right;
                op->left = left;
                formula_stack.push_back(op);
            }
        }
        if (symbol_stack.size() > 0)
            throw std::runtime_error("Syntax Error: Extra Left Parenthesis");
        if (formula_stack.size() > 1)
            throw std::runtime_error("Syntax Error: Too Many Operands");
        if (formula_stack.size() == 0)
            return nullptr;
        return formula_stack.back();
    }

}

#endif

