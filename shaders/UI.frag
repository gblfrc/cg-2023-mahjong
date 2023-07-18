#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UIUniformBufferObject {
	float visible;
	float transparency;
	int objectIdx;
} ubo;

layout(set = 0, binding = 1) uniform sampler2DArray tex;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;
layout(location = 1) out int id;

void main() {

	float alpha = ubo.transparency * texture(tex, vec3(fragUV, 0)).a + (1-ubo.transparency);
	
	outColor = vec4(texture(tex, vec3(fragUV, 0)).rgb, alpha);
	id = ubo.objectIdx;
}