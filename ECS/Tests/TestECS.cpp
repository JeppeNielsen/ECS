//
//  TestECS.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/09/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "TestECS.hpp"
#include "Scene.hpp"

using namespace ECS;
using namespace ECS::Tests;

void TestECS::Run() {

    RunTest("GameObject handle", [] () -> bool {
        GameObject gameObject;
        return (bool)!gameObject;
    });
    
    RunTest("Create GameObject", [] () -> bool {
        Database database;
        Scene scene(database);
        GameObject gameObject = scene.CreateObject();
        return (bool)gameObject;
    });
    
    RunTest("Remove GameObject", [] () -> bool {
        Database database;
        Scene scene(database);
        GameObject gameObject = scene.CreateObject();
        bool wasValid = (bool)gameObject;
        gameObject.Remove();
        scene.Update(0.0f);
        return (bool)!gameObject && wasValid;
    });
    
    RunTest("AddComponent<T>()->data = 123", [] () -> bool {
        struct Component { int data; };
        
        Database database;
        Scene scene(database);
        GameObject gameObject = scene.CreateObject();
        auto component = gameObject.AddComponent<Component>();
        component->data = 123;
        return component->data == 123;
    });

    RunTest("AddComponent<T>(123)", [] () -> bool {
        struct Component { int data; };
        Database database;
        Scene scene(database);
        GameObject gameObject = scene.CreateObject();
        auto component = gameObject.AddComponent<Component>(123);
        return component->data = 123;
    });

    RunTest("GetComponent<T>()", [] () -> bool {
        struct Component { int data; };
        Database database;
        Scene scene(database);
        GameObject gameObject = scene.CreateObject();
        gameObject.AddComponent<Component>(123);
        return gameObject.GetComponent<Component>()->data == 123;
    });
    
    RunTest("RemoveComponent", [] () -> bool {
        struct Component { int data; };
        Database database;
        Scene scene(database);
        GameObject gameObject = scene.CreateObject();
        gameObject.AddComponent<Component>(123);
        gameObject.RemoveComponent<Component>();
        scene.Update(0.0f);
        return !gameObject.GetComponent<Component>();
    });
    
    RunTest("AddReferenceComponent", [] () -> bool {
        struct Component { int data; };
        Database database;
        Scene scene(database);
        GameObject source = scene.CreateObject();
        auto sourceComponent = source.AddComponent<Component>(123);
        GameObject dest = scene.CreateObject();
        auto destComponent = dest.AddReferenceComponent<Component>(source);
        scene.Update(0.0f);
        return sourceComponent == destComponent;
    });
    
    RunTest("AddReferenceComponent, source removed", [] () -> bool {
        struct Component { int data; };
        Database database;
        Scene scene(database);
        GameObject source = scene.CreateObject();
        source.AddComponent<Component>(123);
        GameObject dest = scene.CreateObject();
        dest.AddReferenceComponent<Component>(source);
        source.Remove();
        scene.Update(0.0f);
        source = scene.CreateObject();
        auto sourceComponent = source.AddComponent<Component>(123);
        return dest.GetComponent<Component>()!=nullptr && dest.GetComponent<Component>()!=sourceComponent && sourceComponent;
    });
    
    RunTest("AddSystem", [] () -> bool {
        struct Position {};
        struct PositionSystem : public ECS::System<Position> { void Update(float dt) {}};
        Database database;
        Scene scene(database);
        auto* system = &scene.CreateSystem<PositionSystem>();
        return system;
    });
    
    RunTest("System::ObjectAdded", [] () -> bool {
        using List = std::vector<GameObject>;
        List objectsAdded;
        struct Position {int x;};
        struct PositionSystem : public ECS::System<Position> {
            List* list;
            void Update(float dt) {}
            void ObjectAdded(GameObject object) {
                list->push_back(object);
            }
        };
        Database database;
        Scene scene(database);
        scene.CreateSystem<PositionSystem>().list = &objectsAdded;
        auto object = scene.CreateObject();
        object.AddComponent<Position>(123);
        scene.Update(0.0f);
        return objectsAdded.size() == 1 && objectsAdded[0]==object;
    });
    
    RunTest("System::ObjectRemoved", [] () -> bool {
        using List = std::vector<GameObject>;
        List objectsRemoved;
        struct Position {int x;};
        struct PositionSystem : public ECS::System<Position> {
            List* list;
            void Update(float dt) {}
            void ObjectRemoved(GameObject object) {
                list->push_back(object);
            }
        };
        Database database;
        Scene scene(database);
        scene.CreateSystem<PositionSystem>().list = &objectsRemoved;
        auto object = scene.CreateObject();
        object.AddComponent<Position>(123);
        object.RemoveComponent<Position>();
        scene.Update(0.0f);
        return objectsRemoved.size() == 1 && objectsRemoved[0]==object;
    });
    
    RunTest("Remove object from System on Scene destructor", [] () -> bool {
        struct Position { int pos; };
        
        struct MoveSystem : public ECS::System<Position> {
            int* addedPtr;
            int* removedPtr;
            
            void ObjectAdded(GameObject object) {
                (*addedPtr)++;
            }
            
            void ObjectRemoved(GameObject object) {
                (*removedPtr)++;
            }
            
            void Update(float dt) { }
        };
        
        int addedCounter = 0;
        int removedCounter = 0;
        
        Database database;
        GameObject gameObject;
        {
            Scene scene(database);
            auto& system = scene.CreateSystem<MoveSystem>();
            system.addedPtr = &addedCounter;
            system.removedPtr = &removedCounter;
            gameObject = scene.CreateObject();
            gameObject.AddComponent<Position>(456);
            scene.Update(0.0f);
        }
        return addedCounter == 1 && removedCounter == 1;
    });
}
