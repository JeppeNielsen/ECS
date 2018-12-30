//
//  FileHelper.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 30/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "FileHelper.hpp"
#include <fstream>

using namespace ECS;

void FileHelper::ParseFile(const std::string& filename, const std::function<void(const std::string& line)>& lineRead) {
    std::ifstream file(filename);
    std::string line;
    if (file.is_open()) {
        while ( std::getline (file,line) ) {
            lineRead(line);
        }
        file.close();
    }
}
