#include "../include/writer.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

Writer::Writer(std::shared_ptr<llvm::Module> & llvmModule) : llvmModule(llvmModule) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto targetTriple = llvm::sys::getDefaultTargetTriple();
    llvmModule->setTargetTriple(targetTriple);

    std::string err;
    auto target = llvm::TargetRegistry::lookupTarget(targetTriple, err);

    if (target == nullptr) {
        llvm::errs() << err;
        return;
    }

    llvm::TargetOptions options;
    targetMachine = target->createTargetMachine(
        targetTriple, "generic", "", options, llvm::Reloc::PIC_
    );

    llvmModule->setDataLayout(targetMachine->createDataLayout());
}

Error Writer::output(const std::string & filename) {
    std::error_code err;
    llvm::raw_fd_ostream dest(filename, err, llvm::sys::fs::OF_None);

    if (err) {
        std::cerr << "ERR: could not open output file \"" << filename << "\", " << err.message() << std::endl;
        return ERR_OUTPUT_OBJECT_FILE;
    }

    llvm::legacy::PassManager pass;

    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "ERR: can't emit a file of type llvm::CodeGenFileType::ObjectFile" << std::endl;
        return ERR_OUTPUT_OBJECT_FILE;
    }

    pass.run(*llvmModule);
    dest.flush();

    return ERR_NONE;
}