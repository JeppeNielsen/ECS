//
//  Database.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameObjectId.hpp"
#include "Container.hpp"
#include "IdHelper.hpp"

namespace ECS {

struct Scene;

struct Database {
    using IdHelper = IdHelper<struct ComponentId>;
    using Components = std::vector<std::unique_ptr<IContainer>>;
    
    template<typename T>
    void AssureComponent() {
        const auto id = IdHelper::GetId<T>();
        if (id>=components.size()) {
            components.resize(id + 1);
        }
        if (components[id]) return;
        components[id] = std::make_unique<Container<T>>();
    }
    
    template<typename T>
    Container<T>& ComponentContainer() {
        const auto id = IdHelper::GetId<T>();
        return static_cast<Container<T>&>(*components[id]);
    }
    
    template<typename T, typename... Args>
    T* AddComponent(const GameObjectId objectId, Args&& ... args) {
        AssureComponent<T>();
        ComponentContainer<T>().Create(objectId, args...);
        return ComponentContainer<T>().Get(objectId);
    }
    
    template<typename T>
    T* GetComponent(const GameObjectId objectId) {
        const auto componentId = IdHelper::GetId<T>();
        if (componentId>=components.size()) return nullptr;
        auto& container = ComponentContainer<T>();
        return container.Contains(objectId) ? container.Get(objectId) : nullptr;
    }
    
    void RemoveComponent(const GameObjectId objectId, const std::size_t componentId);
    
    void RemoveAllComponents(const GameObjectId objectId);
    
    bool GameObjectIdValid(const GameObjectId object) const;
    
    GameObjectId CreateObject();
    
    void Remove(const GameObjectId objectId);
    
private:
    Components components;
    
    std::vector<GameObjectId> objects;
    std::size_t available {};
    GameObjectId next {};
};

}
