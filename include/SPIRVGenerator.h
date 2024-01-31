#ifndef SPIRV_GENERATOR_H
#define SPIRV_GENERATOR_H
#include <vector>
#include <variant>
#include "Program.h"
#include "AST.h"

enum OpCode {
    OP_EXT_INST_IMPORT = 11,
    OP_MEMORY_MODEL = 14,
    OP_ENTRY_POINT = 15,
    OP_EXECUTION_MODE = 16,
    OP_CAPABILITY = 17,
    OP_TYPE_VOID = 19,
    OP_TYPE_FLOAT = 22,
    OP_TYPE_VECTOR = 23,
    OP_TYPE_POINTER = 32,
    OP_TYPE_FUNCTION = 33,
    OP_CONSTANT = 43,
    OP_FUNCTION = 54,
    OP_FUNCTION_END = 56,
    OP_VARIABLE = 59,
    OP_STORE = 62,
    OP_DECORATE = 71,
    OP_COMPOSITE_CONSTRUCT = 80,
    OP_LABEL = 248,
    OP_RETURN = 253
};

typedef uint32_t spirv_id;

class SPIRVGenerator {
public:
    SPIRVGenerator(const Program & program);
    std::vector<uint32_t> generate(const FunctionDefinitionAST * definition); 
private:
    const Program & program;
    std::vector<uint32_t> headerSection;
    std::vector<uint32_t> constantSection;
    std::vector<uint32_t> codeSection;

    // Number of IDs generated
    uint32_t ids;
    // Common IDs
    spirv_id entryPoint;
    spirv_id outputVariable;
    spirv_id voidType;
    spirv_id entryFunctionType;
    spirv_id floatType;
    spirv_id vec4Type;

    void generate(const std::unique_ptr<BodyAST> & body);
    spirv_id generate(const std::unique_ptr<ExpressionAST> & body);

    spirv_id requestId();

    template<OpCode opcode>
    inline std::vector<uint32_t> & section() {
        switch(opcode) {
        case OP_CONSTANT: case OP_VARIABLE:
            return constantSection;
        case OP_FUNCTION: case OP_FUNCTION_END: case OP_STORE:
        case OP_COMPOSITE_CONSTRUCT: case OP_LABEL: case OP_RETURN: 
            return codeSection;
        default: 
            return headerSection;
        }
    }

    static constexpr uint32_t op(uint32_t wordCount, OpCode code) {
        return (wordCount << 16) | code;
    }

    static uint32_t emitString(std::vector<uint32_t> & code, const char * s) {
        size_t length = std::strlen(s);
        size_t words = length / 4 + 1;
        for (size_t wordIndex = 0; wordIndex < words; wordIndex++) {
            uint32_t word = 0x00000000;
            size_t i = wordIndex * 4;
            if (i + 0 < length) word |= s[i + 0];
            if (i + 1 < length) word |= s[i + 1] << 8;
            if (i + 2 < length) word |= s[i + 2] << 16;
            if (i + 3 < length) word |= s[i + 3] << 24;    
            code.push_back(word);
        }
        return words;
    }

    template<OpCode opcode>
    void emit(std::vector<std::variant<uint32_t, const char *>> && args) {
        auto & code = section<opcode>();
        const size_t index = code.size();
        code.push_back(0);
        uint32_t words = 1;
        for (auto & var : args) {
            if (std::holds_alternative<const char *>(var)) {
                words += emitString(code, std::get<const char *>(var));
            } else {
                code.push_back(std::get<uint32_t>(var));
                words++;
            }
        }
        code[index] = op(words, opcode);
    }

    template<OpCode opcode, uint32_t... Args>
    void emit() {
        auto & code = section<opcode>();
        constexpr size_t size = sizeof...(Args);
        constexpr uint32_t args[size] = { Args... };
        code.push_back(op(size + 1, opcode));
        for (size_t i = 0; i < size; i++) {
            code.push_back(args[i]);
        }
    }

};

#endif // SPIRV_GENERATOR_H