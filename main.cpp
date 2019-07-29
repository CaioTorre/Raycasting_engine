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

#define BY_PIXEL 1

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
    double field_of_view = 100;
    vec3 viewfinder_size = { B_X/10, B_Y/10, 0 };

    //Background default color
    pixel_color bg_pixel = {0,0,0};
    //COLORREF bg_color = RGB(bg_pixel.r,bg_pixel.g,bg_pixel.b);

    //Camera vector, always centered on its "own" X axis
    obj_camera camera = {
        (vec3){-100, 0, 10},  //anchor
        deg_to_rad(00.0),  //roll
        deg_to_rad(00.0),  //pitch
        deg_to_rad(00.0)   //yaw
    };
    //Camera starts pointing straight towards +X axis (1,0,0)

    //Drawable object linked list
    drawable_obj_llnode *drawables_root = NULL;
    intersects_llnode *intersections_root = NULL;

    int objs = load_shapes_from_file("drawables_list.txt", &drawables_root);
    printf("Loaded %d objects\n", objs);
    /*
    //Spheres
    //add_sphere(&drawables_root, (vec3){0,10,0}, 0.5, 255,255,255, 1);
    //add_sphere(&drawables_root, (vec3){10,10,0}, 0.5, 255,255,255, 1);
    //add_sphere(&drawables_root, (vec3){30,0,0}, 20, 0,0,255, 0.5);
    //add_sphere(&drawables_root, (vec3){30,30,0}, 20, 255,0,255, 1);
    //add_sphere(&drawables_root, (vec3){15,15,15}, 15, 0,255,255, 0.5);
    add_sphere(&drawables_root, (vec3){200,0,20}, 30, 255, 225, 108, 1);

    add_plane_3_points(&drawables_root, (vec3){0,0,0}, (vec3){1,0,0}, (vec3){0,1,0}, 255, 204, 255,1); // R = XY
    //add_plane_3_points(&drawables_root, (vec3){0,0,0}, (vec3){1,0,0}, (vec3){0,0,1}, 0,255,0,0.7); // G = XZ
    add_plane_3_points(&drawables_root, (vec3){200,0,0}, (vec3){200,1,0}, (vec3){200,0,1}, 153, 51, 153,1); // B = YZ

    int i;
    for (i = -70; i <= 70; i += 4) {
        add_line_2_points(&drawables_root, (vec3){0,i,0}, (vec3){1,i,0}, DEFAULT_LINE_WIDTH, 204, 0, 153,1);
        add_line_2_points(&drawables_root, (vec3){3*i,0,0}, (vec3){3*i,1,0}, DEFAULT_LINE_WIDTH, 204, 0, 153,1);
    }
    //add_line_2_points(&drawables_root, (vec3){0,0,0}, (vec3){1,0,0}, 0.07, 60,60,60,1);
    //add_line_2_points(&drawables_root, (vec3){0,10,0}, (vec3){10,10,0}, 0.07, 60,60,60,1);
    //add_line_2_points(&drawables_root, (vec3){0,20,0}, (vec3){1,20,0}, 0.07, 60,60,60,1);
    */
    char command = 'x';
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

    /*do {
        cout << "> ";
        scanf("%c", &command);
        switch (command) {
        case 'p': break; //Pass
        case 'w': camera.anchor.x++; break;
        case 's': camera.anchor.x--; break;
        case 'd': camera.anchor.y++; break;
        case 'a': camera.anchor.y--; break;
        case 'e': camera.anchor.z++; break;
        case 'q': camera.anchor.z--; break;
        //case 'i': camera.anchor = vec3_add(camera.director_x, camera.anchor);
        default: printf("Unknown command\n"); break;
        }
        fflush(stdin);
        SDL_RenderClear(renderer);*/

        vec3 viewfinder_corner = vec3_add( camera.anchor, viewfinder_corner_fromorigin );
        printf("Viewfinder corner: (%.2f, %.2f, %.2f) - len = %.2f\n", viewfinder_corner.x, viewfinder_corner.y, viewfinder_corner.z, vec3_len( viewfinder_corner ));
        vec3 debug_corner = vec3_add( viewfinder_corner, vec3_add( vec3_multi_r( camera.director_y, -1.0 * (double)B_X * viewfinder_size.x / (double)B_X ),vec3_multi_r( camera.director_z, -1.0 * (double)B_Y * viewfinder_size.y / (double)B_Y ) ) );
        printf("Viewfinder max corner: (%.2f, %.2f, %.2f) - len = %.2f\n", debug_corner.x, debug_corner.y, debug_corner.z, vec3_len( debug_corner ));

        //add_sphere(&drawables_root, (vec3){10,10,10}, 0.5, 0,0,255, 1);

        //FILE *fd = fopen("out_res.txt", "w");
        intersect_resultset current_closest, current_candidate;
        drawable_obj_llnode *drawables_list_aux = NULL;
        drawable_obj *drawable_ptr = NULL;//, *draw_target = NULL;
        vec3 current_viewpoint;
    //#ifdef DEBUG
        int freed_intersect_nodes;
    //#endif
        int x, y, vx, vy;
        pixel_color current_pixel;

        for (x = 0; x < B_X; x++) {
            for (y = 0; y < B_Y; y++) {
                vx = BY_PIXEL * (int)(x / BY_PIXEL);
                vy = BY_PIXEL * (int)(y / BY_PIXEL);
                //printf("(%3d,%3d)", x, y);
                //Calculate current viewpoint
                //current_viewpoint = vec3_add(viewfinder_origin, vec3_add(vec3_multi_r(viewfinder_step_x, (double)x), vec3_multi_r(viewfinder_step_y, (double)y)));
                current_viewpoint = vec3_add( viewfinder_corner, vec3_add( vec3_multi_r( camera.director_y, -1.0 * (double)vx * viewfinder_size.x / (double)B_X ),vec3_multi_r( camera.director_z, -1.0 * (double)vy * viewfinder_size.y / (double)B_Y ) ) );
                current_closest = { {INF, INF, INF}, INF };
                drawables_list_aux = drawables_root;
                while (drawables_list_aux != NULL) {
                    drawable_ptr = drawables_list_aux->drawable;
                    switch (drawable_ptr->object_type) {
                    case sphere:
                        //current_candidate =
                        chk_intersect_sphere(&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
                        break;
                    case plane:
                        chk_intersect_plane (&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
                        break;
                    case line:
                        chk_intersect_line  (&(camera.anchor), current_viewpoint, drawable_ptr, &intersections_root);
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
            //printf("\n");
        }
        SDL_RenderPresent(renderer);
        printf("Done!\n");
    //} while (command != 'x');

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
