#include <Windows.h>
#include <cmath>
#include <stdio.h>
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

struct pixel_color_t {
    byte r, g, b;
};
typedef struct pixel_color_t pixel_color;

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
    pixel_color texture_color;
    double texture_alpha;
};
typedef struct drawable_obj_t drawable_obj;

struct drawable_obj_llnode_t {
    drawable_obj *drawable;
    struct drawable_obj_llnode_t *next;
};
typedef struct drawable_obj_llnode_t drawable_obj_llnode;

struct point_lightsource_t {
    vec3 anchor;
    double intensity;
};
typedef struct point_lightsource_t point_lightsource;

enum known_lightsources { point };

struct lightsources_llnode_t {
    void *source;
    enum known_lightsources source_type;
    struct lightsources_llnode_t *next;
};
typedef struct lightsources_llnode_t lightsources_llnode;

struct intersects_llnode_t {
    drawable_obj *associated_object;
    intersect_resultset intersection;
    struct intersects_llnode_t *next;
};
typedef struct intersects_llnode_t intersects_llnode;

//Function headers
void add_drawable_tolist(drawable_obj_llnode **root, drawable_obj *drw_obj);
void add_sphere(drawable_obj_llnode **root, vec3 center, double radius, byte r, byte g, byte b, double alpha);
intersect_resultset chk_intersect_sphere(vec3 *camera, vec3 viewpoint, drawable_obj *this_sphere_obj, intersects_llnode **intersections_root);
void add_intersection_tolist(intersects_llnode **root, drawable_obj *obj, intersect_resultset intersect);
int free_intersection_linkedlist(intersects_llnode **root);
pixel_color calculate_intersection_results(intersects_llnode *root, byte bgr, byte bgg, byte bgb);
void print_intersection_ll(intersects_llnode *root, FILE *desc);
void print_intersection_ll_cout(intersects_llnode *root);

#endif // RAYCAST_H_INCLUDED
