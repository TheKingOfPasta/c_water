in float speed;
out vec4 FragColor;

void main()
{
    vec2 pt = gl_PointCoord * 2.0 - 1.0;

    if(dot(pt, pt) > 1.0)
        discard;

    const float max_speed = 5.5;
    const vec4 base = vec4( 000, 000, 150, 1 );
    const vec4 mid  = vec4( 000, 200, 200, 1 );
    const vec4 high = vec4( 255, 255, 255, 1 );
    const float mid_point = 0.1;

    float t = speed / max_speed;

    if (t > 1.0f)
        t = 1.0f;

    if (t < mid_point)
        FragColor = base * (1 - t / mid_point) + mid * (t / mid_point);
    else
        FragColor = mid * (1 - (t - mid_point) / (1 - mid_point)) + high * ((t - mid_point) / (1 - mid_point));
}
