//
//  testScripting.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include <iostream>
#include "ScriptCompiler.hpp"
#include "ScriptData.hpp"

using namespace ECS;

int main()
{

    std::vector<std::string> includePaths;
    //includePaths.push_back("/Projects/ECS/ECS/TestClang");
    includePaths.push_back("/Projects/ECS/ECS/ECS");
    
    
    ScriptData data;
    data.Parse({"/Projects/ECS/ECS/TestClang/game.cpp"}, includePaths, [](auto& s) {
        return true;
    });
    
    for(auto s : data.components) {
        std::cout << s.name << std::endl;
        for(auto f : s.fields) {
            std::cout << "   "<< f.type<< " " << f.name << std::endl;
        }
    }
    
    for(auto s : data.systems) {
        std::cout << s.name << std::endl;
        for(auto f : s.components) {
            std::cout << "   "<< f << std::endl;
        }
    }
    
    
    return 0;
}

int main_nono() {

    ScriptCompiler compiler;
    compiler.Initialize("/Projects/ECS/clang/", "/Projects/ECS/ECS/ScriptLib/libGame.a");
    compiler.Compile(true, {"/Projects/ECS/ECS/TestClang/game.cpp"}, {});

    return 0;
}
