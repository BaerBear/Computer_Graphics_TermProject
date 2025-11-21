#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>
#include <vector>
#include <random>
#include "Vertex.h"
#include "Axis.h"
#include "ObjModel.h"

std::mt19937 rd(std::random_device{}());
std::uniform_real_distribution<float> speed(1.0f, 2.5f);
std::uniform_real_distribution<float> Fspeed(0.01f, 0.07f);

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

//------------------------------------------------------------
// 유니폼 변수 주의 사항
// 유니폼으로(uniform) 선언한 변수는 할당했으면 반드시 사용할 것.
// 사용하지 않으면 프로그램이 실행되지 않을 수 있음.
//------------------------------------------------------------
struct OBJMODEL {
	OBJ_MODEL m;
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 orbitRotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 selfscale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	GLfloat rotateSpeed = speed(rd);
	GLfloat orbitRadius = 0.0f;
	GLfloat orbitSpeed = speed(rd);

	void reset() {
		translation = glm::vec3(0.0f, 0.0f, 0.0f);
		rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		orbitRotation = glm::vec3(0.0f, 0.0f, 0.0f);
		selfscale = glm::vec3(1.0f, 1.0f, 1.0f);
		scale = glm::vec3(1.0f, 1.0f, 1.0f);
	}
};

GLuint VBO, VAO;

AXIS Axis;
std::vector<OBJMODEL> Model;
OBJMODEL LightModel;
OBJMODEL FloorModel;

bool rotateCW_X = false, rotateCCW_X = false;
bool rotateCW_Y = false, rotateCCW_Y = false;
bool rotateCW_Z = false, rotateCCW_Z = false;

float bgR = 0.f, bgG = 0.f, bgB = 0.f;

bool DrawSolid = true;
bool DrawWireframe = false;
bool DepthTest = true;
bool CullFace = false;
bool ProjectionPerspective = true;

glm::mat4 createOrbitMatrix(int index);
GLfloat transformx(int x) { return ((float)x / (WINDOW_WIDTH / 2)) - 1.0f; }
GLfloat transformy(int y) { return ((WINDOW_HEIGHT - (float)y) / (WINDOW_HEIGHT / 2)) - 1.0f; }
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid KeyBoard(unsigned char key, int x, int y);
GLvoid Timer(int value);
void init();

//--- 필요한 변수 선언
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

// 3D 변환을 위한 변수들 추가
glm::vec3 modelPos = glm::vec3(0.0f, 0.0f, 0.0f);      // 모델 위치
glm::vec3 cameraPos = glm::vec3(7.0f, 7.0f, 7.0f);	   // 카메라 위치
GLfloat cameraAngle = glm::atan(glm::radians(45.0f));   // 초기 각도 (45도 라디안)

// 전역 변수 추가
bool turnOnLight = true;
glm::vec3 lightPos = glm::vec3(0.0f, 1.0f, 1.0f);     // 광원 위치
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);	  // 광원 색
float lightIntensity = 0.5f;						  // 광원 세기

// 조명 공전 관련 변수
float lightOrbitAngle = 0.0f;      // 공전 각도
float lightOrbitRadius = 5.0f;     // 공전 반지름
bool lightOrbitCW = false;         // 시계 방향 공전
bool lightOrbitCCW = false;        // 반시계 방향 공전
float lightOrbitSpeed = 1.0f;      // 공전 속도

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

//--- 메인 함수
void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Example1");

	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();

	//--- 세이더 읽어와서 세이더 프로그램 만들기: 사용자 정의함수 호출
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	shaderProgramID = make_shaderProgram();

	init();

	//--- 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(KeyBoard);
	glutTimerFunc(16, Timer, 1);
	glutMainLoop();
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("Phong_vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("Phong_fragment.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
	GLuint shaderID;
	shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기
	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return false;
	}
	glUseProgram(shaderID); //--- 만들어진 세이더 프로그램 사용하기
	//--- 여러 개의 세이더프로그램 만들 수 있고, 그 중 한개의 프로그램을 사용하려면
	//--- glUseProgram 함수를 호출하여 사용 할 특정 프로그램을 지정한다.
	//--- 사용하기 직전에 호출할 수 있다.
	return shaderID;
}

