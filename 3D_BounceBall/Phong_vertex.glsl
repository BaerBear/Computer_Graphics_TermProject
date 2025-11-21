#version 330 core

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vColor;
layout (location = 2) in vec3 vNormal;

out vec3 FragPos; //--- 객체의 위치값을 프래그먼트 세이더로 보낸다.
out vec3 Normal; //--- 노멀값을 프래그먼트 세이더로 보낸다.
out vec3 vertexColor;

uniform mat4 modelMat; //--- 모델링 변환값
uniform mat4 view; //--- 뷰잉 변환값
uniform mat4 proj; //--- 투영 변환값

void main()
{
    gl_Position = proj * view * modelMat * vec4(vPos, 1.0);
    FragPos = vec3(modelMat * vec4(vPos, 1.0));
    Normal = mat3(transpose(inverse(modelMat))) * vNormal;  // 노멀 변환
    vertexColor = vColor;
}