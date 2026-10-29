// 관찰자와의 거리를 비교해서 더 가까운 기둥 선택 (모양이 이상한 경우 

#include <iostream>
#include <vector>
#include <cmath>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include "shader.h"
#include "Pillar.h"
#include "cart.h"

using namespace std;

GLvoid DrawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid SpecialKeyboard(int key, int x, int y);
GLvoid MouseFunc(int button, int state, int x, int y);
GLvoid MotionFunc(int x, int y);
GLvoid TimerFunction(int value);

void InitBuffer();
void InitGroundBuffer();
void UpdateTrack();
void DrawTrack();
void DrawGround();
vector<GLfloat> BuildTrackMesh();

int screen_w = 1280;
int screen_h = 720;

GLuint shader_program;

vector <Pillar> pillars;

vector <glm::vec3> points;

GLuint track_vao = 0;
GLuint track_vbo = 0;
GLsizei track_vertex_count = 0;
bool track_dirty = true;

GLuint ground_vao = 0;
GLuint ground_vbo = 0;
GLsizei ground_vertex_count = 0;

constexpr float track_half_width = 0.18f;
constexpr float track_half_height = 0.09f;
constexpr int track_sides = 12;

Cart my_cart;

bool cart_move = false;

glm::vec3 pillar_color = { 0.45f, 0.45f, 0.45f };
glm::vec3 pillar_edge_color = { 0.f, 0.f, 0.f };

int pillars_num = 0;

int divide_num = 100;

float size_a = 0.1f;

bool ortho_view = false;
bool create_pillar = false;

bool change_height_1 = false;
bool change_height_2 = false;

float camera_height = 5.f;
constexpr float camera_target_height = 2.f;
constexpr float ground_height = 0.2f;

bool cart_view = false;

float aspect = (float)screen_w / (float)screen_h;

void convertOpenGlXY(int x, int y, float* ox, float* oy)
{
	*ox = (float)((x - (float)screen_w / 2.0) / (float)(screen_w / 2.0));
	*oy = (float)((y - (float)screen_h / 2.0) / (float)(screen_h / 2.0));
}

void unselected()
{
	for (int i = 0; i < pillars_num; ++i) {
		pillars[i].unselected();
	}
}

void is_selected(glm::vec3 mouse_pos, bool side_view)
{
	
	if (pillars_num == 0)
		return;
	cout << "함수 실행" << endl;
	
	int temp_idx{-1};
	float min_length{};
	
	for (int i = 0; i < pillars_num; ++i) {
		float length{};
		if (pillars[i].is_conflict(mouse_pos, length, side_view)) {
			//cout << i << "selected" << endl;
			//cout << length << endl;
			if (temp_idx < 0 || length < min_length) {
				min_length = length;
				temp_idx = i;
			}
		}
	}
	if (temp_idx >= 0) {
		unselected();
		cout << temp_idx << endl;
		pillars[temp_idx].selected();
	}
	else {
		unselected();
	}
}


void is_selected_at_front(float x, float y)
{
	int temp_idx{};
	int cnt{};
	cout << "함수 실행" << endl;
	for (int i = 0; i < pillars_num; ++i) {
		glm::vec3 position(pillars[i].get_position());
		float rad = pillars[i].get_radius() ;
		float hei = pillars[i].get_height();
		if (position.x - rad <= x && x <= position.x + rad && position.y  <= y && y <= position.y + hei * 2) {
			unselected();
			
			cout << i << "selected" << endl;
			if (cnt == 0) {
				temp_idx = i;
				pillars[i].selected();
			}
			if (cnt > 0 && pillars[temp_idx].get_position().z < position.z) {
				cout << pillars[temp_idx].get_position().z << " " << position.z << endl;
				pillars[i].selected();
				temp_idx = i;
			}
			else if(cnt > 0) {
				pillars[temp_idx].selected();
			}
			++cnt;
		}
	}
}


void is_selected_at_side(float x, float y)
{
	int temp_idx{};
	int cnt{};
	cout << "함수 실행" << endl;
	for (int i = 0; i < pillars_num; ++i) {
		glm::vec3 position(pillars[i].get_position());
		float rad = pillars[i].get_radius();
		float hei = pillars[i].get_height();
		if (position.z - rad <= x && x <= position.z + rad && position.y  <= y && y <= position.y + hei * 2) {
			unselected();

			cout << i << "selected" << endl;
			if (cnt == 0) {
				temp_idx = i;
				pillars[i].selected();
			}
			if (cnt > 0 && pillars[temp_idx].get_position().x < position.x) {
				cout << pillars[temp_idx].get_position().x << " " << position.x << endl;
				pillars[i].selected();
				temp_idx = i;
			}
			else if (cnt > 0) {
				pillars[temp_idx].selected();
			}
			++cnt;
		}
	}
}

