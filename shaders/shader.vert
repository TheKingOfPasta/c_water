#include "shaders/particle.glsl"

layout(location=LOCATION_POS) in vec2 pos;
layout(location=LOCATION_VELO) in vec2 velo;

out float speed;

void main()
{
    vec2 v;
    v.x = pos.x / 1920.0 * 2.0 - 1.0;
    v.y = pos.y / 1080.0 * 2.0 - 1.0;

    gl_Position = vec4(v, 0.0, 1.0);
    gl_PointSize = c.radius;
    speed = length(velo);
}
