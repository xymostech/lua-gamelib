#version 330

smooth in vec4 transfer_color;

out vec4 out_color;

void main() {
    out_color = transfer_color;
}