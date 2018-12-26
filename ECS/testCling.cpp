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
#include "ScriptInclude.hpp"

template<typename T>
T* GetFunction(cling::Interpreter& interpreter, const std::string& functionName) {
  void* functionAddr = interpreter.getAddressOfGlobal(functionName);
  T* func = cling::utils::VoidToFunctionPtr<T*>(functionAddr);
  return func;
}

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


    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" ISystem* CreateSystem() { return new Velocity(); } ");

    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void DeleteSystem(ISystem* sys) { delete sys; } ");

    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void CallUpdate(ISystem* system) { system->Update(0.0f); } ");

    interp.declare("#include \"ScriptInclude.hpp\" \n  TypeInfo GetTypeInfo(void* ptr) { Position* pos = (Position*)ptr; return pos->GetType(); } ");
    interp.declare("#include \"ScriptInclude.hpp\" \n extern \"C\" void* CreateComponent() { Position* pos = new Position(); pos->x = 123; pos->y = 456; return pos; } ");

    auto createSystem = GetFunction<ISystem*()>(interp, "CreateSystem");
    auto deleteSystem = GetFunction<void(ISystem*)>(interp, "DeleteSystem");
    auto callUpdate = GetFunction<void(ISystem*)>(interp, "CallUpdate");
    auto getTypeInfo = GetFunction<TypeInfo(void*)>(interp, "_Z11GetTypeInfoPv");
    auto createComponent = GetFunction<void*()>(interp, "CreateComponent");

    void* component = createComponent();

    auto typeInfo = getTypeInfo(component);

    minijson::writer_configuration config;
    config = config.pretty_printing(true);
    minijson::object_writer writer(std::cout, config);
    typeInfo.Serialize(writer);
    writer.close();

    ISystem* system = createSystem();

     system->Update(1.0f);

    deleteSystem(system);
    return 0;
}
