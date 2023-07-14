#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int id;

layout(set = 0, binding = 0) uniform GlobalUniformBufferObject {
	vec3 DlightDir;		// direction of the direct light
	vec3 PlightPos;		//position of the point light
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
	vec3 N = normalize(fragNorm);					// surface normal
	vec3 V = normalize(gubo.eyePos - fragPos);		// viewer direction
	//vec3 L = normalize(gubo.DlightDir);			// light direction for direct light
	
	float transparency = 1.0f;						//CHANGE HERE TO PUT CUSTOM ONE PASSED FROM mahjong.cpp
	const float betaPoint = 2.0f;					// decay exponent of the pointlight
	const float gPoint = 8.0f;

	//vec3 lightPosition = gubo.PlightPos;
	vec3 lightPosition = vec3(5.0f, 10.0f, 5.0f);	//light position


	vec3 albedo = texture(tex, fragUV).rgb;		// main color
	vec3 MD = albedo;
	vec3 MS = ubo.sColor;
	vec3 MA = albedo * ubo.amb;
	vec3 LA = gubo.AmbLightColor;
	

	//pointlight
	vec3 L = (lightPosition - fragPos)/length(lightPosition - fragPos);
	vec3 lightColor = vec3( gubo.DlightColor*pow( gPoint / length(lightPosition - fragPos) , betaPoint) );

	//lambert diffuse
	vec3 Lambert = MD * clamp(dot(L,N),0.0f,1.0f);

	vec3 H = normalize(L + V);						// half vector for Blinn BRDF
	vec3 Blinn = MS * pow(clamp(dot(N, H), 0.0f, 1.0f), ubo.gamma);

	vec3 Ambient = LA * MA;

	outColor = vec4(clamp((Lambert + Blinn + Ambient)*lightColor,0.0f, 1.0f), transparency);
	id = ubo.tileIdx;
}