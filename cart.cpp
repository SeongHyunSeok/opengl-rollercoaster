#include "cart.h"
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <algorithm>
#include <cmath>

Cart::Cart()
	: position{}, height{0.5f}, length{0.5f}, width{0.5f},
	dir_vec{0.f, 0.f, 1.f}, up_vec{0.f, 1.f, 0.f}
{
}


Cart::~Cart()
{
}

void Cart::move(const std::vector<glm::vec3>& points)
{
	if (points.size() < 2)
		return;

	idx %= static_cast<int>(points.size());
	position = points[idx];
	const glm::vec3 delta = points[(idx + 1) % points.size()] - position;
	if (glm::dot(delta, delta) > 0.000001f) {
		const glm::vec3 next_direction = glm::normalize(delta);
		const glm::vec3 rotation_axis = glm::cross(dir_vec, next_direction);
		const float axis_length = glm::length(rotation_axis);
		const float direction_dot = glm::clamp(glm::dot(dir_vec, next_direction), -1.f, 1.f);

		if (axis_length > 0.00001f) {
			const float angle = std::atan2(axis_length, direction_dot);
			const glm::mat4 rotation = glm::rotate(glm::mat4(1.f), angle, rotation_axis / axis_length);
			up_vec = glm::mat3(rotation) * up_vec;
		}
		else if (direction_dot < 0.f) {
			glm::vec3 axis = glm::cross(dir_vec, up_vec);
			if (glm::dot(axis, axis) < 0.000001f)
				axis = glm::vec3(1.f, 0.f, 0.f);
			const glm::mat4 rotation = glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::normalize(axis));
			up_vec = glm::mat3(rotation) * up_vec;
		}

		dir_vec = next_direction;
		glm::vec3 right = glm::cross(dir_vec, up_vec);
		if (glm::dot(right, right) < 0.000001f) {
			const glm::vec3 fallback = std::abs(dir_vec.y) < 0.9f
				? glm::vec3(0.f, 1.f, 0.f)
				: glm::vec3(1.f, 0.f, 0.f);
			right = glm::cross(dir_vec, fallback);
		}
		right = glm::normalize(right);
		up_vec = glm::normalize(glm::cross(right, dir_vec));
	}
	idx = (idx + 1) % static_cast<int>(points.size());
}

void Cart::reset()
{
	idx = 0;
	position = glm::vec3(0.f);
	dir_vec = glm::vec3(0.f, 0.f, 1.f);
	up_vec = glm::vec3(0.f, 1.f, 0.f);
}

void Cart::initBuffer(GLuint shader_program)
{
	constexpr float n = 0.577350269f;
	GLfloat boxNormal[] = {
		-n, -n, -n,
		-n, -n,  n,
		-n,  n, -n,
		-n,  n,  n,
		 n, -n, -n,
		 n, -n,  n,
		 n,  n, -n,
		 n,  n,  n
	};

	GLfloat vBoxPosList[] = {
			-0.5f, -0.5f, -0.5f,
			-0.5f, -0.5f, 0.5f,
			-0.5f, 0.5f, -0.5f,
			-0.5f, 0.5f, 0.5f,
			0.5f, -0.5f, -0.5f,
			0.5f, -0.5f, 0.5f,
			0.5f, 0.5f, -0.5f,
			0.5f, 0.5f, 0.5f
	};

	unsigned int boxIndex[] =
	{
		0,6,4,
		0,2,6,
		0,3,2,
		0,1,3,
		2,7,6,
		2,3,7,
		4,6,7,
		4,7,5,
		0,4,5,
		0,5,1,
		1,5,7,
		1,7,3
	};


	glGenVertexArrays(1, &cart_buffer.vao); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(cart_buffer.vao); //--- VAO를 바인드하기
	glGenBuffers(2, cart_buffer.vbo);

	//--- 1번째 VBO를 활성화하여 바인드하고, 버텍스 속성 (좌표값)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, cart_buffer.vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vBoxPosList), vBoxPosList, GL_STATIC_DRAW);
	glGenBuffers(1, &cart_buffer.EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cart_buffer.EBO); // GL_ELEMENT_ARRAY_BUFFER 버퍼 유형으로 바인딩
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(boxIndex), boxIndex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	glEnableVertexAttribArray(0);

	//--- 2번째 VBO를 활성화 하여 바인드 하고, 버텍스 속성 (색상)을 저장
	glBindBuffer(GL_ARRAY_BUFFER, cart_buffer.vbo[1]);
	//--- 변수 colors에서 버텍스 색상을 복사한다.
	glBufferData(GL_ARRAY_BUFFER, sizeof(boxNormal), boxNormal, GL_STATIC_DRAW);
	//--- 색상값을 attribute 인덱스 1번에 명시한다: 버텍스 당 3*float
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
	//--- attribute 인덱스 1번을 사용 가능하게 함.
	glEnableVertexAttribArray(1);
}

void Cart::draw(GLuint shader_program)
{
	glm::mat4 transform(1.f);

	const glm::vec3 right = glm::normalize(glm::cross(up_vec, dir_vec));
	glm::mat4 orientation(1.f);
	orientation[0] = glm::vec4(right, 0.f);
	orientation[1] = glm::vec4(up_vec, 0.f);
	orientation[2] = glm::vec4(dir_vec, 0.f);
	transform = glm::translate(transform, position) * orientation;

	transform = glm::scale(transform, glm::vec3(width, height, length));


	GLint location = glGetUniformLocation(shader_program, "modelTransform");
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(transform));

	int objColorLocation = glGetUniformLocation(shader_program, "objectColor");
	glUniform3f(objColorLocation, 0.f, 0.f, 1.f);


	glBindVertexArray(cart_buffer.vao);
	glDrawElements(GL_TRIANGLES, 3 * 12, GL_UNSIGNED_INT, 0);;
}

glm::vec3 Cart::get_position() const
{
	return position;
}


glm::vec3 Cart::get_dir_vec() const
{
	return dir_vec;
}

glm::vec3 Cart::get_up_vec() const
{
	return up_vec;
}

