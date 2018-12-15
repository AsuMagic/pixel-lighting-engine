#version 330 core

uniform sampler2D screen_texture;
uniform sampler2D lightmap_texture;
uniform sampler2D palette_shift_texture;

uniform vec2 resolution;

void main()
{
	float color = texelFetch(screen_texture, ivec2(gl_FragCoord.xy), 0).r * 256.0;
	float lightmap_color = texelFetch(lightmap_texture, ivec2(gl_FragCoord.xy), 0).r;

    gl_FragColor.r = texture(palette_shift_texture, vec2(lightmap_color, color / 32.0)).r;
    gl_FragColor.a = 1.0;
}
