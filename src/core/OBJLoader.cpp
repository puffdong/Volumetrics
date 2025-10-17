#define TINYOBJLOADER_IMPLEMENTATION
#include "OBJLoader.hpp"
#include "./core/rendering/Renderer.hpp"
#include "tiny_obj_loader.h"
#include <iostream>
#include <unordered_map>
#include <tuple>
#include <glm/gtx/normal.hpp>

ModelObject::ModelObject(const std::string& filepath) {
    loadThroughTiny(filepath);
}

ModelObject::ModelObject(float width, float depth) {
    createFlatGround(width, depth);
}

ModelObject::ModelObject(float width, float depth, int numRows, int numCols) {
    createFlatGround(width, depth, numRows, numCols);
}

ModelObject::ModelObject(float width, float depth, float height, TextureData* mapTexture)
{
    createHeightmap(width, depth, height, mapTexture);
}

ModelObject::~ModelObject() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void ModelObject::loadThroughTiny(const std::string& filepath) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "./"; // Path to material files

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cout << "TinyObjReader Error: " << reader.Error();
        }
        exit(1);
    }

    // if (!reader.Warning().empty()) {
    //     std::cout << "TinyObjReader Warning: " << reader.Warning();
    // }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    // auto& materials = reader.GetMaterials();

    struct TupleHasher {
        std::size_t operator()(const std::tuple<int, int, int>& tuple) const {
            std::size_t h1 = std::hash<int>{}(std::get<0>(tuple));
            std::size_t h2 = std::hash<int>{}(std::get<1>(tuple));
            std::size_t h3 = std::hash<int>{}(std::get<2>(tuple));

            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };

    // Create data arrays
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    std::unordered_map<std::tuple<int, int, int>, unsigned int, TupleHasher> vertex_map;


    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                auto key = std::make_tuple(idx.vertex_index, idx.normal_index, idx.texcoord_index);

                // Check if the vertex has already been added to the vertex_map
                if (vertex_map.find(key) == vertex_map.end()) {
                    // Add the vertex to the vertex_map and vertices vector
                    unsigned int newIndex = static_cast<unsigned int>(vertices.size() / 8);
                    vertex_map[key] = newIndex;

                    // Vertex position
                    vertices.push_back(attrib.vertices[3 * size_t(idx.vertex_index) + 0]);
                    vertices.push_back(attrib.vertices[3 * size_t(idx.vertex_index) + 1]);
                    vertices.push_back(attrib.vertices[3 * size_t(idx.vertex_index) + 2]);

                    // Vertex normal
                    if (idx.normal_index >= 0) {
                        vertices.push_back(attrib.normals[3 * size_t(idx.normal_index) + 0]);
                        vertices.push_back(attrib.normals[3 * size_t(idx.normal_index) + 1]);
                        vertices.push_back(attrib.normals[3 * size_t(idx.normal_index) + 2]);
                    }
                    else {
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
                    }

                    // Vertex texture coordinates
                    if (idx.texcoord_index >= 0) {
                        vertices.push_back(attrib.texcoords[2 * size_t(idx.texcoord_index) + 0]);
                        vertices.push_back(attrib.texcoords[2 * size_t(idx.texcoord_index) + 1]);
                    }
                    else {
                        vertices.push_back(0.0f);
                        vertices.push_back(0.0f);
                    }
                }

                // Add the index to the indices vector
                indices.push_back(vertex_map[key]);
            }

            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
    }

    GLuint VAO, VBO, EBO;
    GLCall(glGenVertexArrays(1, &VAO));
    GLCall(glGenBuffers(1, &VBO));
    GLCall(glGenBuffers(1, &EBO));

    // Bind and populate VAO
    GLCall(glBindVertexArray(VAO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW));

    // Set vertex attribute pointers
    GLsizei stride = 8 * sizeof(float); // 3 for position, 3 for normals, and 2 for texture coordinates
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0)));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float))));
    GLCall(glEnableVertexAttribArray(2));
    GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float))));

    // Unbind the VAO
    GLCall(glBindVertexArray(0));

    m_VAO = VAO;
    m_VBO = VBO;
    m_EBO = EBO;
    m_indexCount = static_cast<GLsizei>(indices.size());
}

