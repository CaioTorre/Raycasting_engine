#include "raycast.h"

void add_drawable_tolist(drawable_obj_llnode **root, drawable_obj *drw_obj) {
    if ((*root) == NULL) {
        (*root) = (drawable_obj_llnode *)malloc(sizeof(drawable_obj_llnode));
        (*root) -> drawable = drw_obj;
        (*root) -> next = NULL;
    } else {
        drawable_obj_llnode *aux = *root;
        while (aux -> next != NULL) aux = aux->next;
        aux -> next = (drawable_obj_llnode *)malloc(sizeof(drawable_obj_llnode));
        aux -> next -> drawable = drw_obj;
        aux -> next -> next = NULL;
    }
}


void add_sphere(drawable_obj_llnode **root, vec3 center, double radius, byte r, byte g, byte b) {
    obj_sphere *sphere = (obj_sphere*)malloc(sizeof(obj_sphere));
    (*sphere) = { center, radius };
    drawable_obj *drw_sphere = (drawable_obj*)malloc(sizeof(drawable_obj));
    (*drw_sphere) = { sphere, known_drawable_objs::sphere, RGB(r,g,b) };
    add_drawable_tolist(root, drw_sphere);
}

// Here we go
intersect_resultset chk_intersect_sphere(vec3 *camera, vec3 viewpoint, obj_sphere *this_sphere) {
    #ifdef DEBUG
    printf("Checking intersect (%.2f, %.2f, %.2f) & (%.2f, %.2f, %.2f) against (%.2f, %.2f, %.2f) - rad = %.2f\n",
           camera->x, camera->y, camera->z,
           viewpoint.x, viewpoint.y, viewpoint.z,
           this_sphere->center.x, this_sphere->center.y, this_sphere->center.z, this_sphere->radius
           );
    #endif
    double  vx_m_cx = (viewpoint.x - camera->x);
    double  vy_m_cy = (viewpoint.y - camera->y);
    double  vz_m_cz = (viewpoint.z - camera->z);
    double  a =  sqr(vx_m_cx) + sqr(vy_m_cy) + sqr(vz_m_cz);
    double  b =  2 * ( vx_m_cx * (camera->x - this_sphere->center.x) + vy_m_cy * (camera->y - this_sphere->center.y) + vz_m_cz * (camera->z - this_sphere->center.z) );
    double  c =  sqr(this_sphere->center.x) + sqr(this_sphere->center.y) + sqr(this_sphere->center.z);
            c += sqr(camera->x) + sqr(camera->y)+ sqr(camera->z);
            c -= 2 * (this_sphere->center.x * camera->x + this_sphere->center.y * camera->y + this_sphere->center.z * camera->z) + sqr(this_sphere->radius);
    // HELLO BASKHARA MY OLD FRIEND
    double  delta = sqr(b) - 4 * a * c;
    if (delta < 0) return (intersect_resultset){ (vec3){ INF, INF, INF }, INF }; // No collision, will default to bg
    if (delta == 0) { // Single collision, easier to check (tangent)
        double u = (-1 * b + sqrt(delta))/(2 * a);
        vec3 c = {camera->x + u*vx_m_cx, camera->y + u*vy_m_cy, camera->z + u*vz_m_cz};
        return (intersect_resultset){ c, vec3_len(vec3_sub(*camera, c)) };
    }
    // Else it crosses through 2 points, find out the closest one
    double u1 = (-1 * b + sqrt(delta))/(2 * a);
    double u2 = (-1 * b - sqrt(delta))/(2 * a);
    vec3 c1 = {camera->x + u1*vx_m_cx, camera->y + u1*vy_m_cy, camera->z + u1*vz_m_cz}; // Get both points
    vec3 c2 = {camera->x + u2*vx_m_cx, camera->y + u2*vy_m_cy, camera->z + u2*vz_m_cz};
    double d1 = vec3_len(vec3_sub(*camera, c1)); // THERE MAY BE AN EASIER WAY
    double d2 = vec3_len(vec3_sub(*camera, c2)); // IM TIRED
    if (d1 < d2) return (intersect_resultset){ c1, d1 };
    return (intersect_resultset){ c2, d2 };
}
