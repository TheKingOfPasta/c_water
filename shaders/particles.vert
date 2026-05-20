#include "shaders/particle.glsl"

uniform uint NB_PARTICLES;

void main()
{
    if (uint(gl_VertexID) >= NB_PARTICLES)
    {
        gl_Position  = vec4(0.0, 0.0, 2.0, 1.0);
        gl_PointSize = 0.0;
        return;
    }

    vec3 pos = particles[gl_VertexID].pos;
    vec3 rel = pos - s.cam_pos;

    float cosPitch = cos(s.cam_pitch);
    float sinPitch = sin(s.cam_pitch);
    float cosYaw   = cos(s.cam_yaw);
    float sinYaw   = sin(s.cam_yaw);

    float rx =  cosYaw * rel.x - sinYaw * rel.z;
    float ry =  rel.y;
    float rz =  sinYaw * rel.x + cosYaw * rel.z;

    vec3 view;
    view.x =  rx;
    view.y =  cosPitch * ry + sinPitch * rz;
    view.z = -sinPitch * ry + cosPitch * rz;

    if (view.z <= 0.1)
    {
        gl_Position  = vec4(0.0, 0.0, 2.0, 1.0);
        gl_PointSize = 0.0;
        return;
    }

    float f      = 1.0 / tan(radians(30.0));
    float aspect  = c.screen_width / float(c.screen_height);
    float near    = 1.0;
    float far     = 50000.0;

    gl_Position = vec4(
        f / aspect * view.x,
        f * view.y,
        view.z * (far + near) / (far - near) - 2.0 * far * near / (far - near),
        view.z
    );

    gl_PointSize = clamp(2000.0 / view.z, 1.0, 128.0);
}
