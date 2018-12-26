//
//  ScriptCompiler.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once

#include <string>
#include <vector>

namespace ECS {
    class ScriptCompiler {
    public:
        void Initialize(const std::string& clangSdkPath, const std::string& dynamicLibPath);
        bool Compile(bool enableOutput, const std::vector<std::string> &sourceFiles, const std::vector<std::string> &headerFiles);
        
    private:
        std::string clangSdkPath;
        std::string dynamicLibPath;
    };
}
