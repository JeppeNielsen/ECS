//
//  TestReflection.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 29/11/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include "TypeInfo.hpp"
#include <sstream>

using namespace ECS;

struct SubInfo {
    int hejMedDig;

    TYPE_FIELDS_BEGIN
    TYPE_FIELD(hejMedDig)
    TYPE_FIELDS_END
};

struct Point {
    float x;
    float y;
    
    SubInfo subInfo;

    TYPE_FIELDS_BEGIN
    TYPE_FIELD(x)
    TYPE_FIELD(y)
    TYPE_FIELD(subInfo)
    TYPE_FIELDS_END
};

struct Location {
    float x;
    float y;
    
    std::vector<Point> points;
    
    Point pointEmbedded;

    TYPE_FIELDS_BEGIN
    TYPE_FIELD(x)
    TYPE_FIELD(y)
    TYPE_FIELD(points)
    TYPE_FIELD(pointEmbedded)
    TYPE_FIELDS_END
};


int main_reflection() {
    
    Location position {123, 456};
    position.points.push_back({3,5});
    position.points.push_back({13,5});
    position.points.push_back({33,45});
    
    std::stringstream stream;
 {
 
    minijson::writer_configuration config;
    config = config.pretty_printing(true);
    minijson::object_writer writer(stream, config);
    
    auto info = position.GetType();
    info.Serialize(writer);
    writer.close();
 }
 
 std::cout << stream.str() << std::endl;
 
 
    Location newLocation;
    
    minijson::istream_context context(stream);
    auto in = newLocation.GetType();
    in.Deserialize(context);
    
    Location copy = newLocation;
    
    {
    
    minijson::writer_configuration config;
    config = config.pretty_printing(true);
    minijson::object_writer writer(std::cout, config);
    
    auto copyInfo = copy.GetType();
    copyInfo.Serialize(writer);
    writer.close();
    
    }
    
    return 0;
}
