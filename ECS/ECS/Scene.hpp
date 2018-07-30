//
//  Scene.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "Database.hpp"
#include "System.hpp"
#include <set>
#include <assert.h>
#include "GameObject.hpp"

namespace ECS {

struct Scene {
    using SystemIdHelper = IdHelper<struct SystemIdHelper>;
    using Systems = std::vector<std::unique_ptr<ISystem>>;
    using Actions = std::set<GameObjectId>;
    using RemoveComponentActions = std::set<std::pair<GameObjectId, size_t>>;
    
    friend class GameObject;
    
    Scene(Database& database);
    ~Scene();

    template<typename S>
    S& CreateSystem() {
        const auto systemId = SystemIdHelper::GetId<S>();
        if (systemId>=systems.size()) {
            systems.resize(systemId + 1);
        }
        if (!systems[systemId]) {
            systems[systemId] = std::make_unique<S>();
            S& system = static_cast<S&>(*systems[systemId]);
            system.Initialize(*this);
            return system;
        } else {
            return static_cast<S&>(*systems[systemId]);
        }
    }
    
    GameObject CreateObject();
    
    void Update(float dt);
    
    Database& GetDatabase() const;
private:
    
    void RemoveObject(const GameObjectId object);
    
    template<typename T, typename...Args>
    T* AddComponent(const GameObjectId object, Args&& ... args) {
        addComponentActions.insert(object);
        return database.AddComponent<T>(object, args...);
    }
    
    template<typename T>
    void RemoveComponent(const GameObjectId object) {
        const auto id = Database::IdHelper::GetId<T>();
        removeComponentActions.insert(std::make_pair(object, id));
    }
    
    template<typename Actions, typename Func>
    void DoActions(Actions& actions, Func&& func) {
        if (actions.empty()) return;
        for(const auto& object : actions)  {
            func(object);
        }
        actions.clear();
    }
    
    void UpdateSystems(float dt);
    void RemoveObjectFromDatabase(const GameObjectId object);

    Database& database;
    Systems systems;
    ObjectList objects;
    Actions addComponentActions;
    RemoveComponentActions removeComponentActions;
    Actions removeActions;
};
  
template<typename T, typename... Args>
T* GameObject::AddComponent(Args&&... args) const {
    assert(operator bool());
    return scene->AddComponent<T>(id, args...);
}

template<typename T>
T* GameObject::GetComponent() const {
    assert(operator bool());
    return scene->database.GetComponent<T>(id);
}

template<typename T>
void GameObject::RemoveComponent() const {
    assert(operator bool());
    scene->RemoveComponent<T>(id);
}

template<typename...T>
void System<T...>::Initialize(Scene& scene) {
    this->scene = &scene;
    components = std::make_unique<Components>((scene.GetDatabase().AssureComponent<T>(), scene.GetDatabase().ComponentContainer<T>())...);
}
  
}
