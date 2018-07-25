//
//  ecs.cpp
//  ECS
//
//  Created by Jeppe Nielsen on 22/07/2018.
//  Copyright © 2018 Jeppe Nielsen. All rights reserved.
//

#include <memory>
#include <vector>
#include <iostream>
#include <set>
#include <assert.h>

using GameObjectId = std::uint32_t;

static constexpr auto GameObjectIdIndexMask = 0xFFFFF;
static constexpr auto GameObjectIdNull = ~0;

namespace Meta {
    template<std::size_t I = 0, typename Tuple, typename Func>
    typename std::enable_if< I == std::tuple_size<Tuple>::value, void >::type
    for_each(const Tuple& tuple, Func&& func) {
    }

    template<std::size_t I = 0, typename Tuple, typename Func>
    typename std::enable_if< I != std::tuple_size<Tuple>::value, void >::type
    for_each(const Tuple& tuple, Func&& func) {
        func(std::get<I>(tuple));
        for_each<I + 1>(tuple, func);
    }
}

template<typename T>
class IdHelper {
public:
    template<typename...Type>
    static const size_t GetId() {
        static std::size_t id = idCounter++;
        return id;
    }
private:
    static std::size_t idCounter;
};

template<typename T>
std::size_t IdHelper<T>::idCounter {0};

struct IContainer {
    virtual ~IContainer() = default;
    
    bool Contains(const GameObjectId id) const {
        const auto index = id & GameObjectIdIndexMask;
        return index<indicies.size() && indicies[index] != GameObjectIdNull;
    }
    
    virtual void Destroy(const GameObjectId object) = 0;
    
    std::vector<std::uint32_t> indicies;
};

template<typename T>
struct Container : public IContainer {

    Container() = default;
    virtual ~Container() = default;

    Container(const Container &) = delete;
    Container(Container &&) = default;

    Container & operator=(const Container &) = delete;
    Container & operator=(Container &&) = default;
    
    void Create(const GameObjectId id) {
        assert(!Contains(id));
        const auto index = id & GameObjectIdIndexMask;
        if (index>=indicies.size()) {
            indicies.resize(index + 1, GameObjectIdNull);
        }
        indicies[index] = (std::uint32_t)elements.size();
        elements.emplace_back(T());
        references.emplace_back(1);
    }
    
    void Reference(const GameObjectId id, const GameObjectId referenceId) {
        assert(!Contains(id));
        assert(Contains(referenceId));
        const auto index = id & GameObjectIdIndexMask;
        const auto referenceIndex = referenceId & GameObjectIdIndexMask;
        if (index>=indicies.size()) {
            indicies.resize(index + 1, GameObjectIdNull);
        }
        
        indicies[index] = referenceIndex;
        ++references[referenceIndex];
    }
    
    void Destroy(const GameObjectId id) override {
        assert(Contains(id));
        const auto index = id & GameObjectIdIndexMask;
        
        if ((references[index]--)==0) {
            auto tmp = std::move(elements.back());
            elements[index] = std::move(tmp);
            elements.pop_back();
            references[index] = references.back();
            references.pop_back();
            indicies[indicies.size() - 1] = index;
        }
        
        indicies[index] = GameObjectIdNull;
    }
    
    T* Get(const GameObjectId id) {
        const auto index = id & GameObjectIdIndexMask;
        return &elements[indicies[index]];
    }
    
    std::vector<T> elements;
    std::vector<std::uint16_t> references;
};

struct ObjectList {
    using Indicies = std::vector<std::size_t>;
    using Objects = std::vector<GameObjectId>;

    void Add(const GameObjectId object) {
        const auto objectIndex = object & GameObjectIdIndexMask;
        if (objectIndex>=indicies.size()) {
            indicies.resize(objectIndex + 1, GameObjectIdNull);
        }
        indicies[objectIndex] = objects.size();
        objects.emplace_back(object);
    }
    
    void Remove(const GameObjectId object) {
        const auto objectIndex = object & GameObjectIdIndexMask;
        const auto last = objects.back();
        const auto lastIndex = last & GameObjectIdIndexMask;
        indicies[lastIndex] = objectIndex;
        objects[objectIndex] = objects[lastIndex];
        objects.pop_back();
        indicies[objectIndex] = GameObjectIdNull;
    }
    
