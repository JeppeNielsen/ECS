//
//  FieldInfo.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 29/11/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <string>
#include <iostream>
#include "minijson_writer.hpp"
#include "minijson_reader.hpp"

namespace ECS {
    template<typename T, typename I = void>
    struct Serializer {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const T& field) {
            writer.write(name.c_str(), field);
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, T& field) {
            minijson::ignore(context);
        }
    };
    
    template<>
    struct Serializer<int> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const int& field) {
            writer.write(name.c_str(), field);
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, int& field) {
            if (value.type() != minijson::Number) return;
            field = (int)value.as_long();
        }
    };
    
    template<>
    struct Serializer<float> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const float& field) {
            writer.write(name.c_str(), field);
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, float& field) {
            if (value.type() != minijson::Number) return;
            field = (float)value.as_double();
        }
    };
    
    template<typename T>
    struct Serializer<std::vector<T>> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const std::vector<T>& vector) {
            auto array = writer.nested_array(name.c_str());
            for(const auto& item : vector) {
                Serializer<T>::Serialize(array, name, item);
            }
            array.close();
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, std::vector<T>& vector) {
            minijson::parse_array(context, [&](minijson::value jsonValue) {
                vector.push_back(T{});
                auto& item = vector.back();
                Serializer<T>::Deserialize(context, jsonValue, item);
            });
        }
    };
    
    template <typename T>
    struct HasGetTypeMethod {
        struct dummy { };

        template <typename C, typename P>
        static auto test(P * p) -> decltype(std::declval<C>().GetType(), std::true_type());

        template <typename, typename>
        static std::false_type test(...);

        typedef decltype(test<T, dummy>(nullptr)) type;
        static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
    };
    
    template<typename T>
    struct Serializer<T, typename std::enable_if< HasGetTypeMethod<T>::value >::type> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const T& getTypeObject) {
            auto nestedObject = writer.nested_object(name.c_str());
       
            auto type = ((T&)getTypeObject).GetType();
            type.Serialize(nestedObject);
            
            nestedObject.close();
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, T& getTypeObject) {
            auto type = getTypeObject.GetType();
            type.Deserialize(context);
        }
    };

    class FieldInfo {
    public:
        
        FieldInfo() { }
    
        template <class T>
        FieldInfo(const std::string& name, T& data) : field(std::make_shared<Field<T>>(name, data)) {}

        void Serialize(minijson::object_writer& writer) const {
            field->Serialize(writer);
        }
        
        void Deserialize(minijson::istream_context& context, minijson::value jsonValue) const {
            field->Deserialize(context, jsonValue);
        }
        
        const std::string& Name() {
            return field->name;
        }
    private:
        struct IField {
            virtual ~IField() = default;
            virtual void Serialize(minijson::object_writer& writer) const = 0;
            virtual void Deserialize(minijson::istream_context& context, minijson::value jsonValue) const = 0;
            std::string name;
        };
        template <class T>
        struct Field : IField {
            Field(const std::string& name, T& field) : field(&field) { this->name = name; }
            virtual void Serialize(minijson::object_writer& writer) const override {
                Serializer<T>::Serialize(writer, name, *field);
            }
            virtual void Deserialize(minijson::istream_context& context, minijson::value jsonValue) const override {
                Serializer<T>::Deserialize(context, jsonValue, *field);
            }
            T* field;
        };
        
        std::shared_ptr<const IField> field;
    };
}
