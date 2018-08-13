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
#include <sstream>
#include "Scene.hpp"
#include "JsonSerializer.hpp"
#include "JsonDeserializer.hpp"

using namespace ECS;

struct Node {
    std::string nodeName;
    float width;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(nodeName)
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

struct NodeList {
    std::vector<Node> nodes;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(nodes)
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
    parent.AddComponent<NodeList>(std::vector<Node> {
        { "First node", 10.0f},
        { "Second node", 20.0f},
        { "Third node", 30.0f},
    });
    
    auto child = scene.CreateObject();
    child.AddComponent<Position>("Jeppe", node);
    child.Hierarchy().Parent = parent;
    
    
    Node node2 {"Second node",  3000.0f };
    auto child2 = scene.CreateObject();
    child2.AddComponent<Position>("Jeppe", node2);
    child2.Hierarchy().Parent = parent;
    
    JsonSerializer jsonSerializer;
    
    std::stringstream stream;
    
    jsonSerializer.SerializeObject(parent, stream);
    std::cout << stream.str() << std::endl;
    
    JsonDeserializer deserializer;
    
    GameObject deserializedObject = scene.CreateObject();
    deserializer.DeserializeObject(deserializedObject, stream);
    
    return 0;
}
