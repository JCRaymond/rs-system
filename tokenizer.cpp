#include <cctype>
#include <exception>
#include <sstream>

#include "token.cpp"

#include <iostream>

namespace Tokenizer {

   struct SyntaxException : public std::exception {
      std::string message;

      SyntaxException(std::string bad_string, int idx) {
         
         std::ostringstream buf;
         buf << "Unkown character '" << bad_string[idx] << "' at index " << idx << " of \"" << bad_string << "\".";
         message = buf.str();
      }

      const char* what() const throw() {
         return message.c_str();
      }
   };

   namespace {
      bool matches_substr(std::string s, int i, std::string substr) {
         for (int j = 0; i+j < s.length() and j < substr.length(); j++)
            if (s[i+j] != substr[j])
               return false;
         return true;
      }
   }
   std::vector<Token> tokenize(std::string s) {
      std::vector<Token> tokens;
      for (int i=0; i < s.length(); i++) {
         // Ignore vertical whitespace
         if (s[i] == ' ' or s[i] == '\t')
            continue;
         // Check if the next part of the string is a variable
         if (isalpha(s[i])) {
            int j;
            for (j = i+1; j < s.length() and isalpha(s[j]); j++);
            tokens.push_back(Token::Variable(s.substr(i, j-i)));
            i = j - 1;
            continue;
         }
         // Check if the next part of the string is a symbol
         for (auto symbol : Token::symbols) {
            if (matches_substr(s, i, symbol.name())) {
               i += symbol.name().length() - 1;
               tokens.push_back(symbol);
               goto nextiter;
            }
         }
         // If we reach here, s[i] is an invalid character   
         throw SyntaxException(s,i);
         nextiter:; 
      }
      return tokens;
   }

}
