//
//  JsonSerializer.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 02/08/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "JsonSerializer.hpp"
#include "Scene.hpp"
#include "FieldVisitor.hpp"

using namespace ECS;

JsonSerializer::JsonSerializer() {
    fieldVisitor.OnField<int>([](const std::string& name, int& field) {
        std::cout << "Ints -> " << name << " " << field << std::endl;
    });

    fieldVisitor.OnField<float>([](const std::string& name, float& field) {
        std::cout << "Floats -> " << name <<" "<< field << std::endl;
    });

    fieldVisitor.OnField<std::string>([](const std::string& name, std::string& field) {
        std::cout << "String -> " << name << " "<< field << std::endl;
    });

    fieldVisitor.OnBegin([] (const std::string& type) {
        std::cout << "Type started : "<<type << std::endl;
    });

    fieldVisitor.OnEnd([] (const std::string& type) {
        std::cout << "Type ended : "<<type << std::endl;
    });
}

void JsonSerializer::SerializeObject(GameObject go, std::ostream& stream) {
    go.IterateComponents([go, this](auto& componentType) {
        componentType->VisitFields(go.Id(), fieldVisitor);
    });
    for(auto child : go.Hierarchy().Children()) {
        SerializeObject(child, stream);
    }
}
