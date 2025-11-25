프로젝트 개요

이 리포지토리는 OpenGL/GLUT 기반의 간단한 3D Bounce Ball 게임(Computer Graphics Term Project)입니다. 주요 역할은 렌더링, 입력/출력 처리, 카메라 제어, 게임 월드 업데이트 등입니다.

파일 및 연결 관계 (간단 요약)

- `main.cpp`
  - 프로그램 진입점. OpenGL/GLUT 초기화, 셰이더 로드/컴파일, 주요 객체 생성 및 루프 등록을 수행합니다.
  - 포함된 헤더: `Headers.h`, `Vertex.h`, `Axis.h`, `ObjModel.h`, `GameWorld.h`, `InputHandler.h`, `OutputHandler.h`, `Camera.h`.
  - 셰이더 파일(`Phong_vertex.glsl`, `Phong_fragment.glsl`)을 실행 디렉터리에 두어야 합니다(파일 로드 함수 `filetobuf` 사용).
  - 전역으로 선언된 설정 변수(조명, 렌더링 옵션, 윈도우 크기 등)를 통해 입력 핸들러와 출력 핸들러가 상태를 공유합니다.
  - 주요 역할: 윈도우/셰이더/VAO/VBO 초기화, 카메라 및 GameWorld 생성, GLUT 콜백(draw, reshape, keyboard, idle) 등록.

- `Headers.h`
  - 공통 헤더(라이브러리 포함, 전처리기 정의 등)를 모아두는 파일입니다. 프로젝트 전역에서 필요한 OpenGL/GLM/GLUT/GLEW 등의 include를 포함할 가능성이 큽니다.

- `Vertex.h`
  - 정점 구조체(`VERTEX`) 정의와 정점 관련 유틸리티가 들어있습니다.
  - `main.cpp`에서 VAO/VBO 구성 시 `VERTEX` 크기를 사용합니다.

- `Axis.h` / `Axis.cpp`
  - 좌표축(AXIS) 렌더링을 담당하는 클래스입니다. 셰이더 프로그램 ID를 받아 초기화(`init`)하고 `Draw()`로 축을 렌더링합니다.
  - `main.cpp`는 `Axis.init(shaderProgramID)` 및 `Axis.Draw()`를 호출합니다.

- `ObjModel.h` / `ObjModel.cpp`
  - .obj 모델 로더 및 모델 렌더링 클래스입니다. Obj 파일을 읽어 정점/노멀/텍스처 좌표를 파싱하고 GPU로 전송합니다.
  - 쉐이더의 속성 위치 및 매트릭스 유니폼과 호환되도록 구현되어야 합니다.

- `GameWorld.h` / `GameWorld.cpp`
  - 게임 전체 상태(플레이어, 장애물, 충돌 검사, 물리 업데이트 등)를 관리합니다.
  - `GameWorld::initialize()`로 초기 환경을 생성하고 `update(float dt)`로 매 프레임 상태를 갱신하며 `draw()`로 모든 게임 오브젝트를 렌더링합니다.
  - `main.cpp`에서 `GameWorld` 인스턴스를 생성하여 게임 루프에 연결합니다.

- `InputHandler.h` / `InputHandler.cpp`
  - 키보드 및 마우스 입력을 처리하여 게임 상태(플레이어 이동, 토글 옵션 등)에 반영합니다.
  - `main.cpp`에서 `inputHandler->setGameWorld(gameWorld)` 및 `setCamera(camera)` 등을 호출하여 참조를 제공합니다.
  - 키 입력 콜백(`KeyBoardDown`)에서 `inputHandler->handleKeyboard`를 호출합니다.

- `OutputHandler.h` / `OutputHandler.cpp`
  - 키 업 이벤트 등 일부 입력을 받아 게임/렌더링 설정을 변경하거나 디버그 출력을 담당합니다.
  - `main.cpp`에서 `outputHandler->setGameWorld(gameWorld)` 및 `setCamera(camera)` 등을 호출합니다.

- `Camera.h` / `Camera.cpp`
  - 카메라 위치, 타겟, yaw/pitch/roll 제어 및 뷰/프로젝션 매트릭스를 계산합니다.
  - `camera->getViewMatrix()` 및 `camera->getPosition()` 등을 통해 `main.cpp`에서 셰이더로 전달됩니다.

- `Phong_vertex.glsl` / `Phong_fragment.glsl`
  - Phong 조명 모델을 구현한 버텍스 및 프래그먼트 셰이더 파일입니다. `main.cpp`의 `filetobuf`로 읽혀 컴파일됩니다.
  - 셰이더 내부의 유니폼 이름(`lightPos`, `lightColor`, `turnOnLight`, `view`, `proj`, `modelMat`, `viewPos`, `lightIntensity`, `objectColor` 등)은 `main.cpp`와 일치해야 합니다.

빌드 및 실행

1. 요구사항
   - Visual Studio (또는 C++14 호환 컴파일러)
   - GLUT (freeglut), GLEW, GLM 라이브러리
   - 위 라이브러리들이 프로젝트 설정(링커, include 디렉토리)에 올바르게 추가되어야 합니다.

2. 실행 순서(개발 환경에서)
   - 셰이더 파일(`Phong_vertex.glsl`, `Phong_fragment.glsl`)을 실행 파일(.exe)과 같은 폴더에 배치하세요 또는 `main.cpp`의 경로를 변경하세요.
   - 솔루션을 빌드하고 실행하면 윈도우가 열리며 게임이 시작됩니다.

주요 컨트롤 (프로그램 내부 출력에 표기)

- W/A/S/D : 플레이어 이동
- Space : 점프
- C : 게임 시작/일시정지
- X : 게임 리셋
- H : Depth Test 토글
- U : Cull Face 토글
- P/p : 투영 모드 전환(원근/직교)
- R : 조명 토글
- + / - : 조명 세기 조절
- Q : 종료

확장 및 디버그 포인트

- 셰이더 컴파일 에러: `main.cpp`의 `make_vertexShaders`/`make_fragmentShaders`에서 콘솔 에러 메시지를 확인하세요.
- 모델 파일(.obj) 로드 문제: `ObjModel` 클래스의 경로 또는 파일 포맷 호환성을 점검하세요.
- 카메라 동작 문제: `Camera` 클래스의 `orbitAroundTarget`, `getViewMatrix` 구현을 확인하세요.

간단 요약

- `main.cpp`가 중앙 허브이며, 다른 모듈들은 게임 월드, 입력/출력, 카메라, 모델 로딩, 축 렌더링, 셰이더로 기능이 분리되어 있습니다.
- 각 헤더/소스 파일은 해당 기능(렌더링/입력/물리/카메라 등)을 담당하므로 해당 모듈을 찾아 수정/확장하면 됩니다.

파일 위치 예시

- 루트 디렉터리: `main.cpp`, `Headers.h`, `Phong_vertex.glsl`, `Phong_fragment.glsl`, `README.txt`
- 서브 디렉터리(선택적): `src/` 또는 `models/`(obj 파일), `shaders/`(셰이더 파일)

문의사항

- 특정 파일의 내부 구현을 상세히 문서화하기 원하면, 어떤 파일을 우선적으로 설명할지 알려주십시오.