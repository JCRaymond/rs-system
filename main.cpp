#include "tokenizer.cpp"

#include <iostream>

int main() {
   auto tokens = Tokenizer::tokenize("a v b -> c");

   for (auto token : tokens)
      std::cout << token.name() << std::endl;

}
