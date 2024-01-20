#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

#include "../include/lexer.h"

/*
SIMPLE HELLO WORLD

fun print(* byte): int;

fun main(): unit {
    let text: * byte = "hello world"; // stored globally, referenced here
    print(text);
}

GRAPHIC HELLO WORLD

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

    for (auto token : tokens) {
        std::string tok = program.substr(token.startIndex, token.endIndex - token.startIndex);
        std::cout  << tok << ' ';
    }

    return 0;
}