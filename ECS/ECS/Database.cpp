//
//  Database.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "Database.hpp"

using namespace ECS;
void Database::RemoveComponent(const GameObjectId objectId, const std::size_t componentId) {
    if (componentId<components.size() && components[componentId]) {
        components[componentId]->Destroy(objectId);
    }
}

void Database::RemoveAllComponents(const GameObjectId objectId) {
    for(auto& component : components) {
        if (component->Contains(objectId)) {
            component->Destroy(objectId);
        }
    }
}

bool Database::GameObjectIdValid(const GameObjectId object) const {
    const auto pos = GameObjectId(object & GameObjectIdIndexMask);
    return (pos < objects.size() && objects[pos] == object);
}

GameObjectId Database::CreateObject() {
    GameObjectId object;

    if(available) {
        const auto objectIndex = next;
        const auto version = objects[objectIndex] & (~GameObjectIdIndexMask);

        object = objectIndex | version;
        next = objects[objectIndex] & GameObjectIdIndexMask;
        objects[objectIndex] = object;
        --available;
    } else {
        object = GameObjectId(objects.size());
        objects.push_back(object);
    }
    return object;
}

void Database::Remove(const GameObjectId objectId) {
    const auto object = objectId & GameObjectIdIndexMask;
    const auto version = (((objectId >> 20) + 1) & GameObjectIdIndexMask) << 20;
    const auto node = (available ? next : ((object + 1) & GameObjectIdIndexMask)) | version;

    objects[object] = node;
    next = object;
    ++available;
}
