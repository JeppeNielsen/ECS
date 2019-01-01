//
//  ScriptWorld.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 28/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "ScriptWorld.hpp"
#include <cling/Interpreter/Interpreter.h>
#include <cling/Interpreter/Value.h>
#include <cling/Utils/Casting.h>
#include "TypeInfo.hpp"
#include "Container.hpp"
#include <sstream>
#include "FileHelper.hpp"
#include "System.hpp"

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
        if (removeFunction && data) {
            removeFunction(data, componentId);
        }
    }
    
    TypeInfo GetType() {
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

ScriptWorld::~ScriptWorld() {
    Clear();
}

void ScriptWorld::Clear() {
    RemoveFromDatabase();
}

void ScriptWorld::Initialize(const std::string& clingPath, const std::vector<std::string>& includePaths) {

    this->clingPath = clingPath;
    this->includePaths = includePaths;

    
}

void ScriptWorld::RemoveFromDatabase() {
    if (!database) return;
    for(auto componentId : createdComponentIds) {
        database->RemoveCustomComponent(componentId);
    }
    createdComponentIds.clear();
    database = nullptr;
    if (interpreter) {
        delete interpreter;
        interpreter = nullptr;
    }
}

void ScriptWorld::Compile(Database& database, const std::vector<std::string> &cppFiles) {
    RemoveFromDatabase();
    this->database = &database;
    
    {
        std::vector<const char*> arguments;

        arguments.push_back(" ");///Users/Jeppe/Downloads/cling_2018-12-25_mac1012/include/");
        arguments.push_back("-std=c++14");
        
        interpreter = new cling::Interpreter((int)arguments.size(), &arguments[0], clingPath.c_str(), true);
        
        for(const auto& includePath : includePaths) {
            interpreter->AddIncludePath(includePath);
        }
    }
    
    //std::vector<std::string> includePaths;
    //includePaths.push_back("/Projects/ECS/ECS/TestClang");
   // includePaths.push_back("/Projects/ECS/ECS/ECS");
    
    SystemComponentExtractor extractor;
    extractor.Extract(cppFiles, includePaths, [](const auto& componentName){
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
    
    auto databaseComponents = database.GetComponentNameIndices();
    ComponentNameIndicies scriptComponents;
    
    int scriptComponentIndexCounter = -1;
    for(auto index : databaseComponents) {
        std::cout << index.first << "  " << index.second << std::endl;
        if (index.second > scriptComponentIndexCounter) {
            scriptComponentIndexCounter = index.second;
        }
    }
    scriptComponentIndexCounter++;
    startComponentIndex = scriptComponentIndexCounter;
    for(auto s : extractor.components) {
        scriptComponents.push_back(std::make_pair(s.name, scriptComponentIndexCounter++));
    }
    
    ComponentNameIndicies allComponents = databaseComponents;
    for(auto i : scriptComponents) allComponents.push_back(i);
    
    std::string code_ForwardDeclareComponents = Code_ForwardDeclareComponents(allComponents);
    std::string code_AddGetRemove = Code_AddGetRemoveComponent(allComponents);
    std::string code_CreateComponent = Code_CreateComponent(scriptComponents);
    std::string code_RemoveComponent = Code_RemoveComponent(scriptComponents);
    std::string code_GetTypeInfo = Code_GetTypeInfo(extractor.components, startComponentIndex);
    std::string code_CreateSystem = Code_CreateSystem(extractor.systems);
    std::string code_RemoveSystem = Code_RemoveSystem(extractor.systems);
    
    std::string allCode = "";
    
    allCode += "#include \"ScriptInclude.hpp\"\n";
    allCode += "#include \"TypeInfo.hpp\"\n";
    
    allCode += code_ForwardDeclareComponents;
    
    allCode += code_AddGetRemove;
    
    for(auto& cppFile : cppFiles) {
        FileHelper::ParseFile(cppFile, [&allCode] (const auto& line) {
            allCode += line + "\n";
        });
    }
    
    allCode += code_CreateComponent +
        code_RemoveComponent +
        code_GetTypeInfo +
        code_CreateSystem +
        code_RemoveSystem;
    
    std::cout << allCode << std::endl;
    
    interpreter->declare(allCode);
    
    auto createComponent = GetFunction<void*(int)>(*interpreter, "CreateComponent");
    auto removeComponent = GetFunction<void(void*, int)>(*interpreter, "RemoveComponent");
    auto getTypeFunction = GetFunction<TypeInfo(void*, int)>(*interpreter, "_Z11GetTypeInfoPvi");
    
    for (int componentId = startComponentIndex; componentId<scriptComponentIndexCounter; componentId++) {
        createdComponentIds.push_back(componentId);
        database.AddCustomComponent(componentId, new ScriptContainer(componentId, createComponent, removeComponent, getTypeFunction), allComponents[componentId].first);
    }
    endComponentIndex = scriptComponentIndexCounter;
}

void ScriptWorld::AddScene(ECS::Scene &scene, const SerializedScene& serializedScene) {
    auto getNumSystems = GetFunction<int()>(*interpreter, "GetNumSystems");
    auto createSystem = GetFunction<ISystem*(int)>(*interpreter, "CreateSystem");
    
    int numSystems = getNumSystems();
    int maxSystemIndex = scene.GetMaxSystemIndex();
    
    for (int systemId = 0; systemId<numSystems; ++systemId) {
        scene.AddCustomSystem(maxSystemIndex + systemId, createSystem(systemId));
    }
    
    serializedScene.Deserialize(scene);
    scene.Update(0.0f);
}

SerializedScene ScriptWorld::RemoveScene(ECS::Scene &scene) {
    auto getNumSystems = GetFunction<int()>(*interpreter, "GetNumSystems");
    auto removeSystem = GetFunction<void(ISystem*, int)>(*interpreter, "RemoveSystem");
    
    SerializedScene serializedScene;
    serializedScene.Serialize(scene, [this](auto componentId) {
        return componentId >= startComponentIndex && componentId < endComponentIndex;
    });
    
    for(auto go : scene.Objects()) {
        for (int componentId = startComponentIndex; componentId<endComponentIndex; ++componentId) {
            if (go.GetComponent(componentId)!=nullptr) {
                go.RemoveComponent(componentId);
            }
        }
    }
    
    scene.Update(0.0f);
    
    int numSystems = getNumSystems();
    int maxSystemIndex = scene.GetMaxSystemIndex() - numSystems;
    
    for (int systemId = 0; systemId<numSystems; ++systemId) {
        scene.RemoveCustomSystem(maxSystemIndex + systemId);
    }
    return serializedScene;
}

std::vector<std::string> split(const std::string& s, const std::string& seperator) {
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(seperator, pos)) != std::string::npos) {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = pos + seperator.length();
        pos += seperator.length();
    }
    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}

std::string ScriptWorld::Code_ForwardDeclareComponents(const ComponentNameIndicies &componentNameIndices) {
    std::stringstream code;
    for(auto componentNameIndex : componentNameIndices) {
        std::vector<std::string> namespaces = split(componentNameIndex.first, "::");
        std::string name = namespaces.back();
        namespaces.pop_back();
        for(auto& s: namespaces) {
            code << "namespace " << s << " {" << std::endl;
        }
        
        code << "class "<< name <<";"<< std::endl;
    
        for(auto& s : namespaces) {
            code <<"}" << std::endl;
        }
    }
    return code.str();
}

std::string ScriptWorld::Code_AddGetRemoveComponent(const ComponentNameIndicies& componentNameIndices) {
    std::stringstream code;
    for(auto componentNameIndex : componentNameIndices) {
        auto componentName = componentNameIndex.first;
        auto index = componentNameIndex.second;
        code << "template<> "<<componentName<<"* ECS::GameObject::GetComponent<class "+componentName+">() const { return (class "<<componentName<<"*) GetComponent("<<index<<"); }"<<std::endl;
        
        code << "template<> "<<componentName<<"* ECS::GameObject::AddComponent<class "+componentName+">() const { return (class "<<componentName<<"*) AddComponent("<<index<<"); }"<<std::endl;
        
        code << "template<> void ECS::GameObject::RemoveComponent<class "+componentName+">() const { RemoveComponent("<<index<<"); }"<<std::endl;
    }
    return code.str();
}

std::string ScriptWorld::Code_CreateComponent(const ComponentNameIndicies& componentNameIndices) {
    std::stringstream code;
    code << "extern \"C\" void* CreateComponent(int componentId) {"<<std::endl;
    code << "   switch (componentId) {" << std::endl;
    for(auto componentNameIndex : componentNameIndices) {
        code << "       case "<< componentNameIndex.second <<": return new " << componentNameIndex.first << "();"<<std::endl;
    }
    code <<"   }"<<std::endl;
    code << "   return nullptr;"<<std::endl;
    code <<"}"<<std::endl;
    return code.str();
}

std::string ScriptWorld::Code_RemoveComponent(const ComponentNameIndicies& componentNameIndices) {
    std::stringstream code;
    code << "extern \"C\" void RemoveComponent(void* ptr, int componentId) {"<<std::endl;
    code << "   switch (componentId) {" << std::endl;
    for(auto componentNameIndex : componentNameIndices) {
        code << "       case "<< componentNameIndex.second <<": { " << componentNameIndex.first << "* componentPtr = ("<<componentNameIndex.first<<"*)ptr; delete componentPtr; break; }"<<std::endl;
    }
    code <<"   }"<<std::endl;
    code <<"}"<<std::endl;
    return code.str();
}

std::string ScriptWorld::Code_GetTypeInfo(SystemComponentExtractor::Components components, int startScriptComponentIndex) {
    std::stringstream code;
    code << "TypeInfo GetTypeInfo(void* ptr, int componentId) {"<<std::endl;
    code << "   switch (componentId) {" << std::endl;
    for(auto& component : components) {
        code << "       case "<<startScriptComponentIndex<<": {" << std::endl;
        code << "           "<<component.name<<"* c = ("<<component.name<<"*)ptr;"<<std::endl;
        code << "           ECS::TypeInfo info(\""<<component.name<<"\");"<<std::endl;
        for(auto& field : component.fields) {
            code << "           info.AddField<"<<field.type<<">(\""<<field.name<<"\", c->"<<field.name<<");"<<std::endl;
        }
        code << "           return info;"<<std::endl;
        code << "       }" << std::endl;
        startScriptComponentIndex++;
    }
    code <<"   }"<<std::endl;
    code << "   return ECS::TypeInfo(\"\");"<<std::endl;
    code <<"}"<<std::endl;
    return code.str();
}

std::string ScriptWorld::Code_CreateSystem(SystemComponentExtractor::Systems systems) {

    std::stringstream code;
    code << "extern \"C\" int GetNumSystems() { return "<< systems.size() << "; }"<<std::endl;

    code << "extern \"C\" void* CreateSystem(int systemId) {"<<std::endl;
    code << "   switch (systemId) {" << std::endl;
    int index = 0;
    for(auto s : systems) {
        code << "       case "<< index <<": return new " << s.name << "();"<<std::endl;
        index++;
    }
    code <<"   }"<<std::endl;
    code << "   return nullptr;"<<std::endl;
    code <<"}"<<std::endl;
    return code.str();
}

std::string ScriptWorld::Code_RemoveSystem(SystemComponentExtractor::Systems systems) {
    std::stringstream code;
    code << "extern \"C\" void RemoveSystem(ISystem* system, int systemId) {"<<std::endl;
    code << "   switch (systemId) {" << std::endl;
    int index = 0;
    for(auto s : systems) {
        code << "       case "<< index <<": { " << s.name << "* systemPtr = ("<<s.name<<"*)system; delete systemPtr; break; }"<<std::endl;
        index++;
    }
    code <<"   }"<<std::endl;
    code <<"}"<<std::endl;
    return code.str();
}
