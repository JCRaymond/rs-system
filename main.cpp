#include "rs-system.cpp"
#include "timer.hpp"

#include <iostream>
#include <string>
#include <vector>

int main() {
   std::vector<std::string> formulas = {"a->b", "~(a->b)"};
   timer t;
   RSSystem::is_tautology(formulas);
   double time = t.get_time();
   std::cout << "Time: " << time << std::endl;
}