void init() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glUseProgram(shaderProgramID);
	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos"); //--- lightPos 값 전달: (0.0, 0.0, 5.0);
	glUniform3f(lightPosLocation, lightPos.x, lightPos.y, lightPos.z);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor"); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
	glUniform3f(lightColorLocation, lightColor.r, lightColor.g, lightColor.b);
	unsigned int objColorLocation = glGetUniformLocation(shaderProgramID, "objectColor"); //--- object Color값 전달: (1.0, 0.5, 0.3)의 색
	glUniform3f(objColorLocation, 1.0, 0.5, 0.3);
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos"); //--- viewPos 값 전달: 카메라 위치
	glUniform3f(viewPosLocation, cameraPos.x, cameraPos.y, cameraPos.z);

	lightOrbitRadius = sqrt(lightPos.x * lightPos.x + lightPos.z * lightPos.z);
	lightOrbitAngle = glm::degrees(atan2(lightPos.z, lightPos.x));
	if (lightOrbitAngle < 0.0f) lightOrbitAngle += 360.0f;

	Model.push_back(OBJMODEL());
	Model[0].m.init(shaderProgramID);
	Model[0].m.loadFromFile("obj/cube.obj", 1.0f, 1.0f, 0.0f);
	Model[0].selfscale = glm::vec3(1.0f, 1.0f, 1.0f);
	Model[0].orbitRadius = 0.0f;

	LightModel.m.init(shaderProgramID);
	LightModel.m.loadFromFile("obj/cube.obj");
	LightModel.selfscale = glm::vec3(0.5f, 0.5f, 0.5f);
	LightModel.translation = lightPos;

	FloorModel.m.init(shaderProgramID);
	FloorModel.m.loadFromFile("obj/plane.obj");
	FloorModel.translation = glm::vec3(0.0f, 0.0f, 0.0f);
	FloorModel.selfscale = glm::vec3(0.8f, 1.0f, 0.8f);

	Axis.init(shaderProgramID);
}