void ModelObject::render() {
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

GLuint ModelObject::getVAO() {
    return m_VAO;
}

GLsizei ModelObject::getIndexCount() const { // Added getter
    return m_indexCount;
}

void ModelObject::Bind() const {
    glBindVertexArray(m_VAO);
}

void ModelObject::Unbind() const {
    glBindVertexArray(0);
}

void ModelObject::createFlatGround(float width, float depth) {
    // Create vertex data
    std::vector<float> vertices = {
        // Positions                            // Normals          // Texture Coords
        -width / 2.0f, 0.0f, -depth / 2.0f,     0.0f, 1.0f, 0.0f,     0.0f, 0.0f,
         width / 2.0f, 0.0f, -depth / 2.0f,     0.0f, 1.0f, 0.0f,     1.0f, 0.0f,
         width / 2.0f, 0.0f,  depth / 2.0f,     0.0f, 1.0f, 0.0f,     1.0f, 1.0f,
        -width / 2.0f, 0.0f,  depth / 2.0f,     0.0f, 1.0f, 0.0f,     0.0f, 1.0f
    };

    std::vector<unsigned int> indices = { // vector should be up eh????
        2, 1, 0,
        0, 3, 2
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // Bind and populate VAO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    GLsizei stride = 8 * sizeof(float); // 3 for position, 3 for normals, and 2 for texture coordinates
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));

    // Unbind the VAO
    glBindVertexArray(0);

    m_VAO = VAO;
    m_VBO = VBO;
    m_EBO = EBO;
    m_indexCount = static_cast<GLsizei>(indices.size());
}

void ModelObject::createHeightmap(float width, float depth, float height, TextureData* mapTexture)
{
    int textureWidth = mapTexture->height;
    int textureDepth = mapTexture->width;

    int vertexCount = textureWidth * textureDepth;
    unsigned int x, z;

    std::vector<glm::vec3> vertexArray(vertexCount);
    std::vector<glm::vec3> normalArray(vertexCount);
    std::vector<glm::vec2> texCoordArray(vertexCount);

    int highestValue = mapTexture->imageData[0];
    for (x = 0; x < textureWidth; x++)
        for (z = 0; z < textureDepth; z++) {
            int nextValue = mapTexture->imageData[(x + z * textureWidth) * (mapTexture->bpp / 8)];
            if (nextValue > highestValue) {
                highestValue = nextValue;
            }
        }

    float yScale = height / (float)highestValue;
    float xScale = width / (float)textureWidth;
    float zScale = depth / (float)textureDepth;

    int index;
    for (x = 0; x < textureWidth; x++)
        for (z = 0; z < textureDepth; z++)
        {
            index = (x + z * textureWidth);
            // Vertex array
            vertexArray[index].x = (float)x * xScale;
            vertexArray[index].y = mapTexture->imageData[(x + z * textureWidth) * (mapTexture->bpp / 8)] * yScale;
            vertexArray[index].z = (float)z * zScale;
            // Texture coordinates
            texCoordArray[index].x = (float)x / textureWidth;
            texCoordArray[index].y = (float)z / textureDepth;
        }

    glm::vec3 center, up, down, right, left, upLeft, upRight, downRight, downLeft, sum;
    // Is the position next to an edge?
    bool upEdge, downEdge, rightEdge, leftEdge;
    // Normal vectors
    for (x = 0; x < textureWidth; x++)
        for (z = 0; z < textureDepth; z++)
        {
            index = x + z * textureWidth;
            upEdge = (z == 0);
            downEdge = (z == textureDepth - 1);
            rightEdge = (x == textureWidth - 1);
            leftEdge = (x == 0);

            center = vertexArray[index];
            up = upEdge ? glm::vec3(0.f) : vertexArray[index - textureWidth];
            down = downEdge ? glm::vec3(0.f) : vertexArray[index + textureWidth];
            right = rightEdge ? glm::vec3(0.f) : vertexArray[index + 1];
            left = leftEdge ? glm::vec3(0.f) : vertexArray[index - 1];

            if (upEdge)
            {
                upLeft = leftEdge ? glm::triangleNormal(center, down, right) : glm::triangleNormal(center, left, down);
                upRight = rightEdge ? glm::triangleNormal(center, left, down) : glm::triangleNormal(center, down, right);
            }
            else
            {
                upLeft = leftEdge ? glm::triangleNormal(center, right, up) : glm::triangleNormal(center, up, left);
                upRight = rightEdge ? glm::triangleNormal(center, up, left) : glm::triangleNormal(center, right, up);
            }

            if (downEdge)
            {
                downRight = rightEdge ? glm::triangleNormal(center, up, left) : glm::triangleNormal(center, right, up);
                downLeft = leftEdge ? glm::triangleNormal(center, right, up) : glm::triangleNormal(center, up, left);
            }
            else
            {
                downRight = rightEdge ? glm::triangleNormal(center, left, down) : glm::triangleNormal(center, down, right);
                downLeft = leftEdge ? glm::triangleNormal(center, down, right) : glm::triangleNormal(center, left, down);
            }

            sum = glm::normalize((upLeft + upRight + downRight + downLeft) / 4.f);
            normalArray[index].x = sum.x;
            normalArray[index].y = sum.y;
            normalArray[index].z = sum.z;
        }

    int triangleCount = (textureWidth - 1) * (textureDepth - 1) * 2;
    std::vector<unsigned int> indexArray(triangleCount * 3);

    for (x = 0; x < textureWidth - 1; x++)
        for (z = 0; z < textureDepth - 1; z++)
        {
            index = (x + z * (textureWidth - 1)) * 6;
            // Triangle 1
            indexArray[index + 0] = x + 0 + (z + 0) * textureWidth;
            indexArray[index + 1] = x + 0 + (z + 1) * textureWidth;
            indexArray[index + 2] = x + 1 + (z + 0) * textureWidth;
            // Triangle 2
            indexArray[index + 3] = x + 1 + (z + 0) * textureWidth;
            indexArray[index + 4] = x + 0 + (z + 1) * textureWidth;
            indexArray[index + 5] = x + 1 + (z + 1) * textureWidth;
        }

    // 3 for position, 3 for normals, and 2 for texture coordinates
    int dataCount = vertexArray.size() * 8;
    std::vector<float> dataArray(dataCount);
    //std::vector<float> dataArray;

    for (size_t i = 0; i < vertexArray.size(); i++)
    {
        dataArray[i * 8 + 0] = vertexArray[i].x;
        dataArray[i * 8 + 1] = vertexArray[i].y;
        dataArray[i * 8 + 2] = vertexArray[i].z;

        dataArray[i * 8 + 3] = normalArray[i].x;
        dataArray[i * 8 + 4] = normalArray[i].y;
        dataArray[i * 8 + 5] = normalArray[i].z;

        dataArray[i * 8 + 6] = texCoordArray[i].x;
        dataArray[i * 8 + 7] = texCoordArray[i].y;
    }

    GLuint VAO, VBO, EBO;
    GLCall(glGenVertexArrays(1, &VAO));
    GLCall(glGenBuffers(1, &VBO));
    GLCall(glGenBuffers(1, &EBO));

    // Bind and populate VAO
    GLCall(glBindVertexArray(VAO));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, VBO));
    GLCall(glBufferData(GL_ARRAY_BUFFER, dataArray.size() * sizeof(float), dataArray.data(), GL_STATIC_DRAW));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexArray.size() * sizeof(unsigned int), indexArray.data(), GL_STATIC_DRAW));

    // Set vertex attribute pointers
    GLsizei stride = 8 * sizeof(float); // 3 for position, 3 for normals, and 2 for texture coordinates
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0)));
    GLCall(glEnableVertexAttribArray(1));
    GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float))));
    GLCall(glEnableVertexAttribArray(2));
    GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float))));

    // Unbind the VAO
    GLCall(glBindVertexArray(0));

    // Store the VAO and other related information in your ModelObject class, assuming you have the following member variables:
    // GLuint m_VAO;
    // GLuint m_VBO;
    // GLuint m_EBO;
    // GLsizei m_indexCount;
    m_VAO = VAO;
    m_VBO = VBO;
    m_EBO = EBO;
    m_indexCount = static_cast<GLsizei>(indexArray.size());
    m_vertexArray = vertexArray;
    m_normalArray = normalArray;
    m_texCoordArray = texCoordArray;
}

