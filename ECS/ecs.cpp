//
//  ecs.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include <iostream>

#include "Scene.hpp"
#include "Hierarchy.hpp"

using namespace ECS;

struct Position {
    float x;
    float y;
};

struct Velocity {
    float vx;
    float vy;
};

struct VelocitySystem : public System<Position, Velocity> {
    void ObjectAdded(GameObject object) override {
        std::cout << "Object added " << std::endl;
    }
    
    void ObjectRemoved(GameObject object) override {
        std::cout << "Object removed " << std::endl;
    }
    
    void Update(float dt) override {
        for(auto object : Objects()) {
            auto pos = object.GetComponent<Position>();
            auto vel = object.GetComponent<Velocity>();
            pos->x += vel->vx;
            std::cout << pos->x << std::endl;
        }
    }
};

int main() {
    
    Database database;
    Scene scene(database);
    scene.CreateSystem<VelocitySystem>();
    auto object1 = scene.CreateObject();
    
    //scene.AddComponent<Position>(object1);
    //scene.AddComponent<Velocity>(object1)->vx = 2;
    
    object1.AddComponent<Position>(2.0f);
    object1.AddComponent<Velocity>()->vx = 100;
    
    object1.RemoveComponent<Position>();
    
    
    
    if (object1) {
        std::cout << "is valid" << std::endl;
    }
    
    /*object1.Remove();
    object1.Remove();
    object1.Remove();
    */
    if (object1) {
        std::cout << "still valid" << std::endl;
    }
    
    scene.Update(1.0f);
    
    object1.AddComponent<Position>(100.0f);
    
    if (!object1) {
        std::cout << "not valid" << std::endl;
    }
    
    std::vector<GameObject> objects;
    objects.push_back(object1);
    
    if (objects[0] == object1) {
        std::cout << "Same!"<<std::endl;
    }
    
    
    scene.Update(0);
    scene.Update(0);
    
    //scene.RemoveComponent<Position>(object1);
    scene.Update(0);
    //scene.RemoveComponent<Position>(object1);
    scene.Update(0);
    scene.Update(0);
    
    
    auto parent = scene.CreateObject();
    
    auto child1 = scene.CreateObject();
    child1.Hierarchy().Parent = parent;
    
    auto child2 = scene.CreateObject();
    child2.Hierarchy().Parent = parent;
    

    auto& parentHierarchy = parent.Hierarchy();
    
    parent.Hierarchy().IterateChildren([](GameObject go){
        std::cout << go.Id() << std::endl;
    });
    
    return 0;
}
