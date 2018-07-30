//
//  serialization.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 26/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <sstream>
#include "ClassNameHelper.hpp"
#include "IdHelper.hpp"
#include "MetaHelper.hpp"
#include "Hierarchy.hpp"

template <typename T>
struct HasIterateTypeMethod {
    struct dummy { /* something */ };

    template <typename C, typename P>
    static auto test(P * p) -> decltype(std::declval<C>().IterateType(*p), std::true_type());

    template <typename, typename>
    static std::false_type test(...);

    typedef decltype(test<T, dummy>(nullptr)) type;
    static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
};

namespace BinarySerializer {

class Serializer {
public:

    void Begin(const std::string& typeName) {
        std::cout << "Binary begin :" << typeName << std::endl;
    }

    template<typename T>
    void Field(const std::string& name, T& field) {
        std::cout << name << "  :" << "" << std::endl;
    }
    
    void End() {
        std::cout << "Binary ended"  << std::endl;
    }

private:
    
};


}


namespace JsonSerializer {

template<typename T, typename I = void>
struct SerializeType;

class Serializer {
public:

    void Begin(const std::string& typeName) {
        std::cout << "Type begin :" << typeName << std::endl;
    }

    template<typename T>
    void Field(const std::string& name, T& field) {
        SerializeType<T>::Serialize(*this, name, field);
    }
    
    void End() {
        std::cout << "Type ended"  << std::endl;
    }

private:
    
};

template<typename T, typename I>
struct SerializeType {
    static void Serialize(Serializer& serializer, const std::string& name, const T& field) {
        std::cout << " base " << name << std::endl;
    }
    static void Deserialize(Serializer& serializer, const std::string& name, T& field) { }
};

template<>
struct SerializeType<int> {
    static void Serialize(Serializer& serializer, const std::string& name, const int& field) {
        std::cout << " int " << name << std::endl;
    }
    static void Deserialize(Serializer& serializer, const std::string& name, int& field) { }
};

template<>
struct SerializeType<float> {
    static void Serialize(Serializer& serializer, const std::string& name, const float& field) {
        std::cout << " float " << name << "="<< field << std::endl;
    }
    static void Deserialize(Serializer& serializer, const std::string& name, float& field) { }
};

template<typename T>
struct SerializeType<std::vector<T>> {
    static void Serialize(Serializer& serializer, const std::string& name, const std::vector<T>& field) {
        std::cout << " vector " << name << std::endl;
        int index =0;
        for(auto t: field) {
            std::stringstream ss;
            ss<<index++;
            SerializeType<T>::Serialize(serializer, name + " index : " + ss.str() , t);
        }
    }
    static void Deserialize(Serializer& serializer, const std::string& name, std::vector<T>& field) {
    
    }
};

}

#define TYPE_FIELDS_BEGIN \
public: \
template<typename Iterator> \
void IterateType(Iterator& iterator) { \
    iterator.Begin(ClassNameHelper::GetName<std::remove_pointer_t<decltype(this)>>());

#define TYPE_FIELD(field) \
    iterator.Field(#field, field);

#define TYPE_FIELDS_END \
    iterator.End(); \
} \
private:


struct Position {

    float x;
    float y;
    int hello;
    std::string name;
    std::vector<float> floats;

    TYPE_FIELDS_BEGIN
    TYPE_FIELD(x)
    TYPE_FIELD(y)
    TYPE_FIELD(hello)
    TYPE_FIELD(name)
    TYPE_FIELD(floats)
    TYPE_FIELDS_END
};

struct Velocity {
    float vx;
    float vy;
};


template<typename Iterator, typename T>
void Iterate(Iterator& iterator, T& type) {
    ECS::Meta::static_if<HasIterateTypeMethod<T>, T&>(type, [&iterator](auto& type) {
        type.IterateType(iterator);
    });
}

int main_serialize() {

    Position pos1;
    Velocity vel1;
    
    
    JsonSerializer::Serializer jsonSerializer;
    
    Iterate(jsonSerializer, pos1);
    Iterate(jsonSerializer, vel1);
    
    BinarySerializer::Serializer binarySerializer;
    Iterate(binarySerializer, pos1);
    Iterate(binarySerializer, vel1);
    
    
    //IterateFields(pos1);
    
    /*
    serializer.AddType<float>([] (auto& name, auto& field) {
        std::cout << "floats : " << name << "=" << field << std::endl;
    });
    serializer.AddType<int>([] (auto& name, auto& field) {
        std::cout << "ints : " << name << "=" << field << std::endl;
    });
    
    serializer.AddType<std::vector<I>>([] (auto& name, auto& field) {
        std::cout << "ints : " << name << "=" << field << std::endl;
    });
    

    Position pos { 1,2, 999};
    pos.floats.push_back(2.0f);
    pos.floats.push_back(4.0f);
    pos.floats.push_back(6.0f);
    
    JsonSerializer::Serializer json;
    pos.IterateType(json);
    
    
    BinarySerializer::Serializer binary;
    pos.IterateType(binary);
    */
    
   
    return 0;
}
