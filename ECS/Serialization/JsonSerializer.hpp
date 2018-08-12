//
//  JsonSerializer.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 02/08/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "FieldVisitor.hpp"
#include "GameObject.hpp"
#include <ostream>
#include "minijson_writer.hpp"

namespace ECS {
class JsonSerializer {
public:
    JsonSerializer();
    void SerializeObject(GameObject go, std::ostream& stream);
private:

    void Serialize(GameObject go);
    FieldVisitor fieldVisitor;
    minijson::object_writer* current;
    
    struct Stack {
        void Push(minijson::object_writer writer);
        void Push(const std::string& name);
        void PushArray(const std::string& name);
        void Pop();
        minijson::object_writer& current();
        std::vector<minijson::object_writer> stack;
    };
    
    Stack stack;
    
};
}