glm::mat4 createModelMatrix(int index) {
	if (index < 0 || index >= Model.size()) {
		std::cerr << "Invalid model index: " << index << std::endl;
		return glm::mat4(1.0f); // Return identity matrix for invalid index
	}
	glm::mat4 modelMatrix = glm::mat4(1.0f);

	modelMatrix = glm::scale(modelMatrix, Model[index].scale);

	modelMatrix = glm::rotate(modelMatrix, glm::radians(Model[index].orbitRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(Model[index].orbitRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(Model[index].orbitRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	modelMatrix = glm::translate(modelMatrix, Model[index].translation);

	modelMatrix = glm::rotate(modelMatrix, glm::radians(Model[index].rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(Model[index].rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::rotate(modelMatrix, glm::radians(Model[index].rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	modelMatrix = glm::scale(modelMatrix, Model[index].selfscale);

	return modelMatrix;
}

//--- 출력 콜백 함수
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	glUseProgram(shaderProgramID);

	if (DepthTest) glEnable(GL_DEPTH_TEST); // 깊이버퍼 활성화
	else glDisable(GL_DEPTH_TEST);

	if (CullFace) glEnable(GL_CULL_FACE); // 면 제거 활성화
	else glDisable(GL_CULL_FACE);

	glClearColor(bgR, bgG, bgB, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 조명 ON/OFF uniform 전달
	GLint turnOnLightLoc = glGetUniformLocation(shaderProgramID, "turnOnLight");
	glUniform1i(turnOnLightLoc, turnOnLight);

	GLfloat lightIntensityLoc = glGetUniformLocation(shaderProgramID, "lightIntensity");
	glUniform1f(lightIntensityLoc, lightIntensity);

	// 조명 유니폼 전달 (항상 전달, 셰이더에서 turnOnLight로 제어)
	GLint lightPosLoc = glGetUniformLocation(shaderProgramID, "lightPos");
	GLint lightColorLoc = glGetUniformLocation(shaderProgramID, "lightColor");
	GLint viewPosLoc = glGetUniformLocation(shaderProgramID, "viewPos");

	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

	// Uniform 매트릭스 매핑
	GLint viewLoc = glGetUniformLocation(shaderProgramID, "view");
	GLint projLoc = glGetUniformLocation(shaderProgramID, "proj");

	//--------------------------------------------------------------------------
	// Camera (View) 및 Projection 매트릭스 설정
	glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // 뷰 매트릭스
	glm::mat4 proj = glm::mat4(1.0f);
	if (ProjectionPerspective) {
		proj = glm::perspective(cameraAngle, (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f); // 프로젝션 매트릭스
	}
	else if (!ProjectionPerspective) {
		float orthoScale = 5.0f;
		proj = glm::ortho(-orthoScale, orthoScale, -orthoScale, orthoScale, 0.1f, 100.0f);
	}

	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

	// 축 그리기 (축은 회전하지 않음)
	GLint modelLoc = glGetUniformLocation(shaderProgramID, "modelMat");
	glm::mat4 axisModel = glm::mat4(1.0f); // 단위 행렬 (회전 없음)
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(axisModel));
	Axis.Draw();

	glm::mat4 lightModelMatrix = glm::mat4(1.0f);
	lightModelMatrix = glm::translate(lightModelMatrix, lightPos);
	lightModelMatrix = glm::scale(lightModelMatrix, LightModel.selfscale);

	LightModel.m.draw(lightModelMatrix);

	// OBJ 모델 그리기
	for (int i = 0; i < Model.size(); ++i) {
		glm::mat4 modelMatrix = createModelMatrix(i);
		Model[i].m.draw(modelMatrix);
	}
	glm::mat4 FloorModelMatrix = glm::mat4(1.0f);
	FloorModelMatrix = glm::translate(FloorModelMatrix, FloorModel.translation);
	FloorModelMatrix = glm::scale(FloorModelMatrix, FloorModel.selfscale);
	FloorModel.m.draw(FloorModelMatrix);

	glutSwapBuffers(); // 화면에 출력하기
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
}

GLvoid KeyBoard(unsigned char key, int x, int y) //--- 콜백 함수: 키보드 콜백 함수 
{
	switch (key)
	{
	case 'h':
	case 'H': {
		DepthTest = !DepthTest;
		std::cout << "Depth Test: " << (DepthTest ? "Enabled" : "Disabled") << std::endl;
		break;
	}
	case 'u':
	case 'U': {
		CullFace = !CullFace;
		std::cout << "Cull Face: " << (CullFace ? "Enabled" : "Disabled") << std::endl;
		break;
	}
	case 'p': {
		ProjectionPerspective = false;
		std::cout << "Orthographic Projection" << std::endl;
		break;
	}
	case 'P': {
		ProjectionPerspective = true;
		std::cout << "Perspective Projection" << std::endl;
		break;
	}
	case 'm': {
		DrawSolid = true;
		std::cout << "Draw Solid Mode" << std::endl;
		break;
	}
	case 'M': {
		DrawWireframe = true;
		std::cout << "Draw Wireframe Mode" << std::endl;
		break;
	}
	case 'r':
	case 'R': {
		turnOnLight = !turnOnLight;
		std::cout << "Lighting: " << (turnOnLight ? "ON" : "OFF") << std::endl;
		break;
	}
	case '+': {
		lightIntensity += 0.1f;
		if (lightIntensity > 1.0f) lightIntensity = 1.0f;
		std::cout << "Light Intensity: " << lightIntensity << std::endl;
		break;
	}
	case '-': {
		lightIntensity -= 0.1f;
		if (lightIntensity < 0.0f) lightIntensity = 0.0;
		std::cout << "Light Intensity: " << lightIntensity << std::endl;
		break;
	}
	case 'Q':
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value) {

	glutPostRedisplay();
	glutTimerFunc(16, Timer, 1);
}
