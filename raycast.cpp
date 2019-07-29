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
//void add_sphere(drawable_obj_llnode **root, vec3 center, double radius, pixel_color_rgba rgba) {
    // Create sphere in memory
    obj_sphere *sphere = (obj_sphere*)malloc(sizeof(obj_sphere));
    // Fill details about sphere
    sphere -> center = center;
    sphere -> radius = radius;
    //(*sphere) = { center, radius };
    // Create drawable abstraction
    drawable_obj *drw_sphere = (drawable_obj*)malloc(sizeof(drawable_obj));
    // Fill details about drawable
    (*drw_sphere) = { sphere, known_drawable_objs::sphere, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_sphere);
}

void add_plane_3_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, vec3 point3, byte r, byte g, byte b, double alpha) {
//void add_plane_3_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, vec3 point3, pixel_color_rgba rgba) {
    // Create plane in memory
    obj_plane *plane = (obj_plane*)malloc(sizeof(obj_plane));
    // Fill details about plane
    plane -> point = point1;
    plane -> normal = vec3_cross_product( vec3_sub( point2, point1 ), vec3_sub( point3, point1 ) );
    // Create drawable abstraction
    drawable_obj *drw_plane = (drawable_obj*)malloc(sizeof(drawable_obj));
    // Fill details about drawable
    (*drw_plane) = { plane, known_drawable_objs::plane, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_plane);
}

void add_line_2_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, double radius, byte r, byte g, byte b, double alpha) {
//void add_line_2_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, double radius, pixel_color_rgba rgba) {
    // Create line in memory
    obj_line *line = (obj_line*)malloc(sizeof(obj_line));
    // Fill details about line
    line -> point = point1;
    line -> director = vec3_unit( vec3_sub( point1, point2 ) );
    //printf("Director: (%.2f, %.2f, %.2f) - len = %.2f\n", line -> director.x, line -> director.y, line -> director.z, vec3_len(line -> director));
    line -> radius = radius;
    // Create drawable abstraction
    drawable_obj *drw_line = (drawable_obj*)malloc(sizeof(drawable_obj));
    // Fill details about drawable
    (*drw_line) = { line, known_drawable_objs::line, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_line);
}

int load_shapes_from_file(const char *file_location, drawable_obj_llnode **root) {
    FILE *fd = fopen(file_location, "r");
    //char temp = (char) 1;
    int read_objs = 0;
    enum known_drawable_objs current_obj_type;
    vec3 point1, point2, point3;
    double radius;
    int r, g, b;
    double a;
    while (!feof(fd)) {
        //if (temp != (char) 1) ungetc(temp, fd);
        fscanf(fd, "%d %d %d %d %lf ", &current_obj_type, &r, &g, &b, &a);
        //fscanf(fd, "");
        switch (current_obj_type) {
        case sphere:
            fscanf(fd, "%lf %lf %lf %lf ", &(point1.x), &(point1.y), &(point1.z), &radius);
            add_sphere(root, point1, radius, (byte)r, (byte)g, (byte)b, a);
            //printf("Read sphere: (%.2f %.2f %.2f) - %.2f - %d %d %d %.2f\n", point1.x, point1.y, point1.z, radius, r, g, b, a);
            read_objs++;
            break;
        case plane:
            fscanf(fd, "%lf %lf %lf %lf %lf %lf %lf %lf %lf ", &point1.x, &point1.y, &point1.z, &point2.x, &point2.y, &point2.z, &point3.x, &point3.y, &point3.z);
            add_plane_3_points(root, point1, point2, point3, (byte)r, (byte)g, (byte)b, a);
            //printf("Read plane\n");
            read_objs++;
            break;
        case line:
            fscanf(fd, "%lf %lf %lf %lf %lf %lf %lf ", &point1.x, &point1.y, &point1.z, &point2.x, &point2.y, &point2.z, &radius);
            add_line_2_points(root, point1, point2, radius, (byte)r, (byte)g, (byte)b, a);
            //printf("Read line\n");
            read_objs++;
            break;
        default:
            printf("Unknown obj type %d\n", current_obj_type);
            break;
        }
        //temp = fgetc(fd);
    }
    fclose(fd);
    return read_objs;
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
    //double d1 = vec3_len(vec3_sub(*camera, c1)); // THERE MAY BE AN EASIER WAY
    //double d2 = vec3_len(vec3_sub(*camera, c2)); // IM TIRED
    //temp1 = (intersect_resultset){ c1, d1 };
    //temp2 = (intersect_resultset){ c2, d2 };
    temp1 = (intersect_resultset){ c1, vec3_dist( *camera, c1 ) };
    temp2 = (intersect_resultset){ c2, vec3_dist( *camera, c2 ) };
    add_intersection_tolist(intersections_root, this_sphere_obj, temp1);
    add_intersection_tolist(intersections_root, this_sphere_obj, temp2);
    if ( vec3_dist( *camera, c1 ) < vec3_dist( *camera, c2 ) ) return temp1;
    return temp2;
}

