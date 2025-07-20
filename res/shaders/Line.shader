#shader vertex
#version 330 core

// Per-vertex data (from the base VBO)
layout (location = 0) in float aExtrusion; // Will be 0.0 for start, 1.0 for end

// Per-instance data (from the instance VBO)
layout (location = 1) in vec3 aStart; // Start position of the line
layout (location = 2) in vec3 aEnd;   // End position of the line

// Uniforms for transforming the vertices
uniform mat4 projection;
uniform mat4 view;

flat out int instanceID;

void main()
{

    vec3 position = mix(aStart, aEnd, aExtrusion);


    gl_Position = projection * view * vec4(position, 1.0);
    instanceID = gl_InstanceID;
}

#shader fragment
#version 330 core

flat in int instanceID;

// The output color of the fragment
out vec4 FragColor;

void main()
{   
    vec4 color = vec4(1.0);
    if (instanceID == 1) {
        color = vec4(1.0, 0.5, 0.0, 1.0);
    }
    // Set the line color to a solid white
    // FragColor = vec4(1.0, 1.0, 1.0, 1.0);
    FragColor = color;
}
