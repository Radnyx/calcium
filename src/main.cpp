#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cassert>

#include "../include/parser.h"
#include "../include/irgenerator.h"

/*
SIMPLE HELLO WORLD

fun printf(* byte): int;

fun main(): unit {
    let text: * byte = "hello world"; // stored globally, referenced here
    printf(text);
}

GRAPHICS HELLO WORLD

TODO: read https://github.com/KhronosGroup/SPIRV-Guide/tree/master

Refer to https://github.com/KhronosGroup/SPIRV-Tools/blob/main/docs/syntax.md
For an API to generate and assembling SPIR-V code.

Follow up on this: https://community.khronos.org/t/is-there-an-api-for-generating-spir-v-assembly/110472

*/


int main(int, char**) {
    std::string filename = "../../examples/test.ca";
    std::ifstream stream(filename);
    if (!stream) {
        std::cerr << "ERR: could not find file " << filename << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << stream.rdbuf();

    std::string text = buffer.str();
    Program program(text);
    
    // ============ LEXER ============
    
    Lexer lexer(text);
    
    std::vector<Token> tokens;
    auto err = lexer.tokenize(tokens);
    if (err != ERR_NONE) return err;

    // ============ PARSER ============
    
    Parser parser(tokens);

    std::vector<std::unique_ptr<AST>> ast;
    err = parser.parse(ast);

    if (err != ERR_NONE) {
        if (!parser.eof()) {
            auto tok = parser.get();
            std::cerr << "ERR: line " << tok.line << ", column " << tok.column << std::endl;
        }

        return err;
    }

    // ============ SEMANTIC ANALYSIS ============

    // ============ CODE GENERATION ============

    IRGenerator irGenerator(program);
    irGenerator.generate(ast);

    return 0;
}