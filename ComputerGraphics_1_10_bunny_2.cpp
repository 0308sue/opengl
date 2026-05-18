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


// bunny.obj 파일이 실행 파일과 같은 폴더에 있다고 가정
// --- .obj 파일을 읽어 정점 데이터를 추출하는 함수 ---
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
};

bool loadOBJ(const char* path, std::vector<float>& out_data) {
	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec3> temp_normals;

	std::ifstream file(path);
	if (!file.is_open()) return false;

	std::string line;
	while (std::getline(file, line)) {
		std::stringstream ss(line);
		std::string prefix;
		ss >> prefix;

		if (prefix == "v") { // 위치
			glm::vec3 v;
			ss >> v.x >> v.y >> v.z;
			temp_positions.push_back(v);
		}

		else if (prefix == "vn") { // normal
			glm::vec3 n;
			ss >> n.x >> n.y >> n.z;
			temp_normals.push_back(n);
		}
		else if (prefix == "f") { // 면 정보 (v/vt/vn 형태 대응)
			std::string vertexStr;
			for (int i = 0; i < 3; i++) {
				ss >> vertexStr;

				size_t firstSlash = vertexStr.find('/');
				size_t lastSlash = vertexStr.find_last_of('/');

				// 1. 위치 인덱스 추출
				int vIdx = std::stoi(vertexStr.substr(0, firstSlash));

				// 2. 법선 인덱스 추출 
				int nIdx = std::stoi(vertexStr.substr(lastSlash + 1));

				// 위치 데이터 넣기
				glm::vec3 pos = temp_positions[vIdx - 1];
				out_data.push_back(pos.x);
				out_data.push_back(pos.y);
				out_data.push_back(pos.z);

				// 법선 데이터 넣기
				glm::vec3 norm = temp_normals[nIdx - 1];
				out_data.push_back(norm.x);
				out_data.push_back(norm.y);
				out_data.push_back(norm.z);
			}
		}
	}
	return true;
}

const char* vertexShaderSource = "#version 410 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aNormal;\n"
"out vec3 Normal;\n"
"out vec3 FragPos;\n"
"uniform mat3 normalMatrix;\n"
"uniform mat4 model; uniform mat4 view; uniform mat4 projection;\n"
"void main() { FragPos = vec3(model * vec4(aPos, 1.0));\n"
"Normal = normalMatrix * aNormal;\n" 
"gl_Position = projection * view * model * vec4(aPos, 1.0);}\0";

const char* fragmentShaderSource = "#version 410 core\n"
"out vec4 FragColor;\n"
"in vec3 Normal;\n"
"in vec3 FragPos;\n"
"uniform vec3 lightPos;\n"
"uniform vec3 lightColor;\n"
"uniform vec3 objectColor;\n"
"void main() { float ambientStrength = 0.1;\n"
"vec3 ambient = ambientStrength * lightColor;\n"
"vec3 norm = normalize(Normal);"
"vec3 lightDir = normalize(lightPos - FragPos);\n"
"float diff = max(dot(norm, lightDir), 0.0);\n"
"vec3 diffuse = diff * lightColor;\n"
"vec3 result = (ambient + diffuse) * objectColor;"
"FragColor = vec4(result, 1.0); }\0"; // bunny 색상            

int main()
{
	glfwInit();

	    // 🔥 이거 추가 (맥 필수)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow(800, 600, "Stanford Bunny", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glEnable(GL_DEPTH_TEST); // 3D이므로 깊이 테스트 필수

	// 셰이더 컴파일 
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
	std::vector<float> bunnyData;
	if (!loadOBJ("bunny.obj", bunnyData)) {
		std::cout << "모델 파일을 찾을 수 없습니다!" << std::endl;
		return -1;
	}

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, bunnyData.size() * sizeof(float), &bunnyData[0], GL_STATIC_DRAW);
	// 1. 위치 속성 (Location 0)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// 2. normal 속성 (Location 1) 
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	// 렌더링 루프
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);

		// 광원 설정 
		glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 1.2f, 1.0f, 2.0f);
		glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.5f, 0.2f);


		// --- MVP 행렬 설정 ---
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));

		// normal 행렬 계산 (CPU에서 한 번만 수행)
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));

		unsigned int normalMatLoc = glGetUniformLocation(shaderProgram, "normalMatrix");
		glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

		glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -3.0f));
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, bunnyData.size() / 6);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 리소스 해제
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);

	// GLFW 종료
	glfwTerminate();
	return 0;
}