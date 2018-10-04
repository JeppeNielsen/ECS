//
//  Scene.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "Scene.hpp"

using namespace ECS;

Scene::Scene(Database& database)
: database{ database } { }

Scene::~Scene() {
    for(const auto object : objects.objects) {
        RemoveObjectFromDatabase(object);
    }
}

GameObject Scene::CreateObject() {
    const auto object = database.CreateObject();
    objects.Add(object);
    GameObject go = GameObject(*this, object);
    go.AddComponent<Hierarchy>()->owner = go;
    return go;
}

void Scene::Update(float dt) {
    DoActions(addComponentActions, [this] (const auto object) {
        for(const auto& system : systems) {
            if (!system->objects.Contains(object)) {
                system->TryAddObject(object);
            }
        }
    });
    DoActions(removeComponentActions, [this] (const auto& pair) {
        database.RemoveComponent(pair.first, pair.second);
        for(const auto& system : systems) {
            if (system->objects.Contains(pair.first)) {
                system->TryRemoveObject(pair.first);
            }
        }
    });
    
    DoActions(removeActions, [this] (const auto object) {
        RemoveObjectFromDatabase(object);
        objects.Remove(object);
    });
    
    UpdateSystems(dt);
}

Database& Scene::GetDatabase() const {
    return database;
}

bool Scene::IsEmpty() const {
    return objects.objects.empty();
}

void Scene::RemoveObject(const GameObjectId object) {
    removeActions.insert(object);
}

void Scene::UpdateSystems(float dt) {
    for(const auto& system : systems) {
        system->Update(dt);
    }
}

void Scene::RemoveObjectFromDatabase(const GameObjectId object) {
    database.RemoveAllComponents(object);
    for(const auto& system : systems) {
        if (system->objects.Contains(object)) {
            system->TryRemoveObject(object);
        }
    }
    database.Remove(object);
}
