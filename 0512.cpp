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
struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
};

bool loadOBJ(const char *path, std::vector<float> &out_data)
{
	std::vector<glm::vec3> temp_positions;
	std::vector<glm::vec3> temp_normals;

	std::ifstream file(path);
	if (!file.is_open())
		return false;

	std::string line;
	while (std::getline(file, line))
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
		{ // 면 정보 (v/vt/vn 형태 대응)
			std::string vertexStr;
			for (int i = 0; i < 3; i++)
			{
				ss >> vertexStr;

				size_t firstSlash = vertexStr.find('/');
				size_t lastSlash = vertexStr.find_last_of('/');

				// 인덱스 문자열 추출
				int vIdx = std::stoi(vertexStr.substr(0, firstSlash));
				int nIdx = std::stoi(vertexStr.substr(lastSlash + 1));

				// 음수 인덱스 처리 로직 추가
				// 양수면 vIDx -1(0-base), 음수면 temp_positions.size() + vIdx
				int finalVIdx = (vIdx > 0) ? vIdx - 1 : (static_cast<int>(temp_positions.size()) + vIdx);
				int finalNIdx = (nIdx > 0) ? nIdx - 1 : (static_cast<int>(temp_normals.size()) + nIdx);

				if (finalVIdx >= 0 && finalVIdx < temp_positions.size())
				{
					glm::vec3 pos = temp_positions[finalVIdx];
					out_data.push_back(pos.x);
					out_data.push_back(pos.y);
					out_data.push_back(pos.z);
				}
				else
				{
					std::cerr << "Vertex Index Out of Range: " << vIdx << std::endl;
					return false;
				}
				if (finalNIdx >= 0 && finalNIdx < temp_normals.size())
				{
					glm::vec3 norm = temp_normals[finalNIdx];
					out_data.push_back(norm.x);
					out_data.push_back(norm.y);
					out_data.push_back(norm.z);
				}
				else
				{
					std::cerr << "Normal Index Out of Range: " << nIdx << std::endl;
					return false;
				}
			}
		}
	}
	return true;
}

const char *vertexShaderSource = "#version 410 core\n"
								 "layout (location = 0) in vec3 aPos;\n"
								 "layout (location = 1) in vec3 aNormal;\n"
								 "out vec3 Normal;\n"
								 "out vec3 FragPos;\n"
								 "uniform mat3 normalMatrix;\n"
								 "uniform mat4 model; uniform mat4 view; uniform mat4 projection;\n"
								 "void main() { FragPos = vec3(model * vec4(aPos, 1.0));\n"
								 "Normal = normalMatrix * aNormal;\n"
								 "gl_Position = projection * view * model * vec4(aPos, 1.0);}\0";

const char *fragmentShaderSource = "#version 410 core\n"
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

// ========================
// 📌 화면 크기
// ========================
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ========================
// 📌 카메라 관련 변수
// ========================
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);	  // 카메라 위치
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // 바라보는 방향
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);	  // 위 방향

float objectScale = 1.0f;

struct Model
{
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	int vertexCount = 0;
	glm::vec3 position;
	glm::vec3 color;
	float scaleFactor;
};

Model bunny, dragon;

void setupModel(Model &model, const char *path, glm::vec3 pos, glm::vec3 col, float scale)
{
	std::vector<float> data;
	if (!loadOBJ(path, data))
	{
		std::cout << "모델 파일을 찾을 수 없습니다!" << std::endl;
		return;
	}

	model.vertexCount = (int)(data.size() / 6); // 6 floats per vertex (position + normal)
	model.position = pos;
	model.color = col;
	model.scaleFactor = scale;

	glGenVertexArrays(1, &model.VAO);
	glGenBuffers(1, &model.VBO);

	// VAO 바인딩(이후 설정은 이 VAO에 저장됨)
	glBindVertexArray(model.VAO);

	// VBO 바인딩 및 데이터 전송
	glBindBuffer(GL_ARRAY_BUFFER, model.VBO);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);

	// 정점 속성 설정(Vertex Attribute)
	//  위치(Location 0):3개의 float, 간격 6*floats, 시작점 offset은 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	// 법선(Location 1):3개의 float, 간격 6*floats, 시작점 offset은 3*sizeof(float)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	// 바인딩 해제(실수 방지)
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

int main()
{
	glfwInit();
	glfwInit();

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	GLFWwindow *window = glfwCreateWindow(800, 600, "Stanford Bunny", NULL, NULL);
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

	setupModel(bunny, "bunny.obj", glm::vec3(-1.5f, 0.0f, 0.0f), glm::vec3(0.8f, 0.5f, 0.2f), 1.0f);
	setupModel(dragon, "dragon.obj", glm::vec3(1.5f, 0.0f, 0.0f), glm::vec3(0.1f, 0.6f, 0.3f), 1.0f);

	// 렌더링 루프
	while (!glfwWindowShouldClose(window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);

		// 광원 설정
		glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 1.2f, 1.0f, 2.0f);
		glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.8f, 0.5f, 0.2f);

		// 공통 Matrix (View, Projection) 전송
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		// 모델 렌더링
		Model *modelList[] = {&bunny, &dragon};
		for (Model *m : modelList)
		{
			if (m->VAO == 0)
				continue;
			// Model 행렬 계산
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, m->position);
			model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::scale(model, glm::vec3(objectScale * m->scaleFactor));
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
			// 색상 및 normal 행렬 전송
			glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(m->color));
			glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
			glUniformMatrix3fv(glGetUniformLocation(shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
			// 해당 모델의 VAO를 바인딩하고 그리기
			glBindVertexArray(m->VAO);
			glDrawArrays(GL_TRIANGLES, 0, m->vertexCount);
		}
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	// 리소스 해제
	glDeleteVertexArrays(1, &bunny.VAO);
	glDeleteBuffers(1, &bunny.VBO);
	glDeleteVertexArrays(1, &dragon.VAO);
	glDeleteBuffers(1, &dragon.VBO);

	// GLFW 종료
	glfwTerminate();
	return 0;
}