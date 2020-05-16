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

void add_sphere(drawable_obj_llnode **root, vec3 center, double radius, uint8_t r, uint8_t g, uint8_t b, double alpha, uint8_t real) {
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
    (*drw_sphere) = { sphere, known_drawable_objs::sphere, real, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_sphere);
}

void add_plane_3_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, vec3 point3, uint8_t r, uint8_t g, uint8_t b, double alpha, uint8_t real) {
//void add_plane_3_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, vec3 point3, pixel_color_rgba rgba) {
    // Create plane in memory
    obj_plane *plane = (obj_plane*)malloc(sizeof(obj_plane));
    // Fill details about plane
    plane -> point = point1;
    plane -> normal = vec3_cross_product( vec3_sub( point2, point1 ), vec3_sub( point3, point1 ) );
    // Create drawable abstraction
    drawable_obj *drw_plane = (drawable_obj*)malloc(sizeof(drawable_obj));
    // Fill details about drawable
    (*drw_plane) = { plane, known_drawable_objs::plane, real, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_plane);
}

void add_line_2_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, double radius, uint8_t r, uint8_t g, uint8_t b, double alpha, uint8_t real) {
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
    (*drw_line) = { line, known_drawable_objs::line, real, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_line);
}

void add_triangle(drawable_obj_llnode **root, vec3 point1, vec3 point2, vec3 point3, uint8_t r, uint8_t g, uint8_t b, double alpha, uint8_t real) {
//void add_plane_3_points(drawable_obj_llnode **root, vec3 point1, vec3 point2, vec3 point3, pixel_color_rgba rgba) {
    // Create triangle in memory
    obj_triangle *triangle = (obj_triangle*)malloc(sizeof(obj_triangle));
    // Fill details about plane
    triangle -> vert1 = point1;
    triangle -> vert2 = point2;
    triangle -> vert3 = point3;
    // Create drawable abstraction
    drawable_obj *drw_triangle = (drawable_obj*)malloc(sizeof(drawable_obj));
    // Fill details about drawable
    (*drw_triangle) = { triangle, known_drawable_objs::triangle, real, (pixel_color){r,g,b}, alpha };
    add_drawable_tolist(root, drw_triangle);
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
    char model_directory[30], model_line_mode;
    model_vertices_llnode *model_root, **model_aux, *model_fetch;
    model_root = NULL;
    model_aux = &model_root;
    FILE *model_file;
    int model_v1, model_v2, model_v3, model_iterator;
    int helper;
    vec3 min_bounding, max_bounding;
    int realAux;
    while (!feof(fd)) {
        //if (temp != (char) 1) ungetc(temp, fd);
        fscanf(fd, "%d %d %d %d %lf %d ", (int*)&current_obj_type, &r, &g, &b, &a, &realAux);
        //fscanf(fd, "");
        switch (current_obj_type) {
        case sphere:
            fscanf(fd, "%lf %lf %lf %lf ", &(point1.x), &(point1.y), &(point1.z), &radius);
            add_sphere(root, point1, radius, (uint8_t)r, (uint8_t)g, (uint8_t)b, a, (uint8_t)realAux);
            //printf("Read sphere: (%.2f %.2f %.2f) - %.2f - %d %d %d %.2f\n", point1.x, point1.y, point1.z, radius, r, g, b, a);
            read_objs++;
            break;
        case plane:
            fscanf(fd, "%lf %lf %lf %lf %lf %lf %lf %lf %lf ", &point1.x, &point1.y, &point1.z, &point2.x, &point2.y, &point2.z, &point3.x, &point3.y, &point3.z);
            add_plane_3_points(root, point1, point2, point3, (uint8_t)r, (uint8_t)g, (uint8_t)b, a, (uint8_t)realAux);
            printf("Read plane\n");
            read_objs++;
            break;
        case line:
            fscanf(fd, "%lf %lf %lf %lf %lf %lf %lf ", &point1.x, &point1.y, &point1.z, &point2.x, &point2.y, &point2.z, &radius);
            add_line_2_points(root, point1, point2, radius, (uint8_t)r, (uint8_t)g, (uint8_t)b, a, (uint8_t)realAux);
            //printf("Read line\n");
            read_objs++;
            break;
        case triangle:
            fscanf(fd, "%lf %lf %lf %lf %lf %lf %lf %lf %lf ", &point1.x, &point1.y, &point1.z, &point2.x, &point2.y, &point2.z, &point3.x, &point3.y, &point3.z);
            add_triangle(root, point1, point2, point3, (uint8_t)r, (uint8_t)g, (uint8_t)b, a, (uint8_t)realAux);
            printf("Read triangle\n");
            read_objs++;
            break;
        case model:
            fscanf(fd, "%s", model_directory);
            printf("Loading model @ %s...\n", model_directory);
            model_file = fopen(model_directory, "r");
            if (!model_file) { printf("File not found!\n"); break; }
            //printf("Opened file!\n");
            min_bounding = {0.0, 0.0, 0.0};
            max_bounding = {0.0, 0.0, 0.0};
            while (!feof(model_file)) {
                fscanf(model_file, "%c ", &model_line_mode);
                //printf("C = (%c)", model_line_mode);
                switch (model_line_mode) {
                case 'v': // Add vertices to a linked list
                    //printf("Reading vertice\n");
                    (*model_aux) = (model_vertices_llnode*)malloc(sizeof(model_vertices_llnode));
                    fscanf(model_file, "%lf %lf %lf ", &(((*model_aux)->vert).x), &(((*model_aux)->vert).y), &(((*model_aux)->vert).z));
                    ((*model_aux)->vert).x *= 30;
                    ((*model_aux)->vert).y *= 30;
                    ((*model_aux)->vert).z *= 30;
                    //printf("Vertice = (%.2f %.2f %.2f)\n", (*model_aux)->vert.x, (*model_aux)->vert.y, (*model_aux)->vert.z);
                    // Keep track of bounding box
                    if ((*model_aux)->vert.x < min_bounding.x) min_bounding.x = (*model_aux)->vert.x;
                    if ((*model_aux)->vert.y < min_bounding.y) min_bounding.y = (*model_aux)->vert.y;
                    if ((*model_aux)->vert.z < min_bounding.z) min_bounding.z = (*model_aux)->vert.z;
                    if ((*model_aux)->vert.x > max_bounding.x) max_bounding.x = (*model_aux)->vert.x;
                    if ((*model_aux)->vert.y > max_bounding.y) max_bounding.y = (*model_aux)->vert.y;
                    if ((*model_aux)->vert.z > max_bounding.z) max_bounding.z = (*model_aux)->vert.z;
                    (*model_aux)->next = NULL;
                    model_aux = &((*model_aux)->next);
                    //fgetc(model_file);
                    //exit(0);
                    break;
                case 'f': // Fetch vertices and create a triangle with them
                    //printf("Reading face\n");
                    fscanf(model_file, "%d %d %d ", &model_v1, &model_v2, &model_v3);
                    if (read_objs >= MAX_FACES_PERMODEL) break;
                    //printf("Face contains vertices %d, %d & %d\n", model_v1, model_v2, model_v3);
                    model_fetch = model_root;
                    for (model_iterator = 1; model_iterator < model_v1; model_iterator++) model_fetch = model_fetch->next;
                    point1 = model_fetch->vert;
                    model_fetch = model_root;
                    for (model_iterator = 1; model_iterator < model_v2; model_iterator++) model_fetch = model_fetch->next;
                    point2 = model_fetch->vert;
                    model_fetch = model_root;
                    for (model_iterator = 1; model_iterator < model_v3; model_iterator++) model_fetch = model_fetch->next;
                    point3 = model_fetch->vert;
                    add_triangle(root, point1, point2, point3, (uint8_t)r, (uint8_t)g, (uint8_t)b, a, (uint8_t)realAux);
                    read_objs++;
                    break;
                default: //Pass (probably a comment or something)
                    printf("Read line starting with (%c)\n", model_line_mode);

                    while (model_line_mode != '\n') fscanf(model_file, "%c", &model_line_mode);
                    //ungetc(model_line_mode, model_file);
                    break;
                }
            }
            printf("Bounding box: (%.2f %.2f %.2f) x (%.2f %.2f %.2f)\n", min_bounding.x, min_bounding.y, min_bounding.z, max_bounding.x, max_bounding.y, max_bounding.z);
            fclose(model_file);
            fscanf(fd, "%c ", &model_line_mode); //Getting rid of garbage
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
        double angle1 = acos( vec3_dot_product( vec3_sub(this_sphere->center, c), vec3_sub(*camera, c) ) / (vec3_len(vec3_sub(this_sphere->center, c)) * vec3_len(vec3_sub(*camera, c))));
        temp1 = (intersect_resultset){ c, vec3_len(vec3_sub(*camera, c)), abs(angle1) }; // TODO find angle of intersect
        add_intersection_tolist(intersections_root, this_sphere_obj, temp1);
        return temp1;
    }
    // Else it crosses through 2 points, find out the closest one
    double u1 = (-1 * b + sqrt(delta))/(2 * a);
    double u2 = (-1 * b - sqrt(delta))/(2 * a);
    vec3 c1 = {camera->x + u1*vx_m_cx, camera->y + u1*vy_m_cy, camera->z + u1*vz_m_cz}; // Get both points
    vec3 c2 = {camera->x + u2*vx_m_cx, camera->y + u2*vy_m_cy, camera->z + u2*vz_m_cz};
    double angle1;
    double angle2;
    if (this_sphere_obj->real) {
        angle1 = acos( vec3_dot_product( vec3_sub(this_sphere->center, c1), vec3_sub(*camera, c1) ) / (vec3_len(vec3_sub(this_sphere->center, c1)) * vec3_len(vec3_sub(*camera, c1))));
        angle2 = acos( vec3_dot_product( vec3_sub(this_sphere->center, c2), vec3_sub(*camera, c2) ) / (vec3_len(vec3_sub(this_sphere->center, c2)) * vec3_len(vec3_sub(*camera, c2))));
        if (angle1 > PI / 2.0) angle1 = PI - angle1;
        if (angle2 > PI / 2.0) angle2 = PI - angle2;
    } else {
        angle1 = 0.0;
        angle2 = 0.0;
    }
    temp1 = (intersect_resultset){ c1, vec3_dist( *camera, c1 ), abs(angle1) }; // TODO find angle of intersect
    temp2 = (intersect_resultset){ c2, vec3_dist( *camera, c2 ), abs(angle2) }; // TODO find angle of intersect
    add_intersection_tolist(intersections_root, this_sphere_obj, temp1);
    add_intersection_tolist(intersections_root, this_sphere_obj, temp2);
    if ( vec3_dist( *camera, c1 ) < vec3_dist( *camera, c2 ) ) return temp1;
    return temp2;
}

intersect_resultset chk_intersect_plane (vec3 *camera, vec3 viewpoint, drawable_obj *this_plane_obj,  intersects_llnode **intersections_root) {
    obj_plane *this_plane = (obj_plane*)this_plane_obj->object;
    vec3 u = vec3_sub(*camera, viewpoint);
    vec3 w = vec3_sub(this_plane -> point, *camera);
    double d = vec3_dot_product(this_plane -> normal, u);
    if (d == 0) return (intersect_resultset){ (vec3){ INF, INF, INF }, INF };
    double s = -1.0 * vec3_dot_product(this_plane -> normal, w) / d;
    //if ((s > 0 && s <= 1)) { // Then add to intersection list
    if (s > 0) {
        vec3 inters = vec3_add( *camera, vec3_multi_r( u, s ) );
        double angle = acos( d / (vec3_len(this_plane->normal) * vec3_len(u)));
        intersect_resultset temp = { inters, vec3_dist( *camera, inters ), angle };
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
    if (dist <= (this_line -> radius + LINE_RADIUS_THRESHOLD)) {
        vec3 cam_inters = vec3_sub(*camera, c1);
        double dotp = vec3_dot_product(cam_inters, ray_director); // Prevent rays from looping back
        if (dotp > 0) {
        //double theta = acos(dotp/(vec3_len(cam_inters) * vec3_len(ray_director)));
        //if (abs(theta) < PI / 2.0) {
            intersect_resultset temp = { c1, c_dist, 0.0 }; // TODO find angle of intersect
            add_intersection_tolist( intersections_root, this_line_obj, temp );
            return temp;
        }
    }
    return (intersect_resultset){ (vec3){ INF, INF, INF }, INF };
}

intersect_resultset chk_intersect_triangle (vec3 *camera, vec3 viewpoint, drawable_obj *this_triangle_obj, intersects_llnode **intersections_root) {
    obj_triangle *this_triangle = (obj_triangle*)this_triangle_obj->object;
    //Similar to plane intersection
    vec3 triangle_normal = vec3_cross_product(vec3_sub(this_triangle->vert1, this_triangle->vert2), vec3_sub(this_triangle->vert1, this_triangle->vert3));
    vec3 raycast_director = vec3_sub(*camera, viewpoint);
    vec3 w = vec3_sub(this_triangle->vert1, *camera);
    double d = vec3_dot_product(triangle_normal, raycast_director);
    if (d == 0) return (intersect_resultset){ (vec3){ INF, INF, INF }, INF };
    double s = -1.0 * vec3_dot_product(triangle_normal, w) / d;
    if (s > 0) {
        vec3 inters = vec3_add( *camera, vec3_multi_r( raycast_director, s ) );
        // Now check if inters is inside the triangle
        vec3 v1 = vec3_sub(inters, this_triangle->vert1);
        vec3 v2 = vec3_sub(inters, this_triangle->vert2);
        vec3 v3 = vec3_sub(inters, this_triangle->vert3);
        double l1 = vec3_len(v1);
        double l2 = vec3_len(v2);
        double l3 = vec3_len(v3);
        double theta12 = acos( vec3_dot_product(v1, v2) / (l1 * l2));
        double theta13 = acos( vec3_dot_product(v1, v3) / (l1 * l3));
        double theta23 = acos( vec3_dot_product(v2, v3) / (l2 * l3));
        if ( abs(theta12 + theta13 + theta23 - (2.0 * PI)) <= TRIANGLE_ANGLE_THRESHOLD ) {
            double angle = acos( d / (vec3_len(triangle_normal) * vec3_len(raycast_director)));
            intersect_resultset temp = { inters, vec3_dist( *camera, inters ), angle };
            add_intersection_tolist( intersections_root, this_triangle_obj, temp );
            return temp;
        } // Else it misses (outside of triangle)
    }
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

int free_model_vertices_linkedlist(model_vertices_llnode **root) {
    model_vertices_llnode *aux1, *aux2;
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

pixel_color calculate_intersection_results(intersects_llnode *root, uint8_t bgr, uint8_t bgg, uint8_t bgb) {
    if (root == NULL) {
            //printf("Found end of list, returning...\n");
            return (pixel_color){bgr, bgg, bgb};
    }
    pixel_color combination = calculate_intersection_results(root->next, bgr, bgg, bgb);
    //printf("Merging with current intersection...\n");
    pixel_color this_color = root->associated_object->texture_color;
    pixel_color result;
    double current_alpha = root->associated_object->texture_alpha;
    result.r = ( (double)this_color.r * current_alpha ) * ((PI / 2.0) - root->intersection.angle) / (PI / 2.0)+ ( (double)combination.r * ( 1.0 - current_alpha ) );
    result.g = ( (double)this_color.g * current_alpha ) * ((PI / 2.0) - root->intersection.angle) / (PI / 2.0) + ( (double)combination.g * ( 1.0 - current_alpha ) );
    result.b = ( (double)this_color.b * current_alpha ) * ((PI / 2.0) - root->intersection.angle) / (PI / 2.0) + ( (double)combination.b * ( 1.0 - current_alpha ) );
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