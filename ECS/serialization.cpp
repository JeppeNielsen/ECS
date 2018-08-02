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
#include "Scene.hpp"

using namespace ECS;
/*
template <typename T>
struct HasIterateTypeMethod {
    struct dummy {  };

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


namespace ECS {

struct FieldIterator;

namespace FieldIteratorInternal {
    template<typename T, typename I = void>
    struct FieldDelegator;
}

struct FieldIterator {
    void Begin(const std::string& typeName) {
        std::cout << "Type begin :" << typeName << std::endl;
    }

    template<typename T>
    void Field(const std::string& name, T& field) {
        FieldIteratorInternal::FieldDelegator<T>::Iterate(*this, name, field);
    }
    
    template<typename T>
    void Iterate(const std::string& name, T& field) {
        std::cout << name << std::endl;
    }
    
    template<typename T>
    void IterateVector(const std::string& name, const std::vector<T>& vector) {
        std::cout << "vector" << std::endl;
        for(auto& e : vector) {
            Iterate(name, e);
        }
    }
    
    void End() {
        std::cout << "Type ended" << std::endl;
    }
};

namespace FieldIteratorInternal {
    template<typename T, typename I>
    struct FieldDelegator {
        static void Iterate(FieldIterator& iterator, const std::string& name, const T& field) {
            iterator.Iterate(name, field);
        }
    };
    
    template<typename T>
    struct FieldDelegator<std::vector<T>> {
        static void Iterate(FieldIterator& iterator, const std::string& name, const std::vector<T>& field) {
            iterator.IterateVector(name, field);
        }
    };
}
}

*/

class JsonSerializer {
public:

    ECS::FieldVisitor fieldVisitor;
    
    JsonSerializer() {
        fieldVisitor.OnField<int>([](const std::string& name, int& field) {
            std::cout << "Ints -> " << name << " " << field << std::endl;
        });
        
        fieldVisitor.OnField<float>([](const std::string& name, float& field) {
            std::cout << "Floats -> " << name <<" "<< field << std::endl;
        });
        
        fieldVisitor.OnField<std::string>([](const std::string& name, std::string& field) {
            std::cout << "String -> " << name << " "<< field << std::endl;
        });
        
        fieldVisitor.OnBegin([] (const std::string& type) {
            std::cout << "Type started : "<<type << std::endl;
        });
        
        fieldVisitor.OnEnd([] (const std::string& type) {
            std::cout << "Type ended : "<<type << std::endl;
        });
    }

    void Begin(const std::string& typeName) {
        std::cout << "Type begin :" << typeName << std::endl;
    }

    template<typename T>
    void Field(const std::string& name, T& field) {
        
    }
    
    void End() {
        std::cout << "Type ended" << std::endl;
    }
    
    void SerializeObject(GameObject root, std::ostream& stream) {
        root.IterateComponents([root, this](auto& componentType) {
            componentType->VisitFields(root.Id(), fieldVisitor);
        });
    }
    
private:
    
};

/*
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



#define TYPE_FIELDS_BEGIN \
public: \
template<typename Iterator> \
void IterateFields(Iterator& iterator) { \
    iterator.Begin(ClassNameHelper::GetName<std::remove_pointer_t<decltype(this)>>());

#define TYPE_FIELD(field) \
    iterator.Field(#field, field);

#define TYPE_FIELDS_END \
    iterator.End(); \
} \
private:
*/

struct Node {
    std::string name;
    float width;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(name)
    TYPE_FIELD(width)
    TYPE_FIELDS_END
};

struct Position {

    
    std::string name;
    Node node;

    TYPE_FIELDS_BEGIN
    TYPE_FIELD(name)
    TYPE_FIELD(node)
    TYPE_FIELDS_END
};

struct Velocity {
    float vx;
    float vy;
};

int main() {


    Database database;
    Scene scene(database);
    auto go = scene.CreateObject();
    Node node {"First node",  3.0f };
    go.AddComponent<Position>("jeppe", node );
    go.AddComponent<Velocity>(2.0f,4.0f);
    
    JsonSerializer jsonSerializer;
    jsonSerializer.SerializeObject(go, std::cout);

    return 0;
}
