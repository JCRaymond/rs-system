#include "rs-system.cpp"
#include "timer.hpp"

#include <iostream>
#include <string>
#include <vector>

int main() {

   std::vector<std::string> formulas;
   int n = 30;
   for (int i = 1; i <= n; i++) {
      formulas.push_back("a->b");
      formulas.push_back("~(a->b)");
      timer t;
      RSSystem::is_tautology(formulas);
      double time = t.get_time();
      std::cout << "i: " << i << " - Time: " << time << std::endl;
   }
}
