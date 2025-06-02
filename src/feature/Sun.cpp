#include "Sun.hpp"
#include "../rendering/Renderer.h"


Sun::Sun(glm::vec3 direction, glm::vec4 color) 
    : dir(direction), color(color) {
        init_upload_buffer();
    }

void Sun::init_upload_buffer() {

}

void Sun::upload_sun_data() {

}   