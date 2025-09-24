#include "VoxelStructure.hpp"


VoxelStructure::VoxelStructure(int h, int w, int d, glm::vec3 pos, int initValue, float cellSize)
    : height(h), width(w), depth(d), position(pos), cellSize(cellSize), instanceVBO(0) {
    shader = new Shader("C:/Dev/OpenGL/Volumetrics/res/shaders/VoxelShaders/VoxelDebug.shader");
    cube = new ModelObject("C:/Dev/OpenGL/Volumetrics/res/models/VoxelModels/defaultCube.obj");

    numVoxels = height * width * depth;
    voxels.resize(numVoxels);
    for (int i = 0; i < numVoxels; i++) {
        voxels[i] = initValue;
    }

    init_instance_buffer(height, width, depth);
    
}

void VoxelStructure::init_instance_buffer(int h, int w, int d) {
    std::vector<glm::mat4> instanceModelMatrices;
    instanceModelMatrices.reserve(numVoxels);

    for (int currentH = 0; currentH < height; currentH++) {
        for (int currentD = 0; currentD < depth; currentD++) {
            for (int currentW = 0; currentW < width; currentW++) {
                instanceModelMatrices.push_back(getModelMatrix(currentH, currentD, currentW));
            }
        }
    }

    GLuint vao = cube->getVAO();
    glBindVertexArray(vao);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), instanceModelMatrices.data(), GL_STATIC_DRAW);

    GLsizei vec4Size = sizeof(glm::vec4);
    GLsizei mat4Size = sizeof(glm::mat4);

    glEnableVertexAttribArray(3); // First 
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)0);
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4); // Second
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(1 * vec4Size));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5); // Third
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(2 * vec4Size));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6); // Fourth
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(3 * vec4Size));
    glVertexAttribDivisor(6, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void VoxelStructure::delete_instance_buffer() {
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
    }
}

VoxelStructure::~VoxelStructure() {
    delete shader;
    delete cube;
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
    }
}

void VoxelStructure::setVoxelValue(int x, int y, int z, int value) {
    int index = height * width * z + width * y + x;
    if (index >= 0 && index < numVoxels) {
        voxels[index] = value;
    }
}

int VoxelStructure::getVoxelValue(int x, int y, int z) {
    int index = height * width * z + width * y + x;
    if (index >= 0 && index < numVoxels) {
        return voxels[index];
    }
    return 0;
}

glm::vec3 VoxelStructure::getVoxelToWorldSpace(int x, int y, int z) {
    glm::vec3 localPos = glm::vec3(x, y, z) * cellSize;
    return position + localPos; 
    glm::vec3 voxelPos = glm::vec3(x, y, z) * cellSize - position; 
    return voxelPos;
}

void VoxelStructure::drawVoxels(Renderer& renderer, glm::mat4 viewMatrix) {
    shader->Bind();
    shader->SetUniformMat4("proj", renderer.get_proj());
    shader->SetUniformMat4("view", viewMatrix);

    GLuint vao = cube->getVAO();             
    unsigned int indexCount = cube->getIndexCount(); 

    RenderCommand cmd{};
    cmd.vao        = vao;
    cmd.draw_type   = DrawType::ElementsInstanced;
    cmd.primitive = GL_TRIANGLES;
    cmd.count      = indexCount;
    cmd.instance_count = numVoxels;
    cmd.shader     = shader;

    renderer.submit(RenderPass::Forward, cmd);
}

glm::mat4 VoxelStructure::getModelMatrix(int x, int y, int z) {
    glm::vec3 worldPos = getVoxelToWorldSpace(x, y, z);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, worldPos);
    model = glm::scale(model, glm::vec3(cellSize * 0.5f)); 
    return glm::scale(glm::translate(glm::mat4(1.f), worldPos), glm::vec3(this->cellSize * 0.5f));
}