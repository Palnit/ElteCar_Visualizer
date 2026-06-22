#version 450

layout (location = 0) in vec2 fragTexCoord;

layout (binding = 1) uniform sampler2D texSampler;

layout (row_major, binding = 2) uniform Homography {
    mat4 homography;
} homography;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
}