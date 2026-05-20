out vec4 FragColor;

void main()
{
    vec2 p = gl_PointCoord * 2.0 - 1.0;
    if (dot(p, p) > 10.0)
        discard;

    FragColor = vec4(0.35, 0.65, 1.0, 1.0);
}
