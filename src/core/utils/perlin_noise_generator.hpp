#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <thread>
#include <iostream>
#include <GL/glew.h>
#include <chrono>

struct PerlinNoiseTexture {
    int p[512];
    std::vector<float> noise_data;
    unsigned int seed;
    float frequency = 0.05f;

    unsigned int texture_id = 0;
    int width;
    int height;
    int depth;
};

namespace { 
    inline float fade(float t) { return t * t * t * (t * (t * 6 - 15) + 10); }
    inline float lerp(float a, float b, float t) { return a + t * (b - a); }
    
    inline float grad(int hash, float x, float y, float z) {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }
}

inline void init_perlin(PerlinNoiseTexture& tex, int w, int h, int d, float frequency, unsigned int seed) {
    tex.width = w;
    tex.height = h;
    tex.depth = d;
    tex.frequency = frequency;
    tex.texture_id = 0;

    if (seed == 0) {
        std::random_device rd;
        tex.seed = rd();
    } else {
        tex.seed = seed;
    }

    for (int i = 0; i < 256; ++i) tex.p[i] = i;
    std::default_random_engine engine(tex.seed);
    std::shuffle(tex.p, tex.p + 256, engine);
    for (int i = 0; i < 256; ++i) tex.p[256 + i] = tex.p[i]; // avoid overflow
}

inline void re_init_perlin(PerlinNoiseTexture& tex, float frequency, unsigned int seed) {
    init_perlin(tex, tex.width, tex.height, tex.depth, frequency, seed);
}

inline void generate_perlin(PerlinNoiseTexture& tex) {
    tex.noise_data.resize(tex.width * tex.height * tex.depth);

    auto generate_chunk = [&](int z_start, int z_end) {
        const float frequency = tex.frequency;
        int idx = z_start * tex.width * tex.height;
        
        for (int z = z_start; z < z_end; ++z) {
            const float zf_base = z * frequency;
            const int Z = static_cast<int>(std::floor(zf_base)) & 255;
            const float zf_frac = zf_base - std::floor(zf_base);
            const float w = fade(zf_frac);
            
            for (int y = 0; y < tex.height; ++y) {
                const float yf_base = y * frequency;
                const int Y = static_cast<int>(std::floor(yf_base)) & 255;
                const float yf_frac = yf_base - std::floor(yf_base);
                const float v = fade(yf_frac);
                
                for (int x = 0; x < tex.width; ++x, ++idx) {
                    const float xf_base = x * frequency;
                    const int X = static_cast<int>(std::floor(xf_base)) & 255;
                    const float xf_frac = xf_base - std::floor(xf_base);
                    const float u = fade(xf_frac);
                    
                    const int* p = tex.p; 
                    
                    const int a = p[X] + Y;
                    const int b = p[X + 1] + Y;
                    const int aa = p[a] + Z;
                    const int ab = p[a + 1] + Z;
                    const int ba = p[b] + Z;
                    const int bb = p[b + 1] + Z;
                    
                    const float x1 = lerp(grad(p[aa], xf_frac, yf_frac, zf_frac),
                    grad(p[ba], xf_frac - 1, yf_frac, zf_frac), u);
                    const float x2 = lerp(grad(p[ab], xf_frac, yf_frac - 1, zf_frac),
                    grad(p[bb], xf_frac - 1, yf_frac - 1, zf_frac), u);
                    const float x3 = lerp(grad(p[aa + 1], xf_frac, yf_frac, zf_frac - 1),
                    grad(p[ba + 1], xf_frac - 1, yf_frac, zf_frac - 1), u);
                    const float x4 = lerp(grad(p[ab + 1], xf_frac, yf_frac - 1, zf_frac - 1),
                    grad(p[bb + 1], xf_frac - 1, yf_frac - 1, zf_frac - 1), u);
                    
                    const float y1 = lerp(x1, x2, v);
                    const float y2 = lerp(x3, x4, v);
                    
                    tex.noise_data[idx] = (lerp(y1, y2, w) + 1.0f) * 0.5f;
                }
            }
        }
    };

    unsigned int thread_count = std::thread::hardware_concurrency();
    if (thread_count == 0) thread_count = 1;
    
    std::vector<std::thread> threads;
    int slices_per_thread = tex.depth / thread_count;

    std::cout << "Generating Perlin noise (" << thread_count << " threads, " << slices_per_thread << " slices per thread) ... ";
    auto start = std::chrono::high_resolution_clock::now();

    for (unsigned int i = 0; i < thread_count; ++i) {
        int z_start = i * slices_per_thread;
        int z_end = (i == thread_count - 1) ? tex.depth : (i + 1) * slices_per_thread;
        threads.emplace_back(generate_chunk, z_start, z_end);
    }

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Done! (" << duration.count() << " ms)" << std::endl;
}

inline void upload_perlin(PerlinNoiseTexture& tex) {
    if (tex.texture_id != 0) glDeleteTextures(1, &tex.texture_id);

    glGenTextures(1, &tex.texture_id);
    glBindTexture(GL_TEXTURE_3D, tex.texture_id);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, tex.width, tex.height, tex.depth, 0, GL_RED, GL_FLOAT, tex.noise_data.data());
    
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT);
}

inline void destroy_perlin(PerlinNoiseTexture& tex) {
    if (tex.texture_id != 0) {
        glDeleteTextures(1, &tex.texture_id);
        tex.texture_id = 0;
    }
    tex.noise_data.clear();
}