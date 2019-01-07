//
//  testSystemDependencies.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 07/01/2019.
//  Copyright © 2019 Jeppe Nielsen. All rights reserved.
//

#include <iostream>
#include "Scene.hpp"

struct Velocity {
    float vx;
    float vy;
};

struct Transform {


};

struct Mesh {


};

struct OctreeSystem : ECS::System<Transform, Mesh> {
    void ObjectAdded(ECS::GameObject go) override {
        std::cout << "OctreeSystem::ObjectAdded"<<std::endl;
    }
    
    void ObjectRemoved(ECS::GameObject go) override {
        std::cout << "OctreeSystem::ObjecRemoved"<<std::endl;
    }
};

struct RenderSystem : ECS::System<Transform, Mesh> {
    
    void Initialize() override {
        octree = &scene->CreateSystem<OctreeSystem>();
    }
    
    void ObjectAdded(ECS::GameObject go) override {
        std::cout << "RenderSystem::ObjectAdded"<<std::endl;
    }
    
    void ObjectRemoved(ECS::GameObject go) override {
        std::cout << "RenderSystem::ObjecRemoved"<<std::endl;
    }

    OctreeSystem* octree;
};

struct World {
    
    void Add() {
    
    }

};



int main() {
    ECS::Database database;
    ECS::Scene scene(database);
    
    scene.CreateSystem<RenderSystem>();
    
    auto object = scene.CreateObject();
    object.AddComponent<Transform>();
    object.AddComponent<Mesh>();
    
    scene.Update(0.0f);



    return 0;
}

