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

double vec3_dot_product(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

double vec3_dist(vec3 point1, vec3 point2) {
    return vec3_len(vec3_sub(point1, point2));
}

double sqr(double val){
    return val * val;
}

double deg_to_rad(double deg) {
    return deg * PI / 180.0;
}

double det(vec3 line1, vec3 line2, vec3 line3) {
    return (line1.x * line2.y * line3.z + line1.y * line2.z * line3.x + line1.y * line2.z * line3.x) - (line1.z * line2.y * line3.x + line1.x * line2.z * line3.y + line1.y * line2.x * line3.z);
}

double det_col(vec3 col1, vec3 col2, vec3 col3) {
    return (col1.x * col2.y * col3.z + col2.x * col3.y * col1.z + col3.x * col1.y * col2.z) - (col3.x * col2.y * col1.z + col1.x * col3.y * col2.z + col2.x * col2.y * col3.z);
}

vec3 solve_3v_eqn_system(vec3 line1, vec3 line2, vec3 line3, vec3 results) {
    double w = det(line1, line2, line3);
    vec3 col1, col2, col3;
    col1 = {line1.x, line2.x, line3.x};
    col2 = {line1.y, line2.y, line3.y};
    col3 = {line1.z, line2.z, line3.z};
    vec3 solution;
    solution.x = det_col(results, col2, col3) / w;
    solution.y = det_col(col1, results, col3) / w;
    solution.z = det_col(col1, col2, results) / w;
    return solution;
}
