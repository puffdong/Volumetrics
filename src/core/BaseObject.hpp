#pragma once
#include "glm/glm.hpp"
#include "core/utils/ButtonMap.hpp"
#include "core/rendering/Renderer.hpp"
#include "core/resources/ResourceManager.hpp"

class BaseObject {
protected:
    glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale    = glm::vec3(1.0f);

public: 
    BaseObject(glm::vec3 pos = glm::vec3(0.f),
               glm::vec3 rot = glm::vec3(0.f),
               glm::vec3 scale = glm::vec3(1.f));

    virtual ~BaseObject();

    virtual void init(ResourceManager& resources) = 0;
    virtual void tick(float delta, ButtonMap bm);
    virtual void enqueue(Renderer& renderer, ResourceManager& resources) = 0;

    // getters n' setters
    virtual glm::mat4 get_model_matrix() const; // the default behaviour is defined but should be overriden if needed
    glm::vec3 get_position() const { return position; };
    glm::vec3 get_rotation() const { return rotation; };
    glm::vec3 get_scale()    const { return scale; };
    void set_position(const glm::vec3& p) { position = p; };
    void set_rotation(const glm::vec3& r) { rotation = r; };
    void set_scale(const glm::vec3& s) { scale = s; };
};