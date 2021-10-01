#include "tokenizer.cpp"
#include "parser.cpp"

#include <iostream>

int main() {
   auto tokens = Tokenizer::tokenize("andy ^ ~bob ^ carter v bob ^ ~carter");
   Tokenizer::print_tokens(tokens);
   auto formula = Parser::parse(tokens);
   
   Parser::print_formula(formula);
}
