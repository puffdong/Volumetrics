#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform mat4 view_matrix;
uniform vec2 iResolution;
uniform vec3 camera_pos;
// uniform vec3 camera_dir;
uniform float time;

const float MAX_DIST = 100.0;
const float MIN_DIST = 0.001;
const int MAX_STEPS = 128;

uniform vec3 sphere_positions[5];
uniform vec3 sphere_colors[5];
uniform float sphere_radiuses[5];
uniform int num_spheres;

float smin( float a, float b, float k )
{
    k *= 1.0;
    float r = exp2(-a/k) + exp2(-b/k);
    return -k*log2(r);
};

// Signed Distance Function for a sphere
float sdfSphere(vec3 position, vec3 center, float radius) {
    // center.x = center.x + 5 * sin(time);

    return length(position - center) - radius;
};

float sdfTorus(vec3 p, vec2 t)
{   
    return length(vec2(length(p.xz)-t.x, p.y) ) - t.y;
};

// Scene function - add more SDFs here to create complex scenes
float sceneSDF(vec3 position) {
    float tmp = 1000;
    for (int i = 0; i < 5; i++) {
        if (i < num_spheres) {
            tmp = smin(sdfSphere(position, sphere_positions[i], sphere_radiuses[i]), tmp, 2.0);
        }
    }

    return tmp;

    // float hmm = min(sdfSphere(position, vec3(10.0, 10.0, 3.0), 1.0), sdfSphere(position, vec3(-10.0, -10.0, 3.0), 1.0));
    // return hmm; //sdfSphere(position, vec3(10.0, 10.0, 3.0), 1.0);
};

// Raymarching function to calculate the distance to the nearest surface
float rayMarch(vec3 rayOrigin, vec3 rayDirection) {
    float distance = 0.0;
    for (int i = 0; i < MAX_STEPS; i++) {
        vec3 currentPosition = rayOrigin + rayDirection * distance;
        float distToScene = sceneSDF(currentPosition);
        if (distToScene < MIN_DIST) {
            return distance;
        }
        distance += distToScene;
        if (distance > MAX_DIST) {
            break;
        }
    }
    return MAX_DIST;
};

// Shading function to calculate color based on the distance to the scene
vec3 getNormal(vec3 position) {
    vec2 epsilon = vec2(0.001, 0.0);
    float dx = sceneSDF(position + vec3(epsilon.x, epsilon.y, epsilon.y)) - sceneSDF(position - vec3(epsilon.x, epsilon.y, epsilon.y));
    float dy = sceneSDF(position + vec3(epsilon.y, epsilon.x, epsilon.y)) - sceneSDF(position - vec3(epsilon.y, epsilon.x, epsilon.y));
    float dz = sceneSDF(position + vec3(epsilon.y, epsilon.y, epsilon.x)) - sceneSDF(position - vec3(epsilon.y, epsilon.y, epsilon.x));
    return normalize(vec3(dx, dy, dz));
};

vec4 render(vec3 rayOrigin, vec3 rayDirection) {
    float distance = rayMarch(rayOrigin, rayDirection);
    if (distance < MAX_DIST) {
        vec3 hitPoint = rayOrigin + rayDirection * distance;
        vec3 normal = getNormal(hitPoint);
        float lightIntensity = dot(normal, normalize(vec3(1.0, 1.0, 1.0))) * 0.5 + 0.5;
        vec4 result = vec4(1.0 * lightIntensity, 0.5 * lightIntensity, 0.2 * lightIntensity, 1.0);
        return result; // Orange-ish color with diffuse lighting
    }
    return vec4(0.0); // Background color (black)
};



void main() {
    // Normalized pixel coordinates (from -1 to 1)
    vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0; // - 1.0;
    uv.x *= iResolution.x / iResolution.y;

    // Extract rotation matrix from view matrix and invert it
    mat3 viewRotation = mat3(view_matrix);
    mat3 inverseRotation = transpose(viewRotation);

    // Define the ray direction in camera space (looking down negative Z-axis)
    vec3 rayDirectionCameraSpace = normalize(vec3(uv.x, uv.y, -1.0));

    // Transform the ray direction to world space
    vec3 rayDirection = inverseRotation * rayDirectionCameraSpace;

    // Normalize the ray direction after transformation
    rayDirection = normalize(rayDirection);

    vec3 rayOrigin = camera_pos;

    vec4 result = render(rayOrigin, rayDirection);
    color = result;
};