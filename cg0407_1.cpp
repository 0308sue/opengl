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

struct Vertex
{
    glm::vec3 Position; // 정점
    glm::vec3 Normal; // 법선(표면의 방향)
};

bool loadOBJ(const char *path, std::vector<float> &out_data)
{
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec3> temp_normals;
    std::ifstream file(path);
    if (!file.is_open())
        return false;
    std::string line;
    while (std::getline(file, line)) //obj는 v x y z, vn x y z, f v/vt/vn 형태로 저장되어 있음
    {
        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;
        if (prefix == "v")
        { // 위치
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            temp_positions.push_back(v);
        }
        else if (prefix == "vn")
        { // normal
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        }
        else if (prefix == "f")
        { // face 정보 (v/vt/vn 형태 대응)
            std::string vertexStr;
            for (int i = 0; i < 3; i++)
            {
                ss >> vertexStr;
                size_t firstSlash = vertexStr.find('/');
                size_t lastSlash = vertexStr.find_last_of('/');
                // 1. 위치 인덱스 추출
                int vIdx = std::stoi(vertexStr.substr(0, firstSlash));
                // 2. normal 인덱스 추출 (마지막 슬래시 뒤의 숫자)
                int nIdx = std::stoi(vertexStr.substr(lastSlash + 1));
                // 위치 데이터 넣기
                glm::vec3 pos = temp_positions[vIdx - 1];
                out_data.push_back(pos.x);
                out_data.push_back(pos.y);
                out_data.push_back(pos.z);
                // normal 데이터 넣기
                glm::vec3 norm = temp_normals[nIdx - 1];
                out_data.push_back(norm.x);
                out_data.push_back(norm.y);
                out_data.push_back(norm.z);
            }
        }
    }
    return true;
}

// 셰이더 소스는 이전과 동일하되 색상을 고정값으로 출력하도록 수정
const char *vertexShaderSource = "#version 410 core \n"
                                "layout (location = 0) in vec3 aPos;\n"
                                "layout (location = 1) in vec3 aNormal;\n"
                                "out vec3 Normal;\n"
                                "uniform mat4 model;\n"
                                "uniform mat4 view;\n"
                                "uniform mat4 projection;\n"
                                "void main() {\n"
                                "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                "    Normal = mat3(transpose(inverse(model))) * aNormal;\n"
                                "}";

const char *fragmentShaderSource = "#version 410 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main() { FragColor = vec4(0.8, 0.5, 0.2, 1.0); }\n\0"; // bunny 색상

int main() {
    glfwInit();

    // 🔥 이거 추가 (맥 필수)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //창 생성
    GLFWwindow *window = glfwCreateWindow(800, 600, "Stanford Bunny", NULL, NULL);
    glfwMakeContextCurrent(window);

    // GLAD 초기화
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

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, bunnyVertices.size() * sizeof(float), &bunnyVertices[0], GL_STATIC_DRAW);

    // (x, y, z, nx, ny, nz) 형태로 저장되어 있으므로 stride는 6 * float
    // 1. 위치 속성 (Location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 2. normal 속성 (Location 1) - 시작점 오프셋(3 * float 만큼 띄움)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    while (!glfwWindowShouldClose(window))
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); 

        // 화면 지우기
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        // --- MVP 행렬 설정---
        // srt는 모델 행렬에서 처리 (회전과 크기 조절)
        // 여기선 t 뷰에서 처리
        // 물체 이동 : 모델 행렬에서 처리
        // 카메라 이동 : 뷰 행렬에서 처리
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.7f)); // 크기 조절
        model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -3.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
       
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, bunnyVertices.size() / 6);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}