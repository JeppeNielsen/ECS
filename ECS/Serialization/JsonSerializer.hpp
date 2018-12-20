//
//  JsonSerializer.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 02/08/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameObject.hpp"
#include <ostream>
#include "minijson_writer.hpp"

namespace ECS {
class JsonSerializer {
public:
    JsonSerializer();
    void SerializeObject(GameObject go, std::ostream& stream);
private:
    void Serialize(GameObject go, minijson::object_writer& writer);
};
}
