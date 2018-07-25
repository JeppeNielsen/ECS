//
//  System.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <memory>
#include "ObjectList.hpp"
#include "Container.hpp"
#include "GameObject.hpp"
#include "MetaHelper.hpp"

namespace ECS {

class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void TryAddObject(const GameObjectId object) = 0;
    virtual void TryRemoveObject(const GameObjectId object) = 0;
    virtual void Update(float dt) = 0;
    ObjectList objects;
};

template<typename...T>
class System : public ISystem {
public:
    using Components = std::tuple<Container<T> &...>;

    void Initialize(Scene& scene);
    
    bool Match(const GameObjectId object) const {
        bool contains = true;
        Meta::for_each(*components, [&contains, object](const auto& component) {
           contains &= component.Contains(object);
        });
        return contains;
    }
    
    void TryAddObject(const GameObjectId object) override {
        if (!Match(object)) return;
        objects.Add(object);
        ObjectAdded(GameObject(*scene, object));
    }
    
    void TryRemoveObject(const GameObjectId object) override {
        if (Match(object)) return;
        ObjectRemoved(GameObject(*scene, object));
        objects.Remove(object);
    }
    
    struct Iterator {
        Iterator(Scene& scene, ObjectList::Objects& objects, std::size_t index) : scene(scene), objects(objects), index(index) {}
        Iterator(Iterator&&) = default;
        Iterator(Iterator&) = default;
        Iterator(const Iterator&) = default;
        
        Iterator begin() const {
            return Iterator(scene, objects, 0);
        }
        
        Iterator end() const {
            return Iterator(scene, objects, objects.size());
        }
        
        Iterator& operator++() { ++index; return *this; }
        bool operator!=(const Iterator & other) const { return index != other.index; }
        const GameObject operator*() const { return GameObject(scene, objects[index]); }
    
        size_t index;
        Scene& scene;
        std::vector<GameObjectId>& objects;
    };
    
    const Iterator Objects() {
        return Iterator(*scene, objects.objects, 0);
    }
    
protected:
    virtual void ObjectAdded(GameObject object) {}
    virtual void ObjectRemoved(GameObject object) {}
    Scene* scene;
private:
    std::unique_ptr<Components> components;
};

}
