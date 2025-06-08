#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec3 aNormal;   // Vertex normal
layout(location = 2) in vec2 aTexCoord; 
layout(location = 3) in mat4 instanceMatrix; // Instanced model matrix

// Outputs to the fragment shader
// out vec3 Normal; // The normal of the vertex (in world space, or view space depending on calculation)
// out vec2 v_texCoord;

// // Outputs for lighting calculation (typically in view space or world space)
// out vec3 exNormal;     // Normal in view space
// out vec3 exSurfacePos; // Surface position in view space

// Uniforms
// uniform mat4 model; // No longer needed as uniform, using instanceMatrix
uniform mat4 view; // View matrix
uniform mat4 proj; // Projection matrix
flat out vec3 v_color;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    // Transform vertex position to view space using the instance-specific model matrix
    vec4 worldPos = instanceMatrix * vec4(aPos, 1.0);
    vec4 viewPos = view * worldPos;

    float red = rand(vec2(worldPos.x, worldPos.y));
    float green = rand(vec2(worldPos.z, worldPos.w));
    float blue = rand(vec2(worldPos.y, worldPos.x));
    v_color = vec3(red, green, blue);
    // exSurfacePos = vec3(viewPos); // Surface position in view space
    
    // // Calculate normal matrix for transforming normals (from model to world, then to view)
    // // Normal matrix is transpose(inverse(modelViewMatrix)) typically applied to normals
    // // Or, transpose(inverse(modelMatrix)) to get world normals, then transform by view matrix's rotation part.
    // mat3 normalMatrixModel = transpose(inverse(mat3(instanceMatrix)));
    // vec3 worldNormal = normalMatrixModel * aNormal;
    
    // // Transform normal to view space
    // exNormal = mat3(view) * worldNormal; // Normal in view space
    
    // // For original 'Normal' output, if it's intended for something else (e.g. world space normal for reflection)
    // // Normal = worldNormal; // Or keep your original calculation if it had a specific purpose
    // Normal = worldNormal; // For consistency, let's assume 'Normal' is world space normal

    // v_texCoord = aTexCoord;
    
    // Transform the vertex position into clip space



    gl_Position = proj * viewPos;;
}

#shader fragment
// (Fragment shader remains unchanged from your provided code)
#version 330 core

layout(location = 0) out vec4 color;
flat in vec3 v_color;
// in vec3 Normal;   // Normal of the fragment (assuming world space from v_shader)
// in vec2 v_texCoord;

// in vec3 exNormal;     // Normal in view space
// in vec3 exSurfacePos; // Surface position in view space

// uniform vec4 u_Color; // Not used in current fragment shader logic, but kept if planned
// uniform sampler2D u_Texture;

// uniform mat4 view; // Already available

// uniform vec3 sunDir;   // Sun direction in world space
// uniform vec3 sunColor;

void main()
{
    // Normals and positions from vertex shader are already in view space (exNormal, exSurfacePos)
    // vec3 N = normalize(exNormal); // Normal in View Space



    color = vec4(v_color, 1.0);

}