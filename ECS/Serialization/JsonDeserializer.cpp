//
//  JsonDeserializer.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 02/08/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "JsonDeserializer.hpp"
#include "Scene.hpp"
#include "FieldVisitor.hpp"
#include "minijson_writer.hpp"

using namespace ECS;

JsonDeserializer::JsonDeserializer() {
    
    fieldVisitor.OnFieldMultiple<int, float, std::string>([this](const std::string& name, auto& field) {
    });
    
    fieldVisitor.OnBegin([this] (const std::string& type) {
    });

    fieldVisitor.OnEnd([this] (const std::string& type) {
    });
    
    fieldVisitor.OnVectorBegin([this](const std::string& type) {
    });
    
    fieldVisitor.OnVectorEnd([this] (const std::string& type) {
    });
}

void JsonDeserializer::DeserializeObject(GameObject object, std::istream& stream) {
    
    minijson::istream_context context(stream);
    try {
        minijson::parse_object(context, [&] (const char* n, minijson::value v) {
            std::string token = n;
            if (v.type() == minijson::Object && token == "GameObject") {
                Deserialize(object, context, [](GameObject o){
        
                });
            }
        });
    } catch (minijson::parse_error e) {
        std::cout << e.what() << std::endl;
    }
}

void JsonDeserializer::Deserialize(GameObject object, minijson::istream_context &context, const std::function<void(GameObject)>& objectCreated) {
    minijson::parse_object(context, [&] (const char* n, minijson::value v) {
        std::string name = n;
        if (name == "id" && v.type() == minijson::Number) {
           // object->id = (int)v.as_long();
        } else if (name == "Components" && v.type() == minijson::Object) {
            minijson::parse_object(context, [&] (const char* name, minijson::value v) {
                if (v.type() == minijson::Object) {
                    minijson::ignore(context);
                    //minijson::parse_object(context, [&] (const char* n, minijson::value v) {
                       // AddComponent(object, addReferenceComponents, context, n);
                    //});
                    std::cout << "name = " << name << std::endl;
                } else {
                    minijson::ignore(context);
                }
            });
        } else if (name == "Children" && v.type() == minijson::Object) {
            minijson::parse_object(context, [&] (const char* name, minijson::value v) {
                std::string token = name;
                if (v.type() == minijson::Object && token == "GameObject") {
                    GameObject child = object.Scene().CreateObject();
                    child.Hierarchy().Parent = object;
                    Deserialize(child, context, objectCreated);
                }
            });
        } else if (name == "guid" && v.type() == minijson::String) {
            //object->scene->guid = std::string(v.as_string());
        } else if (name == "counter" && v.type() == minijson::Number) {
            //object->scene->idCounter = (int)v.as_long();
        }
        
        if (objectCreated) {
            objectCreated(object);
        }
    });
}



