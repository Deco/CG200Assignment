
#include "matrix.h"

const mat4 mat4::identity = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};

const vec3 vec3::zero   = vec3(0, 0, 0);
const vec3 vec3::unit_x = vec3(1, 0, 0);
const vec3 vec3::unit_y = vec3(0, 1, 0);
const vec3 vec3::unit_z = vec3(0, 0, 1);

const vec4 vec4::zero   = vec4(0, 0, 0, 0);
const vec4 vec4::unit_x = vec4(1, 0, 0, 0);
const vec4 vec4::unit_y = vec4(0, 1, 0, 0);
const vec4 vec4::unit_z = vec4(0, 0, 1, 0);
const vec4 vec4::unit_w = vec4(0, 0, 0, 1);

const quat quat::identity = quat(1, 0, 0, 0);

/*
inline void vec3_set(vec3 *v, NUMBER x, NUMBER y, NUMBER z) {
    v->x = x; v->y = y; v->z = z;
}
inline void vec3_copy(vec3 *v, vec3 *u) {
    v->x = u->x; v->y = u->y; v->z = u->z;
}
inline void vec3_zero(vec3 *v) {
    v->x = 0; v->y = 0; v->z = 0;
}
inline void vec3_add(vec3 *v, vec3 *u) {
    v->x += u->x; v->y += u->y; v->z += u->z;
}
inline void vec3_sub(vec3 *v, vec3 *u) {
    v->x -= u->x; v->y -= u->y; v->z -= u->z;
}
inline NUMBER vec3_dot(vec3 *v, vec3 *u) {
    return (v->x * u->x) + (v->y * u->y) + (v->z * u->z);
}

inline void ang3_set(ang3 *v, NUMBER y, NUMBER p, NUMBER r) {
    v->y = y; v->p = p; v->r = r;
}
inline void ang3_copy(ang3 *v, ang3 *u) {
    v->y = u->y; v->p = u->p; v->r = u->r;
}
inline void ang3_zero(ang3 *v) {
    v->y = 0; v->p = 0; v->r = 0;
}
*/