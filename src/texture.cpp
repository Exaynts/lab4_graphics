#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"
#include <iostream>

Texture::Texture() : m_id(0), m_width(0), m_height(0), m_channels(0) {}

Texture::~Texture() {
	if (m_id) {
		glDeleteTextures(1, &m_id);
	}
}

bool Texture::loadFromFile(const std::string& path) {
	// 1. Загружаем изображение с помощью stb_image
	unsigned char* data = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, 0);
	if (!data) {
		std::cerr << "Failed to load texture: " << path << std::endl;
		return false;
	}

	// 2. Генерируем объект текстуры в OpenGL
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	// 3. Настраиваем параметры текстуры
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// 4. Определяем формат пикселей на основе количества каналов
	GLenum format;
	if (m_channels == 1) format = GL_RED;
	else if (m_channels == 3) format = GL_RGB;
	else if (m_channels == 4) format = GL_RGBA;
	else format = GL_RGB;

	// 5. Загружаем данные в текстуру
	glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	// 6. Освобождаем память изображения
	stbi_image_free(data);
	return true;
}

void Texture::bind(unsigned int unit) const {
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}