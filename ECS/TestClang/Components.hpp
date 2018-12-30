//
//  Components.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 27/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "TypeInfo.hpp"
#include "Event.hpp"

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

struct Touchable {
    bool ClickThrough;
    Event<Touchable> Down;
    Event<Touchable> Up;
    Event<Touchable> Clicked;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(ClickThrough)
    TYPE_FIELDS_END

};

