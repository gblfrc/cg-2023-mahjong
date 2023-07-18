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
	int textureIdx;
	int objectIdx;
} cubo;

layout(set = 0, binding = 1) uniform ShadingUniformBufferObject {
	float amb;
	float sigma;
} subo;

layout(set = 0, binding = 2) uniform sampler2DArray tex;

layout(set = 1, binding = 0) uniform GlobalUniformBufferObject {
	float beta;			// decay factor for point light
	float g;			// distance parameter for point light
	vec3 PlightPos;		// position of the point light
	vec3 PlightColor;	// color of the point light
	vec3 AmbLightColor;	// ambient light
	vec3 eyePos;		// position of the viewer
} gubo;


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

	return fDiffuseON;
}

void main() {

	vec3 N = normalize(fragNorm);
	vec3 V = normalize(gubo.eyePos - fragPos);
	vec3 L = normalize(gubo.PlightPos - fragPos);
	// point light intensity
	vec3 I = gubo.PlightColor * pow(gubo.g/length(gubo.PlightPos - fragPos), gubo.beta);

	vec3 diffuseON = BRDF(V, N, L, texture(tex, vec3(fragUV, cubo.textureIdx)).rgb, subo.sigma);
	vec3 Ambient = texture(tex, vec3(fragUV, cubo.textureIdx)).rgb * subo.amb * gubo.AmbLightColor;

	float alpha = cubo.transparency * texture(tex, vec3(fragUV, cubo.textureIdx)).a + (1-cubo.transparency);
	
	outColor = vec4(clamp(0.95*(diffuseON)*I.rgb + Ambient * 0.05f,0.0,1.0), alpha);
	id = -1;

}