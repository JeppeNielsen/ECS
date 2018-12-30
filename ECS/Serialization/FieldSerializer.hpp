//
//  FieldSerializer.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 20/12/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once

#include "minijson_writer.hpp"
#include "minijson_reader.hpp"
#include "GameObject.hpp"

namespace ECS {
    template<typename T, typename I = void>
    struct FieldSerializer {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const T& field) {
            writer.write(name.c_str(), field);
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, T& field) {
            minijson::ignore(context);
        }
    };
  
    template<>
    struct FieldSerializer<int> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const int& field) {
            writer.write(name.c_str(), field);
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, int& field) {
            if (value.type() != minijson::Number) return;
            field = (int)value.as_long();
        }
    };
    
    template<>
    struct FieldSerializer<float> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const float& field) {
            writer.write(name.c_str(), field);
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, float& field) {
            if (value.type() != minijson::Number) return;
            field = (float)value.as_double();
        }
    };
    
    template<>
    struct FieldSerializer<std::string> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const std::string& field) {
            writer.write(name.c_str(), field);
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, std::string& field) {
            if (value.type() != minijson::String) return;
            field = value.as_string();
        }
    };
    
    template<typename T>
    struct FieldSerializer<std::vector<T>> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const std::vector<T>& vector) {
            auto array = writer.nested_array(name.c_str());
            for(const auto& item : vector) {
                FieldSerializer<T>::Serialize(array, name, item);
            }
            array.close();
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, std::vector<T>& vector) {
            minijson::parse_array(context, [&](minijson::value jsonValue) {
                vector.push_back(T{});
                auto& item = vector.back();
                FieldSerializer<T>::Deserialize(context, jsonValue, item);
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
    struct FieldSerializer<T, typename std::enable_if< HasGetTypeMethod<T>::value >::type> {
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
    
    template<>
    struct FieldSerializer<ECS::GameObject> {
        static void Serialize(minijson::object_writer& writer, const std::string& name, const ECS::GameObject& field) {
            writer.write(name.c_str(), "GameObject");
        }
        
        static void Deserialize(minijson::istream_context& context, minijson::value& value, ECS::GameObject& field) {
            if (value.type() != minijson::String) return;
        //    field = value.as_string();
        }
    };
}
