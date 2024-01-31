#include "../include/SPIRVGenerator.h"
#include <cassert>
#include <string>
#include <fstream>

constexpr char ENTRY_POINT_NAME[] = "main";
constexpr char EXTENSION_NAME[] = "GLSL.std.450";

const size_t ID_BOUND_INDEX = 3;

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

enum Capability {
    CAP_SHADER = 1
};

enum ExecutionModel {
    MODEL_FRAGMENT = 4
};

enum ExecutionMode {
    MODE_ORIGIN_UPPER_LEFT = 7
};

enum Decoration {
    DEC_LOCATION = 30
};

enum StorageClass {
    STORE_OUTPUT = 3
};

enum FunctionControl {
    FUNC_NONE = 0  
};

static constexpr uint32_t op(uint32_t wordCount, OpCode code) {
    return (wordCount << 16) | code;
}

static constexpr uint32_t words(const char * s) {
    return std::char_traits<char>::length(s) / 4 + 1;
}

template<const char * s>
static void emitString(std::vector<uint32_t> & code) {
    constexpr auto length = std::char_traits<char>::length(s);
    for (size_t wordIndex = 0; wordIndex < words(s); wordIndex++) {
        uint32_t word = 0x00000000;
        size_t i = wordIndex * 4;
        if (i + 0 < length) word |= s[i + 0];
        if (i + 1 < length) word |= s[i + 1] << 8;
        if (i + 2 < length) word |= s[i + 2] << 16;
        if (i + 3 < length) word |= s[i + 3] << 24;    
        code.push_back(word);
    }
}

template<OpCode opcode, uint32_t... Args>
static void emit(std::vector<uint32_t> & code) {
    constexpr uint32_t size = sizeof...(Args);
    constexpr uint32_t args[size] = { Args... };
    code.push_back(op(size, opcode));
    for (uint32_t i = 0; i < size; i++) {
        code.push_back(args[i]);
    }
}

template<OpCode opcode>
static void emit(std::vector<uint32_t> & code, std::vector<uint32_t> && args) {
    code.push_back(op(args.size() + 1, opcode));
    code.insert(code.end(), args.begin(), args.end());
}


SPIRVGenerator::SPIRVGenerator(const Program & program) : program(program) {
    ids = 0;
}

