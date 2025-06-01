#pragma once
#include <iostream>  // std::cout, std::cerr, std::endl
#include <fstream>   // std::ofstream
// …your existing includes…
#include <cmath>
#include <vector>
#include <algorithm>
#include <random>
#include <GL/glew.h>
#include "../rendering/Renderer.h"    

class PerlinNoiseTexture {
public:
    PerlinNoiseTexture(int width, int height) : width(width), height(height) {
        generate2DNoise();
        generate2DTexture();
    }

    PerlinNoiseTexture(int width, int height, const std::string& filename) : width(width), height(height) {
        generate2DNoise();
        write2DNoiseToPPM(filename);
    }

    PerlinNoiseTexture(int width, int height, int depth) : width(width), height(height), depth(depth) {
        generate3DNoise();
        generate3DTexture();
    }

    GLuint getTextureID() const { return textureID; }

private:
    int width;
    int height;
    int depth;
    std::vector<float> noiseData;
    GLuint textureID;

    float fade(float t) {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    float grad(int hash, float x, float y, float z = 0.0f) {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
    }

    void generate2DNoise() {
        std::vector<int> p(512);
        for (int i = 0; i < 256; ++i) p[i] = i;
        std::default_random_engine engine(std::random_device{}());
        std::shuffle(p.begin(), p.begin() + 256, engine);
        for (int i = 0; i < 256; ++i) p[256 + i] = p[i];

        noiseData.resize(width * height);
        float frequency = 0.05f;
        float amplitude = 1.0f;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float xf = x * frequency;
                float yf = y * frequency;

                int X = static_cast<int>(std::floor(xf)) & 255;
                int Y = static_cast<int>(std::floor(yf)) & 255;

                xf -= std::floor(xf);
                yf -= std::floor(yf);

                float u = fade(xf);
                float v = fade(yf);

                int aa = p[p[X] + Y];
                int ab = p[p[X] + Y + 1];
                int ba = p[p[X + 1] + Y];
                int bb = p[p[X + 1] + Y + 1];

                float gradAA = grad(aa, xf, yf);
                float gradBA = grad(ba, xf - 1, yf);
                float gradAB = grad(ab, xf, yf - 1);
                float gradBB = grad(bb, xf - 1, yf - 1);

                float lerpX1 = lerp(gradAA, gradBA, u);
                float lerpX2 = lerp(gradAB, gradBB, u);
                float result = lerp(lerpX1, lerpX2, v);

                noiseData[y * width + x] = (result + 1.0f) * 0.5f; // normalize to [0, 1]
            }
        }
    }

    void generate3DNoise() {
        std::vector<int> p(512);
        for (int i = 0; i < 256; ++i) p[i] = i;
        std::default_random_engine engine(std::random_device{}());
        std::shuffle(p.begin(), p.begin() + 256, engine);
        for (int i = 0; i < 256; ++i) p[256 + i] = p[i];

        noiseData.resize(width * height * depth);
        float frequency = 0.05f;
        for (int z = 0; z < depth; ++z) {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    float xf = x * frequency;
                    float yf = y * frequency;
                    float zf = z * frequency;

                    int X = static_cast<int>(std::floor(xf)) & 255;
                    int Y = static_cast<int>(std::floor(yf)) & 255;
                    int Z = static_cast<int>(std::floor(zf)) & 255;

                    xf -= std::floor(xf);
                    yf -= std::floor(yf);
                    zf -= std::floor(zf);

                    float u = fade(xf);
                    float v = fade(yf);
                    float w = fade(zf);

                    int aaa = p[p[p[X] + Y] + Z];
                    int aba = p[p[p[X] + Y + 1] + Z];
                    int aab = p[p[p[X] + Y] + Z + 1];
                    int abb = p[p[p[X] + Y + 1] + Z + 1];
                    int baa = p[p[p[X + 1] + Y] + Z];
                    int bba = p[p[p[X + 1] + Y + 1] + Z];
                    int bab = p[p[p[X + 1] + Y] + Z + 1];
                    int bbb = p[p[p[X + 1] + Y + 1] + Z + 1];

                    float gradAAA = grad(aaa, xf, yf, zf);
                    float gradBAA = grad(baa, xf - 1, yf, zf);
                    float gradABA = grad(aba, xf, yf - 1, zf);
                    float gradBBA = grad(bba, xf - 1, yf - 1, zf);
                    float gradAAB = grad(aab, xf, yf, zf - 1);
                    float gradBAB = grad(bab, xf - 1, yf, zf - 1);
                    float gradABB = grad(abb, xf, yf - 1, zf - 1);
                    float gradBBB = grad(bbb, xf - 1, yf - 1, zf - 1);

                    float lerpX1 = lerp(gradAAA, gradBAA, u);
                    float lerpX2 = lerp(gradABA, gradBBA, u);
                    float lerpX3 = lerp(gradAAB, gradBAB, u);
                    float lerpX4 = lerp(gradABB, gradBBB, u);

                    float lerpY1 = lerp(lerpX1, lerpX2, v);
                    float lerpY2 = lerp(lerpX3, lerpX4, v);

                    float result = lerp(lerpY1, lerpY2, w);

                    noiseData[z * width * height + y * width + x] = (result + 1.0f) * 0.5f; // normalize to [0, 1]
                }
            }
        }
    }

    void generate2DTexture() {
        GLCall(glGenTextures(1, &textureID))
            GLCall(glBindTexture(GL_TEXTURE_2D, textureID))
            GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_FLOAT, noiseData.data()))
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT))
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT))
    }

    void generate3DTexture() {
        GLCall(glGenTextures(1, &textureID))
            GLCall(glBindTexture(GL_TEXTURE_3D, textureID))
            GLCall(glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, width, height, depth, 0, GL_RED, GL_FLOAT, noiseData.data()))
            GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
            GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
            GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT))
            GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT))
            GLCall(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_MIRRORED_REPEAT))
    }

    void write2DNoiseToPPM(const std::string& filename) const {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            return;
        }

        outFile << "P3\n";
        outFile << width << " " << height << "\n";
        outFile << "255\n";

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int value = static_cast<int>(noiseData[y * width + x] * 255);
                outFile << value << " " << value << " " << value << "  ";
            }
            outFile << "\n";
        }

        outFile.close();
    }

};

// Usage Example
// PerlinNoiseTexture perlinTexture2D(512, 512);
// GLuint textureID2D = perlinTexture2D.getTextureID();

// PerlinNoiseTexture perlinTexture3D(128, 128, 128);
// GLuint textureID3D = perlinTexture3D.getTextureID();
