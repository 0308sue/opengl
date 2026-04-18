#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
// stanford_bunny.obj 파일이 실행 파일과 같은 폴더에 있다고 가정
// --- .obj 파일을 읽어 정점 데이터를 추출하는 함수---
bool loadOBJ(const char *path, std::vector<float> &out_vertices)
{
    std::vector<glm::vec3> temp_vertices;
    std::ifstream file(path);
    if (!file.is_open())
        return false;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue; // 빈 줄 건너뛰기
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;
        if (prefix == "v")
        {
            glm::vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (prefix == "f")
        {
            std::string vertexData;
            for (int i = 0; i < 3; i++)
            { // 삼각형이므로 3개의 정점 데이터 읽기
                ss >> vertexData;
                std::stringstream vss(vertexData);
                unsigned int vIdx;
                vss >> vIdx;

                // 범위 체크 (디버깅용 안전장치)
                if (vIdx <= 0 || vIdx > temp_vertices.size())
                {
                    std::cerr << "Error: Invalid index " << vIdx << std::endl;
                    return false;
                }

                // 데이터 복사
                glm::vec3 v = temp_vertices[vIdx - 1];
                out_vertices.push_back(v.x);
                out_vertices.push_back(v.y);
                out_vertices.push_back(v.z);
            }
        }
    }
    return true;
}
// 셰이더 소스는 이전과 동일하되 색상을 고정값으로 출력하도록 수정
const char *vertexShaderSource = "#version 410 core\n"
                                 "layout (location = 0) in vec3 aPos;\n"
                                 "uniform mat4 model; uniform mat4 view; uniform mat4 projection;\n"
                                 "void main() { gl_Position = projection * view * model * vec4(aPos, 1.0); }\0";
const char *fragmentShaderSource = "#version 410 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main() { FragColor = vec4(0.8, 0.5, 0.2, 1.0); }\n\0"; // bunny 색상

int main()
{
    glfwInit();

    // 🔥 이거 추가 (맥 필수)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Stanford Bunny", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST); // 3D이므로 깊이 테스트 필수
    // 셰이더 컴파일 (이전 코드와 동일하게 처리)
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // bunny 데이터 로드
    std::vector<float> bunnyVertices;
    if (!loadOBJ("bunny.obj", bunnyVertices))
    {
        std::cout << "모델 파일을 찾을 수 없습니다!" << std::endl;
        return -1;
    }

    // if (bunnyVertices.empty()) {
    // std::cout << "vertex data empty!" << std::endl;
    // return -1;
    // }   
    // std::cout << bunnyVertices.size() << std::endl;
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, bunnyVertices.size() * sizeof(float), &bunnyVertices[0], GL_STATIC_DRAW);
    // glBufferData(GL_ARRAY_BUFFER,
    // bunnyVertices.size() * sizeof(float),
    // bunnyVertices.data(),   // 👍 이걸로 변경
    // GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    while (!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);
        // --- MVP 행렬 설정---
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.7f)); // 크기 조절
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -3.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, bunnyVertices.size() / 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}