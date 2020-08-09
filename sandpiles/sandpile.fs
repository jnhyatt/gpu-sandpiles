#version 430 core

uniform usampler2D sandpile;
//uniform vec2 dim;

in vec2 tci;

out uint color;

void main() {
//	vec2 off = 1.0 / dim;

	uint center = texture(sandpile, tci).r;
//	uint right = texture(sandpile, tc_i + off.x).r;
//	uint down = texture(sandpile, tc_i - off.y).r;
//	uint left = texture(sandpile, tc_i - off.x).r;
//	uint up = texture(sandpile, tc_i + off.y).r;
//
//	uint incRight = uint(right >= 4u);
//	uint incDown =  uint(down >= 4u);
//	uint incLeft =  uint(left >= 4u);
//	uint incUp =    uint(up >= 4u);
//	uint inc = incRight + incDown + incLeft + incUp;
//
//	bool dec = center >= 4u;

	color = center;//(center - 4u * uint(dec)) + inc;
}
