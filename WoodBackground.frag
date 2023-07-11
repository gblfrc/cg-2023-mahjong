#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

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
} ubo;

layout(set = 1, binding = 1) uniform sampler2D tex;

const float gamma = 110.0f;	// cosine power for the Blinn specular reflection

vec3 BRDF(vec3 V, vec3 N, vec3 Md, vec3 Ms) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 Md - main color of the surface
	//vec3 Ms - specular color

	vec3 L = (gubo.lightPos - fragPos)/length(gubo.lightPos - fragPos);

	//Lambert 
	vec3 fDiffuseLambert = Md * max(dot(L, N), 0.0f);

	//Blinn
	vec3 hlx = normalize(L+V);
	vec3 fSpecularBlinn = MS * pow(clamp( dot(N, hlx),0.0f, 1.0f), gamma);
	
	//return vec3(1,0,0);
	return (fDiffuseLambert + fDiffuseON);
}

void main() {
	const float betaPoint = 4.0f;	// decay exponent of the pointlight
	const float gPoint = 6;
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	vec3 lightDir = gubo.DlightDir;
	//vec3 lightColor = gubo.DlightColor.rgb;

	vec3 DiffSpec = BRDF(EyeDir, lightDir, texture(tex, fragUV).rgb, 1.1f, 1.1f);
	vec3 Ambient = texture(tex, fragUV).rgb * 0.05f;

	//pointlight
	vec3 L = (gubo.DlightDir - fragPos)/length(gubo.DlightDir - fragPos);
	vec3 lightColor = vec3( gubo.DlightColor*pow( gPoint / length(gubo.DlightDir - fragPos) , betaPoint) );

	
	outColor = vec4(clamp(0.95 * (DiffSpec) * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
}