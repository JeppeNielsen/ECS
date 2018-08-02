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

namespace ECS {
class JsonSerializer {
public:
    JsonSerializer();
    void SerializeObject(GameObject go, std::ostream& stream);
private:
    FieldVisitor fieldVisitor;
};
}
