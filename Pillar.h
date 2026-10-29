#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Pillar {
private:
	glm::vec3 position;
	float radius;
	float height;
	bool select = false;
public:
	Pillar();

	Pillar(const glm::vec3, const float, const float);

	~Pillar();

	Pillar(const Pillar& other);

	Pillar& operator=(const Pillar& other);

	//Pillar(Pillar&& other) noexcept;

	//noexept : c++ throw를 하지 않는다.라는 의미
	//&& : R value를 의미

	//Pillar& operator=(Pillar&& other) noexcept;

	glm::vec3 get_position() const;

	float get_radius() const;
	
	float get_height() const;

	void selected();

	void unselected();

	bool is_selected() const;

	void change_height_by_a(float);

	void draw(GLuint, glm::vec3);

	void draw_edge(GLuint);

	void draw_line(GLuint);

	bool is_conflict(glm::vec3, float& length, bool side_view) const;
};
