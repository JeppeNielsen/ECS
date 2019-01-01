#include "ScriptInclude.hpp"

namespace PocketEngine {
  
struct Position {
    float x;
    float y;
};

struct Velocity {
    float vx;
    float vy;
    float vz;
    std::vector<int> numbers;
};

struct Pointer {
    GameObject pointer;
};


struct VelocitySystem : System<Position, Velocity> {
    void ObjectAdded(GameObject go) override {
        std::cout << "Object added" << std::endl;
        go.GetComponent<Velocity>()->vx = 0.1f;
        go.GetComponent<Velocity>()->numbers = { 1,2,3,4,5,6,7};
    }
    
    void ObjectRemoved(GameObject go) override {
        std::cout << "Object removed" << std::endl;
    }
    
    void Update(float dt) override {
        for(auto go : Objects()) {
            auto pos = go.GetComponent<Position>();
            auto vel = go.GetComponent<Velocity>();
            pos->x-=vel->vx * dt;
            pos->y+=vel->vy * dt;
            pos->y+=vel->vz * dt;
            
            std::cout << "position = "<< pos->x << std::endl;
        }
    }
};


struct PointerSystem : System<Pointer, Velocity> {
    
};


struct SimpleSystemSystem : System<Velocity> {
    void ObjectAdded(GameObject go) override {
        std::cout << "Object added " << go.GetComponent<Velocity>()->vx << std::endl;
    }
    
    void ObjectRemoved(GameObject go) override {
        std::cout << "Object removed " << go.GetComponent<Velocity>()->vx << std::endl;
    }
};

}

