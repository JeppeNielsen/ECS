//
//  ScriptCompiler.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "ScriptCompiler.hpp"
#include "CommandRunner.hpp"
using namespace ECS;

void ScriptCompiler::Initialize(const std::string &clangSdkPath, const std::string& dynamicLibPath) {
    this->clangSdkPath = clangSdkPath;
    this->dynamicLibPath = dynamicLibPath;
}

bool ScriptCompiler::Compile(bool enableOutput, const std::vector<std::string> &sourceFiles, const std::vector<std::string> &headerFiles) {
    
    std::string compilerPath = clangSdkPath + "bin/clang++";
    std::string compilerFlags = "-framework Foundation -std=c++14 -stdlib=libc++ -g"; // -dynamiclib
    if (enableOutput) {
        compilerFlags += " -v";
    }
   
    std::string compilerArgs = compilerPath + " " + compilerFlags + " ";
    
    for(auto& header : headerFiles) {
        compilerArgs += header + " ";
    }
    
    for(auto& source : sourceFiles) {
        compilerArgs += source + " ";
    }
    
     std::string outputFile = "-o " + dynamicLibPath;
    
    compilerArgs += outputFile;
    
    CommandRunner::Run(compilerArgs);
    
    return true;
}
