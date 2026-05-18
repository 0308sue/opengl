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

    // --- (추가) 삼각형의 위치를 저장할 변수 (루프 밖) ---
    glm::vec3 trianglePos = glm::vec3(0.5f, 0.0f, 0.0f); // 초기 위치
    float moveSpeed = 0.005f; // 이동 속도


    while (!glfwWindowShouldClose(window)) // 렌더링 루프
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true); 

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 화면 색 설정(Red, Green, Blue, Alpha)
        // 상하좌우 방향키 입력 처리x
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        trianglePos.y += moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        trianglePos.y -= moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        trianglePos.x -= moveSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        trianglePos.x += moveSpeed;

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