#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include "buffer.h"

class Cart {
private:
	glm::vec3 position;
	float width, height;
	float length;

	BUFFER cart_buffer;
	int idx = 0;

	glm::vec3 dir_vec;
	glm::vec3 up_vec;
public:
	Cart();

	~Cart();


	void move(const std::vector<glm::vec3>& points);

	void reset();

	void initBuffer(GLuint);

	void draw(GLuint shader_program);

	glm::vec3 get_position() const;



	glm::vec3 get_dir_vec() const;

	glm::vec3 get_up_vec() const;
};
