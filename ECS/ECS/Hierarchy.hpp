//
//  Hierarchy.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 29/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <vector>
#include "GameObject.hpp"
#include "Property.hpp"

namespace ECS {
    class Scene;
    class Hierarchy {
    public:
        Hierarchy();
        Hierarchy(Hierarchy&&);
        Hierarchy& operator=(const Hierarchy&);
        
        using ChildrenCollection = std::vector<GameObject>;
        
        Property<GameObject> Parent;
        
        const ChildrenCollection& Children();
        
        template<typename Func>
        void IterateChildren(Func&& func) {
            IterateChildrenInternal(owner, func);
        }
    
    private:
        template<typename Func>
        void IterateChildrenInternal(GameObject go, Func&& func) {
            func(go);
            auto& hierarchy = go.Hierarchy();
            for(auto child : hierarchy.Children()) {
                IterateChildrenInternal(child, func);
            }
        }
        
        void Initialize();
    
        GameObject owner;
        ChildrenCollection children;
        
        friend class Scene;
    };
}
