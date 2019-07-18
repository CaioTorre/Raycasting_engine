#include "vec3_lib.h"
#include <cmath>

double vec3_len(vec3 vect) {
    return (double)sqrt(vect.x * vect.x + vect.y * vect.y + vect.z * vect.z);
}

vec3 vec3_add(vec3 start, vec3 end) {
    vec3 result;
    result.x = end.x + start.x;
    result.y = end.y + start.y;
    result.z = end.z + start.z;
    return result;
}
vec3 vec3_sub(vec3 start, vec3 end) {
    vec3 result;
    result.x = end.x - start.x;
    result.y = end.y - start.y;
    result.z = end.z - start.z;
    return result;
}
vec3 vec3_multi_r(vec3 vect, double k) {
    vec3 result;
    result.x = vect.x * k;
    result.y = vect.y * k;
    result.z = vect.z * k;
    return result;
}
vec3 vec3_div_r(vec3 vect, double k) {
    vec3 result;
    result.x = vect.x / k;
    result.y = vect.y / k;
    result.z = vect.z / k;
    return result;
}
vec3 vec3_cross_product(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}
vec3 vec3_rotate_normal(vec3 vect, vec3 normal, double rotation_angle) {
    return vec3_add(vec3_multi_r(vect, cos(rotation_angle)), vec3_multi_r(vec3_cross_product(normal, vect), sin(rotation_angle)));
}
vec3 vec3_unit(vec3 vect) {
    return vec3_div_r(vect, vec3_len(vect));
}

double sqr(double val){
    return val * val;
}

double deg_to_rad(double deg) {
    return deg * PI / 180.0;
}
