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

// Cubic Polynomial Smooth-minimum
vec2 smin( float a, float b, float k )
{
    k *= 6.0;
    float h = max( k-abs(a-b), 0.0 )/k;
    float m = h*h*h*0.5;
    float s = m*k*(1.0/3.0); 
    return (a<b) ? vec2(a-s,m) : vec2(b-s,1.0-m);
}

float hash1( float n )
{
    return fract(sin(n)*43758.5453123);
}

vec3 forwardSF( float i, float n) 
{
    const float PI  = 3.1415926535897932384626433832795;
    const float PHI = 1.6180339887498948482045868343656;
    float phi = 2.0*PI*fract(i/PHI);
    float zi = 1.0 - (2.0*i+1.0)/n;
    float sinTheta = sqrt( 1.0 - zi*zi);
    return vec3( cos(phi)*sinTheta, sin(phi)*sinTheta, zi);
}

vec2 map( vec3 q )
{
    // plane
    vec2 res = vec2( q.y, 2.0 );

    // sphere
    float d = length(q-vec3(0.0,0.1+0.05*sin(time),0.0))-0.1;
    
    // smooth union    
    return smin(res.x,d,0.05);
}

vec2 intersect( in vec3 ro, in vec3 rd )
{
	const float maxd = 10.0;

    vec2 res = vec2(0.0);
    float t = 0.0;
    for( int i=0; i<512; i++ )
    {
	    vec2 h = map( ro+rd*t );
        if( (h.x<0.0) || (t>maxd) ) break;
        t += h.x;
        res = vec2( t, h.y );
    }

    if( t>maxd ) res=vec2(-1.0);
	return res;
}

// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal( in vec3 pos )
{
    vec2 e = vec2(1.0,-1.0)*0.5773*0.005;
    return normalize( e.xyy*map( pos + e.xyy ).x + 
					  e.yyx*map( pos + e.yyx ).x + 
					  e.yxy*map( pos + e.yxy ).x + 
					  e.xxx*map( pos + e.xxx ).x );
}

// https://iquilezles.org/articles/nvscene2008/rwwtt.pdf
float calcAO( in vec3 pos, in vec3 nor, float ran )
{
	float ao = 0.0;
    const int num = 32;
    for( int i=0; i<num; i++ )
    {
        vec3 kk;
        vec3 ap = forwardSF( float(i)+ran, float(num) );
		ap *= sign( dot(ap,nor) ) * hash1(float(i));
        ao += clamp( map( pos + nor*0.01 + ap*0.2 ).x*20.0, 0.0, 1.0 );
    }
	ao /= float(num);
	
    return clamp( ao, 0.0, 1.0 );
}

vec3 render( in vec2 p, vec4 ran, vec3 ro, vec3 rd)
{
    //-----------------------------------------------------
    // camera
    //-----------------------------------------------------


    //-----------------------------------------------------
	// render
    //-----------------------------------------------------
    
	vec3 col = vec3(1.0);

	// raymarch
    vec3 uvw;
    vec2 res = intersect(ro,rd);
    float t = res.x;
    if( t>0.0 )
    {
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal(pos);
		vec3 ref = reflect( rd, nor );
        float fre = clamp( 1.0 + dot(nor,rd), 0.0, 1.0 );
        float occ = calcAO( pos, nor, ran.y ); occ = occ*occ;

        // blend materials        
        col = mix( vec3(0.0,0.05,1.0),
                   vec3(1.0,0.0,0.0),
                   res.y );
        
        col = col*0.72 + 0.2*fre*vec3(1.0,0.8,0.2);
        vec3 lin  = 4.0*vec3(0.7,0.8,1.0)*(0.5+0.5*nor.y)*occ;
             lin += 0.8*vec3(1.0,1.0,1.0)*fre            *(0.6+0.4*occ);
        col = col * lin;
        col += 2.0*vec3(0.8,0.9,1.00)*smoothstep(0.0,0.4,ref.y)*(0.06+0.94*pow(fre,5.0))*occ;
        col = mix( col, vec3(1.0), 1.0-exp2(-0.04*t*t) );

    }

    // gamma and postpro
    col = pow(col,vec3(0.4545));
    col *= 0.9;
    col = clamp(col,0.0,1.0);
    col = col*col*(3.0-2.0*col);
    
    // dithering
    col += (ran.x-0.5)/255.0;
    
	return col;
}


// float smin( float a, float b, float k )
// {
//     k *= 1.0;
//     float r = exp2(-a/k) + exp2(-b/k);
//     return -k*log2(r);
// };

// // Signed Distance Function for a sphere
// float sdfSphere(vec3 position, vec3 center, float radius) {
//     // center.x = center.x + 5 * sin(time);

//     return length(position - center) - radius;
// };

// float sdfTorus(vec3 p, vec2 t)
// {   
//     return length(vec2(length(p.xz)-t.x, p.y) ) - t.y;
// };

