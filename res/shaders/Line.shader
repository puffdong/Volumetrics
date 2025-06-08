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

void main()
{
    // Linearly interpolate between the start and end points using aExtrusion.
    // If aExtrusion is 0.0, we get aStart.
    // If aExtrusion is 1.0, we get aEnd.
    vec3 position = mix(aStart, aEnd, aExtrusion);

    // Standard transformation to clip space
    gl_Position = projection * view * vec4(position, 1.0);
}

#shader fragment
#version 330 core

// The output color of the fragment
out vec4 FragColor;

void main()
{
    // Set the line color to a solid white
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
