//
//  GameObject.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "GameObject.hpp"
#include "Scene.hpp"
#include "Hierarchy.hpp"

using namespace ECS;

GameObject::GameObject() : scene(nullptr), id(GameObjectIdNull) {

}

GameObject::GameObject(class Scene& scene, const GameObjectId id) : scene(&scene), id(id) {

}

GameObject::GameObject(void*) : scene(nullptr), id(GameObjectIdNull) {
}

bool GameObject::operator == (const GameObject& other) const {
    return id == other.id && scene == other.scene;
}

bool GameObject::operator != (const GameObject& other) const {
    return !(id == other.id && scene == other.scene);
}

GameObject::operator bool() const {
    if (!scene) return false;
    return scene->database.GameObjectIdValid(id);
}

void GameObject::Remove() const {
    assert(operator bool());
    scene->RemoveObject(id);
}

Hierarchy& GameObject::Hierarchy() const {
    return *GetComponent<class Hierarchy>();
}

GameObjectId GameObject::Id() const { return id; }

Scene& GameObject::Scene() const { return *scene; }
