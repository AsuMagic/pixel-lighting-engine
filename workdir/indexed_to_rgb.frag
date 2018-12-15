uniform sampler2D texture;
uniform sampler2D palette_texture;

void main()
{
	vec2 uv = gl_FragCoord.xy / vec2(2560.0, 1440.0);

	float color = floor(texture2D(texture, uv).r * 256.0);
    gl_FragColor = texture2D(palette_texture, vec2(color / 32.0, 0.0));
}
