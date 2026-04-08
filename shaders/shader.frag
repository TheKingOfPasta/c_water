in float speed;
out vec4 FragColor;

void main()
{
    vec2 pt = gl_PointCoord * 2.0 - 1.0;

    if(dot(pt, pt) > 1.0)
        discard;

    const float max_speed = 1000;
    const vec4 base = vec4( 000, 000, 150, 1 );
    const vec4 mid  = vec4( 000, 200, 200, 1 );
    const vec4 high = vec4( 255, 255, 255, 1 );
    const float mid_point = 0.9;

	float t = clamp(speed / max_speed, 0.0, 1.0);

	FragColor = mix(mix(base, mid, smoothstep(0.0, mid_point, t)), high, smoothstep(mid_point, 1.0, t));
}
