//
//  Test.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/09/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "Test.hpp"
#include <iostream>

using namespace ECS::Tests;

void Test::RunTest(const std::string& testName, const std::function<bool ()> testCode) const {
    bool result = testCode();
    
    std::cout << testName << ": " << (result ? "succes" : "failed") << std::endl;
}
