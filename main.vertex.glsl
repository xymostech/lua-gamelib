#version 330

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

uniform mat4 perspective_matrix;

smooth out vec4 transfer_color;

void main() {
    vec4 camera_pos = position;
    gl_Position = perspective_matrix * camera_pos;
    transfer_color = color;
}