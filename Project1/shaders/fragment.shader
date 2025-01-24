#version 330 core

in vec2 TexCoord;

out vec4 color;

uniform sampler2D texture1;

void main() {
    float value = texture(texture1, TexCoord).r; // Access red channel for GL_R32F.
    color = vec4(value, value, value, 1.0); // Set color to red (RGBA)
}