void spline_line(vector<glm::vec3>& track_points, const glm::vec3& p0,
	const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3)
{
	for (int i = 0; i < divide_num; ++i) {
		const float t = static_cast<float>(i) / divide_num;
		const float t2 = t * t;
		const float t3 = t2 * t;
		track_points.emplace_back(0.5f * ((2.f * p1)
			+ (-p0 + p2) * t
			+ (2.f * p0 - 5.f * p1 + 4.f * p2 - p3) * t2
			+ (-p0 + 3.f * p1 - 3.f * p2 + p3) * t3));
	}
}

int main(int argc, char** argv) //윈도우 출력하고 콜백함수 설정
{
	pillars.reserve(100);
	points.reserve(10000);

	glutInit(&argc, argv);// glut 초기화
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); // 디스플레이 모드 설정
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitWindowPosition(100, 100); // 윈도우의 위치 지정
	glutInitWindowSize(screen_w, screen_h); // 윈도우의 크기 지정
	glutCreateWindow("Project"); // 윈도우 생성 윈도우 이름

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)// glew 초기화
	{
		std::cout << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized \n";

	glEnable(GL_DEPTH_TEST);

	if (!InitShader(shader_program)) {
		std::cerr << "셰이더를 초기화하지 못했습니다." << std::endl;
		return EXIT_FAILURE;
	}
	InitBuffer();

	glutDisplayFunc(DrawScene); // 출력 함수의 지정
	glutReshapeFunc(Reshape);// 다시 그리기 함수 지정
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(SpecialKeyboard);
	glutMouseFunc(MouseFunc);
	glutMotionFunc(MotionFunc);
	glutTimerFunc(30, TimerFunction, 1);



	glutMainLoop();// 이벤트 처리 시작
}


void InitBuffer()
{
	my_cart.initBuffer(shader_program);
	InitGroundBuffer();
	glGenVertexArrays(1, &track_vao);
	glGenBuffers(1, &track_vbo);
	glBindVertexArray(track_vao);
	glBindBuffer(GL_ARRAY_BUFFER, track_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void InitGroundBuffer()
{
	vector<GLfloat> vertices;
	vertices.reserve(36 * 6);
	auto append_vertex = [&vertices](const glm::vec3& position, const glm::vec3& normal) {
		vertices.insert(vertices.end(), {
			position.x, position.y, position.z,
			normal.x, normal.y, normal.z
		});
	};
	auto append_face = [&append_vertex](const glm::vec3& p0, const glm::vec3& p1,
		const glm::vec3& p2, const glm::vec3& p3, const glm::vec3& normal) {
		append_vertex(p0, normal); append_vertex(p1, normal); append_vertex(p2, normal);
		append_vertex(p0, normal); append_vertex(p2, normal); append_vertex(p3, normal);
	};

	append_face({-.5f,-.5f, .5f}, { .5f,-.5f, .5f}, { .5f, .5f, .5f}, {-.5f, .5f, .5f}, { 0.f, 0.f, 1.f});
	append_face({ .5f,-.5f,-.5f}, {-.5f,-.5f,-.5f}, {-.5f, .5f,-.5f}, { .5f, .5f,-.5f}, { 0.f, 0.f,-1.f});
	append_face({-.5f,-.5f,-.5f}, {-.5f,-.5f, .5f}, {-.5f, .5f, .5f}, {-.5f, .5f,-.5f}, {-1.f, 0.f, 0.f});
	append_face({ .5f,-.5f, .5f}, { .5f,-.5f,-.5f}, { .5f, .5f,-.5f}, { .5f, .5f, .5f}, { 1.f, 0.f, 0.f});
	append_face({-.5f, .5f, .5f}, { .5f, .5f, .5f}, { .5f, .5f,-.5f}, {-.5f, .5f,-.5f}, { 0.f, 1.f, 0.f});
	append_face({-.5f,-.5f,-.5f}, { .5f,-.5f,-.5f}, { .5f,-.5f, .5f}, {-.5f,-.5f, .5f}, { 0.f,-1.f, 0.f});

	ground_vertex_count = static_cast<GLsizei>(vertices.size() / 6);
	glGenVertexArrays(1, &ground_vao);
	glGenBuffers(1, &ground_vbo);
	glBindVertexArray(ground_vao);
	glBindBuffer(GL_ARRAY_BUFFER, ground_vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), nullptr);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);
}

