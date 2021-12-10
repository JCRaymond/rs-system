
main: main.cpp rs-system.cpp token.cpp tokenizer.cpp parser.cpp bitset.hpp timer.hpp
	g++ -std=c++20 main.cpp -o main

main-release: main.cpp rs-system.cpp token.cpp tokenizer.cpp parser.cpp bitset.hpp timer.hpp
	g++ -std=c++20 -O3 main.cpp -o main
