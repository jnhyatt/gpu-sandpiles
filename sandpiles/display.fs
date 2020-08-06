#version 430 core

uniform usampler2D sandpile;

in vec2 tc_i;

out vec4 color;

const vec4 colors[4] = vec4[4](
	vec4(1.0, 1.0, 0.0, 1.0),
	vec4(0.0, 0.75, 0.25, 1.0),
	vec4(0.0, 0.4, 0.9, 1.0),
	vec4(0.5, 0.0, 0.9, 1.0)
);

void main() {
	color = colors[min(texture(sandpile, tc_i).r, 3u)];
}
