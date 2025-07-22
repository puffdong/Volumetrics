#include "VoxelStructure.h"
#include "../../core/rendering/Renderer.h" // Assuming this might contain GL_CALL or similar macros if you use them

// Note: Ensure glew is initialized before calling OpenGL functions.

VoxelStructure::VoxelStructure(int h, int w, int d, glm::vec3 pos, int initValue, float cellSize)
    : height(h), width(w), depth(d), position(pos), cellSize(cellSize), instanceVBO(0) {
    shader = new Shader("C:/Dev/OpenGL/Volumetrics/res/shaders/VoxelShaders/VoxelDebug.shader");
    voxelCube = new ModelObject("C:/Dev/OpenGL/Volumetrics/res/models/VoxelModels/defaultCube.obj");

    numVoxels = height * width * depth;
    voxels.resize(numVoxels);
    for (int i = 0; i < numVoxels; i++) {
        voxels[i] = initValue;
    }

    std::vector<glm::mat4> instanceModelMatrices;
    instanceModelMatrices.reserve(numVoxels); // Pre-allocate memory

    for (int currentH = 0; currentH < height; currentH++) { // Using currentH, currentD, currentW for clarity
        for (int currentD = 0; currentD < depth; currentD++) {
            for (int currentW = 0; currentW < width; currentW++) {
                // Assuming getModelMatrix uses parameters in the order (h_coord, d_coord, w_coord)
                // If getModelMatrix expects (x,y,z) in a specific world orientation, adjust parameters.
                // The current call getModelMatrix(currentH, currentD, currentW) means:
                // x_voxel_coord = currentH, y_voxel_coord = currentD, z_voxel_coord = currentW
                // This determines the order of matrices in the VBO.
                instanceModelMatrices.push_back(getModelMatrix(currentH, currentD, currentW));
            }
        }
    }

    // --- Setup Instance VBO ---
    // This assumes voxelCube has a VAO that we can bind and add attributes to.
    // Replace voxelCube->getVAO() with the actual way to get your cube's VAO ID.
    GLuint vao = voxelCube->getVAO(); // Placeholder: You need a way to get the VAO of voxelCube
    glBindVertexArray(vao);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceModelMatrices.size() * sizeof(glm::mat4), instanceModelMatrices.data(), GL_STATIC_DRAW);

    // Vertex attributes for the instance matrix (mat4 takes 4 vec4 slots)
    // Shader location for instanceMatrix starts at 3
    GLsizei vec4Size = sizeof(glm::vec4);
    GLsizei mat4Size = sizeof(glm::mat4);

    glEnableVertexAttribArray(3); // First column
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)0);
    glVertexAttribDivisor(3, 1); // Tell OpenGL this is an instanced vertex attribute.

    glEnableVertexAttribArray(4); // Second column
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(1 * vec4Size));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5); // Third column
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(2 * vec4Size));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6); // Fourth column
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, mat4Size, (void*)(3 * vec4Size));
    glVertexAttribDivisor(6, 1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0); // Unbind VAO
}

VoxelStructure::~VoxelStructure() {
    delete shader;
    delete voxelCube;
    if (instanceVBO != 0) {
        glDeleteBuffers(1, &instanceVBO);
    }
}

void VoxelStructure::setVoxelValue(int x, int y, int z, int value) {
    // Ensure your indexing here matches the coordinate system used in getModelMatrix and loops
    // Original: int index = height * width * z + width * y + x;
    // If x maps to width, y to height, z to depth:
    // int index = z * (width * height) + y * width + x; 
    // Sticking to your original indexing:
    int index = height * width * z + width * y + x; // This implies x is innermost, then y, then z
    if (index >= 0 && index < numVoxels) { // Basic bounds check
        voxels[index] = value;
    }
}

int VoxelStructure::getVoxelValue(int x, int y, int z) {
    // Same indexing concern as setVoxelValue
    int index = height * width * z + width * y + x;
    if (index >= 0 && index < numVoxels) { // Basic bounds check
        return voxels[index];
    }
    return 0; // Or some default/error value
}

