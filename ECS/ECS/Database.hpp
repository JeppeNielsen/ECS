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
    using ComponentsIndexed = std::vector<std::unique_ptr<IContainer>>;
    using Components = std::vector<IContainer*>;
    
    template<typename T>
    void AssureComponent() {
        const auto id = IdHelper::GetId<T>();
        if (id>=componentsIndexed.size()) {
            componentsIndexed.resize(id + 1);
        }
        if (componentsIndexed[id]) return;
        componentsIndexed[id] = std::make_unique<Container<T>>();
        components.push_back(componentsIndexed[id].get());
    }
    
    template<typename T>
    Container<T>& ComponentContainer() const {
        const auto id = IdHelper::GetId<T>();
        return static_cast<Container<T>&>(*componentsIndexed[id]);
    }
    
    template<typename T, typename... Args>
    T* AddComponent(const GameObjectId objectId, Args&& ... args) {
        AssureComponent<T>();
        ComponentContainer<T>().Create(objectId, args...);
        return ComponentContainer<T>().Get(objectId);
    }
    
    template<typename T>
    T* AddReferenceComponent(const GameObjectId objectId, const GameObjectId referenceId) {
        AssureComponent<T>();
        ComponentContainer<T>().Reference(objectId, referenceId);
        return ComponentContainer<T>().Get(objectId);
    }
    
    template<typename T>
    T* GetComponent(const GameObjectId objectId) const {
        const auto componentId = IdHelper::GetId<T>();
        if (componentId>=componentsIndexed.size()) return nullptr;
        auto& container = ComponentContainer<T>();
        return container.Contains(objectId) ? container.Get(objectId) : nullptr;
    }
    
    template<typename Func>
    void IterateComponents(const GameObjectId objectId, Func&& func) const {
        for(auto& component : components) {
            if (!component->Contains(objectId)) continue;
            func(component);
        }
    }
    
    void RemoveComponent(const GameObjectId objectId, const std::size_t componentId);
    
    void RemoveAllComponents(const GameObjectId objectId);
    
    bool GameObjectIdValid(const GameObjectId object) const;
    
    GameObjectId CreateObject();
    
    void Remove(const GameObjectId objectId);
    
private:
    ComponentsIndexed componentsIndexed;
    Components components;
    
    std::vector<GameObjectId> objects;
    std::size_t available {};
    GameObjectId next {};
};

}
