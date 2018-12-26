#include "ScriptInclude.hpp"

namespace PocketEngine {
  
struct Position {
    float x;
    float y;
};

struct Velocity {
    float vx;
    float vy;
    std::vector<int> numbers;
};

struct Pointer {
    GameObject pointer;
};

struct VelocitySystem : System<Position, Velocity> {
    void ObjectAdded(GameObject go) override {
        std::cout << "Object added" << std::endl;
    }
    
    void ObjectRemoved(GameObject go) override {
        std::cout << "Object removed" << std::endl;
    }
    
    void Update(float dt) override {
        for(auto go : Objects()) {
            auto pos = go.GetComponent<Position>();
            auto vel = go.GetComponent<Velocity>();
            pos->x+=vel->vx * dt;
            pos->y+=vel->vy * dt;
        }
    }
};

struct PointerSystem : System<Pointer, Velocity> {
    
    


};

}

