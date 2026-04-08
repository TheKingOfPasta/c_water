#include "shaders/particle.glsl"

layout(location=LOCATION_POS) in vec2 pos;
layout(location=LOCATION_VELO) in vec2 velo;

out float speed;

void main()
{
    if (pos.x > c.sx || pos.y > c.sy || pos.x < 0 || pos.y < 0)
        return;

    vec2 v;
    v.x = pos.x / c.sx * 2.0 - 1.0;
    v.y = pos.y / c.sy * 2.0 - 1.0;

    gl_Position = vec4(v, 0.0, 1.0);
    gl_PointSize = c.radius;
    speed = length(velo);
}
