#include <glad/glad.h>
#include <GLFW/glfw3.h>


int main()
{
    glfwInit(); // GLFW 초기화
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // OpenGL 버전 설정
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL", NULL, NULL); glfwMakeContextCurrent(window); // 윈도우 생성
    // 이 창에서 OpenGL 렌더링을 수행하도록 설정
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // GLAD 초기화
    while (!glfwWindowShouldClose(window)) // 렌더링 루프
    {
        glClearColor(0.2f, 0.3f, 0.7f, 1.0f); // 화면 색 설정(Red, Green, Blue, Alpha)
        glClear(GL_COLOR_BUFFER_BIT); // 화면 초기화
        glfwSwapBuffers(window); // 화면 출력
        glfwPollEvents(); // 이벤트 처리
    }
    glfwTerminate();

}