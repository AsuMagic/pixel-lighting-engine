#version 330 core

uniform sampler2D screen_texture;
uniform sampler2D palette_texture;
uniform sampler2D lightmap_texture;
uniform sampler2D palette_shift_texture;

uint screen_color_indexed()
{
	return uint(texelFetch(screen_texture, ivec2(gl_FragCoord.xy), 0).r * 255.0);
}

vec4 indexed_to_rgb(uint index)
{
	return texelFetch(palette_texture, ivec2(index, 0), 0);
}

float light_level()
{
	return texelFetch(lightmap_texture, ivec2(gl_FragCoord.xy), 0).r;
}

uint apply_lighting(uint index, float light)
{
	return uint(texture(palette_shift_texture, vec2(light, float(index) / 32.0)).r * 255.0);
}

void main()
{
	uint index = screen_color_indexed();
	index = apply_lighting(index, light_level());
    gl_FragColor = indexed_to_rgb(index);
}
