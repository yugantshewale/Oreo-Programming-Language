#pragma once
#include<iostream>
#include<variant>
#include "tokenization.hpp"
#include "arena.hpp"
struct NodeTermIntLit{
    Token int_lit;
};
struct NodeTermIdent{
    Token ident;// for decalration of variables or identifiers 

};
struct NodeExpr;
struct NodeTermParen{
    NodeExpr* expr;
};
struct NodeBinExprAdd{
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExprMul{
     NodeExpr* lhs;
     NodeExpr* rhs;
};
struct NodeBinExprSub{
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExprDiv{
    NodeExpr* lhs;
    NodeExpr* rhs;
};
struct NodeBinExpr{
    std::variant<NodeBinExprAdd*,NodeBinExprMul*,NodeBinExprSub*,NodeBinExprDiv*> expr;
    //NodeBinExprAdd* add;
};
struct NodeTerm{
    std::variant<NodeTermIntLit*,NodeTermIdent*,NodeTermParen*> expr;
};
struct NodeExpr {
    std::variant<NodeTerm*,NodeBinExpr*> expr; //int_lit or ident
};
struct NodeStmtExit{
    NodeExpr* expr; //exit(Expr) is the syntax
};
struct NodeStmtLet{
    Token ident;//variable name
    NodeExpr* expr;//expression to assign to the variable
};
struct NodeStmt;
struct NodeStmtScope{
    std::vector<NodeStmt*> stmts;
};
struct NodeStmtIf{
    NodeExpr* expr;
    NodeStmtScope* scope;
};
struct NodeStmt{
    std::variant<NodeStmtExit*,NodeStmtLet*,NodeStmtScope*,NodeStmtIf*> stmt;
};
struct NodeProg{
    std::vector<NodeStmt> stmts;
};

class Parser{
    public:
        inline explicit Parser(std::vector<Token> tokens):m_tokens(std::move(tokens))
                        ,m_allocator(4*1024*1024)
        {}
        std::optional<NodeTerm*> parse_term(){
            if(auto int_lit = try_consume(TokenType::int_lit)){
                auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
                term_int_lit->int_lit = int_lit.value();
                auto term = m_allocator.alloc<NodeTerm>();
                term->expr = term_int_lit;
                return term;
            }
            else if(auto ident = try_consume(TokenType::ident))
            {
                auto term_ident = m_allocator.alloc<NodeTermIdent>();
                term_ident->ident = ident.value();
                auto term = m_allocator.alloc<NodeTerm>();
                term->expr = term_ident;
                return term;
                //return NodeExpr{.expr = NodeExprIdent{.ident = consume()}};
            }else if(auto open_paren = try_consume(TokenType::open_par)){
                auto expr = parse_expr(); 
                if(!expr.has_value()){
                    std::cerr<<"Expected an expression\n";
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_par,"Expected ')' ");
                auto term_paren = m_allocator.alloc<NodeTermParen>();
                term_paren->expr = expr.value();   
                auto term =  m_allocator.alloc<NodeTerm>();
                term->expr = term_paren;
                return term;
            }
            else{
                return {};
            }
        }
        std::optional<NodeExpr*> parse_expr(int min_prec=0){
            std::optional<NodeTerm*> term_lhs = parse_term();
            if(!term_lhs.has_value())
                return{};
            auto expr_lhs = m_allocator.alloc<NodeExpr>();
            expr_lhs->expr = term_lhs.value();
            while(true){
                std::optional<Token> curr_tok =peak();
                std::optional<int> prec;
                if(curr_tok.has_value() ){
                    prec = bin_prec(curr_tok->type);
                    if(!prec.has_value() or prec < min_prec){
                        break;
                    }
                }else{
                    break;
                }
                Token op = consume();//operator
                int next_min_prec = prec.value()+1;
                auto expr_rhs = parse_expr(next_min_prec);
                if(!expr_rhs.has_value()){
                    std::cerr<<"Unable to parse expr\n";
                    exit(EXIT_FAILURE);
                }
                auto Expr = m_allocator.alloc<NodeBinExpr>();
                auto expr_lhs2 = m_allocator.alloc<NodeExpr>();;
                if(op.type == TokenType::plus){
                    auto add = m_allocator.alloc<NodeBinExprAdd>();
                    expr_lhs2->expr = expr_lhs->expr;
                    add->lhs = expr_lhs2;
                    add->rhs = expr_rhs.value();
                    Expr->expr = add;
                }else if(op.type == TokenType::star){
                    auto mul = m_allocator.alloc<NodeBinExprMul>();
                    expr_lhs2->expr = expr_lhs->expr;
                    mul->lhs = expr_lhs2;
                    mul->rhs = expr_rhs.value();
                    Expr->expr = mul;
                }else if(op.type == TokenType::sub){
                    auto sub = m_allocator.alloc<NodeBinExprSub>();
                    expr_lhs2->expr = expr_lhs->expr;
                    sub->lhs = expr_lhs2;
                    sub->rhs = expr_rhs.value();
                    Expr->expr = sub;
                }else if(op.type == TokenType::div){
                    auto div = m_allocator.alloc<NodeBinExprDiv>();
                    expr_lhs2->expr = expr_lhs->expr;
                    div->lhs = expr_lhs2;
                    div->rhs = expr_rhs.value();
                    Expr->expr = div;
                }
                expr_lhs->expr = Expr;
            }
        return expr_lhs;        
        }
        std::optional<NodeStmtExit> parse_exit() {
            std::optional<NodeStmtExit> exit_node;
            while(peak().has_value()){
                if(peak().value().type==TokenType::exit && peak(1).has_value() && peak(1).value().type == TokenType::open_par){
                    consume();
                    consume();
                    if(auto node_expr = parse_expr()){
                        exit_node = NodeStmtExit{.expr = node_expr.value()};
                    }else{
                        std::cerr<<"Invalid Expression ";
                        exit(EXIT_FAILURE);
                    }
                    if(peak().has_value() && peak().value().type == TokenType::close_par){
                        consume(); //consume the close par
                    }else{
                        std::cerr<<"Invalid Expression missing ')' before ':' \n";
                        exit(EXIT_FAILURE);
                    }
                    if(peak().has_value() && (peak().value().type == TokenType::semi)){
                        consume();
                    }else{
                        std::cerr<<"Invalid Expression missing ';' \n";
                        exit(EXIT_FAILURE);
                    }

                }
            }
            //m_index = 0;
            return exit_node;
        }
        std::optional<NodeStmtScope*> parse_scope(){
            if(!try_consume(TokenType::open_curly).has_value())
                return {};
            auto scope = m_allocator.alloc<NodeStmtScope>();
            while(auto stmt = parse_stmt()){
                scope->stmts.push_back(stmt.value());
            }
            try_consume(TokenType::close_curly,"Expected '}' ");
            return scope;   
        }
        std::optional<NodeStmt*> parse_stmt() {
            //Exit statement parsing
            if(peak().value().type==TokenType::exit && peak(1).has_value() && peak(1).value().type == TokenType::open_par){
                consume();
                consume();//consume the open par
                auto stmt_exit = m_allocator.alloc<NodeStmtExit>();
                //NodeStmtExit stmt_exit;
                if(auto node_expr = parse_expr()){//evaluate the expression
                    stmt_exit->expr = node_expr.value();
                   // stmt_exit = NodeStmtExit{.expr = node_expr.value()};
                }else{
                    std::cerr<<"Invalid Expression ";
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_par,"Invalid Expression missing ')' before ':'");
                try_consume(TokenType::semi,"Invalid Expression missing ';'");
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->stmt = stmt_exit;
                return stmt;
                //return NodeStmt{.stmt = stmt_exit};
            }
            else if(peak().has_value() and peak().value().type == TokenType::let and peak(1).has_value() and 
            peak(1).value().type == TokenType::ident and peak(2).has_value() and peak(2).value().type == TokenType::equals){
                consume();
                auto stmt_let = m_allocator.alloc<NodeStmtLet>();
                stmt_let->ident = consume();
                //NodeStmtLet stmt_let=NodeStmtLet{.ident = consume()};
                consume();//= sign consumed
                if(auto node_expr = parse_expr()){//evaluate the expression
                    //stmt_let = NodeStmtLet{.expr = node_expr.value()}; this means initializitn stmt_let with new obj
                    stmt_let->expr = node_expr.value();
                    //stmt_let.expr = node_expr.value();
                }else{
                    std::cerr<<"Invalid Expression ";
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::semi,"Invalid Expression missing ';'");
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->stmt = stmt_let;
                return stmt;
            }else if(peak().has_value() and peak().value().type == TokenType::open_curly)
            {
                if(auto scope = parse_scope()){
                    auto stmt = m_allocator.alloc<NodeStmt>();
                    stmt->stmt=scope.value();
                    return stmt;
                }else{
                    std::cerr<<"Invalid Scope\n";
                    exit(EXIT_FAILURE);
                }
            }
            else if(auto If = try_consume(TokenType::If)){
                try_consume(TokenType::open_par,"Expected '('");
                auto stmt_if = m_allocator.alloc<NodeStmtIf>();
                if(auto exp = parse_expr()){
                    stmt_if->expr = exp.value();
                }else{
                    std::cerr<<"Invalid Expression\n";
                    exit(EXIT_FAILURE);
                }
                try_consume(TokenType::close_par,"Expected ')'");
                if(auto scope = parse_scope())
                {
                    stmt_if->scope = scope.value();
                }else{
                    std::cerr<<"Invalid Scope of if statement\n";
                }
                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->stmt = stmt_if ;
                return stmt;

            }
            else{
                return {};
            }
        }
        std::optional<NodeProg> parse_prog() {
            NodeProg prog;
            while(peak().has_value())
            {
                if(auto stmt = parse_stmt())
                {
                    prog.stmts.push_back(*stmt.value());
                }else{
                    std::cerr<<"Invalid Statement \n";
                    exit(EXIT_FAILURE);
                }
            }
            return prog;
        }
    private:
        [[nodiscard]] inline std::optional<Token> peak(int offset = 0) const{
            if(m_index+offset >=  m_tokens.size()){
                return {};//out of range
            }else {
                return m_tokens.at(m_index+offset);
            }
        }
        inline Token try_consume(TokenType type,std::string err_msg){
            if(peak().has_value() and peak().value().type == type){
                return consume();
            }else{
                std::cerr<<err_msg<<"\n";
                exit(EXIT_FAILURE);
            }
        }
        inline std::optional<Token> try_consume(TokenType type){
            if(peak().has_value() and peak().value().type == type){
                return consume();
            }else{
                return {};
            }
        }
        inline Token consume(){
            return m_tokens.at(m_index++); 
        }
        const std::vector<Token>m_tokens;
        size_t m_index = 0;
        ArenaAllocator m_allocator;
};