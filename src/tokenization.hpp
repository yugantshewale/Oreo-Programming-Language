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
    int line;
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
        int line_cnt = 1;
        while(peak().has_value()){
            if(std::isalpha(peak().value()))//identifying keywords and identifiers
            {
                buf.push_back(consume());
                while(peak().has_value() and std::isalnum(peak().value())){
                    buf.push_back(consume());
                }
                if(buf == "exit"){
                    tokens.push_back({TokenType::exit,line_cnt});
                    buf.clear();
                    continue;
                }else if(buf == "let"){
                    tokens.push_back({TokenType::let,line_cnt});
                    buf.clear();
                    continue;
                }
                else if(buf == "if"){
                    tokens.push_back({TokenType::If,line_cnt});
                    buf.clear();
                    continue;
                }
                else if(buf == "elif"){
                    tokens.push_back({TokenType::elif,line_cnt});
                    buf.clear();
                    continue;
                }
                else if(buf == "else"){
                    tokens.push_back({TokenType::Else,line_cnt});
                    buf.clear();
                    continue;
                }
                else{//if it is not exit then it is an identifier
                        tokens.push_back({TokenType::ident,line_cnt, buf});
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
                tokens.push_back({TokenType::int_lit,line_cnt, buf});
                buf.clear();
                continue;
            }
            else if(peak().value() == '('){
                consume();
                tokens.push_back({TokenType::open_par,line_cnt});
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
                tokens.push_back({TokenType::close_par,line_cnt});
                continue;
            }
            else if(peak().value() == '='){
                consume();
                tokens.push_back({TokenType::equals,line_cnt});
                continue;
            }
            else if(peak().value() == ';'){
                consume();
                tokens.push_back({TokenType::semi, line_cnt});
                continue;
            }
            else if(peak().value() == '+'){
                consume();
                tokens.push_back({TokenType::plus, line_cnt});
            }
            else if(peak().value() == '*'){
                consume();
                tokens.push_back({TokenType::star, line_cnt});
            }
            else if(peak().value() == '-'){
                consume();
                tokens.push_back({TokenType::sub, line_cnt});
            }
            else if(peak().value() == '/'){
                consume();
                tokens.push_back({TokenType::div, line_cnt});
            }
            else if(peak().value() == '{'){
                consume();
                tokens.push_back({TokenType::open_curly, line_cnt});
            }
            else if(peak().value() == '}'){
                consume();
                tokens.push_back({TokenType::close_curly, line_cnt});
            }
            else if(peak().value() == '\n'){
                consume();
                line_cnt++;
            }
            else if(std::isspace(peak().value())){
                consume();
                continue;
            }
            else{
                std::cerr<<"Invalid Token \n";
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