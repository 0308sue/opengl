#include <glad/glad.h>   // OpenGL 함수들을 실제로 사용할 수 있게 해주는 헤더
#include <GLFW/glfw3.h>  // 창 생성, 키보드 입력 처리, OpenGL context 생성용
#include <iostream>      // cout 같은 출력용
#include <glm/glm.hpp>   // glm 기본 수학 타입(vec, mat 등)
#include <glm/gtc/matrix_transform.hpp> // rotate, translate 같은 변환 함수
#include <glm/gtc/type_ptr.hpp>         // glm 행렬 주소를 OpenGL에 넘길 때 사용

// 창 크기 바뀌면 viewport도 같이 바꿔주는 함수
// 선언을 먼저 해두고, 아래에서 실제 함수 내용을 작성함
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

// -------------------- Vertex Shader 코드 --------------------
// 이건 GPU에서 실행되는 코드이고,
// 정점(position, color)을 받아서 최종 위치와 색 정보를 넘겨주는 역할
const char* vertexShaderSource = "#version 410 core\n"
"layout (location = 0) in vec3 aPos;\n"      // 0번 속성 자리로 위치값(vec3)을 받음
"layout (location = 1) in vec3 aColor;\n"    // 1번 속성 자리로 색값(vec3)을 받음
"out vec3 ourColor;\n"                       // fragment shader로 색을 넘길 출력 변수
"uniform mat4 transform;\n"                  // CPU에서 보내줄 변환 행렬(회전, 이동 등)
"void main()\n"
"{\n"
"   gl_Position = transform * vec4(aPos, 1.0);\n" // 정점 위치 aPos에 transform을 적용해서 새로운 위치 계산
"   ourColor = aColor;\n"                          // 입력받은 색을 그대로 다음 단계로 넘김
"}\n";

// -------------------- Fragment Shader 코드 --------------------
// 이건 픽셀 색을 최종적으로 정하는 GPU 코드
const char* fragmentShaderSource = "#version 410 core\n"
"out vec4 FragColor;\n"      // 최종 출력 색
"in vec3 ourColor;\n"        // vertex shader에서 넘어온 색
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n" // RGB 색에 alpha=1.0(완전 불투명) 붙여서 출력
"}\n";

// 창 크기가 바뀔 때 호출되는 callback 함수
// width, height에 맞게 viewport도 다시 맞춰줘야 화면이 정상적으로 보임
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ESC 키 누르면 창 닫히게 하는 입력 처리 함수
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// shader 컴파일이 제대로 됐는지 검사하는 함수
// 에러 나면 콘솔에 이유 출력
void checkShaderCompile(unsigned int shader)
{
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "Shader Compile Error:\n" << infoLog << std::endl;
    }
}

// shader program 링크가 제대로 됐는지 검사하는 함수
void checkProgramLink(unsigned int program)
{
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cout << "Program Link Error:\n" << infoLog << std::endl;
    }
}

