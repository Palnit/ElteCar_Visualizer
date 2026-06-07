#version 450


layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec2 inTexCoord;

layout (location = 2) in vec3 inInstancePos;


layout (row_major, binding = 0) uniform Model {
    mat4 model;
} model;

layout (location = 0) out vec2 fragTexCoord;
layout (location = 1) out uint instanceIndex;

void main() {
    gl_Position = vec4((inPosition + inInstancePos), 1.0) * model.model;
    fragTexCoord = inTexCoord;
    instanceIndex = gl_VertexIndex;
}
