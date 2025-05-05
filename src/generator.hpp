#pragma once
#include<iostream>
#include<fstream>
#include<sstream>
#include<cassert>
#include<algorithm>
#include "parser.hpp"
class Generator{
    public:
        inline Generator(NodeProg prog) :m_prog(std::move(prog)) 
        {
        }
        void gen_term(const NodeTerm* term){
            struct TermVisitor {
                Generator& gen;
                void operator()(const NodeTermIntLit* term_int_lit) const{
                    gen.m_output << "    mov rax, "<<term_int_lit->int_lit.value.value()<<"\n";
                    gen.push("rax");
                }
                void operator()(const NodeTermIdent* term_ident) const{
                    auto it = std::find_if(gen.m_vars.cbegin(),gen.m_vars.cend(),
                    [&](const Var& var){return var.name == term_ident->ident.value.value();});
                    if(it == gen.m_vars.cend())
                    {
                        std::cerr<<"Undeclared identifier : "<<term_ident->ident.value.value()<<"\n";
                        exit(EXIT_FAILURE);  
                    }
                    std::stringstream offset;
                    offset<<"QWORD [rsp + " <<(gen.m_stack_size-(*it).stack_loc-1)*8<<"]";
                    gen.push(offset.str());
                }
                void operator()(const NodeTermParen* term_paren) const {
                    gen.gen_expr(term_paren->expr);

                }
            };
            TermVisitor visitor({.gen = *this});
            std::visit(visitor, term->expr);
        }
        void gen_bin_expr(const NodeBinExpr* bin_expr){
            struct BinExprVisitor{
                Generator& gen;
                void operator()(const NodeBinExprSub* sub) const {
                    gen.gen_expr(sub->rhs);
                    gen.gen_expr(sub->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_output<<"    sub rax,rbx\n";
                    gen.push("rax");

                }
                void operator()(const NodeBinExprAdd* add) const {
                    gen.gen_expr(add->rhs);
                    gen.gen_expr(add->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_output<<"    add rax,rbx\n";
                    gen.push("rax");

                }
                void operator()(const NodeBinExprMul* mul) const {
                    gen.gen_expr(mul->rhs);
                    gen.gen_expr(mul->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_output<<"    mul rbx\n";
                    gen.push("rax");   
                }
                void operator()(const NodeBinExprDiv* div) const {
                    gen.gen_expr(div->rhs);
                    gen.gen_expr(div->lhs);
                    gen.pop("rax");
                    gen.pop("rbx");
                    gen.m_output<<"    div rbx\n";
                    gen.push("rax");

                }
            };
            BinExprVisitor visitor{.gen = *this};
            std::visit(visitor,bin_expr->expr); 
        }
        void gen_expr(const NodeExpr* expression){
            struct ExprVisitor{
                Generator& gen;
                void operator()(const NodeTerm* term) const{
                    gen.gen_term(term);
                }
                void operator()(const NodeBinExpr* bin_expr) const{
                    gen.gen_bin_expr(bin_expr);
                    // gen->gen_expr(bin_expr->add->lhs);
                    // gen->gen_expr(bin_expr->add->rhs);
                    // gen->pop("rax");
                    // gen->pop("rbx");
                    // gen->m_output<<"    add rax,rbx\n";
                    // gen->push("rax");

                }
            };
            ExprVisitor visitor{.gen = *this};
            std::visit(visitor,expression->expr);
        }
        void gen_scope(const NodeStmtScope* scope){
            begin_scope();
                    for(const NodeStmt* stmt : scope->stmts){
                        gen_stmt(stmt);
                    }
                    end_scope();
        }
        void gen_stmt(const NodeStmt* statement){
            struct StmtVisitor{
                Generator& gen;
                void operator()(const NodeStmtExit* stmt_exit) const {//abcd
                    gen.gen_expr(stmt_exit->expr);
                    gen.pop("rcx");
                    gen.m_output << "    call ExitProcess\n"; 
                }
                void operator()(const NodeStmtLet* stmt_let) {//wxyz
                    auto it = std::find_if(gen.m_vars.cbegin(),gen.m_vars.cend(),
                    [&](const Var& var){return var.name ==   stmt_let->ident.value.value();});
                    if(it != gen.m_vars.cend()) // check if var declared already
                    { 
                        std::cerr<<"Identifier "<<stmt_let->ident.value.value()<<" already used: \n";
                        exit(EXIT_FAILURE);
                    }
                    gen.m_vars.push_back({.name = stmt_let->ident.value.value(),.stack_loc = gen.m_stack_size});
                    gen.gen_expr(stmt_let->expr);
                }
                void operator()(const NodeStmtScope* scope) {
                    //gen->m_vars.at(2); /
                    gen.begin_scope();
                    for(const NodeStmt* stmt : scope->stmts){ //
                        gen.gen_stmt(stmt);
                    }
                    gen.end_scope();
                }
                void operator()(const NodeStmtIf* If) const {
                    gen.gen_expr(If->expr );
                    gen.pop("rax");
                    std::string label = gen.create_label();
                    gen.m_output <<"    test rax, rax\n";
                    gen.m_output <<"    jz "<<label<<"\n";
                    gen.gen_scope(If->scope);
                    gen.m_output <<label<<":\n";
                }
            };
            StmtVisitor visitor{.gen = *this};
            std::visit(visitor,statement->stmt);// if its exit it will call anything from abcd or if its let it will call from wxyz
        }


        [[nodiscard]] std::string gen_prog()  //const is used to make sure that the function does not modify the class members
        //[[nodiscard]] is used to make sure that the return value is not ignored
        {
            //std::stringstream output;
                m_output << "section .text\n";
                m_output << "    global main\n";
                m_output << "    extern ExitProcess\n";
                m_output << "\nmain:\n";
                //loop over statements
                for(const NodeStmt& stmt : m_prog.stmts){
                    gen_stmt(&stmt);
                }
                //Exit 0 if nothing was there else there will be an early return from program 
                m_output << "    mov rcx, 0\n";  
                m_output << "    call ExitProcess\n";
                return m_output.str();    
        }


    private:
    void push(const std::string reg) {
        m_output<<"    push "<<reg<<"\n";
        m_stack_size++;
    }
    void pop(const std::string reg) {
        m_output<<"    pop "<<reg<<"\n";
        m_stack_size--;
    }
    void begin_scope(){
        m_scope.push_back(m_vars.size());
    }
    void end_scope(){
        size_t cnt = m_vars.size()-m_scope.back();//pop count
        m_output<<"    add rsp, "<<cnt*8<<"\n";
        m_stack_size-= cnt;
        for(int i=0; i< cnt; i++){
            m_vars.pop_back();
        }
        m_scope.pop_back();
    }
    std::string create_label(){
        std::stringstream ss;
        ss <<"label"<<m_label_cnt++;
        return ss.str();
    }
    struct Var{
        std::string name;
        size_t stack_loc;
    };  
    const NodeProg m_prog;
    std::stringstream m_output;
    size_t m_stack_size = 0; // our very own stack ptr
    std::vector<Var> m_vars{}  ;
    std::vector<size_t> m_scope{};
    int m_label_cnt = 0;
   // std::map<std::string,Var> m_vars{};
};