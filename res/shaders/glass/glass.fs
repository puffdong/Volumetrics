#version 330 core
layout(location = 0) out vec4 o_color;

in vec2 v_uv;

uniform sampler2D u_src_color; // framebuffer sampling :)
uniform sampler2D u_depth_texture;
uniform vec2 u_glass_pane_position;
uniform float u_radius;

uniform float u_far;
uniform float u_near;

uniform vec2 u_resolution;
uniform vec3 u_mouse_pos;

float linearize_depth(float d,float z_near,float z_far)
{
    return z_near * z_far / (z_far + d * (z_near - z_far));
}

vec2 norm_pos_to_screen_pos(vec2 norm_pos) {
    return vec2(norm_pos);
}

vec3 sdgCircle( in vec2 p, in float r ) 
{
    float d = length(p);
    return vec3( d-r, p/d );
}

vec3 sdg_circle( in vec2 p, in vec2 pos, in float r ) 
{
    float d = length(p-pos);
    return vec3( d-r, (p-pos)/d );
}

// an IQ function
float smin( float a, float b, float k )
{
    k *= 1.0/(1.0-sqrt(0.5));
    float h = max( k-abs(a-b), 0.0 )/k;
    return min(a,b) - k*0.5*(1.0+h-sqrt(1.0-h*(h-2.0)));
}

vec2 uv_to_texture(vec2 uv) { return (uv * vec2(1.0 / (u_resolution.x / u_resolution.y), 1.0) / 2) + 0.5; }

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy-u_resolution.xy) / u_resolution.y;

    const float radius = 0.5;
    vec3 dg = sdgCircle(uv, radius);


    const float radi = 0.01;
    vec2 left = vec2(0.15, 0.0);
    vec2 right = vec2(-0.15, 0.0);

    if (u_mouse_pos.z > 0.01) {
        vec2 mouse = -(1.0 * u_mouse_pos.xy-u_resolution.xy) / u_resolution.y;
        left = u_mouse_pos.xy;
    }

    vec3 circle_left = sdg_circle(uv, left, radi);
    vec3 circle_right = sdg_circle(uv, right, radi);

    float k = 0.2;
    float d = smin(circle_left.x, circle_right.x, k);
    vec2 g = vec2(smin(circle_left.y, circle_right.y, k), smin(circle_left.z, circle_right.z, k));

    // central differenes based gradient, for comparison
    // g = vec2(dFdx(d),dFdy(d))/(2.0/u_resolution.y);

    // -- coloring -- //
    // vec3 col = (d>0.0) ? vec3(0.9,0.6,0.3) : vec3(0.4,0.7,0.85);
    // col *= 1.0 + vec3(0.5*g,0.0);
    vec3 col = vec3(0.5 + 0.5 * g, 1.0);

    float clamped_d = clamp(d, -1.0, 1.0);
    // vec3 col = vec3(abs(clamped_d), abs(clamped_d), abs(clamped_d));
    col *= 1.0 - 0.5 * exp(-16.0 * abs(d));

	// col *= 0.9 + 0.1 * cos(150.0 * d);
	col *= 0.9 + 0.1 * cos(150.0 / (10 * (0.1 + abs(clamped_d))));

    // col = mix(col, vec3(1.0), 1.0 - smoothstep(0.0, 0.01, abs(d)));
    col = mix(col, vec3(1.0), d);

    vec4 circle_sdf_color = vec4(col, 1.0);

    if (d > 0.0) { // we are outside the thing
        circle_sdf_color = mix(texture(u_src_color, v_uv), vec4(col, 1.0), 0.0); // v_uv <=> uv_to_texture(uv) 
    } else {
        vec3 I = normalize(vec3(0.0, 0.0, -1.0)); // coming straight toward the screeeeen
        vec3 N = normalize(vec3(g, 0.0)); // surface normal
        float eta = 1.0 / 1.5;              // air → glass (n1=1.0, n2=1.5)

        vec3 refracted = refract(I, N, eta);

        vec2 sloppy_refract = vec2(d, -d);
        // circle_sdf_color = mix(texture(u_src_color, v_uv + refracted.xy * 0.1), vec4(col, 1.0), 0.2);
        // circle_sdf_color = texture(u_src_color, v_uv + (refracted.xy * 0.05));
        // circle_sdf_color = mix(vec4(refracted, 1.0), circle_sdf_color, 0.1);
        circle_sdf_color = vec4(g.x, g.y, 0.0, 1.0);
        circle_sdf_color = mix(texture(u_src_color, v_uv + sloppy_refract * 0.1), vec4(col, 1.0), 0.2);
        // circle_sdf_color = vec4(col, 1.0);
    }

    o_color = circle_sdf_color;

}
    // float depth_sample = texture(u_depth_texture, v_uv).r;
    // vec4 depth_whatever_color = vec4(gl_FragCoord.x, gl_FragCoord.y, depth_sample, 1.0);
    // o_color = texture(u_src_color, v_uv);
    // vec4 src_color = texture(u_src_color, gl_FragCoord.xy / u_resolution);
    // vec4 src_color = texture(u_src_color, v_uv);
    // o_color = mix(src_color, circle_sdf_color, 1.0);
    // o_color = vec4(uv.x, uv.y, depth_sample, 1.0);
// }