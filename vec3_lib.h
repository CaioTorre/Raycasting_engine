#ifndef VEC3_LIB_H_INCLUDED
#define VEC3_LIB_H_INCLUDED

#define PI 3.14159265

struct vec3_t {
    double x, y, z;
};
typedef struct vec3_t vec3;

double vec3_len(vec3 vect);
vec3 vec3_add(vec3 start, vec3 end);
vec3 vec3_sub(vec3 start, vec3 end);
vec3 vec3_multi_r(vec3 vect, double k);
vec3 vec3_div_r(vec3 vect, double k);
vec3 vec3_cross_product(vec3 a, vec3 b);
vec3 vec3_rotate_normal(vec3 vect, vec3 normal, double angle);
vec3 vec3_unit(vec3 vect);

double sqr(double val);
double deg_to_rad(double deg);

#endif // VEC3_LIB_H_INCLUDED
