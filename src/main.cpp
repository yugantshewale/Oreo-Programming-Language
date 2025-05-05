#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<optional>
//#include "tokenization.hpp" // since generator.hpp includes parser.hpp and parser.hpp includes tokenization.hpp
//#include "parser.hpp"
#include "generator.hpp"
#include "arena.hpp"


int main(int argc,char* argv[]){ // argc stores cnt of arguments and argv stores number of strings it is a char pointer
    //the first arg is always the execuatable itself
    if(argc != 2){
        std::cerr<<"Incorrect Usage. Correct Usage is ...."<<"\n";
        std::cerr<<"proapp <input.yogo>"<<"\n";
        return EXIT_FAILURE;
    }
    //reading file
    std::string contents;
   
    {
        std::stringstream contents_stream;
        std::fstream input(argv[1], std::ios::in);
        contents_stream<<input.rdbuf();
        contents = contents_stream.str();
    }
    //displaying contents
    Tokenizer tokenizer(std::move(contents));
    std::vector<Token> tokens = tokenizer.tokenize();
    Parser parser(std::move(tokens)); 
    std::optional<NodeProg>prog = parser.parse_prog();
    if(!prog.has_value()){
        std::cerr<<"No exit statement found \n";
        exit(EXIT_FAILURE);
    }
    Generator generator(prog.value());
    {
        std::fstream file("../out.asm", std::ios::out);
        file<<generator.gen_prog();
    }
    system("C:\\msys64\\usr\\bin\\nasm.exe -f win64 ../out.asm -o out.obj");
    system("gcc out.obj -o out.exe -lkernel32 -lmsvcrt -mconsole");

    return EXIT_SUCCESS;
}