    bool Contains(const GameObjectId object) const {
        const auto index = object & GameObjectIdIndexMask;
        return index<indicies.size() && indicies[index] != GameObjectIdNull;
    }

    Indicies indicies;
    Objects objects;
};

struct Scene;

struct Database {
    using IdHelper = IdHelper<struct ComponentId>;
    using Components = std::vector<std::unique_ptr<IContainer>>;
    
    template<typename T>
    void AssureComponent() {
        const auto id = IdHelper::GetId<T>();
        if (id>=components.size()) {
            components.resize(id + 1);
        }
        if (components[id]) return;
        components[id] = std::make_unique<Container<T>>();
    }
    
    template<typename T>
    Container<T>& ComponentContainer() {
        const auto id = IdHelper::GetId<T>();
        return static_cast<Container<T>&>(*components[id]);
    }
    
    template<typename T>
    T* AddComponent(const GameObjectId objectId) {
        AssureComponent<T>();
        ComponentContainer<T>().Create(objectId);
        return ComponentContainer<T>().Get(objectId);
    }
    
    void RemoveComponent(const GameObjectId objectId, const std::size_t componentId) {
        if (componentId<components.size() && components[componentId]) {
            components[componentId]->Destroy(objectId);
        }
    }
    
    template<typename T>
    T* GetComponent(const GameObjectId objectId) {
        const auto componentId = IdHelper::GetId<T>();
        if (componentId>=components.size()) return nullptr;
        auto& container = ComponentContainer<T>();
        return container.Contains(objectId) ? container.Get(objectId) : nullptr;
    }
    
    void RemoveAllComponents(const GameObjectId objectId) {
        for(auto& component : components) {
            if (component->Contains(objectId)) {
                component->Destroy(objectId);
            }
        }
    }
    
    bool GameObjectIdValid(const GameObjectId object) {
        const auto pos = GameObjectId(object & GameObjectIdIndexMask);
        return (pos < objects.size() && objects[pos] == object);
    }
    
    GameObjectId CreateObject() {
        GameObjectId object;

        if(available) {
            const auto objectIndex = next;
            const auto version = objects[objectIndex] & (~GameObjectIdIndexMask);

            object = objectIndex | version;
            next = objects[objectIndex] & GameObjectIdIndexMask;
            objects[objectIndex] = object;
            --available;
        } else {
            object = GameObjectId(objects.size());
            objects.push_back(object);
            // traits_type::entity_mask is reserved to allow for null identifiers
            //assert(entity < traits_type::entity_mask);
        }
        return object;
    }
    
    void Remove(const GameObjectId objectId) {
        const auto object = objectId & GameObjectIdIndexMask;
        const auto version = (((objectId >> 20) + 1) & GameObjectIdIndexMask) << 20;
        const auto node = (available ? next : ((object + 1) & GameObjectIdIndexMask)) | version;

        objects[object] = node;
        next = object;
        ++available;
    }
    
private:
    Components components;
    
    std::vector<GameObjectId> objects;
    std::size_t available {};
    GameObjectId next {};
};

struct ISystem {
    virtual ~ISystem() = default;
    virtual void TryAddObject(const GameObjectId object) = 0;
    virtual void TryRemoveObject(const GameObjectId object) = 0;
    virtual void Update(float dt) = 0;
    ObjectList objects;
};

struct Scene;

class GameObject {
public:
    GameObject(Scene& scene, const GameObjectId id) : scene(scene), id(id) { }
    ~GameObject() = default;
    GameObject(GameObject&&) = default;
    GameObject(GameObject&) = default;
    GameObject(const GameObject&) = default;

    GameObject & operator=(const GameObject &) = default;
    GameObject & operator=(GameObject &&) = default;
    
    bool operator == (const GameObject& other) const {
        return id == other.id && &scene == &other.scene;
    }
    
    bool operator != (const GameObject& other) const {
        return !(id == other.id && &scene == &other.scene);
    }
    
    explicit operator bool() const;
    
    template<typename T>
    T* AddComponent() const;
    
    template<typename T>
    T* GetComponent() const;
    
