#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <cmath>
#include <unistd.h>

#include <SDL.h>

#include "vec3_lib.h"
#include "raycast.h"

using namespace std;

#define B_X 640
#define B_Y 480

#define DRAW
//#define DEBUG

int main(int argc, char *args[])
{
    SDL_Window *window = NULL;
    //SDL_Surface *screenSurface = NULL;
    SDL_Renderer *renderer;
    SDL_Event event;
    if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        cout << "SDL could not start, error " << SDL_GetError() << endl;
        exit(1);
    }
    SDL_CreateWindowAndRenderer(B_X, B_Y, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0,0,0,0);
    SDL_RenderClear(renderer);
    cout << "Cleared renderer" << endl;

    // ============================= Start of raycasting =============================
    //Settings
    double field_of_view = 30;
    vec3 viewfinder_size = { B_X/10, B_Y/10, 0 };

    //Background default color
    pixel_color bg_pixel = {0,255,0};
    //COLORREF bg_color = RGB(bg_pixel.r,bg_pixel.g,bg_pixel.b);

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
    intersects_llnode *intersections_root = NULL;

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
    printf("Viewfinder midpointer: (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_midpointer.x, viewfinder_midpointer.y, viewfinder_midpointer.z, vec3_len(viewfinder_midpointer));

    //Calculating viewfinder versors
    angle alpha_rot = atan( viewfinder_size.x / ( 2.0 * field_of_view ) );
    vec3  viewfinder_helper = vec3_div_r( vec3_rotate_normal( viewfinder_midpointer, camera.director_z, -alpha_rot  ), cos(alpha_rot) );
    vec3  viewfinder_step_x = vec3_div_r( vec3_sub( viewfinder_helper, viewfinder_midpointer ), (double)B_X / 2.0 );
          alpha_rot = atan( viewfinder_size.y / ( 2.0 * field_of_view ) );
          viewfinder_helper = vec3_div_r( vec3_rotate_normal( viewfinder_midpointer, camera.director_y, alpha_rot ), cos(alpha_rot) );
    vec3  viewfinder_step_y = vec3_div_r( vec3_sub( viewfinder_helper, viewfinder_midpointer ), (double)B_Y / 2.0 );
    vec3  viewfinder_origin = //vec3_add(camera.anchor,
                                       vec3_sub(
                                                viewfinder_midpointer,
                                                vec3_add(
                                                         vec3_multi_r( viewfinder_step_x, -(double)B_X / 2.0 ),
                                                         vec3_multi_r( viewfinder_step_y, -(double)B_Y / 2.0 )
                                                    )
                                                //)
                                       );
    printf("Viewfinder origin: (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_origin.x, viewfinder_origin.y, viewfinder_origin.z, vec3_len(viewfinder_origin));

    //Spheres
    add_sphere(&drawables_root, (vec3){0,0,0}, 7, 255,0,0, 1);
    add_sphere(&drawables_root, (vec3){10,0,0}, 7, 0,0,255, 0.5);
    add_sphere(&drawables_root, (vec3){10,10,0}, 7, 255,0,255, 1);
    add_sphere(&drawables_root, (vec3){5,5,5}, 3, 0,255,255, 0.5);
    //add_sphere(&drawables_root, (vec3){10,10,10}, 0.5, 0,0,255, 1);

    //FILE *fd = fopen("out_res.txt", "w");
    intersect_resultset current_closest, current_candidate;
    drawable_obj_llnode *drawables_list_aux = NULL;
    drawable_obj *drawable_ptr = NULL;//, *draw_target = NULL;
    vec3 current_viewpoint;
//#ifdef DEBUG
    int freed_intersect_nodes;
//#endif
    int x, y;
    pixel_color current_pixel;
    for (x = 0; x < B_X; x++) {
        for (y = 0; y < B_Y; y++) {
            //Calculate current viewpoint
            current_viewpoint = vec3_add(viewfinder_origin, vec3_add(vec3_multi_r(viewfinder_step_x, (double)x), vec3_multi_r(viewfinder_step_y, (double)y)));
            current_closest = { {INF, INF, INF}, INF };
            drawables_list_aux = drawables_root;
            while (drawables_list_aux != NULL) {
                drawable_ptr = drawables_list_aux->drawable;
                switch (drawable_ptr->object_type) {
                case sphere:
                    current_candidate = chk_intersect_sphere(&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
                    break;
                default:
                    printf("Something went wrong\n");
                    break;
                }
                drawables_list_aux = drawables_list_aux -> next;
            }
            //add_intersection_tolist(&intersections_root, &bg_helper,  { (vec3){INF, INF, INF}, INF });
            //print_intersection_ll(intersections_root, fd);
            current_pixel = calculate_intersection_results(intersections_root, bg_pixel.r, bg_pixel.g, bg_pixel.b);
            //fprintf(fd, "Hey\n");
            //printf("(%3d,%3d,%3d)\n", current_pixel.r, current_pixel.g, current_pixel.b);
            //printf("\n");

            //if (current_pixel.r != bg_pixel.r) { printf("Found differently colored pixel!\n"); }

            SDL_SetRenderDrawColor(renderer, current_pixel.r, current_pixel.g, current_pixel.b, 255);
            SDL_RenderDrawPoint(renderer, x, y);
            //SDL_RenderPresent(renderer);

//#ifdef DEBUG
            //freed_intersect_nodes = free_intersection_linkedlist(&intersections_root);
            //fprintf(fd,"Freed %d nodes\n", freed_intersect_nodes);
//#else
            free_intersection_linkedlist(&intersections_root);
//#endif // DEBUG
        }
    }
    SDL_RenderPresent(renderer);
    printf("Done!\n");

    while(1) {
        if ( SDL_PollEvent(&event) && event.type == SDL_QUIT ) {
            SDL_DestroyRenderer( renderer );
            SDL_DestroyWindow( window );
            SDL_Quit();
            exit(0);
            }//break;
    }

    SDL_DestroyRenderer( renderer );
    SDL_DestroyWindow( window );

    //Quit SDL subsystems
    SDL_Quit();


    //ReleaseDC(myconsole, mydc);
    //cin.ignore();
    return 0;
}
