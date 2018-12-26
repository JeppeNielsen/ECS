//
//  ScriptInclude.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 23/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <iostream>
#include <vector>

#include "System.hpp"

using namespace ECS;

struct Position {
    int x;
    int y;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(x)
    TYPE_FIELD(y)
    TYPE_FIELDS_END
};

struct Velocity : System<Position> {
    void Update(float dt) override {
        std::cout << "Update from Velocity system"<< std::endl;
    }

};
