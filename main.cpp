#include "rs-system.cpp"
#include "timer.hpp"

#include <iostream>
#include <string>
#include <vector>

int main() {
   // The vector below contains the formula(s) to handle, written in infix
   // notation. Having more than 1 formula is equivalent to connecting all
   // of the formulas by `v`.
   // 
   // Syntax for formulas:
   //    ~  - NOT
   //    ^  - AND
   //    v  - OR
   //    -> - IMPLIES
   //    () - standard parenthesis
   //    variables can be any sequence of characters a-z or A-Z.
   //
   //    Whitespace is ignored.
   //    Upon lack of parenthesis, standard order of operations is used:
   //    NOT, AND, OR, then IMPLIES. Many ORs or ANDs stringed together is valid:
   //    "a v b v c" or "a ^ b ^ c". IMPLIES is right resolved: "a -> b -> c" means
   //    "a -> (b -> c)".

   std::vector<std::string> formulas;
   int n;
   std::cout << "How many formulas: ";
   std::cin >> n;
   for (int i = 0; i < n; i++) {
      std::string formula;
      std::cout << "Formula #" << (i+1) << ": ";
      std::cin >> formula;
      formulas.push_back(formula);
   }

   timer t;
   // First argument is an iterable of strings containing the formulas
   // Second argument (option) is whether or not to print the leaves (default: false)
   bool is_tautology = RSSystem::is_tautology(formulas, true);
   double time = t.get_time();

   if (is_tautology)
      std::cout << "Formula is a tautology!" << std::endl;
   else
      std::cout << "Formula is NOT a tautology!" << std::endl;

   std::cout << "Time: " << time << std::endl;
}
