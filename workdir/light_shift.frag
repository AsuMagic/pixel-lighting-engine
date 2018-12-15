uniform sampler2D texture;
uniform sampler2D lightmap_texture;
uniform sampler2D palette_shift_texture;

uniform vec2 resolution;

void main()
{
	vec2 uv = gl_FragCoord.xy / resolution;

	float color = texture2D(texture, uv).r * 256.0;
	float lightmap_color = 1.0 * floor(texture2D(lightmap_texture, uv).r * 8.0) / 8.0;
    gl_FragColor = vec4(texture2D(palette_shift_texture, vec2(lightmap_color, color / 32.0)).r, 0.0, 0.0, 1.0);
}
