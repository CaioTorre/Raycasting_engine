#include <Windows.h>
#include <cmath>
#include "vec3_lib.h"
#ifndef RAYCAST_H_INCLUDED
#define RAYCAST_H_INCLUDED

//Revealed to me in a dream
#define KX 8 //5
#define KY 17 //7

#define INF (double)INFINITE

typedef double angle;

struct obj_camera_t {
    vec3 anchor;
    angle roll, pitch, yaw;
    vec3 director_x, director_y, director_z;
};
typedef struct obj_camera_t obj_camera;

struct intersect_resultset_t {
    vec3 intersect_point;
    double distance;
};
typedef struct intersect_resultset_t intersect_resultset;

struct obj_sphere_t {
    vec3 center;
    double radius;
};
typedef struct obj_sphere_t obj_sphere;

enum known_drawable_objs { sphere };

struct drawable_obj_t {
    void *object;
    enum known_drawable_objs object_type;
    COLORREF texture_color;
    double texture_alpha;
};
typedef struct drawable_obj_t drawable_obj;

struct drawable_obj_llnode_t {
    drawable_obj *drawable;
    struct drawable_obj_llnode_t *next;
};
typedef struct drawable_obj_llnode_t drawable_obj_llnode;


void add_drawable_tolist(drawable_obj_llnode **root, drawable_obj *drw_obj);

void add_sphere(drawable_obj_llnode **root, vec3 center, double radius, byte r, byte g, byte b);

intersect_resultset chk_intersect_sphere(vec3 *camera, vec3 viewpoint, obj_sphere *this_sphere);



#endif // RAYCAST_H_INCLUDED
