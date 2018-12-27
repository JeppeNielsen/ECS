//
//  TypeInfo.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 29/11/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <string>
#include <vector>
#include <memory>
#include "FieldInfo.hpp"
#include "ClassNameHelper.hpp"
#include "minijson_reader.hpp"
#include "minijson_writer.hpp"

namespace ECS {
    class TypeInfo {
    public:
        TypeInfo(const std::string& name) : name(name) { }
        
        template<typename T>
        void AddField(const std::string& name, T& field) {
            fields.emplace_back(FieldInfo(name, field));
        }
        
        void Serialize(minijson::object_writer& writer) {
            for(auto& field : fields) {
                field.Serialize(writer);
            }
        }
        
        void Deserialize(minijson::istream_context& context) {
            minijson::parse_object(context, [&] (const char* name, minijson::value jsonValue) {
                FieldInfo field;
                if (TryFindInfo(name, field)) {
                    field.Deserialize(context, jsonValue);
                }
            });
        }
        
        const std::string& Name() {
            return name;
        }
        
    private:
    
        bool TryFindInfo(const std::string& name, FieldInfo& info) {
            for(auto& field : fields) {
                if (field.Name() == name) {
                    info = field;
                    return true;
                }
            }
            return false;
        }
    
        std::string name;
        using Fields = std::vector<FieldInfo>;
        Fields fields;
    };
    
#define TYPE_FIELDS_BEGIN \
public: \
ECS::TypeInfo GetType() { \
    const static auto& name = ClassNameHelper::GetName<std::remove_const_t<std::remove_pointer_t<decltype(this)>>>(); \
    ECS::TypeInfo info(name);

#define TYPE_FIELD(field) \
    info.AddField(#field, this->field);

#define TYPE_FIELDS_END \
        return info; \
     \
} \
private:
}