void DrawGround()
{
	const float ground_width = 2.f * camera_height * aspect + 0.5f;
	const float ground_depth = 2.f * camera_height + 0.5f;
	glm::mat4 transform(1.f);
	transform = glm::translate(transform, glm::vec3(0.f, -ground_height * 0.5f, 0.f));
	transform = glm::scale(transform, glm::vec3(ground_width, ground_height, ground_depth));

	const GLint model_location = glGetUniformLocation(shader_program, "modelTransform");
	glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(transform));
	const GLint color_location = glGetUniformLocation(shader_program, "objectColor");
	glUniform3f(color_location, 0.12f, 0.55f, 0.18f);

	glBindVertexArray(ground_vao);
	glDrawArrays(GL_TRIANGLES, 0, ground_vertex_count);
	glBindVertexArray(0);
}

vector<GLfloat> BuildTrackMesh()
{
	vector<GLfloat> vertices;
	if (points.size() < 3)
		return vertices;

	const size_t ring_count = points.size();
	vector<glm::vec3> ring_positions(ring_count * track_sides);
	vector<glm::vec3> ring_normals(ring_count * track_sides);
	glm::vec3 frame_up(0.f, 1.f, 0.f);

	for (size_t i = 0; i < ring_count; ++i) {
		const glm::vec3 tangent_delta = points[(i + 1) % ring_count]
			- points[(i + ring_count - 1) % ring_count];
		const glm::vec3 tangent = glm::dot(tangent_delta, tangent_delta) > 0.000001f
			? glm::normalize(tangent_delta)
			: glm::vec3(0.f, 0.f, 1.f);

		frame_up -= tangent * glm::dot(frame_up, tangent);
		if (glm::dot(frame_up, frame_up) < 0.000001f) {
			const glm::vec3 fallback = std::abs(tangent.y) < 0.9f
				? glm::vec3(0.f, 1.f, 0.f)
				: glm::vec3(1.f, 0.f, 0.f);
			frame_up = fallback - tangent * glm::dot(fallback, tangent);
		}
		frame_up = glm::normalize(frame_up);
		const glm::vec3 side = glm::normalize(glm::cross(tangent, frame_up));
		frame_up = glm::normalize(glm::cross(side, tangent));

		for (int side_index = 0; side_index < track_sides; ++side_index) {
			const float angle = glm::two_pi<float>() * side_index / track_sides;
			const float side_amount = std::cos(angle);
			const float up_amount = std::sin(angle);
			const glm::vec3 normal = glm::normalize(
				(side_amount / track_half_width) * side
				+ (up_amount / track_half_height) * frame_up);
			const size_t ring_index = i * track_sides + side_index;
			ring_normals[ring_index] = normal;
			ring_positions[ring_index] = points[i]
				+ track_half_width * side_amount * side
				+ track_half_height * up_amount * frame_up;
		}
	}

	vertices.reserve(ring_count * track_sides * 6 * 6);
	auto append = [&vertices, &ring_positions, &ring_normals](size_t index) {
		const glm::vec3& position = ring_positions[index];
		const glm::vec3& normal = ring_normals[index];
		vertices.insert(vertices.end(), {
			position.x, position.y, position.z,
			normal.x, normal.y, normal.z
		});
	};

	for (size_t i = 0; i < ring_count; ++i) {
		const size_t next_ring = (i + 1) % ring_count;
		for (int side_index = 0; side_index < track_sides; ++side_index) {
			const int next_side = (side_index + 1) % track_sides;
			const size_t a = i * track_sides + side_index;
			const size_t b = i * track_sides + next_side;
			const size_t c = next_ring * track_sides + next_side;
			const size_t d = next_ring * track_sides + side_index;
			append(a); append(d); append(c);
			append(a); append(c); append(b);
		}
	}

	return vertices;
}

void UpdateTrack()
{
	points.clear();
	my_cart.reset();

	if (pillars_num >= 3) {
		for (int i = 0; i < pillars_num; ++i) {
			spline_line(points,
				pillars[(i - 1 + pillars_num) % pillars_num].get_position(),
				pillars[i].get_position(),
				pillars[(i + 1) % pillars_num].get_position(),
				pillars[(i + 2) % pillars_num].get_position());
		}
	}

	const vector<GLfloat> track_mesh = BuildTrackMesh();
	track_vertex_count = static_cast<GLsizei>(track_mesh.size() / 6);
	glBindBuffer(GL_ARRAY_BUFFER, track_vbo);
	glBufferData(GL_ARRAY_BUFFER, track_mesh.size() * sizeof(GLfloat),
		track_mesh.empty() ? nullptr : track_mesh.data(), GL_DYNAMIC_DRAW);
	track_dirty = false;
}