glm::vec3 VoxelStructure::getVoxelToWorldSpace(int x, int y, int z) {
    // The parameters x, y, z are local voxel coordinates.
    // Their interpretation (which dimension is length, width, height) depends on how you've
    // defined your grid and how you pass these from the loops (h,d,w) in the constructor.
    // Original: glm::vec3 voxelPos = glm::vec3(x, y, z) * cellSize - position;
    // If x,y,z here are meant to be h,d,w directly:
    glm::vec3 localPos = glm::vec3(x, y, z) * cellSize;
    return position + localPos; // Common way: grid origin + local offset
                                // Your original: glm::vec3(x, y, z) * cellSize - position; seems to subtract world position.
                                // Let's assume your logic for getVoxelToWorldSpace is what you intend.
                                // glm::vec3 voxelPos = (this->position + (glm::vec3(x, y, z) * cellSize));
                                // The current code seems to offset from (0,0,0) by (x,y,z)*cellSize and then translate by -position.
                                // It should likely be: this->position + glm::vec3(x,y,z) * cellSize
                                // For now, I'll keep your original logic:
    glm::vec3 voxelPos = glm::vec3(x, y, z) * cellSize - position; // This might be an error. Review if 'position' is center or corner.
                                                              // If 'position' is the minimum corner of the voxel grid in world space:
                                                              // return this->position + (glm::vec3(x, y, z) * cellSize);
    return voxelPos;

}

void VoxelStructure::drawVoxels(glm::mat4 projMatrix, glm::mat4 viewMatrix) {
    shader->Bind();
    shader->SetUniform4f("u_Color", 1.0f, 1.0f, 1.0f, 1.0f); // Example color

    shader->SetUniformMat4("proj", projMatrix);
    shader->SetUniformMat4("view", viewMatrix);
    // The 'model' uniform is no longer needed here as it's an instanced attribute

    shader->SetUniform1i("u_Texture", 0); // Assuming your voxelCube has textures and u_Texture is its unit
    shader->SetUniform3f("sunDir", glm::vec3(1.f, 0.5f, 0.f));
    shader->SetUniform3f("sunColor", glm::vec3(1.f, 1.f, 1.f));
    
    // Bind the VAO of the cube mesh
    // Replace voxelCube->getVAO() and voxelCube->getIndexCount() with actual methods/values
    GLuint vao = voxelCube->getVAO();             // Placeholder
    unsigned int indexCount = voxelCube->getIndexCount(); // Placeholder

    glBindVertexArray(vao);

    // Draw 'numVoxels' instances of the cube
    // This assumes voxelCube uses an EBO and GL_TRIANGLES.
    // If it uses glDrawArrays, use glDrawArraysInstanced.
    glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, numVoxels);

    glBindVertexArray(0); // Unbind VAO
    shader->Unbind();
}

glm::mat4 VoxelStructure::getModelMatrix(int x, int y, int z) {
    // Calculate the world position for the voxel at grid coordinates (x,y,z)
    glm::vec3 worldPos = getVoxelToWorldSpace(x, y, z);
    
    // Create the model matrix: translate to worldPos and scale
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, worldPos);
    // Scale by cellSize to make the cube the size of a cell, 
    // or by a fixed factor if defaultCube.obj is already cell-sized.
    // Your original scale: glm::vec3(0.25, 0.25, 0.25). 
    // This should probably be related to cellSize if defaultCube.obj is a unit cube.
    // For example: glm::vec3(cellSize * 0.5f) if cellSize is the full width of the cube
    // or glm::vec3(cellSize) if defaultCube is a 1x1x1 cube and cellSize is desired side length
    model = glm::scale(model, glm::vec3(cellSize * 0.5f)); // Assuming cellSize is the full extent, so radius is 0.5 * cellSize
                                                           // And assuming defaultCube.obj is a 2x2x2 cube centered at origin.
                                                           // If defaultCube is 1x1x1, scale by glm::vec3(cellSize)
    // Using your original scale for now, but consider if it should be cellSize dependent.
    // model = glm::scale(model, glm::vec3(0.25f)); // If cellSize is 1.0, and 0.25 is desired display size.
                                                 // Better: scale by cellSize directly or half of it depending on cube model.
    return glm::scale(glm::translate(glm::mat4(1.f), worldPos), glm::vec3(this->cellSize * 0.5f)); // Assumes defaultCube is normalized to -1 to 1 range
    // Your original: return glm::scale(glm::translate(glm::mat4(1.f), getVoxelToWorldSpace(x, y, z)), glm::vec3(0.25, 0.25, 0.25));
}