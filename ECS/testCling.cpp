//
//  testCling.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 26/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <cling/Utils/Casting.h>
#include "Database.hpp"
#include "System.hpp"
#include "Scene.hpp"
#include "Components.hpp"
#include "JsonSerializer.hpp"
#include "TestECS.hpp"

using namespace ECS;

template<typename T>
T* GetFunction(cling::Interpreter& interpreter, const std::string& functionName) {
  void* functionAddr = interpreter.getAddressOfGlobal(functionName);
  T* func = cling::utils::VoidToFunctionPtr<T*>(functionAddr);
  return func;
}

using createComponentFunction = void*(int);
using removeComponentFunction = void(void*, int);
using getComponentTypeInfoFunction = TypeInfo(void*, int);

struct ScriptComponent {
    
    void* data;
    int componentId;
    removeComponentFunction* removeFunction;
    getComponentTypeInfoFunction* getTypeInfoFunction;
    
    ScriptComponent() : data(nullptr), removeFunction(nullptr), getTypeInfoFunction(nullptr) {}
    
    ~ScriptComponent() {
        if (removeFunction) {
            removeFunction(data, componentId);
        }
    }
    
    ECS::TypeInfo GetType() {
        return getTypeInfoFunction(data, componentId);
    }
};

struct ScriptContainer : Container<ScriptComponent> {

    int componentId;
    createComponentFunction* createFunction;
    removeComponentFunction* removeFunction;
    getComponentTypeInfoFunction* getTypeInfoFunction;

    ScriptContainer(int componentId,
    createComponentFunction* createFunction,
    removeComponentFunction* removeFunction,
    getComponentTypeInfoFunction* getTypeInfoFunction)
    :
    componentId(componentId),
    createFunction(createFunction),
    removeFunction(removeFunction),
    getTypeInfoFunction(getTypeInfoFunction) {}
    
    void CreateDefault(const GameObjectId id) override {
        Container<ScriptComponent>::CreateDefault(id);
        ScriptComponent& component = elements.back();
        component.data = createFunction(componentId);
        component.componentId = componentId;
        component.removeFunction = removeFunction;
        component.getTypeInfoFunction = getTypeInfoFunction;
    }
    
    void* GetInstance(const GameObjectId id) override {
        if (!Contains(id)) return nullptr;
        ScriptComponent* component = Get(id);
        return component->data;
    }
};

struct TransformSystem : System<Transform> {
    void Update(float dt) override {
        std::cout << "Update from Transform system" << std::endl;
    }
    
    void ObjectAdded(GameObject go) override {
        std::cout << "Object added from Transform system"<< std::endl;
    }
    
    void ObjectRemoved(GameObject go) override {
        std::cout << "Object removed from Transform system"<< std::endl;
    }
    
};

