//
//  SerializedScene.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 01/01/2019.
//  Copyright © 2019 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "Scene.hpp"
#include <string>
#include <unordered_map>

namespace ECS {
    class SerializedScene {
    public:
        SerializedScene() = default;
        SerializedScene(const SerializedScene&) = default;
        SerializedScene(SerializedScene&&) = default;
        void Serialize(Scene& scene, const std::function<bool(int)>& componentFilter);
        void Deserialize(Scene& scene) const;
    private:
        using SerializedData = std::unordered_map<GameObjectId, std::string>;
        SerializedData data;
    };
}
