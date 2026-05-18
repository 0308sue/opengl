#include <glad/glad.h>
#include <GLFW/glfw3.h> 
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char* vertexShaderSource = "#version 410 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"uniform mat4 transform;\n"
"void main()\n"
"{\n"
"   gl_Position = transform * vec4(aPos, 1.0);\n"
"   ourColor = aColor;\n"
"}\n";          

const char* fragmentShaderSource = "#version 410 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(ourColor, 1.0f);\n"
"}\n";  

// --- 상태 저장 변수들 ---
glm::vec3 trianglePos = glm::vec3(0.5f, 0.0f, 0.0f); // 초기 위치
float triangleScale = 1.0f; // 초기 크기
bool isDragging = false; // 드래그 상태 저장 변수
double lastMouseX, lastMouseY; // 마지막 마우스 위치 저장 변수

//마우스 휠 콜백 함수
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
    triangleScale += (float)yoffset * 0.1f; // 휠 스크롤에 따라 크기 조절
    if (triangleScale < 0.1f) triangleScale = 0.1f; // 최소 크기 제한
}

int main()
{
    glfwInit(); // GLFW 초기화
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); 
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // OpenGL 버전 설정
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL);
    if(window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // 윈도우 생성

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }           

    glViewport(0, 0, 800, 600);

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
 
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);     

    float vertices[] = {
        // 첫 번째 삼각형 (위치 x, y, z / 색상 r, g, b)
        -0.9f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, // 좌측 하단 (빨강)
        -0.1f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, // 우측 하단 (초록)
        -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // 중앙 상단 (파랑)
        // 두 번째 삼각형
        0.1f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, // 좌측 하단 (노랑)
        0.9f, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // 우측 하단 (하늘색)
        0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f // 중앙 상단 (보라색)
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    glfwSetScrollCallback(window, scroll_callback); // 스크롤 콜백 등록

    while (!glfwWindowShouldClose(window)) // 렌더링 루프
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true); 

        // 마우스 드래그 처리
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);
            if (!isDragging) // 드래그 시작
            {
                isDragging = true;
            }
            else // 드래그 중
            {
                float deltaX = (float)(mouseX - lastMouseX);
                float deltaY = (float)(mouseY - lastMouseY);
                trianglePos.x += deltaX * (2.0f/800.0f); // 마우스 이동에 따라 위치 조절
                trianglePos.y -= deltaY * (2.0f/600.0f); // Y축은 반대로 이동
            }
            lastMouseX = mouseX;
            lastMouseY = mouseY;
        } else {
            isDragging = false; // 드래그 종료
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 화면 색 설정(Red, Green, Blue, Alpha)
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glBindVertexArray(VAO);

        // 첫 번째 삼각형 (왼쪽: 기존처럼 자동 회전) ---
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-0.5f, 0.0f, 0.0f));
        model1 = glm::rotate(model1, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // 두 번째 삼각형 (오른쪽: 키보드로 제어) ---
        glm::mat4 model2 = glm::mat4(1.0f);
        // 키보드로 변경된 trianglePos를 행렬에 적용
        model2 = glm::translate(model2, trianglePos);
        model2 = glm::rotate(model2, -(float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));
        model2 = glm::scale(model2, glm::vec3(triangleScale)); // 크기 조절 적용

        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model2));
        glDrawArrays(GL_TRIANGLES, 3, 3);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}