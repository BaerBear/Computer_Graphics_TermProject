#include "ObjModel.h"
#include <iostream>

void OBJ_MODEL::init(GLuint shaderProgramID) {
    shaderProgramID_ = shaderProgramID;

    // 메모리 초기화
    objModel_.vertices = nullptr;
    objModel_.faces = nullptr;
    objModel_.vertex_count = 0;
    objModel_.face_count = 0;

    std::cout << "OBJ Model initialized" << std::endl;
}

void OBJ_MODEL::loadFromFile(const char* filename) {
    // 랜덤 색상 사용 (기본값)
    color_ = glm::vec3(-1.0f, -1.0f, -1.0f);
    
    // 1. OBJ 파일 읽기
    read_obj_file(filename, &objModel_);

    if (objModel_.vertex_count == 0) {
        std::cerr << "Failed to load OBJ file: " << filename << std::endl;
        return;
    }

    std::cout << "OBJ file loaded successfully: " << filename << std::endl;
    std::cout << "Vertices: " << objModel_.vertex_count << ", Faces: " << objModel_.face_count << std::endl;

    // 2. OpenGL 버텍스 배열로 변환
    convertToVertexArray();

    // 3. OpenGL 설정
    setupOpenGL();
}

void OBJ_MODEL::loadFromFile(const char* filename, float r, float g, float b) {
    // 지정된 색상 사용
    color_ = glm::vec3(r, g, b);
    
    // 1. OBJ 파일 읽기
    read_obj_file(filename, &objModel_);

    if (objModel_.vertex_count == 0) {
        std::cerr << "Failed to load OBJ file: " << filename << std::endl;
        return;
    }

    std::cout << "OBJ file loaded successfully: " << filename << " with color RGB(" 
              << r << ", " << g << ", " << b << ")" << std::endl;
    std::cout << "Vertices: " << objModel_.vertex_count << ", Faces: " << objModel_.face_count << std::endl;

    // 2. OpenGL 버텍스 배열로 변환
    convertToVertexArray();

    // 3. OpenGL 설정
    setupOpenGL();
}

void OBJ_MODEL::convertToVertexArray() {
    vertices_.clear();

    for (size_t i = 0; i < objModel_.face_count; ++i) {
        Face& face = objModel_.faces[i];

        // 삼각형의 세 정점 가져오기
        glm::vec3 v0(objModel_.vertices[face.v1].x, objModel_.vertices[face.v1].y, objModel_.vertices[face.v1].z);
        glm::vec3 v1(objModel_.vertices[face.v2].x, objModel_.vertices[face.v2].y, objModel_.vertices[face.v2].z);
        glm::vec3 v2(objModel_.vertices[face.v3].x, objModel_.vertices[face.v3].y, objModel_.vertices[face.v3].z);

        // 페이스 노말 계산 (외적)
        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

        for (int j = 0; j < 3; ++j) {
            unsigned int vertexIndex;
            if (j == 0) vertexIndex = face.v1;
            else if (j == 1) vertexIndex = face.v2;
            else vertexIndex = face.v3;

            if (vertexIndex < objModel_.vertex_count) {
                VERTEX vertex;
                vertex.x = objModel_.vertices[vertexIndex].x;
                vertex.y = objModel_.vertices[vertexIndex].y;
                vertex.z = objModel_.vertices[vertexIndex].z;
                
                // 색상 설정: color_가 (-1, -1, -1)이면 랜덤, 아니면 지정된 색상 사용
                if (color_.x < 0.0f) {
                    // 랜덤 색상
                    vertex.r = static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
                    vertex.g = static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
                    vertex.b = static_cast <float>(rand()) / static_cast <float>(RAND_MAX);
                } else {
                    // 지정된 색상
                    vertex.r = color_.x;
                    vertex.g = color_.y;
                    vertex.b = color_.z;
                }

                // 노말 추가
                vertex.nx = normal.x;
                vertex.ny = normal.y;
                vertex.nz = normal.z;

                vertices_.push_back(vertex);
            }
        }
    }

    std::cout << "Converted to vertex array. Total vertices: " << vertices_.size() << std::endl;
}

void OBJ_MODEL::setupOpenGL() {
    if (VAO_ != 0) {
        glDeleteVertexArrays(1, &VAO_);
    }
    if (VBO_ != 0) {
        glDeleteBuffers(1, &VBO_);
    }

    glGenVertexArrays(1, &VAO_);
    glGenBuffers(1, &VBO_);

    glBindVertexArray(VAO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);

    glBufferData(GL_ARRAY_BUFFER,
        vertices_.size() * sizeof(VERTEX),
        vertices_.data(),
        GL_STATIC_DRAW);

    // 위치 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // 색상 (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX),
        (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // 노멀 (location = 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX),
        (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    std::cout << "OpenGL setup completed. VAO: " << VAO_ << ", VBO: " << VBO_ << std::endl;
}

void OBJ_MODEL::draw(const glm::mat4& modelMatrix) {
    if (vertices_.empty()) {
        std::cerr << "No vertices to draw!" << std::endl;
        return;
    }

    // 외부에서 받은 변환 행렬을 셰이더에 전달
    GLint modelLoc = glGetUniformLocation(shaderProgramID_, "modelMat");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // 렌더링
    glBindVertexArray(VAO_);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices_.size()));
    glBindVertexArray(0);
}

void OBJ_MODEL::printModelInfo() const {
    std::cout << "=== OBJ Model Info ===" << std::endl;
    std::cout << "Vertex count: " << objModel_.vertex_count << std::endl;
    std::cout << "Face count: " << objModel_.face_count << std::endl;
    std::cout << "Rendered vertices: " << vertices_.size() << std::endl;
}

void OBJ_MODEL::cleanup() {
    if (objModel_.vertices) {
        free(objModel_.vertices);
        objModel_.vertices = nullptr;
    }
    if (objModel_.faces) {
        free(objModel_.faces);
        objModel_.faces = nullptr;
    }

    if (VAO_ != 0) {
        glDeleteVertexArrays(1, &VAO_);
        VAO_ = 0;
    }
    if (VBO_ != 0) {
        glDeleteBuffers(1, &VBO_);
        VBO_ = 0;
    }
}