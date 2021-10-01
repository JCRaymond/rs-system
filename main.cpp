#include "tokenizer.cpp"
#include "parser.cpp"

#include <iostream>

int main() {
   auto tokens = Tokenizer::tokenize("a ^ ~b ^ c v b ^ ~c");
   Tokenizer::print_tokens(tokens);
   auto formula = Parser::parse(tokens);
   
   Parser::print_formula(formula);
}
