#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <GL/glut.h>

#include "vec3_lib.h"
#include "raycast.h"
#include "bazinho.h"

#define B_X 640
#define B_Y 480

#define THREAD_COUNT_HORZ 16
#define THREAD_COUNT_VERT 12

#define THREAD_CANVAS_WIDTH  B_X/THREAD_COUNT_HORZ
#define THREAD_CANVAS_HEIGHT B_Y/THREAD_COUNT_VERT

#define BY_PIXEL 1

#define DRAW
//#define USE_THREADS

//#define SEM_KEY 0x1234
//#define DEBUG

obj_camera camera;
drawable_obj_llnode *drawables_root = NULL;
vec3 viewfinder_corner;
pixel_color bg_pixel = {0,0,0};

GLFWwindow* window;

GLubyte window_bitmap[B_Y][B_X][3] = {0x00};

struct cartesian_t {
    int x, y;
};

void trace_for_pixel(int x, int y);

void init(void)
{  
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_FLAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void display(void)
{
    while (1) {
        glClear(GL_COLOR_BUFFER_BIT);
        glRasterPos2i(-1, -1);
        glDrawPixels(B_X, B_Y, GL_RGB,
                        GL_UNSIGNED_BYTE, window_bitmap);
        glFlush();
    }
}

void *thread_loop(void* arg) {
    glutDisplayFunc(display);
    glutMainLoop();
}

void *artistThread(void *arg) {
    cartesian_t bounds = *(cartesian_t *)arg;
    intersect_resultset current_closest, current_candidate;
    drawable_obj_llnode *drawables_list_aux = NULL;
    drawable_obj *drawable_ptr = NULL;//, *draw_target = NULL;
    intersects_llnode *intersections_root = NULL;
    vec3 current_viewpoint;
    vec3 viewfinder_size = { B_X/10, B_Y/10, 0 };
//#ifdef DEBUG
    int freed_intersect_nodes;
//#endif
    int x, y, vx, vy;
    pixel_color current_pixel;
    //printf("Thread (%d, %d) starting\n", bounds.x, bounds.y);
    int upper_x = fmin(bounds.x + THREAD_CANVAS_WIDTH,  B_X);
    int upper_y = fmin(bounds.y + THREAD_CANVAS_HEIGHT, B_Y);
    for (x = bounds.x; x < upper_x; x++) {
        for (y = bounds.y; y < upper_y; y++) {
            vx = BY_PIXEL * (int)(x / BY_PIXEL);
            vy = BY_PIXEL * (int)(y / BY_PIXEL);
            //Calculate current viewpoint
            current_viewpoint = vec3_add( viewfinder_corner, vec3_add( vec3_multi_r( camera.director_y, -1.0 * (double)vx * viewfinder_size.x / (double)B_X ),vec3_multi_r( camera.director_z, -1.0 * (double)vy * viewfinder_size.y / (double)B_Y ) ) );
            current_closest = { {INF, INF, INF}, INF };
            drawables_list_aux = drawables_root;
            while (drawables_list_aux != NULL) {
                drawable_ptr = drawables_list_aux->drawable;
                switch (drawable_ptr->object_type) {
                case sphere:
                    chk_intersect_sphere   (&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
                    break;
                case plane:
                    chk_intersect_plane    (&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
                    break;
                case line:
                    chk_intersect_line     (&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
                    break;
                case triangle:
                    chk_intersect_triangle (&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
                    break;
                default:
                    printf("Something went wrong\n");
                    break;
                }
                drawables_list_aux = drawables_list_aux -> next;
            }
            current_pixel = calculate_intersection_results(intersections_root, bg_pixel.r, bg_pixel.g, bg_pixel.b);
            window_bitmap[B_Y-y][x][0] = current_pixel.r;
            window_bitmap[B_Y-y][x][1] = current_pixel.g;
            window_bitmap[B_Y-y][x][2] = current_pixel.b;
            free_intersection_linkedlist(&intersections_root);
        }
    }
    //printf("Thread (%d, %d) finished\n", bounds.x, bounds.y);
    int *res =  (int *)malloc(sizeof(int));
    *res = 0;
    pthread_exit(res);
}

pthread_t windowThread;

int main(int argc, char *args[])
{
    printf("Starting...\n");

    glutInit(&argc, args);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(B_X, B_Y);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("A E S T H E T I C");
    init();

    // ============================= Start of raycasting =============================
    //Settings
    double field_of_view = 100;
    vec3 viewfinder_size = { B_X/10, B_Y/10, 0 };

    //Background default color
    //pixel_color bg_pixel = {0,0,0};
    //COLORREF bg_color = RGB(bg_pixel.r,bg_pixel.g,bg_pixel.b);

    //Camera vector, always centered on its "own" X axis
    camera = {
        (vec3){-300, 0, 10},  //anchor
        deg_to_rad(00.0),  //roll
        deg_to_rad(00.0),  //pitch
        deg_to_rad(00.0)   //yaw
    };
    //Camera starts pointing straight towards +X axis (1,0,0)

    //Drawable object linked list
    intersects_llnode *intersections_root = NULL;

    int objs = load_shapes_from_file("scenes/vaporwave.txt", &drawables_root);
    printf("Loaded %d objects\n", objs);

    //add_triangle(&drawables_root, (vec3){0,-20,0}, (vec3){0,20,0}, (vec3){0,0,50}, 255, 204, 255,1);

    //do {
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

    //Anchor the viewfinder via FOV
    vec3 viewfinder_midpointer = vec3_multi_r( camera.director_x, field_of_view );
    printf("Viewfinder midpointer (from origin): (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_midpointer.x, viewfinder_midpointer.y, viewfinder_midpointer.z, vec3_len( viewfinder_midpointer ));

    //Calculating viewfinder versors
    vec3 viewfinder_helper_x = vec3_multi_r( camera.director_y, .5 * (double)viewfinder_size.x );
    vec3 viewfinder_helper_y = vec3_multi_r( camera.director_z, .5 * (double)viewfinder_size.y );
    vec3 viewfinder_corner_fromorigin = vec3_add( viewfinder_midpointer, vec3_add( viewfinder_helper_x, viewfinder_helper_y ) );

    viewfinder_corner = vec3_add( camera.anchor, viewfinder_corner_fromorigin );
    printf("Viewfinder corner: (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_corner.x, viewfinder_corner.y, viewfinder_corner.z, vec3_len( viewfinder_corner ));
    vec3 debug_corner = vec3_add( viewfinder_corner, vec3_add( vec3_multi_r( camera.director_y, -1.0 * (double)B_X * viewfinder_size.x / (double)B_X ),vec3_multi_r( camera.director_z, -1.0 * (double)B_Y * viewfinder_size.y / (double)B_Y ) ) );
    printf("Viewfinder max corner: (%.2f, %.2f, %.2f) - len = %.2f\n", debug_corner.x, debug_corner.y, debug_corner.z, vec3_len( debug_corner ));

    //add_sphere(&drawables_root, (vec3){10,10,10}, 0.5, 0,0,255, 1);

    cartesian_t threadArgs[THREAD_COUNT_HORZ * THREAD_COUNT_VERT];
    pthread_t artistThreads[THREAD_COUNT_HORZ * THREAD_COUNT_VERT];
    printf("Done calculating, spawning threads...\n");
    int x, y, threadStatus;
    for (x = 0; x < THREAD_COUNT_HORZ; x++) {
        for (y = 0; y < THREAD_COUNT_VERT; y++) {
            //printf("Creating %d,%d (%d)\n", x, y, x * THREAD_COUNT_HORZ + y);
            threadArgs[x * THREAD_COUNT_HORZ + y] = { x * THREAD_CANVAS_WIDTH, y * THREAD_CANVAS_HEIGHT };
            threadStatus = pthread_create(&artistThreads[x * THREAD_COUNT_HORZ + y], NULL, artistThread, (void*)&threadArgs[x * THREAD_COUNT_HORZ + y]);
            if (threadStatus != 0) {
                printf("pthread_create exited with status %d\n", threadStatus);
                exit(threadStatus);
            }
        }
    }
    printf("Done spawning threads\n");

    pthread_create(&windowThread, NULL, thread_loop, (void*)NULL);

    scanf("%d", &x);

    // SDL_RenderPresent(renderer);
    printf("Done!\n");
    //} while (command != 'x');

    // while(1) {
    //     if ( SDL_PollEvent(&event) && event.type == SDL_QUIT ) {
    //         SDL_DestroyRenderer( renderer );
    //         SDL_DestroyWindow( window );
    //         SDL_Quit();
    //         exit(0);
    //         }//break;
    // }

    // SDL_DestroyRenderer( renderer );
    // SDL_DestroyWindow( window );

    //Quit SDL subsystems
    // SDL_Quit();


    //ReleaseDC(myconsole, mydc);
    //cin.ignore();
    return 0;
}