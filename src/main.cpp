#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#include "texture.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// Чтение шейдера из файла
std::string readShaderFile(const std::string& filepath) {
	std::ifstream file(filepath);
	if (!file.is_open()) {
		std::cerr << "Failed to open shader file: " << filepath << std::endl;
		return "";
	}
	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

GLuint compileShader(GLenum type, const std::string& source) {
	GLuint shader = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
		return 0;
	}
	return shader;
}

GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
	std::string vertexSource = readShaderFile(vertexPath);
	std::string fragmentSource = readShaderFile(fragmentPath);
	if (vertexSource.empty() || fragmentSource.empty())
		return 0;
	GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
	if (!vertexShader || !fragmentShader)
		return 0;
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);
	int success;
	char infoLog[512];
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		return 0;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return program;
}

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent; // будет вычислено
};

// Вычисление касательных для набора вершин и индексов
void computeTangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
	std::vector<glm::vec3> tangents(vertices.size(), glm::vec3(0.0f));
	for (size_t i = 0; i < indices.size(); i += 3) {
		unsigned int i1 = indices[i];
		unsigned int i2 = indices[i + 1];
		unsigned int i3 = indices[i + 2];
		Vertex& v1 = vertices[i1];
		Vertex& v2 = vertices[i2];
		Vertex& v3 = vertices[i3];

		glm::vec3 edge1 = v2.position - v1.position;
		glm::vec3 edge2 = v3.position - v1.position;
		glm::vec2 deltaUV1 = v2.texCoord - v1.texCoord;
		glm::vec2 deltaUV2 = v3.texCoord - v1.texCoord;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		glm::vec3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangents[i1] += tangent;
		tangents[i2] += tangent;
		tangents[i3] += tangent;
	}
	for (size_t i = 0; i < vertices.size(); ++i) {
		vertices[i].tangent = glm::normalize(tangents[i]);
	}
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Normal Mapping", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		});
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glEnable(GL_DEPTH_TEST);

	// Исходные данные без касательных
	std::vector<Vertex> vertices = {
		// Передняя грань
		{{-0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f,  0.5f}, { 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		// Задняя грань
		{{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f,-1.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f,-1.0f}, {0.0f, 1.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f,-1.0f}, {1.0f, 0.0f}},
		// Левая грань
		{{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		// Правая грань
		{{ 0.5f, -0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		// Нижняя грань
		{{-0.5f, -0.5f, -0.5f}, { 0.0f,-1.0f, 0.0f}, {0.0f, 0.0f}},
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f,-1.0f, 0.0f}, {1.0f, 1.0f}},
		{{-0.5f, -0.5f,  0.5f}, { 0.0f,-1.0f, 0.0f}, {0.0f, 1.0f}},
		// Верхняя грань
		{{-0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
		{{-0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
	};

	std::vector<unsigned int> indices = {
		0,1,2, 0,2,3,   4,5,6, 4,6,7,
		8,9,10, 8,10,11, 12,13,14, 12,14,15,
		16,17,18, 16,18,19, 20,21,22, 20,22,23
	};

	// Вычисляем касательные
	computeTangents(vertices, indices);

	// Создание VAO, VBO
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	// Атрибуты
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
	glEnableVertexAttribArray(3);

	// Загрузка текстур
	Texture diffuseMap;
	if (!diffuseMap.loadFromFile("textures/container.jpg")) {
		std::cerr << "Failed to load diffuse map" << std::endl;
		return -1;
	}
	Texture normalMap;
	if (!normalMap.loadFromFile("textures/container_normal.png")) {
		std::cerr << "Failed to load normal map. Continuing without normal mapping." << std::endl;
		// Можно продолжить, но нормали будут из геометрии
	}

	GLuint shaderProgram = createShaderProgram("shaders/vertex.glsl", "shaders/fragment.glsl");
	if (!shaderProgram) return -1;
	glUseProgram(shaderProgram);

	// Uniforms
	GLint modelLoc = glGetUniformLocation(shaderProgram, "uModel");
	GLint viewLoc = glGetUniformLocation(shaderProgram, "uView");
	GLint projLoc = glGetUniformLocation(shaderProgram, "uProjection");
	GLint lightPosLoc = glGetUniformLocation(shaderProgram, "uLightPos");
	GLint viewPosLoc = glGetUniformLocation(shaderProgram, "uViewPos");
	GLint lightColorLoc = glGetUniformLocation(shaderProgram, "uLightColor");
	GLint matAmbientLoc = glGetUniformLocation(shaderProgram, "uMaterialAmbient");
	GLint matDiffuseLoc = glGetUniformLocation(shaderProgram, "uMaterialDiffuse");
	GLint matSpecularLoc = glGetUniformLocation(shaderProgram, "uMaterialSpecular");
	GLint matShininessLoc = glGetUniformLocation(shaderProgram, "uMaterialShininess");
	GLint diffuseMapLoc = glGetUniformLocation(shaderProgram, "uDiffuseMap");
	GLint normalMapLoc = glGetUniformLocation(shaderProgram, "uNormalMap");

	glUniform1i(diffuseMapLoc, 0);
	glUniform1i(normalMapLoc, 1);

	glUniform3f(matAmbientLoc, 0.2f, 0.2f, 0.2f);
	glUniform3f(matDiffuseLoc, 1.0f, 1.0f, 1.0f);
	glUniform3f(matSpecularLoc, 0.8f, 0.8f, 0.8f);

	glUniform1f(matShininessLoc, 64.0f);
	glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

	glm::vec3 lightPos(2.0f, 2.0f, 2.0f);
	glm::vec3 cameraPos(2.0f, 2.0f, 2.0f);
	glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
	glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));
	glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

	float angle = 0.0f;
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		angle += 0.001f;
		glm::mat4 model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// Активация текстур
		diffuseMap.bind(0);
		normalMap.bind(1);

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glfwTerminate();
	return 0;
}