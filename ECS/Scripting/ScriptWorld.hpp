//
//  ScriptWorld.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 28/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <vector>
#include <string>
#include "Database.hpp"
#include "SystemComponentExtractor.hpp"
#include "Scene.hpp"
#include "SerializedScene.hpp"

namespace cling {
    class Interpreter;
}

namespace ECS {
    class ScriptWorld {
    public:
        ~ScriptWorld();
    
        void Initialize(const std::string& clingPath, const std::vector<std::string>& includePaths);
        void Compile(Database& database, const std::vector<std::string>& cppFiles);
        void RemoveFromDatabase();
        void Clear();
        void AddScene(Scene& scene, const SerializedScene& serializedScene);
        SerializedScene RemoveScene(Scene& scene);
        
    private:
    
        using ComponentNameIndicies = std::vector<Database::NameIndex>;
    
        std::string Code_ForwardDeclareComponents(const ComponentNameIndicies& componentNameIndices);
        std::string Code_AddGetRemoveComponent(const ComponentNameIndicies& componentNameIndices);
        std::string Code_CreateComponent(const ComponentNameIndicies& componentNameIndices);
        std::string Code_RemoveComponent(const ComponentNameIndicies& componentNameIndices);
        std::string Code_GetTypeInfo(SystemComponentExtractor::Components components, int startScriptComponentIndex);
        std::string Code_CreateSystem(SystemComponentExtractor::Systems systems);
        std::string Code_RemoveSystem(SystemComponentExtractor::Systems systems);
        
        std::string clingPath;
        std::vector<std::string> includePaths;
        cling::Interpreter* interpreter = nullptr;
        Database* database = nullptr;
        std::vector<int> createdComponentIds;
        int startComponentIndex;
        int endComponentIndex;
    };
}