// // Scene function - add more SDFs here to create complex scenes
// float sceneSDF(vec3 position, out vec4 out_color) {
//     float tmp = 1000;
//     vec3 tmp_color = vec3(0.0, 0.0, 0.0)
//     for (int i = 0; i < 5; i++) {
//         if (i < num_spheres) {
//             tmp = smin(sdfSphere(position, sphere_positions[i], sphere_radiuses[i]), tmp, 2.0);

//         }
//     }

//     return tmp;

//     // float hmm = min(sdfSphere(position, vec3(10.0, 10.0, 3.0), 1.0), sdfSphere(position, vec3(-10.0, -10.0, 3.0), 1.0));
//     // return hmm; //sdfSphere(position, vec3(10.0, 10.0, 3.0), 1.0);
// };

// // Raymarching function to calculate the distance to the nearest surface
// float rayMarch(vec3 rayOrigin, vec3 rayDirection) {
//     float distance = 0.0;
//     for (int i = 0; i < MAX_STEPS; i++) {
//         vec3 currentPosition = rayOrigin + rayDirection * distance;
//         float distToScene = sceneSDF(currentPosition);
//         if (distToScene < MIN_DIST) {
//             return distance;
//         }
//         distance += distToScene;
//         if (distance > MAX_DIST) {
//             break;
//         }
//     }
//     return MAX_DIST;
// };

// // Shading function to calculate color based on the distance to the scene
// vec3 getNormal(vec3 position) {
//     vec2 epsilon = vec2(0.001, 0.0);
//     float dx = sceneSDF(position + vec3(epsilon.x, epsilon.y, epsilon.y)) - sceneSDF(position - vec3(epsilon.x, epsilon.y, epsilon.y));
//     float dy = sceneSDF(position + vec3(epsilon.y, epsilon.x, epsilon.y)) - sceneSDF(position - vec3(epsilon.y, epsilon.x, epsilon.y));
//     float dz = sceneSDF(position + vec3(epsilon.y, epsilon.y, epsilon.x)) - sceneSDF(position - vec3(epsilon.y, epsilon.y, epsilon.x));
//     return normalize(vec3(dx, dy, dz));
// };

// vec4 render(vec3 rayOrigin, vec3 rayDirection) {
//     float distance = rayMarch(rayOrigin, rayDirection);
//     if (distance < MAX_DIST) {
//         vec3 hitPoint = rayOrigin + rayDirection * distance;
//         vec3 normal = getNormal(hitPoint);
//         float lightIntensity = dot(normal, normalize(vec3(1.0, 1.0, 1.0))) * 0.5 + 0.5;
//         vec4 result = vec4(1.0 * lightIntensity, 0.5 * lightIntensity, 0.2 * lightIntensity, 1.0);
//         return result; // Orange-ish color with diffuse lighting
//     }
//     return vec4(0.0); // Background color (black)
// };



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

    // vec4 result = render(rayOrigin, rayDirection);

    // color = result;

    // vec2 px = (2.0*fragCoord-iResolution.xy)/iResolution.y;
    // vec4 ran = texelFetch( iChannel0, ivec2(fragCoord)&1023,0);
    vec4 ran = vec4(1.0 * sin(time));
    vec3 col = render( uv, ran, rayOrigin, rayDirection);   
    
    color = vec4( col, 1.0 );   

    
};

// // The MIT License
// // Copyright © 2024 Inigo Quilez
// // Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions: The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

// // More info: https://iquilezles.org/articles/smin/


// // Cubic Polynomial Smooth-minimum
// vec2 smin( float a, float b, float k )
// {
//     k *= 6.0;
//     float h = max( k-abs(a-b), 0.0 )/k;
//     float m = h*h*h*0.5;
//     float s = m*k*(1.0/3.0); 
//     return (a<b) ? vec2(a-s,m) : vec2(b-s,1.0-m);
// }

// float hash1( float n )
// {
//     return fract(sin(n)*43758.5453123);
// }

// vec3 forwardSF( float i, float n) 
// {
//     const float PI  = 3.1415926535897932384626433832795;
//     const float PHI = 1.6180339887498948482045868343656;
//     float phi = 2.0*PI*fract(i/PHI);
//     float zi = 1.0 - (2.0*i+1.0)/n;
//     float sinTheta = sqrt( 1.0 - zi*zi);
//     return vec3( cos(phi)*sinTheta, sin(phi)*sinTheta, zi);
// }

// vec2 map( vec3 q )
// {
//     // plane
//     vec2 res = vec2( q.y, 2.0 );

//     // sphere
//     float d = length(q-vec3(0.0,0.1+0.05*sin(iTime),0.0))-0.1;
    
//     // smooth union    
//     return smin(res.x,d,0.05);
// }

// vec2 intersect( in vec3 ro, in vec3 rd )
// {
// 	const float maxd = 10.0;

