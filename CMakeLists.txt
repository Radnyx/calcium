cmake_minimum_required(VERSION 3.5)
project(Calcium VERSION 0.1.0 LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD 17)

execute_process(COMMAND llvm-config "--cxxflags" OUTPUT_VARIABLE CXXFLAGS)
set(CMAKE_CXX_FLAGS "${CXXFLAGS}")

include(CTest)
enable_testing()

file(GLOB SOURCES
    include/*.h
    src/*.cpp
)

execute_process(COMMAND llvm-config "--libdir" OUTPUT_VARIABLE LLVM_LIBDIR)
execute_process(COMMAND llvm-config "--includedir" OUTPUT_VARIABLE LLVM_INCLUDEDIR)
link_directories(${LLVM_LIBDIR})
include_directories(${LLVM_INCLUDEDIR})

add_executable(Calcium ${SOURCES})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(Calcium ws2_32.lib)

target_link_libraries(Calcium
  clangAnalysis.lib
  clangAnalysisFlowSensitive.lib
  clangAnalysisFlowSensitiveModels.lib
  clangAPINotes.lib
  clangARCMigrate.lib
  clangAST.lib
  clangASTMatchers.lib
  clangBasic.lib
  clangCodeGen.lib
  clangCrossTU.lib
  clangDependencyScanning.lib
  clangDirectoryWatcher.lib
  clangDriver.lib
  clangDynamicASTMatchers.lib
  clangEdit.lib
  clangExtractAPI.lib
  clangFormat.lib
  clangFrontend.lib
  clangFrontendTool.lib
  clangHandleCXX.lib
  clangHandleLLVM.lib
  clangIndex.lib
  clangIndexSerialization.lib
  clangInterpreter.lib
  clangLex.lib
  clangParse.lib
  clang-repl.lib
  clangRewrite.lib
  clangRewriteFrontend.lib
  clangSema.lib
  clangSerialization.lib
  clangStaticAnalyzerCheckers.lib
  clangStaticAnalyzerCore.lib
  clangStaticAnalyzerFrontend.lib
  clangSupport.lib
  clangTesting.lib
  clangTooling.lib
  clangToolingASTDiff.lib
  clangToolingCore.lib
  clangToolingInclusions.lib
  clangToolingInclusionsStdlib.lib
  clangToolingRefactoring.lib
  clangToolingSyntax.lib
  clangTransformer.lib
)

target_link_libraries(Calcium
  LLVMWindowsManifest
  LLVMXRay
  LLVMLibDriver
  LLVMDlltoolDriver
  LLVMTextAPIBinaryReader
  LLVMCoverage
  LLVMLineEditor
  LLVMX86TargetMCA
  LLVMX86Disassembler
  LLVMX86AsmParser
  LLVMX86CodeGen
  LLVMX86Desc
  LLVMX86Info
  LLVMOrcDebugging
  LLVMOrcJIT
  LLVMWindowsDriver
  LLVMMCJIT
  LLVMJITLink
  LLVMInterpreter
  LLVMExecutionEngine
  LLVMRuntimeDyld
  LLVMOrcTargetProcess
  LLVMOrcShared
  LLVMDWP
  LLVMDebugInfoLogicalView
  LLVMDebugInfoGSYM
  LLVMOption
  LLVMObjectYAML
  LLVMObjCopy
  LLVMMCA
  LLVMMCDisassembler
  LLVMLTO
  LLVMPasses
  LLVMHipStdPar
  LLVMCFGuard
  LLVMCoroutines
  LLVMipo
  LLVMVectorize
  LLVMLinker
  LLVMInstrumentation
  LLVMFrontendOpenMP
  LLVMFrontendOffloading
  LLVMFrontendOpenACC
  LLVMFrontendHLSL
  LLVMFrontendDriver
  LLVMExtensions
  LLVMDWARFLinkerParallel
  LLVMDWARFLinkerClassic
  LLVMDWARFLinker
  LLVMGlobalISel
  LLVMMIRParser
  LLVMAsmPrinter
  LLVMSelectionDAG
  LLVMCodeGen
  LLVMTarget
  LLVMObjCARCOpts
  LLVMCodeGenTypes
  LLVMIRPrinter
  LLVMInterfaceStub
  LLVMFileCheck
  LLVMFuzzMutate
  LLVMScalarOpts
  LLVMInstCombine
  LLVMAggressiveInstCombine
  LLVMTransformUtils
  LLVMBitWriter
  LLVMAnalysis
  LLVMProfileData
  LLVMSymbolize
  LLVMDebugInfoBTF
  LLVMDebugInfoPDB
  LLVMDebugInfoMSF
  LLVMDebugInfoDWARF
  LLVMObject
  LLVMTextAPI
  LLVMMCParser
  LLVMIRReader
  LLVMAsmParser
  LLVMMC
  LLVMDebugInfoCodeView
  LLVMBitReader
  LLVMFuzzerCLI
  LLVMCore
  LLVMRemarks
  LLVMBitstreamReader
  LLVMBinaryFormat
  LLVMTargetParser
  LLVMTableGen
  LLVMSupport
  LLVMDemangle
)