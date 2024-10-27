#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;      // Vertex position
layout(location = 1) in vec3 aNormal;   // Vertex normal
layout(location = 2) in vec2 aTexCoord; // Vertex texture coordinates

// Outputs to the fragment shader
out vec3 Normal; // The normal of the vertex
out vec2 v_texCoord;

//lab copying stuff
out vec3 exNormal;
out vec3 exSurfacePos;

// Uniforms
uniform mat4 model; // Model matrix
uniform mat4 view; // View matrix
uniform mat4 proj; // Projection matrix



void main()
{
    vec4 vertPos = view * model * vec4(aPos, 1.0);
    exSurfacePos = vec3(vertPos);
	
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    exNormal = mat3(view) * normalMatrix * aNormal;
	
	// Transform the normal vector and pass it to the fragment shader
	Normal = mat3(transpose(inverse(model))) * aNormal;

    v_texCoord = aTexCoord;
    // Transform the vertex position into clip space
    gl_Position = proj * view * model * vec4(aPos, 1.0);
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec3 Normal;   // Normal of the fragment
in vec2 v_texCoord;

//lab stuff copying
in vec3 exNormal;
in vec3 exSurfacePos;

uniform vec4 u_Color;
uniform sampler2D u_Texture;

uniform mat4 view;

uniform vec3 sunDir;
uniform vec3 sunColor;

void main()
{
    // Normalize inputs
    vec3 N = normalize(exNormal);
    vec3 sunDirection = mat3(view) * sunDir;

    vec3 L = normalize(sunDirection);

    // Compute the cosine of the angle between the normal and sun direction
    float cosTheta = dot(N, L);

    // Apply step function to get sharp shading boundaries
    float toonShading = step(0.5, cosTheta);

    // Calculate light attenuation based on distance
    float distance = length(exSurfacePos);
    float attenuation = 1.0 / (distance * distance);

    // Assign color based on toon shading value
    
    float shade = max(dot(N, L), 0.0);
    vec3 icolor;
    if (shade > 0.5)
        icolor = sunColor; // Lit by the sun
    else
        icolor = vec3(0.2, 0.2, 0.2); // In shadow

    color = vec4(icolor, 1.0) * texture(u_Texture, v_texCoord);
};