    template<typename T>
    void RemoveComponent() const;
    
    void Remove() const;

private:
    Scene& scene;
    GameObjectId id;
};


struct Scene {
    using SystemIdHelper = IdHelper<struct SystemIdHelper>;
    using Systems = std::vector<std::unique_ptr<ISystem>>;
    using Actions = std::set<GameObjectId>;
    using RemoveComponentActions = std::set<std::pair<GameObjectId, size_t>>;
    
    friend class GameObject;
    
    Scene(Database& database) : database{ database } {
    }
    
    ~Scene() {
        for(const auto object : objects.objects) {
            RemoveObjectFromDatabase(object);
        }
    }

    template<typename S>
    S& CreateSystem() {
        const auto systemId = SystemIdHelper::GetId<S>();
        if (systemId>=systems.size()) {
            systems.resize(systemId + 1);
        }
        if (!systems[systemId]) {
            systems[systemId] = std::make_unique<S>();
            S& system = static_cast<S&>(*systems[systemId]);
            system.Initialize(*this);
            return system;
        } else {
            return static_cast<S&>(*systems[systemId]);
        }
    }
    
    GameObject CreateObject() {
        const auto object = database.CreateObject();
        objects.Add(object);
        return GameObject(*this, object);
    }
    
    void Update(float dt) {
        DoActions(addComponentActions, [this] (const auto object) {
            for(const auto& system : systems) {
                if (!system->objects.Contains(object)) {
                    system->TryAddObject(object);
                }
            }
        });
        DoActions(removeComponentActions, [this] (const auto pair) {
            database.RemoveComponent(pair.first, pair.second);
            for(const auto& system : systems) {
                if (system->objects.Contains(pair.first)) {
                    system->TryRemoveObject(pair.first);
                }
            }
        });
        
        DoActions(removeActions, [this] (const auto object) {
            RemoveObjectFromDatabase(object);
            objects.Remove(object);
        });
        
        UpdateSystems(dt);
    }
    
    Database& GetDatabase() {
        return database;
    }
private:
    
    void RemoveObject(const GameObjectId object) {
        removeActions.insert(object);
    }
    
    template<typename T>
    T* AddComponent(const GameObjectId object) {
        addComponentActions.insert(object);
        return database.AddComponent<T>(object);
    }
    
    template<typename T>
    void RemoveComponent(const GameObjectId object) {
        const auto id = Database::IdHelper::GetId<T>();
        removeComponentActions.insert(std::make_pair(object, id));
    }
    
    template<typename Actions, typename Func>
    void DoActions(Actions& actions, Func&& func) {
        if (actions.empty()) return;
        for(const auto& object : actions)  {
            func(object);
        }
        actions.clear();
    }
    
    void UpdateSystems(float dt) {
        for(const auto& system : systems) {
            system->Update(dt);
        }
    }
    
    void RemoveObjectFromDatabase(const GameObjectId object) {
        database.RemoveAllComponents(object);
        for(const auto& system : systems) {
            if (system->objects.Contains(object)) {
                system->TryRemoveObject(object);
            }
        }
        database.Remove(object);
    }

    class Database& database;
    Systems systems;
    ObjectList objects;
    Actions addComponentActions;
    RemoveComponentActions removeComponentActions;
    Actions removeActions;
};

template<typename...T>
struct System : public ISystem {
    using Components = std::tuple<Container<T> &...>;

    void Initialize(Scene& scene);
    
    bool Match(const GameObjectId object) const {
        bool contains = true;
        Meta::for_each(*components, [&contains, object](const auto& component) {
           contains &= component.Contains(object);
        });
        return contains;
    }
    
    void TryAddObject(const GameObjectId object) override {
        if (!Match(object)) return;
        objects.Add(object);
        ObjectAdded(GameObject(*scene, object));
    }
    
    void TryRemoveObject(const GameObjectId object) override {
        if (Match(object)) return;
        ObjectRemoved(GameObject(*scene, object));
        objects.Remove(object);
    }
    
    struct Iterator {
    
        Iterator(Scene& scene, ObjectList::Objects& objects, std::size_t index) : scene(scene), objects(objects), index(index) {}
        Iterator(Iterator&&) = default;
        Iterator(Iterator&) = default;
        Iterator(const Iterator&) = default;
        
