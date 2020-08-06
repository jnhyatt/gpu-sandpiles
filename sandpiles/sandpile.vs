#version 430 core

layout (location = 0) in vec2 tc;

out vec2 tc_i;

void main() {
	tc_i = tc;
	gl_Position = vec4(2.0 * tc - 1.0, 0.0, 1.0);
}
