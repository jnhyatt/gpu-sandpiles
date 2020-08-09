#version 430 core

layout (location = 0) in vec2 tc;

smooth out vec2 tci;

void main() {
	tci = tc;
	gl_Position = vec4(2.0 * tc - 1.0, 0.0, 1.0);
}