        Iterator begin() const {
            return Iterator(scene, objects, 0);
        }
        
        Iterator end() const {
            return Iterator(scene, objects, objects.size());
        }
        
        Iterator& operator++() { ++index; return *this; }
        bool operator!=(const Iterator & other) const { return index != other.index; }
        const GameObject operator*() const { return GameObject(scene, objects[index]); }
    
        size_t index;
        Scene& scene;
        std::vector<GameObjectId>& objects;
    };
    
    const Iterator Objects() {
        return Iterator(*scene, objects.objects, 0);
    }
    
protected:
    virtual void ObjectAdded(GameObject object) {}
    virtual void ObjectRemoved(GameObject object) {}
    Scene* scene;
private:
    std::unique_ptr<Components> components;
};


template<typename...T>
void System<T...>::Initialize(Scene& scene) {
    this->scene = &scene;
    components = std::make_unique<Components>((scene.GetDatabase().AssureComponent<T>(), scene.GetDatabase().ComponentContainer<T>())...);
}

GameObject::operator bool() const {
    return scene.database.GameObjectIdValid(id);
}

template<typename T>
T* GameObject::AddComponent() const {
    assert(operator bool());
    return scene.AddComponent<T>(id);
}

template<typename T>
T* GameObject::GetComponent() const {
    assert(operator bool());
    return scene.database.GetComponent<T>(id);
}

template<typename T>
void GameObject::RemoveComponent() const {
    assert(operator bool());
    scene.RemoveComponent<T>(id);
}

void GameObject::Remove() const {
    assert(operator bool());
    scene.RemoveObject(id);
}

struct Position {
    float x;
    float y;
};

struct Velocity {
    float vx;
    float vy;
};

struct Cloner {
    GameObject Target;
};

struct VelocitySystem : public System<Position, Velocity> {
    void ObjectAdded(GameObject object) override {
        std::cout << "Object added " << std::endl;
    }
    
    void ObjectRemoved(GameObject object) override {
        std::cout << "Object removed " << std::endl;
    }
    
    void Update(float dt) override {
        for(auto object : Objects()) {
            auto pos = object.GetComponent<Position>();
            auto vel = object.GetComponent<Velocity>();
            pos->x += vel->vx;
            std::cout << pos->x << std::endl;
        }
    }
};

int main() {

    
    Database database;
    Scene scene(database);
    scene.CreateSystem<VelocitySystem>();
    auto object1 = scene.CreateObject();
    
    //scene.AddComponent<Position>(object1);
    //scene.AddComponent<Velocity>(object1)->vx = 2;
    
    object1.AddComponent<Position>()->x = 123;
    object1.AddComponent<Velocity>();
    
    object1.RemoveComponent<Position>();
    

    
    if (object1) {
        std::cout << "is valid" << std::endl;
    }
    
    /*object1.Remove();
    object1.Remove();
    object1.Remove();
    */
    if (object1) {
        std::cout << "still valid" << std::endl;
    }
    
    scene.Update(1.0f);
    
    object1.AddComponent<Position>()->x = 234;
    
    if (!object1) {
        std::cout << "not valid" << std::endl;
    }
    
    std::vector<GameObject> objects;
    objects.push_back(object1);
    
    if (objects[0] == object1) {
        std::cout << "Same!"<<std::endl;
    }
    
    
    scene.Update(0);
    scene.Update(0);
    
    //scene.RemoveComponent<Position>(object1);
    scene.Update(0);
    //scene.RemoveComponent<Position>(object1);
    scene.Update(0);
    scene.Update(0);
    
/*
    Container<Position> positions;
    positions.Create(0);
    bool contains1 = positions.Contains(0);
    
    auto& pos = positions.Get(0);
    pos.x = 12.0f;
    
    positions.Reference(1, 0);
    
    auto& pos2 = positions.Get(1);
    
    positions.Destroy(0);
    positions.Destroy(1);
    
    //positions.Destroy(0);
    //bool contains2 = positions.Contains(0);
    */

    std::cout << " ecs system "<< std::endl;
}
