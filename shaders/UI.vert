#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UIUniformBufferObject {
	float visible;
	float transparency;
	int objectIdx;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;

layout(location = 0) out vec2 outUV;

void main() {
	gl_Position = vec4(inPosition * ubo.visible, 1.0);
	outUV = inUV;
}