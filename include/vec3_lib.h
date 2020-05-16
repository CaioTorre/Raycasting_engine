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

double vec3_dot_product(vec3 a, vec3 b);
double vec3_dist(vec3 point1, vec3 point2);

double sqr(double val);
double deg_to_rad(double deg);

double det(vec3 line1, vec3 line2, vec3 line3);
double det_col(vec3 col1, vec3 col2, vec3 col3);
vec3 solve_3v_eqn_system(vec3 line1, vec3 line2, vec3 line3, vec3 results);

#endif // VEC3_LIB_H_INCLUDED
