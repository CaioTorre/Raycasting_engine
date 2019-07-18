#include "raycast.h"
#include <iostream>

using namespace std;

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

void add_sphere(drawable_obj_llnode **root, vec3 center, double radius, byte r, byte g, byte b, double alpha) {
    obj_sphere *sphere = (obj_sphere*)malloc(sizeof(obj_sphere));
    (*sphere) = { center, radius };
    drawable_obj *drw_sphere = (drawable_obj*)malloc(sizeof(drawable_obj));
    (*drw_sphere) = { sphere, known_drawable_objs::sphere, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_sphere);
}

// Here we go
intersect_resultset chk_intersect_sphere(vec3 *camera, vec3 viewpoint, drawable_obj *this_sphere_obj, intersects_llnode **intersections_root) {
    obj_sphere *this_sphere = (obj_sphere*)this_sphere_obj->object;
    intersect_resultset temp1, temp2;
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
        temp1 = (intersect_resultset){ c, vec3_len(vec3_sub(*camera, c)) };
        add_intersection_tolist(intersections_root, this_sphere_obj, temp1);
        return temp1;
    }
    // Else it crosses through 2 points, find out the closest one
    double u1 = (-1 * b + sqrt(delta))/(2 * a);
    double u2 = (-1 * b - sqrt(delta))/(2 * a);
    vec3 c1 = {camera->x + u1*vx_m_cx, camera->y + u1*vy_m_cy, camera->z + u1*vz_m_cz}; // Get both points
    vec3 c2 = {camera->x + u2*vx_m_cx, camera->y + u2*vy_m_cy, camera->z + u2*vz_m_cz};
    double d1 = vec3_len(vec3_sub(*camera, c1)); // THERE MAY BE AN EASIER WAY
    double d2 = vec3_len(vec3_sub(*camera, c2)); // IM TIRED
    temp1 = (intersect_resultset){ c1, d1 };
    temp2 = (intersect_resultset){ c2, d2 };
    add_intersection_tolist(intersections_root, this_sphere_obj, temp1);
    add_intersection_tolist(intersections_root, this_sphere_obj, temp2);
    if (d1 < d2) return temp1;
    return temp2;
}

void add_intersection_tolist(intersects_llnode **root, drawable_obj *obj, intersect_resultset intersect) {
    if ((*root) == NULL) {
        //printf("Setting root as %.2f\n", intersect.distance);
        (*root) = (intersects_llnode *)malloc(sizeof(intersects_llnode));
        (*root) -> associated_object = obj;
        (*root) -> intersection = intersect;
        (*root) -> next = NULL;
    } else {
        if (((*root) -> intersection).distance > intersect.distance) {
            //printf("Resetting root\n");
            intersects_llnode *aux = (intersects_llnode *)malloc(sizeof(intersects_llnode));
            aux -> associated_object = obj;
            aux -> intersection = intersect;
            aux -> next = (*root);
            (*root) = aux;
        } else {
            //printf("Finding place for %.2f...", intersect.distance);
            intersects_llnode *aux = (*root);
            intersects_llnode *ant;
            while (aux != NULL && ((aux -> intersection).distance < intersect.distance)) {
                //printf("(current is %.2f)...", (aux->intersection).distance);
                ant = aux;
                aux = aux->next;
            }
            //printf("Found place!\n");
            ant -> next = (intersects_llnode *)malloc(sizeof(intersects_llnode));
            (ant -> next) -> associated_object = obj;
            (ant -> next) -> intersection = intersect;
            (ant -> next) -> next = aux;
        }
    }
    //print_intersection_ll_cout(*root);
}

int free_intersection_linkedlist(intersects_llnode **root) {
    /*if ((*root) == NULL) return 0;
    int v = free_intersection_linkedlist(&((*root)->next));
    free(*root);
    return v + 1;*/
    intersects_llnode *aux1, *aux2;
    aux1 = *root;
    int res = 0;
    while(aux1 != NULL) {
        aux2 = aux1;
        aux1 = aux1->next;
        free(aux2);
        res++;
    }
    (*root) = NULL;
    return res;
}

pixel_color calculate_intersection_results(intersects_llnode *root, byte bgr, byte bgg, byte bgb) {
    if (root == NULL) {
            //printf("Found end of list, returning...\n");
            return (pixel_color){bgr, bgg, bgb};
    }
    pixel_color combination = calculate_intersection_results(root->next, bgr, bgg, bgb);
    //printf("Merging with current intersection...\n");
    pixel_color this_color = root->associated_object->texture_color;
    pixel_color result;
    double current_alpha = root->associated_object->texture_alpha;
    result.r = ( (double)this_color.r * current_alpha ) + ( (double)combination.r * ( 1.0 - current_alpha ) );
    result.g = ( (double)this_color.g * current_alpha ) + ( (double)combination.g * ( 1.0 - current_alpha ) );
    result.b = ( (double)this_color.b * current_alpha ) + ( (double)combination.b * ( 1.0 - current_alpha ) );
    //printf("(%3d,%3d,%3d)", result.r, result.g, result.b);
    return result;
}

void print_intersection_ll(intersects_llnode *root, FILE *desc) {
    if (root == NULL) {
            fprintf(desc, "#\n");
            return;
    }
    fprintf(desc, "(%2d,%2d,%2d)->", root->associated_object->texture_color.r, root->associated_object->texture_color.g, root->associated_object->texture_color.b);
    print_intersection_ll(root->next, desc);
}

void print_intersection_ll_cout(intersects_llnode *root) {
    if (root == NULL) {
            printf("#\n");
            return;
    }
    printf("(%2d,%2d,%2d)->", root->associated_object->texture_color.r, root->associated_object->texture_color.g, root->associated_object->texture_color.b);
    print_intersection_ll_cout(root->next);
}

