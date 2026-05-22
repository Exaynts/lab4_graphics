#pragma once
#include <glad/glad.h>
#include <string>

class Texture {
public:
	Texture();
	~Texture();
	bool loadFromFile(const std::string& path);
	void bind(unsigned int unit = 0) const;
	void unbind() const;
	unsigned int getId() const { return m_id; }
	int getWidth() const { return m_width; }
	int getHeight() const { return m_height; }

private:
	unsigned int m_id;
	int m_width, m_height, m_channels;
};