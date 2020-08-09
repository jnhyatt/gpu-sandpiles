#version 430 core

uniform usampler2D sandpile;

smooth in vec2 tci;

out vec4 color;

const vec4 colors[4] = vec4[4](
	vec4(0.2, 0.0, 0.0, 1.0),
	vec4(1.0, 0.0, 0.0, 1.0),
	vec4(1.0, 0.2, 0.2, 1.0),
	vec4(1.0, 1.0, 1.0, 1.0)
);

void main() {
	color = colors[texture(sandpile, tci).r];
}
