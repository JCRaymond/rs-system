#ifndef TOKENIZER_TOKEN_CPP
#define TOKENIZER_TOKEN_CPP

#include <map>
#include <vector>
#include <string>

namespace Tokenizer {

    class Token {
        unsigned val;

    public:
        static std::vector<Token> symbols;

    private:
        static std::map<std::string, Token> name_token_map;
        static std::vector<std::string> token_names;

    public:
        static const Token LParen;
        static const Token Not;
        static const Token And;
        static const Token Or;
        static const Token Implies;
        static const Token RParen;

    private:
        static const unsigned variables_start;
        static unsigned next_value;

    public:
        static int num_variables() {
            return next_value - variables_start;
        }

        int id() const {
            return val - variables_start;
        }

        static Token Variable(std::string name) {
            auto search = name_token_map.find(name);
            if (search != name_token_map.end())
                return search->second;
            return Token(name);
        }

        Token(unsigned val) : val(val) {}

        Token(std::string name, bool is_symbol) {
            val = next_value;
            if (is_symbol)
                symbols.push_back(*this);
            else
                name_token_map.insert({name, Token(val)});

            token_names.push_back(name);
            next_value++;
        }

        Token(std::string name) : Token(name, false) {}

        bool operator==(const Token &token) const {
            return this->val == token.val;
        }

        unsigned value() const {
            return val;
        }

        const std::string name() const {
            return *const_cast<const std::string*>(&token_names[val]);
        }

        bool is_variable() const {
            return val >= variables_start;
        }

        bool is_symbol() const {
            return val < variables_start;
        }

        unsigned precedence() const {
            return variables_start - val;
        }
    };
    std::vector<Token> Token::symbols;
    std::map<std::string, Token> Token::name_token_map;
    std::vector<std::string> Token::token_names;
    unsigned Token::next_value = 0;
    const Token Token::LParen  = Token("(",true);
    const Token Token::Not     = Token("~",true);
    const Token Token::And     = Token("^",true);
    const Token Token::Or      = Token("v",true);
    const Token Token::Implies = Token("->",true);
    const Token Token::RParen  = Token(")",true);
    const unsigned Token::variables_start = Token::next_value;
};

#endif

