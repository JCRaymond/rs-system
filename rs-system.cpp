#ifndef RS_SYSTEM_RS_SYSTEM_CPP
#define RS_SYSTEM_RS_SYSTEM_CPP

#include <vector>
#include <list>
#include <utility>
#include <iostream>

#include "parser.cpp"
#include "tokenizer.cpp"
#include "bitset.hpp"

namespace RSSystem {
   using Parser::Formula;
   using Parser::UnaryFormula;
   using Parser::BinaryFormula;
   using Parser::FormulaType;
   using Parser::FormulaFactory;

   using Tokenizer::Token;

   using FormulaStrings = std::vector<std::string>;
   using IndecomposableSequence = std::vector<Formula*>;
   using DecomposableSequence = std::list<Formula*>;
   using FormulaSequence = std::pair<IndecomposableSequence, DecomposableSequence>;

   Formula* negate(Formula* formula) {
      auto neg_formula = FormulaFactory::makeUnaryFormula(Tokenizer::Token::Not);
      neg_formula->right = formula;
      return neg_formula;
   }

   bool is_tautology(FormulaStrings str_formulas) {
      std::list<FormulaSequence> sequences;
      DecomposableSequence init_decomps;
      for (auto str_formula : str_formulas) {
         auto tokens = Tokenizer::tokenize(str_formula);
         auto formula = Parser::parse(tokens);
         init_decomps.push_back(formula);
      }
      IndecomposableSequence init_indecomps;
      sequences.emplace_back(init_indecomps,init_decomps);
      auto curr_seq_it = sequences.begin();

      char n;
      int i = 0;
      while (curr_seq_it != sequences.end()) {
         ++i;
         auto& [i_seq, d_seq] = *curr_seq_it;
         auto curr_formula_it = d_seq.begin();
         int j = 0;
         while (curr_formula_it != d_seq.end()) {
            auto curr_formula = *curr_formula_it;

//#define DEBUG
#ifdef DEBUG
            std::cout << std::endl;
            std::cout << i << ' ' << ++j << std::endl;
            Parser::print_formulas(i_seq);
            Parser::print_formulas(d_seq);
            Parser::print_formula(curr_formula);
            std::cin >> n;
#endif

            switch (curr_formula->type) {
               case FormulaType::Atom:
                  i_seq.push_back(curr_formula);
                  curr_formula_it = d_seq.erase(curr_formula_it);
                  break;
               case FormulaType::Unary: {
                  auto op = (UnaryFormula*)curr_formula;
                  if (op->token == Token::Not) {
                     auto sub_formula = op->right;
                     switch (sub_formula->type) {
                        case FormulaType::Atom:
                           i_seq.push_back(curr_formula);
                           curr_formula_it = d_seq.erase(curr_formula_it);
                           break;
                        case FormulaType::Unary: {
                           auto sub_op = (UnaryFormula*)sub_formula;
                           if (op->token == Token::Not) {
                              auto sub_sub_formula = op->right;
                              *curr_formula_it = sub_sub_formula;
                           }
                           break;
                        }
                        case FormulaType::Binary: {
                           auto op = (BinaryFormula*)sub_formula;
                           if (op->token == Token::And) {
                              d_seq.insert(curr_formula_it, negate(op->left));
                              *curr_formula_it = negate(op->right);
                              --curr_formula_it;
                           } else if (op->token == Token::Or) {
                              *curr_formula_it = negate(op->left);
                              sequences.emplace(curr_seq_it, i_seq, d_seq);
                              *curr_formula_it = negate(op->right);
                              --curr_seq_it;
                              goto break_outer;
                           } else if (op->token == Token::Implies) {
                              *curr_formula_it = op->left;
                              sequences.emplace(curr_seq_it, i_seq, d_seq);
                              *curr_formula_it = negate(op->right);
                              --curr_seq_it;
                              goto break_outer;
                           }
                           break;
                        }
                     }
                  }
                  break;
               }
               case FormulaType::Binary: {
                  auto op = (BinaryFormula*)curr_formula;
                  if (op->token == Token::And) {
                     *curr_formula_it = op->left;
                     sequences.emplace(curr_seq_it, i_seq, d_seq);
                     *curr_formula_it = op->right;
                     --curr_seq_it;
                     goto break_outer;
                  } else if (op->token == Token::Or) {
                     d_seq.insert(curr_formula_it, op->left);
                     *curr_formula_it = op->right;
                     --curr_formula_it;
                  } else if (op->token == Token::Implies) {
                     d_seq.insert(curr_formula_it, negate(op->left));
                     *curr_formula_it = op->right;
                     --curr_formula_it;
                  }
                  break;
               }
               
            }
         }
         ++curr_seq_it;
         break_outer:;
      }

      i = 0;
      std::vector<bool> in_pos;
      std::vector<bool> in_neg;
      for (auto [i_seq, d_seq] : sequences) {
         in_pos.reset();
         in_neg.reset();
         i++;
         std::cout << "Leaf number " << i << "/" << sequences.size() << ": ";
         Parser::print_formulas(i_seq);

      }
      return true;
   }
}

#endif

