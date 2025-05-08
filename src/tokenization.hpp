#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<optional>
enum class TokenType 
{
    exit,
    int_lit,
    semi,
    open_par,
    close_par,
    ident,
    let,
    equals,
    plus,
    star,
    sub,
    div,
    open_curly,
    close_curly,
    If,
    elif,
    Else,
}; 
std::optional<int> bin_prec(TokenType type){
    switch(type){
        case TokenType::sub:
        case TokenType::plus:
            return 0;
        case TokenType::div:
        case TokenType::star:
            return 1;
        default:
            return {};
    }
}
struct Token
{
    TokenType type;
    std::optional<std::string> value{};
};
class Tokenizer
{
public:
    inline Tokenizer(std::string src)
    : m_src(std::move(src))// construct with initalizer list
    {

    }
    inline std::vector<Token> tokenize() {
        std::vector<Token> tokens{};
        std::string buf;
        while(peak().has_value()){
            if(std::isalpha(peak().value()))//identifying keywords and identifiers
            {
                buf.push_back(consume());
                while(peak().has_value() and std::isalnum(peak().value())){
                    buf.push_back(consume());
                }
                if(buf == "exit"){
                    tokens.push_back({.type=TokenType::exit});
                    buf.clear();
                    continue;
                }else if(buf == "let"){
                    tokens.push_back({.type = TokenType::let});
                    buf.clear();
                    continue;
                }
                else if(buf == "if"){
                    tokens.push_back({.type = TokenType::If});
                    buf.clear();
                    continue;
                }
                else if(buf == "elif"){
                    tokens.push_back({.type = TokenType::elif});
                    buf.clear();
                    continue;
                }
                else if(buf == "else"){
                    tokens.push_back({.type = TokenType::Else});
                    buf.clear();
                    continue;
                }
                else{//if it is not exit then it is an identifier
                        tokens.push_back({.type=TokenType::ident,.value = buf});
                        buf.clear();
                        continue;
                }
            }    
            else if(std::isdigit(peak().value()))
            {
                buf.push_back(consume());
                while(peak().has_value() && std::isdigit(peak().value())){
                    buf.push_back(consume());
                    }
                tokens.push_back({.type=TokenType::int_lit, .value=buf});
                buf.clear();
                continue;
            }
            else if(peak().value() == '('){
                consume();
                tokens.push_back({.type=TokenType::open_par});
                continue;
            }
            else if(peak().value() == '/' and peak(1).has_value() and peak(1).value() == '/'){
                consume();
                consume();
                while(peak().has_value() and peak().value() != '\n')
                {
                    consume();
                }
            }
            else if(peak().value() == '/' and peak(1).has_value() and peak(1).value() == '*'){
                consume();
                consume();
                while(peak().has_value())
                {
                    if(peak().has_value() and peak().value() == '*' and peak().has_value() and peak().value() == '/')
                        break;
                    consume();
                }
                if(peak().has_value()){
                    consume();
                }
                if(peak().has_value()){
                    consume();
                }
            }
            else if(peak().value() == ')'){
                consume();
                tokens.push_back({.type=TokenType::close_par});
                continue;
            }
            else if(peak().value() == '='){
                consume();
                tokens.push_back({.type=TokenType::equals});
                continue;
            }
            else if(peak().value() == ';'){
                consume();
                tokens.push_back({.type=TokenType::semi});
                continue;
            }
            else if(peak().value() == '+'){
                consume();
                tokens.push_back({.type = TokenType::plus});
            }
            else if(peak().value() == '*'){
                consume();
                tokens.push_back({.type = TokenType::star});
            }
            else if(peak().value() == '-'){
                consume();
                tokens.push_back({.type = TokenType::sub});
            }
            else if(peak().value() == '/'){
                consume();
                tokens.push_back({.type = TokenType::div});
            }
            else if(peak().value() == '{'){
                consume();
                tokens.push_back({.type = TokenType::open_curly});
            }
            else if(peak().value() == '}'){
                consume();
                tokens.push_back({.type = TokenType::close_curly});
            }
            else if(std::isspace(peak().value())){
                consume();
                continue;
            }
            else{
                std::cerr<<"You messed up \n";
                exit(EXIT_FAILURE);
            }
        }
        m_index = 0;
        return tokens;        
    }

private:
    [[nodiscard]] inline std::optional<char> peak(int offset = 0) const{
        if(m_index+offset >=  m_src.length()){//out of range
            return {};
        }else {
            return m_src.at(m_index+offset);
        }
    }

    inline char consume(){
        return m_src.at(m_index++); 
    }
    const std::string m_src;
    size_t m_index = 0;
};