void ModelObject::createFlatGround(float width, float depth, int numRows, int numCols) {
    // Calculate quad dimensions
    float quadWidth = width / static_cast<float>(numCols);
    float quadDepth = depth / static_cast<float>(numRows);

    // Create vertex data
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    for (int row = 0; row < numRows; ++row) {
        for (int col = 0; col < numCols; ++col) {
            float x = col * quadWidth - width / 2.0f;
            float z = row * quadDepth - depth / 2.0f;

            // Add positions, normals, and texture coordinates for each vertex of the quad
            vertices.insert(vertices.end(), {
                // Vertex 0 (top-left of quad)
                x, 0.0f, z,                             0.0f, 1.0f, 0.0f,      static_cast<float>(col) / numCols, static_cast<float>(row) / numRows,
                // Vertex 1 (top-right of quad)
                x + quadWidth, 0.0f, z,                 0.0f, 1.0f, 0.0f,      static_cast<float>(col + 1) / numCols, static_cast<float>(row) / numRows,
                // Vertex 2 (bottom-right of quad)
                x + quadWidth, 0.0f, z + quadDepth,    0.0f, 1.0f, 0.0f,      static_cast<float>(col + 1) / numCols, static_cast<float>(row + 1) / numRows,
                // Vertex 3 (bottom-left of quad)
                x, 0.0f, z + quadDepth,                 0.0f, 1.0f, 0.0f,      static_cast<float>(col) / numCols, static_cast<float>(row + 1) / numRows
                });

            // Add indices for the quad
            unsigned int baseIndex = (row * numCols + col) * 4;
            indices.insert(indices.end(), {
                baseIndex + 0, baseIndex + 3, baseIndex + 2, // Triangle 1 (v0, v3, v2) - CCW
                baseIndex + 0, baseIndex + 2, baseIndex + 1  // Triangle 2 (v0, v2, v1) - CCW
                });
        }
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    GLsizei stride = 8 * sizeof(float);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(6 * sizeof(float)));

    glBindVertexArray(0);

    m_VAO = VAO;
    m_VBO = VBO;
    m_EBO = EBO;

    m_indexCount = static_cast<GLsizei>(indices.size());
}