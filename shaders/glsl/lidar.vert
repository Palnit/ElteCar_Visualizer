#version 450

layout (row_major, binding = 0) uniform Camera {
    mat4 view;
    mat4 proj;
} ubo;


layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 inColor;

layout (location = 0) out vec4 fragColor;

void main() {
    gl_Position = inPosition * ubo.view * ubo.proj;
    gl_PointSize = 3.f;
    fragColor = inColor;
}