int main()
{
    // GLFW 시작
    glfwInit();

    // -------------------- OpenGL 버전 및 옵션 설정 --------------------
    // 맥에서는 보통 OpenGL 4.1 Core Profile을 많이 사용함
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // OpenGL 메이저 버전 4
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1); // OpenGL 마이너 버전 1 -> 즉 OpenGL 4.1
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 최신 방식(Core Profile) 사용
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS 호환용 옵션

    // 창 생성
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rotation and Revolution", NULL, NULL);

    // 창 생성 실패하면 종료
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 이제 이 창을 현재 OpenGL이 그릴 대상으로 설정
    glfwMakeContextCurrent(window);

    // 창 크기 바뀔 때 자동으로 framebuffer_size_callback 함수가 실행되도록 등록
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // GLAD 초기화
    // 이걸 해야 OpenGL 함수들을 실제로 사용할 수 있음
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // OpenGL이 그릴 영역 설정
    // 여기서는 창 전체를 다 쓰겠다는 뜻
    glViewport(0, 0, 800, 600);

 
    // 1. 셰이더 만들기
    // shader = GPU에서 실행되는 프로그램
 
    // vertex shader 객체 생성
    // vertex shader = 정점 위치/속성 처리
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);

    // 아까 문자열로 써둔 vertex shader 코드를 이 shader 객체에 넣음
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);

    // 실제로 GPU가 이해할 수 있게 컴파일
    glCompileShader(vertexShader);

    // 컴파일 에러 확인
    checkShaderCompile(vertexShader);

    // fragment shader 객체 생성
    // fragment shader = 픽셀 색 결정
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    // fragment shader 코드 넣기
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);

    // 컴파일
    glCompileShader(fragmentShader);

    // 컴파일 에러 확인
    checkShaderCompile(fragmentShader);

    // shader program 생성
    // vertex shader + fragment shader를 묶어서 실제로 쓸 프로그램으로 만드는 단계
    unsigned int shaderProgram = glCreateProgram();

    // program에 vertex shader 붙이기
    glAttachShader(shaderProgram, vertexShader);

    // program에 fragment shader 붙이기
    glAttachShader(shaderProgram, fragmentShader);

    // 둘을 하나의 program으로 링크
    glLinkProgram(shaderProgram);

    // 링크 에러 확인
    checkProgramLink(shaderProgram);

    // 이제 shaderProgram 안에 필요한 내용이 다 들어갔으므로
    // 개별 shader 객체는 삭제해도 됨
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

 
    // 2. 정점 데이터 준비
 

    // 첫 번째 삼각형
    // position 3개 + color 3개 = 한 정점당 총 6개 float
    // 이 삼각형은 원점 근처에 있어서 그냥 회전시키면 원점을 중심으로 자전하게 됨
    float triangle1[] = {
        // position               // color
        -0.2f, -0.2f, 0.0f,       1.0f, 0.0f, 0.0f, // 왼쪽 아래 점, 빨강
         0.2f, -0.2f, 0.0f,       0.0f, 1.0f, 0.0f, // 오른쪽 아래 점, 초록
         0.0f,  0.2f, 0.0f,       0.0f, 0.0f, 1.0f  // 위쪽 점, 파랑
    };

    // 두 번째 삼각형
    // 이것도 자기 중심이 원점에 오도록 만들어둠
    // 나중에 transform으로 오른쪽으로 이동시키고 회전시키면 공전처럼 보이게 만들 수 있음
    float triangle2[] = {
        // position               // color
        -0.15f, -0.15f, 0.0f,     1.0f, 1.0f, 0.0f, // 노랑
         0.15f, -0.15f, 0.0f,     0.0f, 1.0f, 1.0f, // 하늘색
         0.0f,   0.15f, 0.0f,     1.0f, 0.0f, 1.0f  // 자홍
    };

    
    // 3. 첫 번째 삼각형용 VAO / VBO 설정
    // VBO = 실제 정점 데이터 저장소
    // VAO = 그 데이터를 어떻게 읽을지 저장하는 설정 묶음

    unsigned int VAO1, VBO1;

    // VAO 1개 생성
    glGenVertexArrays(1, &VAO1);

    // VBO 1개 생성
    glGenBuffers(1, &VBO1);

    // 지금부터 VAO1에 대한 설정을 저장하겠다는 뜻
    glBindVertexArray(VAO1);

    // GL_ARRAY_BUFFER 대상으로 VBO1을 바인딩
    // 즉 정점 데이터 저장용 버퍼로 VBO1을 쓰겠다는 뜻
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);

    // CPU 배열 triangle1 데이터를 GPU의 VBO1로 복사
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle1), triangle1, GL_STATIC_DRAW);

    // location 0 = position
    // 한 정점당 6개의 float 중 앞의 3개가 위치값
    glVertexAttribPointer(
        0,                      // shader의 layout(location = 0)
        3,                      // x, y, z 총 3개
        GL_FLOAT,               // float 타입
        GL_FALSE,               // 정규화 안 함
        6 * sizeof(float),      // 한 정점 전체 크기(위치 3 + 색 3)
        (void*)0                // 시작 위치는 맨 앞(위치부터 시작)
    );
    glEnableVertexAttribArray(0); // 0번 속성 활성화

    // location 1 = color
    // 한 정점당 6개의 float 중 뒤의 3개가 색값
    glVertexAttribPointer(
        1,                              // shader의 layout(location = 1)
        3,                              // r, g, b 총 3개
        GL_FLOAT,                       // float 타입
        GL_FALSE,                       // 정규화 안 함
        6 * sizeof(float),              // 한 정점 전체 크기
        (void*)(3 * sizeof(float))      // 앞의 위치 3개를 건너뛰고 색부터 시작
    );
    glEnableVertexAttribArray(1); // 1번 속성 활성화

    // 4. 두 번째 삼각형용 VAO / VBO 설정

    unsigned int VAO2, VBO2;

    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle2), triangle2, GL_STATIC_DRAW);

    // 두 번째 삼각형도 구조는 같음
    // position 3개 + color 3개
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(
        1,
        3,
        GL_FLOAT,
        GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // 지금은 설정 끝났으니 바인딩 해제
    // 설정 자체가 사라지는 건 아니고, 그냥 현재 선택만 풀어주는 느낌
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // shader 안의 uniform 변수 "transform" 위치 가져오기
    // 나중에 CPU에서 행렬 값을 보낼 때 이 위치를 사용함
    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");

    // 배경색 설정
    // 매 프레임 glClear 할 때 이 색으로 화면을 지움
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

 
    // 5. 렌더링 루프
 
    // 창이 닫히기 전까지 계속 반복
    while (!glfwWindowShouldClose(window))
    {
        // ESC 입력 처리
        processInput(window);

        // 화면 지우기
        glClear(GL_COLOR_BUFFER_BIT);

        // 지금부터 이 shader program 사용
        glUseProgram(shaderProgram);

        // GLFW가 시작된 이후의 시간(초 단위)
        // 이 값을 회전에 쓰면 시간이 흐를수록 계속 돌게 됨
        float time = (float)glfwGetTime();


        // 1) 첫 번째 삼각형: 원점을 중심으로 자전

        // 단위행렬로 시작
        glm::mat4 transform1 = glm::mat4(1.0f);

        // z축 기준으로 회전
        // triangle1이 원점 근처에 있으므로 그냥 rotate만 해도 자전처럼 보임
        transform1 = glm::rotate(transform1, time, glm::vec3(0.0f, 0.0f, 1.0f));

        // 현재 transform1 행렬을 shader의 uniform transform에 보냄
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform1));

        // 첫 번째 삼각형용 VAO 사용
        glBindVertexArray(VAO1);

        // 정점 3개를 삼각형 1개로 그림
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // --------------------------------------------
        // 2) 두 번째 삼각형:
        //    원점을 중심으로 공전 + 자기 중심 자전
        // --------------------------------------------

        glm::mat4 transform2 = glm::mat4(1.0f);

        // 먼저 원점을 기준으로 회전
        // 이 회전 자체가 "공전 궤도 방향"을 만들어주는 역할
        transform2 = glm::rotate(transform2, time, glm::vec3(0.0f, 0.0f, 1.0f));

        // 그 다음 오른쪽으로 이동
        // 즉 원점에서 반지름 0.6만큼 떨어진 위치로 보냄
        // 위에서 회전하고 나서 이동하므로 결과적으로 원점 주위를 도는 것처럼 보임
        transform2 = glm::translate(transform2, glm::vec3(0.0f, 0.9f, 0.0f));
        
        // 이동된 상태에서 자기 자신도 더 빠르게 회전
        // 그래서 공전 + 자전을 동시에 하게 됨
        transform2 = glm::rotate(transform2, time * 2.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        // 두 번째 삼각형용 transform 전달
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(transform2));

        // 두 번째 삼각형용 VAO 바인딩ㄷㄷ
        glBindVertexArray(VAO2);

        // 두 번째 삼각형 그리기
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 백버퍼/프론트버퍼 교체
        // 지금 그린 결과를 화면에 실제로 보여줌
        glfwSwapBuffers(window);

        // 키보드, 마우스, 창 이벤트 처리
        glfwPollEvents();
    }

 
    // 6. 종료 전 리소스 정리
 

    // 첫 번째 삼각형 관련 객체 삭제
    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);

    // 두 번째 삼각형 관련 객체 삭제
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);

    // shader program 삭제
    glDeleteProgram(shaderProgram);

    // GLFW 종료
    glfwTerminate();
    return 0;
}