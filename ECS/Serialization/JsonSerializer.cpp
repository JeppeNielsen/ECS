//
//  JsonSerializer.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 02/08/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "JsonSerializer.hpp"
#include "Scene.hpp"
#include "minijson_writer.hpp"

using namespace ECS;

JsonSerializer::JsonSerializer() {
    
}

void JsonSerializer::SerializeObject(GameObject go, std::ostream& stream) {
    minijson::writer_configuration config;
    config = config.pretty_printing(true);
    minijson::object_writer writer(stream, config);
    Serialize(go, writer);
    writer.close();
}

void JsonSerializer::Serialize(GameObject go, minijson::object_writer& writer) {

    auto gameObject = writer.nested_object("GameObject");
    auto components = gameObject.nested_object("Components");
        go.IterateComponents([go, &components, this](auto& componentType) {
            componentType->Serialize(go.Id(), components);
        });
    components.close();
    
    if (!go.Hierarchy().Children().empty()) {
        auto children = gameObject.nested_array("Children");
        for(auto child : go.Hierarchy().Children()) {
            Serialize(child, children);
        }
        children.close();
    }
    gameObject.close();
}
