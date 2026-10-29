#version 330
//--- in_Position: attribute index 0
//--- in_Color: attribute index 1

layout (location = 0) in vec3 vPos; // 위치 변수: attribute position 0
layout (location = 1) in vec3 vNormal; // 컬러 변수: attribute position 1


uniform mat4 modelTransform; //--- 모델링 변환 행렬: uniform 변수로 선언
uniform mat4 viewTransform;
uniform mat4 projTransform;

out vec3 FragPos;
out vec3 Normal; //--- 프래그먼트 세이더에게 전달


void main(void)
{
	gl_Position = projTransform * viewTransform * modelTransform * vec4(vPos, 1.0);
	FragPos = vec3(modelTransform * vec4(vPos, 1.0));
	Normal = transpose(inverse(mat3(modelTransform))) * vNormal;
}