//     vec2 res = vec2(0.0);
//     float t = 0.0;
//     for( int i=0; i<512; i++ )
//     {
// 	    vec2 h = map( ro+rd*t );
//         if( (h.x<0.0) || (t>maxd) ) break;
//         t += h.x;
//         res = vec2( t, h.y );
//     }

//     if( t>maxd ) res=vec2(-1.0);
// 	return res;
// }

// // https://iquilezles.org/articles/normalsSDF
// vec3 calcNormal( in vec3 pos )
// {
//     vec2 e = vec2(1.0,-1.0)*0.5773*0.005;
//     return normalize( e.xyy*map( pos + e.xyy ).x + 
// 					  e.yyx*map( pos + e.yyx ).x + 
// 					  e.yxy*map( pos + e.yxy ).x + 
// 					  e.xxx*map( pos + e.xxx ).x );
// }

// // https://iquilezles.org/articles/nvscene2008/rwwtt.pdf
// float calcAO( in vec3 pos, in vec3 nor, float ran )
// {
// 	float ao = 0.0;
//     const int num = 32;
//     for( int i=0; i<num; i++ )
//     {
//         vec3 kk;
//         vec3 ap = forwardSF( float(i)+ran, float(num) );
// 		ap *= sign( dot(ap,nor) ) * hash1(float(i));
//         ao += clamp( map( pos + nor*0.01 + ap*0.2 ).x*20.0, 0.0, 1.0 );
//     }
// 	ao /= float(num);
	
//     return clamp( ao, 0.0, 1.0 );
// }

// vec3 render( in vec2 p, vec4 ran )
// {
//     //-----------------------------------------------------
//     // camera
//     //-----------------------------------------------------
// 	float an = 0.1*iTime;
// 	vec3 ro = vec3(0.4*sin(an),0.15,0.4*cos(an));
//     vec3 ta = vec3(0.0,0.05,0.0);
//     // camera matrix
//     vec3 ww = normalize( ta - ro );
//     vec3 uu = normalize( cross(ww,vec3(0.0,1.0,0.0) ) );
//     vec3 vv = normalize( cross(uu,ww));
// 	// create view ray
// 	vec3 rd = normalize( p.x*uu + p.y*vv + 1.7*ww );

//     //-----------------------------------------------------
// 	// render
//     //-----------------------------------------------------
    
// 	vec3 col = vec3(1.0);

// 	// raymarch
//     vec3 uvw;
//     vec2 res = intersect(ro,rd);
//     float t = res.x;
//     if( t>0.0 )
//     {
//         vec3 pos = ro + t*rd;
//         vec3 nor = calcNormal(pos);
// 		vec3 ref = reflect( rd, nor );
//         float fre = clamp( 1.0 + dot(nor,rd), 0.0, 1.0 );
//         float occ = calcAO( pos, nor, ran.y ); occ = occ*occ;

//         // blend materials        
//         col = mix( vec3(0.0,0.05,1.0),
//                    vec3(1.0,0.0,0.0),
//                    res.y );
        
//         col = col*0.72 + 0.2*fre*vec3(1.0,0.8,0.2);
//         vec3 lin  = 4.0*vec3(0.7,0.8,1.0)*(0.5+0.5*nor.y)*occ;
//              lin += 0.8*vec3(1.0,1.0,1.0)*fre            *(0.6+0.4*occ);
//         col = col * lin;
//         col += 2.0*vec3(0.8,0.9,1.00)*smoothstep(0.0,0.4,ref.y)*(0.06+0.94*pow(fre,5.0))*occ;
//         col = mix( col, vec3(1.0), 1.0-exp2(-0.04*t*t) );

//     }

//     // gamma and postpro
//     col = pow(col,vec3(0.4545));
//     col *= 0.9;
//     col = clamp(col,0.0,1.0);
//     col = col*col*(3.0-2.0*col);
    
//     // dithering
//     col += (ran.x-0.5)/255.0;
    
// 	return col;
// }




// #define AA 0

// void mainImage( out vec4 fragColor, in vec2 fragCoord )
// {

// #if AA>1
//     vec3 col = vec3(0.0);
//     for( int m=0; m<AA; m++ )
//     for( int n=0; n<AA; n++ )
//     {
//         vec2 px = fragCoord + vec2(float(m),float(n))/float(AA);
//         vec4 ran = texelFetch( iChannel0, ivec2(px*float(AA))&1023,0);

//         vec2 p = (2.0*px-iResolution.xy)/iResolution.y;
//     	col += render( p, ran );    
//     }
//     col /= float(AA*AA);
    
// #else
//     vec2 px = (2.0*fragCoord-iResolution.xy)/iResolution.y;
//     vec4 ran = texelFetch( iChannel0, ivec2(fragCoord)&1023,0);
//     vec3 col = render( px, ran );
// #endif    
    
//     fragColor = vec4( col, 1.0 );
// }