#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int id;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	vec3 DlightDir;		// direction of the direct light
	vec3 DlightColor;	// color of the direct light
	vec3 AmbLightColor;	// ambient light
	vec3 eyePos;		// position of the viewer
} gubo;

layout(set = 1, binding = 0) uniform UniformBufferObject {
	float amb;
	float gamma;
	vec3 sColor;
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	int tileIdx;
	int suitIdx;
} ubo;

layout(set = 1, binding = 1) uniform sampler2D tex;

void main() {
	vec3 N = normalize(fragNorm);				// surface normal
	vec3 V = normalize(gubo.eyePos - fragPos);	// viewer direction
	vec3 L = normalize(gubo.DlightDir);			// light direction
	vec3 H = normalize(L + V);					// half vector for Blinn BRDF
	float transparency = 1.0f;					//CHANGE HERE TO PUT CUSTOM ONE PASSED FROM mahjong.cpp

	vec3 albedo = texture(tex, fragUV).rgb;		// main color
	vec3 MD = albedo;
	vec3 MS = ubo.sColor;
	vec3 MA = albedo * ubo.amb;
	vec3 LA = gubo.AmbLightColor;
	
	vec3 Lambert = MD * clamp(dot(L,N),0.0f,1.0f);
	vec3 Blinn = MS * pow(clamp(dot(N, H), 0.0f, 1.0f), ubo.gamma);
	vec3 Ambient = LA * MA;

	outColor = vec4(clamp(Lambert + Blinn + Ambient,0.0f, 1.0f), 1.0f);
	id = ubo.tileIdx;
}