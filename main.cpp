#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <cmath>

#include "vec3_lib.h"
#include "raycast.h"

using namespace std;

#define B_X 640
#define B_Y 480

//typedef double angle;

/*
struct vec3_t {
    double x, y, z;
};
typedef struct vec3_t vec3;
*/
/*
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
*/
//struct drawable

/*
double vec3_len(vec3 vect);
vec3 vec3_add(vec3 start, vec3 end);
vec3 vec3_sub(vec3 start, vec3 end);
vec3 vec3_multi_r(vec3 vect, double k);
vec3 vec3_div_r(vec3 vect, double k);
vec3 vec3_cross_product(vec3 a, vec3 b);
vec3 vec3_rotate_normal(vec3 vect, vec3 normal, double angle);
vec3 vec3_unit(vec3 vect);
*/

/*
void add_drawable_tolist(drawable_obj_llnode **root, drawable_obj *drw_obj);

void add_sphere(drawable_obj_llnode **root, vec3 center, double radius, byte r, byte g, byte b);

intersect_resultset chk_intersect_sphere(vec3 *camera, vec3 viewpoint, obj_sphere *this_sphere);
*/

#define DRAW
//#define DEBUG

int main()
{
    //Get a console handle
    HWND myconsole = GetConsoleWindow();
    //Get a handle to device context
    HDC mydc = GetDC(myconsole);

    //DOnt ask
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SMALL_RECT windowSize = {0, 0, (int)(B_X/KX), (int)(B_Y/KY)};

	if(!SetConsoleWindowInfo(hConsole, TRUE, &windowSize))
	{
		cout << "SetConsoleWindowInfo failed with error " << GetLastError() << endl;
		return -1;
	}

	if(!SetConsoleTitle("A E S T H E T I C"))
	{
		cout << "SetConsoleTitle failed with error " << GetLastError() << endl;
		return -1;
	}
    //End dont ask

    // ============================= Start of raycasting =============================
    //Settings
    double field_of_view = 10;
    vec3 viewfinder_size = { B_X/10, B_Y/10, 0 };

    //Background default color
    COLORREF bg_color = RGB(0,255,0);

    //Camera vector, always centered on its "own" X axis
    obj_camera camera = {
        (vec3){0, 0, 15},  //anchor
        deg_to_rad(00.0),  //roll
        deg_to_rad(90.0),  //pitch
        deg_to_rad(00.0)   //yaw
    };
    //Camera starts pointing straight towards +X axis (1,0,0)

    //Drawable object linked list
    drawable_obj_llnode *drawables_root = NULL;

    //Calculating roll, pitch and yaw
    camera.director_x = {1,0,0};
    camera.director_y = {0,1,0};
    camera.director_z = {0,0,1};
    //Calculate roll  (rotating along camera.director_x)
    camera.director_y = vec3_rotate_normal(camera.director_y, camera.director_x, camera.roll);
    camera.director_z = vec3_rotate_normal(camera.director_z, camera.director_x, camera.roll);
    //Calculate pitch (rotating along camera.director_y)
    camera.director_x = vec3_rotate_normal(camera.director_x, camera.director_y, camera.pitch);
    camera.director_z = vec3_rotate_normal(camera.director_z, camera.director_y, camera.pitch);
    //Calculate yaw   (rotating along camera.director_z)
    camera.director_x = vec3_rotate_normal(camera.director_x, camera.director_z, camera.yaw);
    camera.director_y = vec3_rotate_normal(camera.director_y, camera.director_z, camera.yaw);

    printf("Camera directors after transformations:\n");
    printf("X: (%.2f, %.2f, %.2f) - len = %.2f\n", camera.director_x.x, camera.director_x.y, camera.director_x.z, vec3_len(camera.director_x));
    printf("Y: (%.2f, %.2f, %.2f) - len = %.2f\n", camera.director_y.x, camera.director_y.y, camera.director_y.z, vec3_len(camera.director_y));
    printf("Z: (%.2f, %.2f, %.2f) - len = %.2f\n", camera.director_z.x, camera.director_z.y, camera.director_z.z, vec3_len(camera.director_z));

    //while (1) {}
    /*
    vec3 camera = { 0, 0, 10 };
    vec3 camera_helper = {10, 10, 10};
    vec3 camera_dir = vec3_sub(camera, camera_helper);
         camera_dir = vec3_unit(camera_dir);
    */
    //camera_dir = vec3_div_r(camera_dir, vec3_len(camera_dir));
    //printf("Camera director len (should be 1): %.2f\n", vec3_len(camera_dir));

    //Anchor the viewfinder via FOV
    //vec3 viewfinder_midpointer = vec3_multi_r(camera_dir, field_of_view);
    //vec3 viewfinder_anchor = vec3_add(camera, viewfinder_midpointer);
    vec3 viewfinder_midpointer = vec3_multi_r( camera.director_x, field_of_view );
    printf("Viewfinder midpointer: (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_midpointer.x, viewfinder_midpointer.y, viewfinder_midpointer.z, vec3_len(viewfinder_midpointer));

    //Calculating viewfinder versors
    //First rotation
    angle alpha_rot = atan( viewfinder_size.x / ( 2.0 * field_of_view ) );
    vec3  viewfinder_helper = vec3_div_r( vec3_rotate_normal( viewfinder_midpointer, camera.director_z, -alpha_rot  ), cos(alpha_rot) );
    vec3  viewfinder_step_x = vec3_div_r( vec3_sub( viewfinder_helper, viewfinder_midpointer ), (double)B_X / 2.0 );
          alpha_rot = atan( viewfinder_size.y / ( 2.0 * field_of_view ) );
          viewfinder_helper = vec3_div_r( vec3_rotate_normal( viewfinder_midpointer, camera.director_y, alpha_rot ), cos(alpha_rot) );
    vec3  viewfinder_step_y = vec3_div_r( vec3_sub( viewfinder_helper, viewfinder_midpointer ), (double)B_Y / 2.0 );

    vec3  viewfinder_origin = vec3_add(camera.anchor,
                                       vec3_sub(
                                                viewfinder_midpointer,
                                                vec3_add(
                                                         vec3_multi_r( viewfinder_step_x, -(double)B_X / 2.0 ),
                                                         vec3_multi_r( viewfinder_step_y, -(double)B_Y / 2.0 )
                                                    )
                                                )
                                       );
    printf("Viewfinder origin: (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_origin.x, viewfinder_origin.y, viewfinder_origin.z, vec3_len(viewfinder_origin));
    //printf("Viewfinder middle w origin: (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_midpointer.x, viewfinder_midpointer.y, viewfinder_midpointer.z, vec3_len(viewfinder_midpointer));

    //while(1){}
    /*
    //First rotation
    double alpha_rot = atan(viewfinder_size.x / ( 2.0 * field_of_view ));// * 180.0 / PI;
    vec3 viewfinder_helper1 = vec3_div_r(vec3_rotate_normal(viewfinder_midpointer, {0, 0, 1}, alpha_rot), cos(alpha_rot));
    // viewfinder_step_x = vec3_div_r(vec3_multi_r(vec3_sub(viewfinder_helper1, viewfinder_midpointer), 2.0), (double)B_X);
    vec3 viewfinder_step_x = vec3_div_r(vec3_sub(viewfinder_helper1, viewfinder_midpointer), (double)B_X / 2.0);
    //vec3 viewfinder_helper2 = vec3_div_r(vec3_rotate_normal(viewfinder_midpointer, {0, 0, 1}, -1.0 * alpha_rot), cos(alpha_rot));
    vec3 viewfinder_helper_norm = vec3_rotate_normal(viewfinder_helper1, {0, 0, 1}, -PI/2.0);
         viewfinder_helper_norm = vec3_unit(viewfinder_helper_norm);
         //viewfinder_helper_norm = vec3_div_r(viewfinder_helper_norm, vec3_len(viewfinder_helper_norm)); //Normalizing vector to size 1
    //Second rotation
    double newlen = vec3_len(vec3_sub(camera, viewfinder_helper1));
    double beta_rot = atan(viewfinder_size.y / ( 2.0 * newlen ));// * 180.0 / PI;
    //double beta_rot = atan(viewfinder_size.y / ( 2.0 * sqrt(sqr(field_of_view) + sqr(viewfinder_size.x/2.0)) )) * 180.0 / PI;
    vec3 viewfinder_origin_raw = vec3_div_r(vec3_rotate_normal(viewfinder_helper1, viewfinder_helper_norm, beta_rot), cos(beta_rot));
    vec3 viewfinder_origin = vec3_add(camera, viewfinder_origin_raw);

    // vec3 viewfinder_step_y = vec3_div_r(vec3_multi_r(vec3_sub(viewfinder_origin_raw, viewfinder_helper1), 2.0), (double)B_Y);
    vec3 viewfinder_step_y = vec3_div_r(vec3_sub(viewfinder_origin_raw, viewfinder_helper1), (double)B_Y / 2.0);
    */

    //Spheres
    add_sphere(&drawables_root, (vec3){0,0,0}, 0.5, 255,0,0);
    add_sphere(&drawables_root, (vec3){10,0,0}, 0.5, 0,0,255);
    //add_sphere(&drawables_root, (vec3){0,1,10}, 0.5, 255,0,0);
    //add_sphere(&drawables_root, (vec3){0,10,10}, 0.5, 0,0,255);
    //add_sphere(&drawables_root, (vec3){10,10,10}, 0.5, 0,0,255);
    /*
    obj_sphere sphere1 = { { 0,0,0 }, 0.5 };
    drawable_obj drw_sphere1 = { &sphere1, known_drawable_objs::sphere, RGB(255,0,0) };
    add_drawable_tolist(&drawables_root, &drw_sphere1);
    obj_sphere sphere2 = { { 1,0,0 }, 0.5 };
    drawable_obj drw_sphere2 = { &sphere2, known_drawable_objs::sphere, RGB(0,0,255) };
    add_drawable_tolist(&drawables_root, &drw_sphere2);
    obj_sphere sphere3 = { { 0,1,0 }, 0.5 };
    drawable_obj drw_sphere3 = { &sphere3, known_drawable_objs::sphere, RGB(255,0,255) };
    add_drawable_tolist(&drawables_root, &drw_sphere3);
    */
    /*obj_sphere sphere2 = { { 16.0, 20.0, 10.0 }, 9.0 };
    drawable_obj drw_sphere2 = { &sphere2, known_drawable_objs::sphere, RGB(0,0,255) };
    add_drawable_tolist(&drawables_root, &drw_sphere2);
    obj_sphere sphere3 = { { 20.0, 20.0, 15.0 }, 10.0 };
    drawable_obj drw_sphere3 = { &sphere3, known_drawable_objs::sphere, RGB(255,127,255) };
    add_drawable_tolist(&drawables_root, &drw_sphere3);
    */

    intersect_resultset current_closest, current_candidate;
    drawable_obj_llnode *drawables_list_aux = NULL;
    drawable_obj *drawable_ptr = NULL, *draw_target = NULL;
    vec3 current_viewpoint;
    int x, y;
    for (x = 0; x < B_X; x++) {
        for (y = 0; y < B_Y; y++) {
            //Calculate current viewpoint
            current_viewpoint = vec3_add(viewfinder_origin, vec3_add(vec3_multi_r(viewfinder_step_x, (double)x), vec3_multi_r(viewfinder_step_y, (double)y)));
            //Find closest object in list
            current_closest = { {INF, INF, INF}, INF };
            drawables_list_aux = drawables_root;
            draw_target = NULL;
            while (drawables_list_aux != NULL) {
                drawable_ptr = drawables_list_aux->drawable;
                switch (drawable_ptr->object_type) {
                case sphere:
                    current_candidate = chk_intersect_sphere(&(camera.anchor), current_viewpoint, (obj_sphere*)drawable_ptr->object);
                    break;
                default:
                    printf("Something went wrong\n");
                    break;
                }
                //printf("Current candidate = (%.2f, %.2f, %.2f), dist = %.2f\n", current_candidate.intersect_point.x, current_candidate.intersect_point.y, current_candidate.intersect_point.z, current_candidate.distance);
                if (current_candidate.distance < current_closest.distance) {
                    current_closest = current_candidate;
                    draw_target = drawable_ptr;
                }
                drawables_list_aux = drawables_list_aux -> next;
            }
#ifdef DRAW
            if (current_closest.distance == INF || draw_target == NULL) {
                SetPixel(mydc, x, y, bg_color);
            } else {
                SetPixel(mydc, x, y, draw_target->texture_color);
            }
#endif
        }
    }

    while(1) {}

    ReleaseDC(myconsole, mydc);
    cin.ignore();
    return 0;
}

//Functions

/*
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
*/
/*
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
*/