std::vector<uint32_t> SPIRVGenerator::generate(const FunctionDefinitionAST * definition) {
    auto name = program.extract(definition->prototype->name);

    headerSection = {
        0x07230203, // MAGIC NUMBER
        0x00010000, // VERSION NUMBER
        0x00000000, // VENDOR ID (can be reserved: https://github.com/KhronosGroup/SPIRV-Headers)
        0x00000000, // ID BOUND
        0x00000000
    };

    // OpCapability
    emit<OP_CAPABILITY, CAP_SHADER>(headerSection);
    // OpExtension
    // OpExtInstImport
    headerSection.push_back(op(2 + words(EXTENSION_NAME), OP_EXT_INST_IMPORT));
    headerSection.push_back(requestId());
    emitString<EXTENSION_NAME>(headerSection);
    // OpMemoryModel
    emit<OP_MEMORY_MODEL, 0, 1>(headerSection);
    // OpEntryPoint
    headerSection.push_back(op(4 + words(ENTRY_POINT_NAME), OP_ENTRY_POINT));
    headerSection.push_back(MODEL_FRAGMENT);
    headerSection.push_back(entryPoint = requestId());
    emitString<ENTRY_POINT_NAME>(headerSection);
    headerSection.push_back(outputVariable = requestId());
    // OpExecutionMode
    emit<OP_EXECUTION_MODE>(headerSection, { entryPoint, MODE_ORIGIN_UPPER_LEFT });
    // OpString
    // OpSourceExtension
    // OpSource
    // OpSourceContinued
    // OpName
    // OpMemberName
    // Annotation instructions
    emit<OP_DECORATE>(headerSection, {
        outputVariable,
        DEC_LOCATION,
        0 // location = 0
    });
    // Type declarations
    emit<OP_TYPE_VOID>(headerSection, { voidType = requestId() });
    emit<OP_TYPE_FUNCTION>(headerSection, { entryFunctionType = requestId(), voidType });
    emit<OP_TYPE_FLOAT>(headerSection, { floatType = requestId(), 32 /* bits */ });
    emit<OP_TYPE_VECTOR>(headerSection, {
        vec4Type = requestId(),
        floatType,
        4 // components
    });

    spirv_id outputPointerType;
    emit<OP_TYPE_POINTER>(headerSection, {
        outputPointerType = requestId(),
        STORE_OUTPUT,
        vec4Type // pointer to vec4
    });

    // Input/Output variables
    // TODO: generate from function prototype
    emit<OP_VARIABLE>(headerSection, {
        outputPointerType,
        outputVariable,
        STORE_OUTPUT
    });

    // Global variables

    // Function declarations
    
    // Function definitions
    emit<OP_FUNCTION>(codeSection, {
        voidType,
        entryPoint,
        FUNC_NONE,
        entryFunctionType
    });

    emit<OP_LABEL>(codeSection, { requestId() });

    generate(definition->body);

    emit<OP_RETURN>(codeSection);
    emit<OP_FUNCTION_END>(codeSection);

    headerSection[ID_BOUND_INDEX] = ids;

    std::vector<uint32_t> result(headerSection.begin(), headerSection.end());
    result.insert(result.end(), constantSection.begin(), constantSection.end());
    result.insert(result.end(), codeSection.begin(), codeSection.end());

    std::ofstream fs(name + ".spv", std::ios::out | std::ios::binary);
    fs.write(reinterpret_cast<const char *>(result.data()), result.size() * 4);
    fs.close();

    // TODO: optimizer pass, see: https://github.com/KhronosGroup/SPIRV-Tools/blob/main/examples/cpp-interface/main.cpp

    return result;
}

void SPIRVGenerator::generate(const std::unique_ptr<BodyAST> & body) {
    for (auto & statement : body->statements) {
        if (statement->isReturn()) {
            auto returnStatement = reinterpret_cast<const ReturnAST *>(statement.get());
            emit<OP_STORE>(codeSection, {
                outputVariable, 
                generate(returnStatement->expression)
            });
        }
    }
}

spirv_id SPIRVGenerator::generate(const std::unique_ptr<ExpressionAST> & expression) {
    switch (expression->getExpressionID()) {
        case EXPRESSION_FUNCTION_CALL:
        {
            auto functionCall = reinterpret_cast<const FunctionCallAST *>(expression.get());
            auto name = program.extract(functionCall->name);
            assert(name == "vec4");
            // TODO: handle other function calls and constructors
            // TODO: handle constant vec4 (all arguments constant)

            spirv_id id = requestId();
            codeSection.push_back(op(7, OP_COMPOSITE_CONSTRUCT));
            codeSection.push_back(vec4Type);
            codeSection.push_back(id);
            
            for (auto & arg : functionCall->arguments) {
                codeSection.push_back(generate(arg));
            }

            return id;
        }
        case EXPRESSION_FLOAT_LITERAL:
        {
            // TODO: cache re-used constants
            static_assert(sizeof(float) == sizeof(uint32_t)); // TODO: support other sizes

            auto floatLiteral = reinterpret_cast<const FloatLiteralAST *>(expression.get());
            float value = std::stof(program.extract(floatLiteral->text));
            const uint32_t * data = reinterpret_cast<const uint32_t *>(&value);

            spirv_id id = requestId();
            emit<OP_CONSTANT>(constantSection, { floatType, id, *data });

            return id;
        }
        break;
        default:
            assert(false);
    }
    return -1;
}

spirv_id SPIRVGenerator::requestId() {
    return ++ids;
}