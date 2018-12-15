#version 330 core

uniform sampler2D screen_texture;
uniform sampler2D palette_texture;

void main()
{
	float color = texelFetch(screen_texture, ivec2(gl_FragCoord.xy), 0).r * 256.0;
    gl_FragColor = texelFetch(palette_texture, ivec2(color, 0.0), 0);
}
