#shader vertex
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
out vec2 vUV;
void main() {
    // vUV = aUV*8; // hah cool stuff when multiplying what, makes sense tho
    vUV = aUV;
    gl_Position = vec4(aPos, 0.0, 1.0);
}

#shader fragment
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform sampler2D u_Scene;
uniform sampler2D volumetrics_tex;
void main() {
    // FragColor = texture(u_Scene, vUV) + texture(volumetrics_tex, vUV);
    vec4 vol_col = texture(volumetrics_tex, vUV * 2);
    vec4 scene_col = texture(u_Scene, vUV);
    FragColor = vol_col;
}