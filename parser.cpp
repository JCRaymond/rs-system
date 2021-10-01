#ifndef PARSER_PARSE_CPP
#define PARSER_PARSE_CPP

#include <vector>
#include <exception>
#include <iostream>

#include "token.cpp"

namespace Parser {

   namespace {
      struct FormulaFactory;
   }

   enum FormulaType {
      Atom,
      Unary,
      Binary
   };

   struct Formula {
      friend FormulaFactory;
      FormulaType type;
      Tokenizer::Token token;

   protected:
      Formula(Tokenizer::Token token, FormulaType type) : token(token), type(type) {}

   private:
      Formula(Tokenizer::Token token) : Formula(token, FormulaType::Atom) {}
   };

   struct UnaryFormula : Formula {
      friend FormulaFactory;
      Formula* right;

   private:
      UnaryFormula(Tokenizer::Token token) : Formula(token, FormulaType::Unary), right(nullptr) {}
   };

   struct BinaryFormula : Formula {
      friend FormulaFactory;
      Formula* right;
      Formula* left;

   private:
      BinaryFormula(Tokenizer::Token token) : Formula(token, FormulaType::Binary), right(nullptr), left(nullptr) {}
   };

   namespace {
      struct FormulaFactory {
         static Formula* makeFormula(Tokenizer::Token token) {
            return new Formula(token);
         }

         static UnaryFormula* makeUnaryFormula(Tokenizer::Token token) {
            return new UnaryFormula(token);
         }

         static BinaryFormula* makeBinaryFormula(Tokenizer::Token token) {
            return new BinaryFormula(token);
         }
      };
   }

   void __print_formula(Parser::Formula* formula, Tokenizer::Token parent) {
      switch (formula->type) {
         case Parser::FormulaType::Atom:
            std::cout << formula->token.name();
            break;
         case Parser::FormulaType::Unary: {
               auto unary_op = (Parser::UnaryFormula*)formula;
               std::cout << unary_op->token.name();
               bool bigterm = unary_op->right->type != Parser::FormulaType::Atom; 
               if (bigterm)
                  std::cout << '(';
               __print_formula(unary_op->right, unary_op->token);
               if (bigterm)
                  std::cout << ')';
               break;
            }
         case Parser::FormulaType::Binary: {
               auto binary_op = (Parser::BinaryFormula*)formula;
               bool unwrapable = binary_op->token == Tokenizer::Token::Or or binary_op->token == Tokenizer::Token::And;
               bool unwrap = unwrapable and binary_op->token == parent;
               if (!unwrap)
                  std::cout << '(';
               __print_formula(binary_op->left, binary_op->token);
               std::cout << binary_op->token.name();
               __print_formula(binary_op->right, binary_op->token);
               if (!unwrap)
                  std::cout << ')';
               break;
            }
      }
   }

   void _print_formula(Parser::Formula* formula) {
      switch (formula->type) {
         case Parser::FormulaType::Atom:
            std::cout << formula->token.name();
            break;
         case Parser::FormulaType::Unary: {
               auto unary_op = (Parser::UnaryFormula*)formula;
               std::cout << unary_op->token.name();
               __print_formula(unary_op->right, unary_op->token);
               break;
            }
         case Parser::FormulaType::Binary: {
               auto binary_op = (Parser::BinaryFormula*)formula;
               __print_formula(binary_op->left, binary_op->token);
               std::cout << binary_op->token.name();
               __print_formula(binary_op->right, binary_op->token);
               break;
            }
      }
   }

   void print_formula(Parser::Formula* formula) {
      _print_formula(formula);
      std::cout << std::endl;
   }

   void print_formulas(std::vector<Formula*> formulas) {
      std::cout << '[';
      if (formulas.size() > 0) {
         print_formula(formulas[0]);
         for (int i = 1; i < formulas.size(); i++) {
            std::cout << ", ";
            print_formula(formulas[i]);
         }
      }
      std::cout << ']' << std::endl;
   }

   Formula* parse(std::vector<Tokenizer::Token> tokens) {
      std::vector<Tokenizer::Token> symbol_stack;
      std::vector<Formula*> formula_stack;
      for (auto token : tokens) {
         if (token.is_variable()) {
            formula_stack.push_back(FormulaFactory::makeFormula(token)); 
            continue;
         }
         if (token == Tokenizer::Token::RParen) {
            while (symbol_stack.size() > 0 and symbol_stack.back() != Tokenizer::Token::LParen) {
               Tokenizer::Token symbol = symbol_stack.back();
               symbol_stack.pop_back();

               int operands = (symbol == Tokenizer::Token::Not) ? 1 : 2;
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
               and symbol_stack.back() != Tokenizer::Token::LParen 
               and precedence < symbol_stack.back().precedence()){
            Tokenizer::Token symbol = symbol_stack.back();
            symbol_stack.pop_back();

            int operands = (symbol == Tokenizer::Token::Not) ? 1 : 2;
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
      while (symbol_stack.size() > 0 and symbol_stack.back() != Tokenizer::Token::LParen) {
         Tokenizer::Token symbol = symbol_stack.back();
         symbol_stack.pop_back();

         int operands = (symbol == Tokenizer::Token::Not) ? 1 : 2;
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
      return formula_stack.back();
   }

}

#endif

