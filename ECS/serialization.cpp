//
//  serialization.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 26/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include <iostream>
#include <vector>
#include <string>
#include "Scene.hpp"
#include "JsonSerializer.hpp"

using namespace ECS;

struct Node {
    std::string name;
    float width;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(name)
    TYPE_FIELD(width)
    TYPE_FIELDS_END
};

struct Position {

    
    std::string name;
    Node node;

    TYPE_FIELDS_BEGIN
    TYPE_FIELD(name)
    TYPE_FIELD(node)
    TYPE_FIELDS_END
};

struct Velocity {
    float vx;
    float vy;
};

int main() {


    Database database;
    Scene scene(database);
    auto parent = scene.CreateObject();
    Node node {"First node",  3.0f };
    parent.AddComponent<Position>("jeppe", node );
    parent.AddComponent<Velocity>(2.0f,4.0f);
    
    auto child = scene.CreateObject();
    child.AddComponent<Position>("Jeppe", node);
    child.Hierarchy().Parent = parent;
    
    JsonSerializer jsonSerializer;
    jsonSerializer.SerializeObject(parent, std::cout);

    return 0;
}
