//
//  Container.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 25/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once

#include "GameObjectId.hpp"
#include <vector>
#include <assert.h>

namespace ECS {

struct IContainer {
    virtual ~IContainer() = default;
    
    bool Contains(const GameObjectId id) const {
        const auto index = id & GameObjectIdIndexMask;
        return index<indicies.size() && indicies[index] != GameObjectIdNull;
    }
    
    virtual void Destroy(const GameObjectId object) = 0;
    
    std::vector<std::uint32_t> indicies;
};

template<typename T>
struct Container : public IContainer {

    Container() = default;
    virtual ~Container() = default;

    Container(const Container &) = delete;
    Container(Container &&) = default;

    Container & operator=(const Container &) = delete;
    Container & operator=(Container &&) = default;
    
    void CreateIndex(const GameObjectId id) {
        assert(!Contains(id));
        const auto index = id & GameObjectIdIndexMask;
        if (index>=indicies.size()) {
            indicies.resize(index + 1, GameObjectIdNull);
        }
        indicies[index] = (std::uint32_t)elements.size();
        references.emplace_back(1);
    }
    
    template<typename... Args>
    std::enable_if_t<!std::is_constructible<T, Args...>::value, void>
    Create(const GameObjectId id, Args&& ... args) {
        CreateIndex(id);
        elements.emplace_back(T{std::forward<Args>(args)...});
    }
    
    template<typename... Args>
    std::enable_if_t<std::is_constructible<T, Args...>::value, void>
    Create(const GameObjectId id, Args&& ... args) {
        CreateIndex(id);
        elements.emplace_back(std::forward<Args>(args)...);
    }
    
    void Reference(const GameObjectId id, const GameObjectId referenceId) {
        assert(!Contains(id));
        assert(Contains(referenceId));
        const auto index = id & GameObjectIdIndexMask;
        const auto referenceIndex = referenceId & GameObjectIdIndexMask;
        if (index>=indicies.size()) {
            indicies.resize(index + 1, GameObjectIdNull);
        }
        
        indicies[index] = referenceIndex;
        ++references[referenceIndex];
    }
    
    void Destroy(const GameObjectId id) override {
        assert(Contains(id));
        const auto index = id & GameObjectIdIndexMask;
        
        if ((references[index]--)==0) {
            auto tmp = std::move(elements.back());
            elements[index] = std::move(tmp);
            elements.pop_back();
            references[index] = references.back();
            references.pop_back();
            indicies[indicies.size() - 1] = index;
        }
        
        indicies[index] = GameObjectIdNull;
    }
    
    T* Get(const GameObjectId id) {
        const auto index = id & GameObjectIdIndexMask;
        return &elements[indicies[index]];
    }
    
    std::vector<T> elements;
    std::vector<std::uint16_t> references;
};

}
