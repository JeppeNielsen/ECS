//
//  Components.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 27/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "TypeInfo.hpp"

struct Transform {
    float x;
    float y;
    float rotation;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(x)
    TYPE_FIELD(y)
    TYPE_FIELD(rotation)
    TYPE_FIELDS_END
};
