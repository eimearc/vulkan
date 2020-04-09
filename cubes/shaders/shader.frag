#version 410 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(fragColor,1.0f);
    outColor = vec4(1.0f, 0.0f, 0.0f ,1.0f);
}