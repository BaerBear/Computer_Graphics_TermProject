#pragma once
#include <gl/glew.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <gl/glm/gtc/type_ptr.hpp>
#include "ObjRead.h"
#include "Vertex.h"
#include <vector>

class OBJ_MODEL {
public:
    void init(GLuint shaderProgramID);
    void loadFromFile(const char* filename);
    void loadFromFile(const char* filename, float r, float g, float b);  // RGB 색상 지정 버전
    void draw(const glm::mat4& modelMatrix);  // 외부에서 변환 행렬을 받는 함수
    void cleanup();

    // 기본적인 조회 함수들만 유지
    void printModelInfo() const;

private:
    GLuint VAO_ = 0, VBO_ = 0;
    GLuint shaderProgramID_;
    MODEL objModel_;
    std::vector<VERTEX> vertices_;
    
    glm::vec3 color_ = glm::vec3(-1.0f, -1.0f, -1.0f);  // -1은 랜덤 색상 사용을 의미

    void convertToVertexArray();
    void setupOpenGL();
};