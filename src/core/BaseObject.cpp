#include "BaseObject.hpp"

BaseObject::BaseObject(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
: position(pos), rotation(rot), scale(scale) {}

void BaseObject::tick(float delta, ButtonMap bm) {}
BaseObject::~BaseObject() {}

glm::mat4 BaseObject::get_model_matrix() const {
    glm::mat4 m(1.0f);
    m = glm::translate(m, position);
    m = glm::rotate(m, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f)); // roll
    m = glm::rotate(m, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f)); // yaw
    m = glm::rotate(m, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f)); // pitch
    m = glm::scale(m, scale);
    return m;
}
