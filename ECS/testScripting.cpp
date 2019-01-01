//
//  testScripting.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "ScriptWorld.hpp"
#include <iostream>
#include "ScriptCompiler.hpp"
#include "SystemComponentExtractor.hpp"
#include "Components.hpp"
#include "Scene.hpp"
#include "JsonSerializer.hpp"
#include "JsonDeserializer.hpp"
#include <sstream>

using namespace ECS;


int main() {

    Database database;
    database.AssureComponent<Hierarchy>();
    database.AssureComponent<Transform>();
    database.AssureComponent<Touchable>();

    std::vector<std::string> includePaths = {
        "/Projects/ECS/ECS/Data/",
        "/Projects/ECS/ECS/ECS/",
        "/Projects/ECS/ECS/Reflection/",
        "/Projects/ECS/ECS/Serialization/",
        "/Projects/ECS/ECS/Json/",
        "/Projects/ECS/ECS/Helpers/",
        "/Projects/ECS/ECS/TestClang/",
    };


    ScriptWorld scriptWorld;
    
    scriptWorld.Initialize("/Projects/ECS/cling", includePaths);
    
    scriptWorld.Compile(database, {"/Projects/ECS/ECS/TestClang/game.cpp"});
   
    Scene scene(database);
    scriptWorld.AddScene(scene, {});
    
    auto object = scene.CreateObject();
    object.AddComponent(4);
    object.AddComponent(5);
    object.AddComponent<Transform>(2.0f,3.0f, 180.0f);
    
    scene.CreateObject().AddComponent(5);
    
    for(int i=0; i<10; i++ ) {
        scene.Update(1.0f);
    }
    
    auto serializedScene = scriptWorld.RemoveScene(scene);
    scriptWorld.Compile(database, {"/Projects/ECS/ECS/TestClang/game2.cpp"});
    
    scriptWorld.AddScene(scene, serializedScene);
    
    for(int i=0; i<10; i++ ) {
        scene.Update(1.0f);
    }
    
    JsonSerializer jsonifier;
    jsonifier.SerializeObject(object, std::cout);
    
    /*
    std::stringstream stream;
    {
        JsonSerializer jsonSerializer;
        jsonSerializer.SerializeComponents(object, stream, [](int componentId) { return true; });
    }
    auto object2 = scene.CreateObject();
    
    scene.Update(1.0f);
    
    {
        JsonDeserializer jsonDeserializer;
        jsonDeserializer.DeserializeComponents(object2, stream);
        {
            JsonSerializer jsonSerializer;
            jsonSerializer.SerializeObject(object2, std::cout);
        }
        
        std::cout << stream.str();
    }
    */

    return 0;
}






int main_extract()
{

    std::vector<std::string> includePaths;
    //includePaths.push_back("/Projects/ECS/ECS/TestClang");
    includePaths.push_back("/Projects/ECS/ECS/ECS");
    
    
    SystemComponentExtractor extractor;
    extractor.Extract({"/Projects/ECS/ECS/TestClang/game.cpp"}, includePaths, [](auto& s) {
        return true;
    });
    
    for(auto s : extractor.components) {
        std::cout << s.name << std::endl;
        for(auto f : s.fields) {
            std::cout << "   "<< f.type<< " " << f.name << std::endl;
        }
    }
    
    for(auto s : extractor.systems) {
        std::cout << s.name << std::endl;
        for(auto f : s.components) {
            std::cout << "   "<< f << std::endl;
        }
    }
    
    return 0;
}

int main_nono() {

    ScriptCompiler compiler;
    compiler.Initialize("/Projects/ECS/clang/", "/Projects/ECS/ECS/ScriptLib/libGame.a");
    compiler.Compile(true, {"/Projects/ECS/ECS/TestClang/game.cpp"}, {});

    return 0;
}
