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

   bool is_tautology(FormulaStrings str_formulas, bool print_leaves = false) {
      // A list of pairs of indecomposable and decomposable sequences
      std::list<FormulaSequence> sequences;

      // Convert the input strings to formula objects
      DecomposableSequence init_decomps;
      for (auto str_formula : str_formulas) {
         auto tokens = Tokenizer::tokenize(str_formula);
         auto formula = Parser::parse(tokens);
         init_decomps.push_back(formula);
      }
      IndecomposableSequence init_indecomps;
      sequences.emplace_back(init_indecomps,init_decomps);

      // Begin with the first sequence
      auto curr_seq_it = sequences.begin();

      // bitsets to keep track of which variables are in indecomposable
      // sequences. `in_pos` keeps track of what variables are in the
      // sequence, `in_neg` keeps track of what negation of variables are
      // in the sequence.
      bitset in_pos(Token::num_variables());
      bitset in_neg(Token::num_variables());
      bool is_fundamental;
      int i = 0;

      // Loop through all in/decomposable sequence pairs, as more may be created
      while (curr_seq_it != sequences.end()) {
         auto& [i_seq, d_seq] = *curr_seq_it;
         auto curr_formula_it = d_seq.begin();

         // Loop through the first potentiall decomposable sequence
         while (curr_formula_it != d_seq.end()) {
            auto curr_formula = *curr_formula_it;
            // Split into cases based on what the current formula type is
            switch (curr_formula->type) {
               // Case: next formula is a single variable - add to i_seq
               case FormulaType::Atom:
                  i_seq.push_back(curr_formula);
                  curr_formula_it = d_seq.erase(curr_formula_it);
                  break;
               // Case: next formula is the negation of something
               case FormulaType::Unary: {
                  auto op = (UnaryFormula*)curr_formula;
                  if (op->token == Token::Not) {
                     auto sub_formula = op->right;
                     // Split into cases based on what is being negated
                     switch (sub_formula->type) {
                        // Case: next formula is the negation of a variable - add to i_seq
                        case FormulaType::Atom:
                           i_seq.push_back(curr_formula);
                           curr_formula_it = d_seq.erase(curr_formula_it);
                           break;
                        // Case: next formula is negation of a negation - cancel them
                        case FormulaType::Unary: {
                           auto sub_op = (UnaryFormula*)sub_formula;
                           if (op->token == Token::Not) {
                              auto sub_sub_formula = sub_op->right;
                              *curr_formula_it = sub_sub_formula;
                           }
                           break;
                        }
                        // Case: next formula is negation of a binary operation
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

         // Full indecomposable sequence found, check if fundamental
         i++;

         // Reset bitsets to keep track of which variables and their
         // negations are in the indecomposable sequence.
         in_pos.reset();
         in_neg.reset();

         // This could actually be done more efficiently if I didn't
         // need to print out the whole sequence. I could use bitsets
         // instead of lists of formulas for i_seq above, which would
         // remove redundancies, and I could terminate processing a
         // sequence if the current indecomposable portion is fundamental
         for (auto formula : i_seq) {
            switch (formula->type) {
               // Case: variable
               case FormulaType::Atom: {
                  in_pos.set(formula->token.id());
                  break;
               }
               // Case: negation of a variable
               case FormulaType::Unary: {
                  in_neg.set(((UnaryFormula*)formula)->right->token.id());
                  break;
               }
            }
            if ((in_pos & in_neg).any())
               break;
         }

         is_fundamental = (in_pos & in_neg).any();
         if (print_leaves) {
            std::cout << "Leaf number " << i << ": " << Parser::to_str(i_seq) << " - " << (is_fundamental ? "fundamental" : "not fundamental") << std::endl;
            if (!is_fundamental)
               std::cout << "Full tree cannot be fundamental, terminating..." << std::endl;
         }

         // If the most recent indecomposable sequence is not fundamental, return false
         if (!(in_pos & in_neg).any())
            return false;

         // Go to next sequence if there is one
         ++curr_seq_it;
         break_outer:;
      }
      return true;
   }
}

#endif

