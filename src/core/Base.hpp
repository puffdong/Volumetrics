#pragma once
#include <unordered_map>

#include "glm/glm.hpp"
#include "core/utils/ButtonMap.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/resources/ResourceManager.hpp"

class Base {
protected:
    uint64_t _id = 0; // 0 : uninitialized, any other, its in the scene 
    Base* _parent = nullptr; // if nullptr, it is the de facto base for this lil branch
    std::unordered_map<int, Base*> _children;

    glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale    = glm::vec3(1.0f);

    bool visible = true;

public:
    Base(glm::vec3 pos = glm::vec3(0.f),
         glm::vec3 rot = glm::vec3(0.f),
         glm::vec3 scale = glm::vec3(1.f), 
         Base* parent = nullptr);

    virtual ~Base();

    virtual void init(ResourceManager& resources, uint64_t id) = 0;
    virtual void tick(float delta, ButtonMap bm);
    virtual void enqueue(Renderer& renderer, ResourceManager& resources) = 0;

    // getters n' setters
    virtual glm::mat4 get_model_matrix() const; // the default behaviour is defined but should be overriden if needed
    glm::vec3 get_position() const { return position; };
    glm::vec3 get_rotation() const { return rotation; };
    glm::vec3 get_scale()    const { return scale; };
    bool get_visibility()    const { return visible; };
    void set_position(const glm::vec3& p) { position = p; };
    void set_rotation(const glm::vec3& r) { rotation = r; };
    void set_scale(const glm::vec3& s) { scale = s; };
    void set_visibility(const bool v) { visible = v; };
};