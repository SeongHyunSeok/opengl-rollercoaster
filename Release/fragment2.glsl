#version 330
//--- out_Color: 버텍스 세이더에서 입력받는 색상 값
//--- FragColor: 출력할 색상의 값으로 프레임 버퍼로 전달 됨.

in vec3 FragPos;
in vec3 Normal; //--- 버텍스 세이더에게서 전달 받음

out vec4 FragColor; //--- 색상 출력

uniform vec3 lightPos; //--- 조명의 위치
uniform vec3 lightColor; //--- 조명의 색
uniform vec3 objectColor;

uniform vec3 viewPos;

void main(void)
{
	float ambientLight = 0.6; //--- 주변 조명 계수
	vec3 ambient = ambientLight * lightColor;
	
	vec3 normalVector = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diffuseLight = max(dot(normalVector, lightDir), 0.0);
	vec3 diffuse = diffuseLight * lightColor;

	/*
	int shininess = 128; //--- 광택 계수
	vec3 viewDir = normalize(viewPos - FragPos); //--- 관찰자의 방향
	vec3 reflectDir = reflect(-lightDir, normalVector); //--- 반사 방향 : reflect 함수 입사 벡터의 반사 방향 계산
	float specularLight = max(dot(viewDir, reflectDir), 0.0); //--- V 와 R 의 내적값으로 강도 조절 : 음수 방지
	specularLight = pow(specularLight, shininess); //--- 승을 해주어 하이라이트를 만들어준다
	vec3 specular = specularLight * lightColor; //--- 거울 반사 조명값 : 거울반사값 * 조명색상값
	*/

	vec3 result = (ambient + diffuse) * objectColor;

	FragColor = vec4 (result, 1.0);
}