int main() {

    std::vector<const char*> arguments;

    arguments.push_back(" ");///Users/Jeppe/Downloads/cling_2018-12-25_mac1012/include/");
    arguments.push_back("-std=c++14");

    cling::Interpreter interp((int)arguments.size(), &arguments[0], "/Users/Jeppe/Downloads/cling_2018-12-25_mac1012/", true);


    interp.AddIncludePath("/Projects/ECS/ECS/TestClang/");
    interp.AddIncludePath("/Projects/ECS/ECS/ECS/");
    interp.AddIncludePath("/Projects/ECS/ECS/Reflection/");
    interp.AddIncludePath("/Projects/ECS/ECS/Serialization/");
    interp.AddIncludePath("/Projects/ECS/ECS/Json/");
    interp.AddIncludePath("/Projects/ECS/ECS/Helpers/");


    std::string code = "";
    code += "#include \"ScriptInclude.hpp\" \n";
   
    code += "template<> Hierarchy* ECS::GameObject::GetComponent<class Hierarchy>() const { return (class Hierarchy*) GetComponent(0); } \n";
    code += "template<> Transform* ECS::GameObject::GetComponent<Transform>() const { return (Transform*) GetComponent(1); } \n";
    code += "template<> Position* ECS::GameObject::GetComponent<Position>() const { return (Position*) GetComponent(2); } \n";
    
    code += "template<> Hierarchy* ECS::GameObject::AddComponent<class Hierarchy>() const { return (class Hierarchy*) AddComponent(0); } \n";
    code += "template<> Transform* ECS::GameObject::AddComponent<Transform>() const { return (Transform*) AddComponent(1); } \n";
    code += "template<> Position* ECS::GameObject::AddComponent<Position>() const { return (Position*) AddComponent(2); } \n";
   
    code += "template<> void ECS::GameObject::RemoveComponent<class Hierarchy>() const { RemoveComponent(0); } \n";
    code += "template<> void ECS::GameObject::RemoveComponent<Transform>() const { RemoveComponent(1); } \n";
    code += "template<> void ECS::GameObject::RemoveComponent<Position>() const { RemoveComponent(2); } \n";
   
   
    code += "extern \"C\" void AddComponent(GameObject go) { go.AddComponent<Position>()->x = 123; go.GetComponent<Position>()->y = 456; } \n";
    code += "TypeInfo GetTypeInfo(void* ptr, int id) { Position* comp = (Position*)ptr; return comp->GetType(); } \n";
    code += "extern \"C\" void* CreateComponent(int id) { return new Position(); } \n";
    code += "extern \"C\" void RemoveComponent(void* ptr, int id) { Position* component = (Position*)ptr; delete component; } \n";
    code += "extern \"C\" void Test(GameObject go) { go.RemoveComponent<Position>(); std::cout << \"Will removed Component\"<<go.GetComponent<Transform>()->x; } \n";
    code += "extern \"C\" ISystem* CreateSystem(int id) { return new VelocitySystem(); } \n";
    
    
/*

    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" ISystem* CreateSystem() { return new Velocity(); } ");

    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void DeleteSystem(ISystem* sys) { delete sys; } ");

    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void CallUpdate(ISystem* system) { system->Update(0.0f); } ");

    interp.declare("#include \"ScriptInclude.hpp\" \n  TypeInfo GetTypeInfo(void* ptr) { Position* pos = (Position*)ptr; return pos->GetType(); } ");
    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void* CreateComponent() { Position* pos = new Position(); pos->x = 123; pos->y = 456; return pos; } ");

interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void InitializeDatabase(Database& database) { database.AssureComponent<Velocity>(); } ");

interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void AddComponent(GameObject go) { go.AddComponent<Velocity>(); } ");
interp.declare("#include \"ScriptInclude.hpp\" \n  TypeInfo GetComponentType(void* ptr) { GameObject* go = (GameObject*)ptr; return go->GetComponent<Position>()->GetType(); } ");
    */
    
    interp.declare(code);
    
    

    /*auto createSystem = GetFunction<ISystem*()>(interp, "CreateSystem");
    auto deleteSystem = GetFunction<void(ISystem*)>(interp, "DeleteSystem");
    auto createComponent = GetFunction<void*()>(interp, "CreateComponent");
    auto initializeDatabase = GetFunction<void(Database&)>(interp, "InitializeDatabase");
    
    auto getComponentType = GetFunction<TypeInfo(void*)>(interp, "_Z16GetComponentTypePv");
    */
    
    
    
/*
    void* component = createComponent();

    

    {
        auto typeInfo = getTypeInfo(component);
        minijson::writer_configuration config;
        config = config.pretty_printing(true);
        minijson::object_writer writer(std::cout, config);
        typeInfo.Serialize(writer);
        writer.close();
    }

    ISystem* system = createSystem();

     system->Update(1.0f);

    deleteSystem(system);
    */
    
    Database database;
    database.AssureComponent<Hierarchy>();
    database.AssureComponent<Transform>();
    
    auto createComponent = GetFunction<void*(int)>(interp, "CreateComponent");
    auto removeComponent = GetFunction<void(void*, int)>(interp, "RemoveComponent");
    auto getTypeFunction = GetFunction<TypeInfo(void*, int)>(interp, "_Z11GetTypeInfoPvi");
    auto createSystem = GetFunction<ISystem*(int)>(interp, "CreateSystem");
    
    database.AddCustomComponent(2, new ScriptContainer(2, createComponent, removeComponent, getTypeFunction), "Position");
    
    
    Scene scene(database);
    //scene.CreateSystem<TransformSystem>();
    scene.AddCustomSystem(0, createSystem(0));
    
    GameObject go = scene.CreateObject();
    
    go.AddComponent<Transform>()->x = 661;
    
    auto addComponent = GetFunction<void(GameObject)>(interp, "AddComponent");// "_Z12AddComponentPv");
     auto testFunction = GetFunction<void(GameObject)>(interp, "Test");// "_Z12AddComponentPv");
    
    addComponent(go);
    
    scene.Update(0.0f);
    
    JsonSerializer serializer;
    serializer.SerializeObject(go, std::cout);
    
    testFunction(go);
    
    scene.Update(0.0f);
    
    serializer.SerializeObject(go, std::cout);
    
    /*
    {
        auto typeInfo = getComponentType(&go);
        minijson::writer_configuration config;
        config = config.pretty_printing(true);
        minijson::object_writer writer(std::cout, config);
        typeInfo.Serialize(writer);
        writer.close();
    }
    */
    
    return 0;
}
