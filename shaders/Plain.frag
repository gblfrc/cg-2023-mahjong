#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int id;

layout(set = 0, binding = 0) uniform UniformBufferObject {
	mat4 mvpMat;
	mat4 mMat;
	mat4 nMat;
	float transparency;
	int textureIdx;
	int objectIdx;
} ubo;

layout(set = 0, binding = 1) uniform sampler2DArray tex;

void main() {

	// Use alpha channel from texture if ubo.transparency is set to 1
	// Set 1.0f as alpha channel otherwise
	float alpha = ubo.transparency * texture(tex, vec3(fragUV, ubo.textureIdx)).a + (1.0f-ubo.transparency);
	// Outputs
	outColor = vec4(texture(tex, vec3(fragUV, ubo.textureIdx)).rgb, alpha);
	id = ubo.objectIdx;
}