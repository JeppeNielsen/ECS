//
//  FieldVisitor.hpp
//  ECS
//
//  Created by Jeppe Nielsen on 02/08/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <iostream>
#include "ClassNameHelper.hpp"
#include "IdHelper.hpp"
#include <vector>

namespace ECS {

class FieldVisitor;
    
template <typename T>
struct HasVisitFieldsMethod {
    struct dummy { /* something */ };

    template <typename C, typename P>
    static auto test(P * p) -> decltype(std::declval<C>().VisitFields(*p), std::true_type());

    template <typename, typename>
    static std::false_type test(...);

    typedef decltype(test<T, dummy>(nullptr)) type;
    static const bool value = std::is_same<std::true_type, decltype(test<T, dummy>(nullptr))>::value;
};

namespace FieldVisitorDetails {
    template<typename T, typename I = void>
    struct FieldVisitorDelegator;
}

class FieldVisitor {
private:
    using Id = IdHelper<struct FieldVisitorTypeId>;
    using Fields = std::vector<std::function<void(const std::string&, void*)>>;
public:
    using TypeNameFunction = std::function<void(const std::string&)>;
    
    void Begin(const std::string& typeName) {
        onBegin(typeName);
    }
    
    void End(const std::string& typeName) {
        onEnd(typeName);
    }

    template<typename T>
    void Field(const std::string& name, T& field) {
        FieldVisitorDetails::FieldVisitorDelegator<T>::Visit(*this, name, field);
    }
    
    template<typename T, typename Func>
    void OnField(Func&& func) {
        const auto id = Id::GetId<std::remove_const_t<T>>();
        if (id>=fields.size()) {
            fields.resize(id + 1, nullptr);
        }
        fields[id] = [&func] (const std::string& name, void* ptr) {
            func(name, *static_cast<T*>(ptr));
        };
    }
    
    void OnBegin(const TypeNameFunction& function) {
        onBegin = function;
    }

    void OnEnd(const TypeNameFunction& function) {
        onEnd = function;
    }

private:
    template<typename T>
    void Visit(const std::string& name, T& field) {
        const auto id = Id::GetId<std::remove_const_t<T>>();
        if (id>=fields.size()) return;
        if (!fields[id]) return;
        fields[id](name, (void*)static_cast<T*>(&field));
    }
    
    template<typename T>
    void VisitVector(const std::string& name, std::vector<T>& vector) {
        std::cout << "vector" << std::endl;
        for(auto& e : vector) {
            Visit(name, e);
        }
    }
    
    template<typename T, typename I>
    friend class FieldVisitorDetails::FieldVisitorDelegator;
    
    Fields fields;
    TypeNameFunction onBegin;
    TypeNameFunction onEnd;
};

namespace FieldVisitorDetails {
    template<typename T, typename I>
    struct FieldVisitorDelegator {
        static void Visit(FieldVisitor& visitor, const std::string& name, T& field) {
            visitor.Visit(name, field);
        }
    };
    
    template<typename T>
    struct FieldVisitorDelegator<std::vector<T>> {
        static void Visit(FieldVisitor& visitor, const std::string& name, std::vector<T>& field) {
            visitor.VisitVector(name, field);
        }
    };
    
    template<typename T>
    struct FieldVisitorDelegator<T, typename std::enable_if< HasVisitFieldsMethod<T>::value >::type> {
        static void Visit(FieldVisitor& visitor, const std::string& name, T& field) {
            field.VisitFields(visitor);
        }
    };
}




}

#define TYPE_FIELDS_BEGIN \
public: \
template<typename Visitor> \
void VisitFields(Visitor& visitor) { \
    const auto& name = ClassNameHelper::GetName<std::remove_pointer_t<decltype(this)>>(); \
    visitor.Begin(name);

#define TYPE_FIELD(field) \
    visitor.Field(#field, this->field);

#define TYPE_FIELDS_END \
    visitor.End(name); \
} \
private:

