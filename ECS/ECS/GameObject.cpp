//
//  GameObject.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "GameObject.hpp"
#include "Scene.hpp"

using namespace ECS;

bool GameObject::operator == (const GameObject& other) const {
    return id == other.id && &scene == &other.scene;
}

bool GameObject::operator != (const GameObject& other) const {
    return !(id == other.id && &scene == &other.scene);
}

GameObject::operator bool() const {
    return scene.database.GameObjectIdValid(id);
}

void GameObject::Remove() const {
    assert(operator bool());
    scene.RemoveObject(id);
}
