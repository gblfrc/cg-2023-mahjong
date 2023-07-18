#version 450
#extension GL_ARB_separate_shader_objects : enable

#define M_PI 3.1415926535897932384626433832795

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;
layout(location = 3) in vec4 fragTan; //ADDED

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

layout(set = 1, binding = 1) uniform sampler2DArray tex;

vec3 BRDF(vec3 V, vec3 N, vec3 L, vec3 Md, float F0, float metallic, float roughness) {
	//vec3 V  - direction of the viewer
	//vec3 N  - normal vector to the surface
	//vec3 L  - light vector (from the light model)
	//vec3 Md - main color of the surface
	//float F0 - Base color for the Fresnel term
	//float metallic - parameter that mixes the diffuse with the specular term.
	//                 in particular, parmeter K seen in the slides is: float K = 1.0f - metallic;
	//float roughness - Material roughness (parmeter rho in the slides).
	//specular color Ms is not passed, and implicitely considered white: vec3 Ms = vec3(1.0f);

	float d,f,g;

	vec3 hlx = normalize(L+V);

	float LdotN = max(0.00001f, dot(L, N));
	float VdotN = max(0.00001f, dot(V, N));
	float HdotN = max(0.00001f, dot(hlx, N));
	float HdotV = max(0.00001f, dot(hlx, V));

	//Lambert Direct light
	//vec3 fDiffuseLambert = Md * max(LdotN, 0.0f);
	vec3 fDiffuseLambert = Md * clamp(LdotN, 0.0f, 1.0f);

	//GGX D
	//d = (roughness*roughness) / (M_PI * (clamp(dot(N, hlx), 0.00001f, 1.0f)*clamp(dot(N, hlx), 0.00001f, 1.0f)*(roughness*roughness-1)+1) * (clamp(dot(N, hlx), 0.00001f, 1.0f)*clamp(dot(N, hlx), 0.00001f, 1.0f)*(roughness*roughness-1)+1) );
	d = (roughness*roughness) / 
			(M_PI * (clamp(HdotN, 0.0f, 1.0f)*clamp(HdotN, 0.0f, 1.0f)*(roughness*roughness-1)+1) * 
				(clamp(HdotN, 0.0f, 1.0f)*clamp(HdotN, 0.0f, 1.0f)*(roughness*roughness-1)+1) );

	//F
	F0=1;
	//f = F0 + (1-F0)*pow(1-clamp(dot(V, hlx), 0.00001f, 1.0f), 5);
	f = F0 + (1-F0)*pow(1-clamp(HdotV, 0.0f, 1.0f), 5);

	//GGX G
	//float gV = 2 / ( 1 + sqrt(1+roughness*roughness*((1-pow(dot(N, V),2))/(pow(dot(N, V),2)))) );
	//float gL = 2 / ( 1 + sqrt(1+roughness*roughness*((1-pow(dot(N, L),2))/(pow(dot(N, L),2)))) );
	float gV = 2 / ( 1 + sqrt(1+roughness*roughness*((1-VdotN*VdotN)/(VdotN*VdotN))) );
	float gL = 2 / ( 1 + sqrt(1+roughness*roughness*((1-LdotN*LdotN)/(LdotN*LdotN))) );
	g = gV*gL;

	//Specular color
	vec3 Ms = vec3(1.0f);

	//vec3 fSpecularGGX = Ms * ( d*f*g / (4*clamp(dot(V, N),0.00001f, 1.0f)) );
	vec3 fSpecularGGX = Ms * ( d*f*g / (4*clamp(VdotN,0.0f, 1.0f)) );
	//vec3 fSpecularGGX = Ms * ( d*f*g / (4*clamp(dot(L, N),0.00001f, 1.0f)*clamp(dot(V, N),0.00001f, 1.0f)) );

	//coefficient for interpolation
	float k = 1.0f - metallic;

	vec3 result = k*fDiffuseLambert + (1-k)*fSpecularGGX;
	//vec3 result = k*fDiffuseLambert + metallic*fSpecularGGX;

	
	//return vec3(1,1,0);
	return result;
}

void main() {
	const float betaPoint = 4.0f;	// decay exponent of the pointlight
	const float gPoint = 6;
	vec3 Norm = normalize(fragNorm);
	vec3 EyeDir = normalize(gubo.eyePos - fragPos);
	
	vec3 lightDir = gubo.DlightDir;
	//vec3 lightColor = gubo.DlightColor.rgb;

	vec3 DiffSpec = BRDF(EyeDir, Norm, lightDir, texture(tex, vec3(fragUV, 0)).rgb, 1.0f, 1.0f, 1.1f); //CHANGE VALUES
	vec3 Ambient =texture(tex, vec3(fragUV, 0)).rgb * 0.05f;

	//pointlight
	vec3 L = (gubo.DlightDir - fragPos)/length(gubo.DlightDir - fragPos);
	vec3 lightColor = vec3( gubo.DlightColor*pow( gPoint / length(gubo.DlightDir - fragPos) , betaPoint) );

	
	outColor = vec4(clamp(0.95 * (DiffSpec) * lightColor.rgb + Ambient,0.0,1.0), 1.0f);
}