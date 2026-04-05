in float speed;
out vec4 FragColor;

void main()
{
    vec2 pt = gl_PointCoord * 2.0 - 1.0;

    if(dot(pt, pt) > 1.0)
        discard;

    float t = clamp(speed, 0, 1);

    vec4 c1 = vec4(0.0, 0.0, 0.0, 1);
    vec4 c2 = vec4(0.0, 0.6, 0.8, 1);

    FragColor = c1 * t + c2 * (1 - t);
}
