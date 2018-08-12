//
//  testPartialSpecializations.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 05/08/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include <iostream>
#include <vector>
#include "FieldVisitor.hpp"

template<typename T, typename Enable = void>
struct VisitorClass {
    static void OnField(T& field) {
        std::cout << field << std::endl;
    }
};

template<typename T>
struct VisitorClass<std::vector<T>> {
    static void OnField(std::vector<T>& field) {
        for(auto& f : field) {
            VisitorClass<T>::OnField(f);
        }
    }
};


struct VisitorDelegator {
    
    template<typename T>
    static void Field(T& field) {
        VisitorClass<T>::OnField(field);
    }

};

template<typename VisitorClass>
struct Visitor {

    template<typename T>
    void Field(T& field) {
        VisitorClass::Field(field);
    }
};


struct IDelegator {
    virtual void Invoke(void*) = 0;
};

template<typename T, typename Iterator>
struct Delegator : public IDelegator {
    Iterator iterator;
    
    void Invoke(void* instance) {
        T* i = static_cast<T*>(instance);
        i->VisitFields(iterator);
    }
};

struct IContainer {
    virtual void Invoke(IDelegator* del) = 0;
};

template<typename T>
struct Container : public IContainer {
    
    T instance;
    
    void Invoke(IDelegator* det) override {
        det->Invoke(&instance);
    }
};


struct Transform {

    float x;
    float y;

    TYPE_FIELDS_BEGIN
    TYPE_FIELD(x)
    TYPE_FIELD(y)
    TYPE_FIELDS_END
};

struct Iterator {

void Begin(const std::string& type) {

}

template<typename T>
void Field(const std::string& name, T& field) {
    std::cout << name << " = "<<field << std::endl;
}

void End(const std::string& type) {
    
}

};

int main_partial() {

    IContainer* container = new Container<Transform>();
    
    Container<Transform>* ptr = static_cast<Container<Transform>*>(container);
    ptr->instance.x = 12;
    ptr->instance.y = 24;
    
    IDelegator* delegator = new Delegator<Transform, Iterator>;
    container->Invoke(delegator);
    
    

    


    int hello = 3;
    std::string bla = "wfiouherg";
    
    Visitor<VisitorDelegator> vis;
    vis.Field(hello);
    vis.Field(bla);


    return 0;
}
