#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int id;

layout(set = 0, binding = 0) uniform CommonUniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	float transparency;
} cubo;

layout(set = 0, binding = 1) uniform ShadingUniformBufferObject {
	float amb;
	float gamma;
	vec3 sColor;
} subo;

layout(set = 0, binding = 2) uniform sampler2D tex;

layout(set = 1, binding = 0) uniform GlobalUniformBufferObject {
	float beta;			// decay factor for point light
	float g;			// distance parameter for point light
	vec3 PlightPos;		// position of the point light
	vec3 PlightColor;	// color of the point light
	vec3 AmbLightColor;	// ambient light
	vec3 eyePos;		// position of the viewer
} gubo;

void main() {

	vec3 N = normalize(fragNorm);								// surface normal
	vec3 V = normalize(gubo.eyePos - fragPos);					// viewer direction
	vec3 L = normalize(gubo.PlightPos - fragPos);				// light direction
	vec3 H = normalize(L + V);									// half vector for Blinn BRDF
	float intensityCoeff = pow((gubo.g/length(gubo.PlightPos - fragPos)), gubo.beta);
	vec3 I = intensityCoeff * gubo.PlightColor;					// Light intensity
	float alpha = cubo.transparency;							// transparency of the tile

	vec3 albedo = texture(tex, fragUV).rgb;
	vec3 MD = albedo;
	vec3 MS = subo.sColor;
	vec3 MA = albedo * subo.amb;
	vec3 LA = gubo.AmbLightColor;
	
	vec3 Lambert = MD * clamp(dot(L,N),0.0f,1.0f);
	vec3 Blinn = MS * pow(clamp(dot(N, H), 0.0f, 1.0f), subo.gamma);
	vec3 Ambient = LA * MA;

	outColor = vec4(clamp((Lambert + Blinn + Ambient),0.0f, 1.0f), alpha);
	id = -1;
}