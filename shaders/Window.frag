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
} ubo;

layout(set = 2, binding = 0) uniform sampler2DArray tex;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, float sigma) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//float sigma - Roughness of the model

	//Oren Nayar
	float dotLN = dot(L, N);
	float dotVN = dot(V, N);
	float thetaI = acos(dotLN);
	float thetaR = acos(dotVN);
	float alpha = max(thetaI, thetaR);
	float beta = min(thetaI, thetaR);
	float a = 1 - 0.5*(sigma*sigma/(sigma*sigma+0.33));
	float b = 0.45*(sigma*sigma/(sigma*sigma+0.09));
	vec3 vI = normalize(L-dotLN*N);
	vec3 vR = normalize(V-dotVN*N);
	float g = max (0, dot(vI, vR));
	vec3 elle = Md*clamp(dotLN, 0.0f, 1.0f);
	vec3 fDiffuseON = elle*(a+b*g*sin(alpha)*tan(beta));

	//Lambert 
	vec3 fDiffuseLambert = Md * max(dot(L, N), 0.0f);
	
	//return vec3(1,0,0);
	return (fDiffuseLambert + fDiffuseON);
}

void main() {
	const float betaPoint = 1.0f;	// decay exponent of the pointlight
	const float gPoint = 10.0f;
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	//vec3 lightDir = gubo.DlightDir;
	//vec3 lightColor = gubo.DlightColor.rgb;
	vec3 lightPosition = vec3(0.0f, 10.0f, 0.0f);

	//pointlight
	vec3 L = (lightPosition - fragPos)/length(lightPosition - fragPos);
	vec3 lightColor = vec3( gubo.DlightColor*pow( gPoint / length(lightPosition - fragPos) , betaPoint) );

	vec3 DiffSpec = BRDF(EyeDir, Norm, L, texture(tex, vec3(fragUV, 0)).rgb, 1.1f);
	vec3 Ambient = texture(tex, vec3(fragUV, 0)).rgb * 0.05f;

	
	outColor = vec4(clamp(0.95 * (DiffSpec) * lightColor.rgb + Ambient,0.0,1.0), texture(tex, vec3(fragUV, 0)).a);
	id = -1;
}