void DrawTrack()
{
	if (track_vertex_count == 0)
		return;

	const glm::mat4 identity(1.f);
	const GLint model_location = glGetUniformLocation(shader_program, "modelTransform");
	glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(identity));
	const GLint color_location = glGetUniformLocation(shader_program, "objectColor");
	glUniform3f(color_location, 0.12f, 0.12f, 0.12f);

	glBindVertexArray(track_vao);
	glDrawArrays(GL_TRIANGLES, 0, track_vertex_count);
	glBindVertexArray(0);
}



void DrawScene() {
	//--- 변경된 배경색 설정
	glClearColor(0.5f, 0.8f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, screen_w, screen_h);

	GLint viewTransform = glGetUniformLocation(shader_program, "viewTransform");
	glm::mat4 viewMat = glm::lookAt(
		glm::vec3(0.f, camera_target_height + 5.f, 10.f),
		glm::vec3(0.f, camera_target_height, 0.f),
		glm::vec3(0.f, 1.f, 0.f));
	//평면도
	if (create_pillar)
		viewMat = glm::lookAt(glm::vec3(0.f, camera_height, 0.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, -1.f));
	else if (change_height_1)
		viewMat = glm::lookAt(glm::vec3(0.f, camera_target_height, camera_height),
			glm::vec3(0.f, camera_target_height, 0.f), glm::vec3(0.f, 1.f, 0.f));
	else if (change_height_2)
		viewMat = glm::lookAt(glm::vec3(camera_height, camera_target_height, 0.f),
			glm::vec3(0.f, camera_target_height, 0.f), glm::vec3(0.f, 1.f, 0.f));
	else if (cart_view && cart_move) {
		const glm::vec3 dir_vec = my_cart.get_dir_vec();
		const glm::vec3 cart_up = my_cart.get_up_vec();
		const glm::vec3 world_up(0.f, 1.f, 0.f);
		const float verticality = std::abs(glm::dot(dir_vec, world_up));
		glm::vec3 projected_up = world_up - dir_vec * glm::dot(world_up, dir_vec);
		glm::vec3 camera_up = cart_up;
		if (glm::dot(projected_up, projected_up) > 0.000001f) {
			projected_up = glm::normalize(projected_up);
			// 같은 반구를 유지해 보정 과정에서 180도 뒤집히지 않게 한다.
			if (glm::dot(projected_up, cart_up) < 0.f)
				projected_up = -projected_up;
			// 평탄한 구간은 수평선을 유지하고, 수직 구간은 카트 자세를 따른다.
			const float horizon_blend = 1.f - glm::smoothstep(0.9f, 0.995f, verticality);
			camera_up = glm::normalize(glm::mix(cart_up, projected_up, horizon_blend));
		}
		// 카트 내부의 운전자 눈높이. 앞쪽으로 조금 이동해 트랙 진행 방향을 안정적으로 본다.
		const glm::vec3 camera_pos = my_cart.get_position() + camera_up * 0.22f + dir_vec * 0.12f;
		viewMat = glm::lookAt(camera_pos, camera_pos + dir_vec, camera_up);
		glUniformMatrix4fv(viewTransform, 1, GL_FALSE, glm::value_ptr(viewMat));

	}
	glUniformMatrix4fv(viewTransform, 1, GL_FALSE, glm::value_ptr(viewMat));

	GLint projTransform = glGetUniformLocation(shader_program, "projTransform");

	// 원근 투영
	glm::mat4 projMat = glm::perspective(glm::radians(45.f), aspect, 0.1f, 10000.f);

	// 직각 투영
	if (ortho_view)
		projMat = glm::ortho(-camera_height * aspect, camera_height * aspect, -camera_height, camera_height, 0.1f, 1000.f);

	glUniformMatrix4fv(projTransform, 1, GL_FALSE, glm::value_ptr(projMat));

	// Light
	int lightPosLocation = glGetUniformLocation(shader_program, "lightPos"); //--- lightPos 값 전달: (0.0, 0.0, 5.0);
	glUniform3f(lightPosLocation, 0.f, 0.f, 5.0f);
	int lightColorLocation = glGetUniformLocation(shader_program, "lightColor"); //--- lightColor 값 전달: (1.0, 1.0, 1.0) 백색
	glUniform3f(lightColorLocation, 1.f, 1.f, 1.f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	// 컨테이너에 담을 공통된 데이터(위치, 색상, 크기)들을 묶어서 관리
	DrawGround();

	for (int i = 0; i < pillars_num; ++i)
		pillars[i].draw(shader_program, pillar_color);

	// 스플라인 곡선은 제어점이 바뀐 경우에만 다시 계산한다.
	if (pillars_num >= 3) {
		if (track_dirty)
			UpdateTrack();
		DrawTrack();

		// 1인칭 시점에서는 카트 자체를 숨겨 내부 면이 시야를 가리지 않게 한다.
		if (cart_move && !cart_view)
			my_cart.draw(shader_program);
	}


	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(0.5f);
	for (int i = 0; i < pillars_num; ++i)
		pillars[i].draw(shader_program, pillar_edge_color);

	

	glutSwapBuffers(); //--- 화면에 출력하기
}



void Reshape(int w, int h) {
	if (w <= 0)
		w = 1;
	if (h <= 0)
		h = 1;
	screen_w = w;
	screen_h = h;
	aspect = static_cast<float>(w) / static_cast<float>(h);
	glViewport(0, 0, w, h);
}

void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		glutLeaveMainLoop();
		break;
	case 'z':
		ortho_view = true;
		create_pillar = true;
		change_height_1 = false;
		change_height_2 = false;
		break;
	case 'x':
		ortho_view = false;
		create_pillar = false;
		change_height_1 = false;
		change_height_2 = false;
		cart_view = false;
		break;
	case 'c':
		cout << pillars.capacity() << endl;
		pillars.clear();
		points.clear();
		cout << pillars.capacity() << endl;
		pillars_num = 0;
		cart_move = false;
		my_cart.reset();
		track_dirty = true;
		break;
	case 'a':
		ortho_view = true;
		create_pillar = false;
		change_height_1 = true;
		change_height_2 = false;
		cart_view = false;
		break;
	case 's':
		ortho_view = true;
		create_pillar = false;
		change_height_1 = false;
		change_height_2 = true;
		cart_view = false;
		break;
	case 'q':
		for (int i = 0; i < pillars_num; ++i) {
			if (pillars[i].is_selected()) {
				pillars[i].change_height_by_a(size_a);
				track_dirty = true;
				break;
			}
		}
		break;
	case 'w':
		for (int i = 0; i < pillars_num; ++i) {
			if (pillars[i].is_selected()) {
				pillars[i].change_height_by_a(-size_a);
				track_dirty = true;
				break;
			}
		}
		break;
	case 'm':
		if (!cart_move)
			cart_move = true;
		else
			cart_move = false;
		break;
	case 'n':
		ortho_view = false;
		create_pillar = false;
		change_height_1 = false;
		change_height_2 = false;
		cart_view = true;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

GLvoid SpecialKeyboard(int key, int x, int y)
{

	glutPostRedisplay();
}

GLvoid MouseFunc(int button, int state, int x, int y)
{
	
	if (create_pillar == true && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::cout << x << ", " << y << std::endl;
		float ox{};
		float oy{};
		
		convertOpenGlXY(x, y, &ox, &oy);
		//std::cout << ox << ", " << oy << std::endl;
		pillars.emplace_back(glm::vec3(ox * camera_height * aspect, 0, oy * camera_height), 0.5f, 1.f);
		++pillars_num;
		track_dirty = true;

		
	}
	if (change_height_1 == true && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::cout << x << ", " << y << std::endl;
		float ox{};
		float oy{};
		convertOpenGlXY(x, y, &ox, &oy);
		//std::cout << ox * camera_height * aspect << ", " << -(oy * camera_height) << std::endl;
		is_selected(glm::vec3(ox * camera_height * aspect,
			camera_target_height - oy * camera_height, 0), false);
	}
	if (change_height_2 == true && button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		std::cout << x << ", " << y << std::endl;
		float ox{};
		float oy{};
		convertOpenGlXY(x, y, &ox, &oy);
		//std::cout << ox * camera_height * aspect << ", " << -(oy * camera_height) << std::endl;
		// +X에서 원점을 보는 측면 카메라의 화면 오른쪽은 월드 -Z 방향이다.
		is_selected(glm::vec3(0, camera_target_height - oy * camera_height,
			-ox * camera_height * aspect), true);
	}
	glutPostRedisplay();
}

GLvoid MotionFunc(int x, int y)
{

}

GLvoid TimerFunction(int value)
{
	if (track_dirty)
		UpdateTrack();
	if (cart_move && points.size() >= 2) {
		my_cart.move(points);
		glutPostRedisplay();// 화면 재 출력
	}
	
	glutTimerFunc(30, TimerFunction, value);// 타이머함수 재 설정
}
