//
//  FileHelper.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 30/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <string>
#include <functional>

namespace ECS {
    class FileHelper {
    public:
        static void ParseFile(const std::string& filename, const std::function<void(const std::string& line)>& lineRead);
    };
}
