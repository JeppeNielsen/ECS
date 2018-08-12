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
#include "minijson_writer.hpp"

using namespace ECS;

JsonSerializer::JsonSerializer() {
    
    fieldVisitor.OnFieldMultiple<int, float, std::string>([this](const std::string& name, auto& field) {
        stack.current().write(name.c_str(), field);
    });
    
    fieldVisitor.OnBegin([this] (const std::string& type) {
        stack.Push(type);
    });

    fieldVisitor.OnEnd([this] (const std::string& type) {
        stack.Pop();
    });
    
    fieldVisitor.OnVectorBegin([this](const std::string& type) {
        stack.PushArray(type);
    });
    
    fieldVisitor.OnVectorEnd([this] (const std::string& type) {
        stack.Pop();
    });
}

void JsonSerializer::SerializeObject(GameObject go, std::ostream& stream) {
    minijson::writer_configuration config;
    config = config.pretty_printing(true);
    minijson::object_writer writer(stream, config);
    stack.Push(writer);
    Serialize(go);
    stack.Pop();
}

void JsonSerializer::Serialize(GameObject go) {
    stack.Push("GameObject");
    go.IterateComponents([go, this](auto& componentType) {
        componentType->VisitFields(go.Id(), fieldVisitor);
    });
    if (!go.Hierarchy().Children().empty()) {
        stack.PushArray("Children");
        for(auto child : go.Hierarchy().Children()) {
            Serialize(child);
        }
        stack.Pop();
    }
    stack.Pop();
}

void JsonSerializer::Stack::Push(const minijson::object_writer writer) {
    stack.push_back(writer);
}

void JsonSerializer::Stack::Push(const std::string &name) {
    stack.push_back(current().nested_object(name.c_str()));
}

void JsonSerializer::Stack::PushArray(const std::string &name) {
    stack.push_back(current().nested_array(name.c_str()));
}

minijson::object_writer& JsonSerializer::Stack::current() {
    return stack.back();
}

void JsonSerializer::Stack::Pop() {
    current().close();
    stack.pop_back();
}
