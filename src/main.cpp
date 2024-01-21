#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

#include "../include/parser.h"

/*
SIMPLE HELLO WORLD

fun printf(* byte): int;

fun main(): unit {
    let text: * byte = "hello world"; // stored globally, referenced here
    printf(text);
}

GRAPHICS HELLO WORLD

TODO: read https://github.com/KhronosGroup/SPIRV-Guide/tree/master

*/

int main(int, char**) {
    std::string filename = "../examples/test.ca";
    std::ifstream stream(filename);
    if (!stream) {
        std::cerr << "ERR: could not find file " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();

    const std::string program = buffer.str();
    Lexer lexer(program);
    
    std::vector<Token> tokens;
    auto err = lexer.tokenize(tokens);
    if (err != NO_ERR) return 1;

    std::unique_ptr<AST> ast;
    Parser parser(tokens);
    err = parser.parse(ast);
    if (err != NO_ERR) return 1;

    return 0;
}