intersect_resultset chk_intersect_plane (vec3 *camera, vec3 viewpoint, drawable_obj *this_plane_obj,  intersects_llnode **intersections_root) {
    obj_plane *this_plane = (obj_plane*)this_plane_obj->object;
    /*vec3 ray_director = vec3_sub(*camera, viewpoint);
    double upper = vec3_dot_product( this_plane->normal, vec3_sub( this_plane->point, *camera ) );
    double lower = -1.0 * vec3_dot_product( ray_director, this_plane->normal );
    double t = upper / lower;
    if (t > 0) { //In front of camera
        vec3 inters = vec3_add(*camera, vec3_multi_r(ray_director, t));
        intersect_resultset temp = { inters, vec3_dist( *camera, inters ) };
        add_intersection_tolist( intersections_root, this_plane_obj, temp );
        return temp;
    }*/

    vec3 u = vec3_sub(*camera, viewpoint);
    vec3 w = vec3_sub(this_plane -> point, *camera);
    double d = vec3_dot_product(this_plane -> normal, u);
    if (d == 0) return (intersect_resultset){ (vec3){ INF, INF, INF }, INF };
    double s = -1.0 * vec3_dot_product(this_plane -> normal, w) / d;
    //if ((s > 0 && s <= 1)) { // Then add to intersection list
    if (s > 0) {
        vec3 inters = vec3_add( *camera, vec3_multi_r( u, s ) );
        //intersect_resultset temp = { inters, vec3_len( vec3_sub( *camera, inters ) ) };
        intersect_resultset temp = { inters, vec3_dist( *camera, inters ) };
        add_intersection_tolist( intersections_root, this_plane_obj, temp );
        return temp;
    }
    //printf("Found intersection with s = %.2f\n", s);

    return (intersect_resultset){ (vec3){ INF, INF, INF }, INF };
}

intersect_resultset chk_intersect_line  (vec3 *camera, vec3 viewpoint, drawable_obj *this_line_obj,   intersects_llnode **intersections_root) {
    obj_line *this_line = (obj_line*)this_line_obj->object;
    vec3 ray_director = vec3_sub(*camera, viewpoint);
    vec3 n_closest = vec3_cross_product(ray_director, this_line->director);
    if (vec3_len(n_closest) == 0) return (intersect_resultset){ (vec3){ INF, INF, INF }, INF }; //Parallel lines
    vec3 n2 = vec3_cross_product(this_line -> director, n_closest );
    vec3 n1 = vec3_cross_product(ray_director, vec3_cross_product(this_line -> director, ray_director));
    vec3 c1 = vec3_add( *camera, vec3_multi_r( ray_director, vec3_dot_product( vec3_sub( *camera, this_line -> point ), n2 ) / vec3_dot_product( ray_director, n2 ) ) );
    vec3 c2 = vec3_add( this_line -> point, vec3_multi_r( this_line -> director, vec3_dot_product( vec3_sub( this_line -> point, *camera ), n1 ) / vec3_dot_product( this_line -> director, n1 ) ) );
    double dist = vec3_dist( c1, c2 );
    double c_dist = vec3_dist( *camera, c1 );
    //printf("Dist from line = %.2f, dist from camera = %.2f\n", dist, c_dist);
    if (dist <= this_line -> radius) {
        vec3 cam_inters = vec3_sub(*camera, c1);
        double dotp = vec3_dot_product(cam_inters, ray_director); // Prevent rays from looping back
        double theta = acos(dotp/(vec3_len(cam_inters) * vec3_len(ray_director)));
        if (abs(theta) < PI / 2.0) {
            intersect_resultset temp = { c1, c_dist };
            add_intersection_tolist( intersections_root, this_line_obj, temp );
            return temp;
        }
    }
    /*
    vec3 n_unit = vec3_unit(n_closest);
    double dist = abs(vec3_dot_product(n_unit, vec3_sub(*camera, this_line -> point)));
    //vec3 point_diff = vec3_sub(this_line->point, *camera);
    //double dist = vec3_dot_product(n_closest, point_diff) / vec3_len(n_closest);
    if (dist <= this_line -> radius) {
        vec3 n2 = vec3_cross_product(this_line -> director, n_closest);
        vec3 inters = vec3_add(*camera, vec3_multi_r(ray_director, (vec3_dot_product(vec3_sub(*camera, this_line->point), n2)) / vec3_dot_product(ray_director, n2)));
        //cout << "Found intersection w line, dist = " << dist << " vs " << this_line -> radius << endl;
        intersect_resultset temp = { inters, vec3_dist(*camera, inters) };
        add_intersection_tolist( intersections_root, this_line_obj, temp );
        return temp;
    }*/
    return (intersect_resultset){ (vec3){ INF, INF, INF }, INF };
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
            while (aux -> next != NULL &&
                   !(
                     (intersect.distance < (aux -> next -> intersection).distance)
                     &&
                     (intersect.distance >= (aux -> intersection).distance)
                     )
                   )
                aux = aux -> next;

            if (aux -> next == NULL) {
                aux -> next = (intersects_llnode *)malloc(sizeof(intersects_llnode));
                aux = aux -> next;
                aux -> associated_object = obj;
                aux -> intersection = intersect;
                aux -> next = NULL;
            } else {
                intersects_llnode *aux2 = (intersects_llnode *)malloc(sizeof(intersects_llnode));
                aux2 -> associated_object = obj;
                aux2 -> intersection = intersect;
                aux2 -> next = aux -> next;
                aux -> next = aux2;
            }
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

