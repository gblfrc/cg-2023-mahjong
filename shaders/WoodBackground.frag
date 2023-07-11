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

void main() {
	const float betaPoint = 4.0f;	// decay exponent of the pointlight
	const float gPoint = 6;

	vec3 N = normalize(fragNorm);				// surface normal
	vec3 V = normalize(gubo.eyePos - fragPos);	// viewer direction
	vec3 MD = texture(tex, fragUV).rgb;			// diffuse color
	vec3 MA = MD;								// ambient color
	vec3 MS = vec3(1);							// specular color
	//vec3 ME = texture(texEmit, fragUV).rgb;		// emission color


	// Write the shader here

	//pointlight
	vec3 L = (gubo.DlightDir - fragPos)/length(gubo.DlightDir - fragPos);
	vec3 lightColor = vec3( gubo.DlightColor*pow( gPoint / length(gubo.DlightDir - fragPos) , betaPoint) );

	//Lambert
	vec3 fDiffuseLambert = MD * max(dot(L, N), 0.0f);

	//Blinn
	vec3 hlx = normalize(L+V);
	vec3 fSpecularBlinn = MS * pow(clamp( dot(N, hlx),0.0f, 1.0f), gamma);


	// output color
	outColor = vec4( clamp(lightColor.rgb*(fDiffuseLambert + fSpecularBlinn), 0.00001f, 1.0f), 1.0f);
}
