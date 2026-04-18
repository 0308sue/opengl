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

    unsigned int transformLoc = glGetUniformLocation(shaderProgram, "transform");

    while (!glfwWindowShouldClose(window)) // 렌더링 루프
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true); 

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);


        // 첫번째 삼각형 (왼쪽)
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-0.5f, 0.0f, 0.0f)); // 왼쪽으로 이동
        model1 = glm::rotate(model1, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // 회전 
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(model1));
        glDrawArrays(GL_TRIANGLES, 0, 3);   

        // 두번째 삼각형 (오른쪽)
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(0.5f, 0.0f, 0.0f)); // 오른쪽으로 이동
        model2 = glm::rotate(model2, -(float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f)); // 회전
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