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




namespace JsonSerializer {

template<typename T, typename I = void>
struct SerializeType;

class Serializer {
public:
    template<typename T>
    void SerializeField(const std::string& name, T* field) {
        SerializeType<T>::Serialize(*this, name, *field);
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

struct IFieldInfo {
    IFieldInfo(const std::string& name) : name(name) {}
    virtual ~IFieldInfo() = default;
    virtual void Set(const void* value) = 0;
    virtual std::unique_ptr<IFieldInfo> Clone() = 0;
    virtual void Serialize(void* serializer) = 0;
    
    std::string name;
};

template<typename S, typename T>
struct FieldInfo : public IFieldInfo {
    FieldInfo(const std::string name, T& field)
    : IFieldInfo(name), field(&field) { }
    
    void Set(const void* value) override {
        *field = *static_cast<const T*>(value);
    }
    
    std::unique_ptr<IFieldInfo> Clone() override {
        return std::make_unique<FieldInfo<S, T>>(name, *field);
    }
    
    void Serialize(void* serializer) override {
        S* s = static_cast<S*>(serializer);
        s->template SerializeField<T>(name, field);
    }
    
    T* field;
};

template<typename S>
struct TypeInfo {
    using Fields = std::vector<std::unique_ptr<IFieldInfo>>;

    TypeInfo() = default;
    TypeInfo(const TypeInfo& other) {
        name = other.name;
        for(auto& field : other.fields) {
            fields.emplace_back(field->Clone());
        }
    }
    TypeInfo(TypeInfo&&) = default;
    TypeInfo & operator=(TypeInfo &&) = default;
    
    TypeInfo & operator=(const TypeInfo & other) {
        name = other.name;
        fields.clear();
        for(auto& field : other.fields) {
            fields.emplace_back(field->Clone());
        }
        return *this;
    }
    
    template<typename T>
    void AddField(const std::string name, T& field) {
        fields.emplace_back(std::make_unique<FieldInfo<S, T>>(name, field));
    }
    
    template<typename T>
    void SetField(const std::string& name, const T& value) {
        for(auto& field : fields) {
            if (field->name == name) {
                field->Set(&value);
                break;
            }
        }
    }
    
    void Serialize(S& serializer) {
        for(auto& field : fields) {
            field->Serialize(&serializer);
        }
    }

    std::string name;
    Fields fields;
};

#define TYPE_FIELDS_BEGIN \
public: \
template<typename S> \
TypeInfo<S> GetType() { \
    TypeInfo<S> type; \
    type.name = ClassNameHelper::GetName<std::remove_pointer_t<decltype(this)>>();

#define TYPE_FIELD(field) \
    type.AddField(#field, field);

#define TYPE_FIELDS_END \
    return type; \
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



int main() {

    JsonSerializer::Serializer serializer;
    
    
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
    */

    Position pos { 1,2, 999};
    pos.floats.push_back(2.0f);
    pos.floats.push_back(4.0f);
    pos.floats.push_back(6.0f);
    
    
    auto type = pos.GetType<decltype(serializer)>();
    type.SetField("x", 100.0f);
    type.SetField("floats", std::vector<float> {5,6,7,10,20,30,40});
    
    type.Serialize(serializer);
    
    
    return 0;
}
