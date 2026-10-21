#include <stddef.h>

#include "math.h"
#include "vec.h"
#include "str.h"
#include "alloc.h"
#include "os.h"
#include "dynarray.h"
#include "mesh.h"
#include "texture.h"
#include "model.h"
#include "camera.h"
#include "light_dir.h"
#include "light_point.h"
#include "graph.h"
#include "font.h"
#include "render.h"
#include "gui.h"

struct menu
{
	struct gui *gui_root;

	struct gui *owner_label;
	struct gui *FPS_label;
	struct gui *FPS_value;

	struct gui *tab;
	struct gui *window_scene;
	struct gui *window_new;
	struct gui *window_light;
	struct gui *window_camera;
	struct gui *window_animation;
	struct gui *window_test;

	struct gui *model_label;
	struct gui *model_value;
	struct gui *x_value;
	struct gui *y_value;
	struct gui *z_value;
	struct gui *text_label;
	struct gui *texture_value;
	struct gui *norm_label;
	struct gui *norm_value;
	struct gui *delete_button;
	struct gui *left_arrow_button;
	struct gui *right_arrow_button;
	struct gui *clear_button;
	struct gui *slider_roty;
	struct gui *slider_roty_label;
	struct gui *slider_r;
	struct gui *slider_r_label;
	struct gui *slider_g;
	struct gui *slider_g_label;
	struct gui *slider_b;
	struct gui *slider_b_label;
	struct gui *slider_sp;
	struct gui *slider_sp_label;
	struct gui *slider_ns;
	struct gui *slider_ns_label;

	struct gui *new_model_label;
	struct gui *new_model_value;
	struct gui *new_x_value;
	struct gui *new_y_value;
	struct gui *new_z_value;
	struct gui *new_text_label;
	struct gui *new_texture_value;
	struct gui *new_norm_label;
	struct gui *new_norm_value;
	struct gui *new_create_button;

	struct gui *check_light_label;
	struct gui *check_light;
	struct gui *check_shadow_label;
	struct gui *check_shadow;
	struct gui *check_point_label;
	struct gui *check_point;
	struct gui *slider_light_label;
	struct gui *slider_light;
	struct gui *slider_light_x_label;
	struct gui *slider_light_x;
	struct gui *slider_light_y_label;
	struct gui *slider_light_y;
	struct gui *slider_light_z_label;
	struct gui *slider_light_z;

	struct gui *slider_fov_label;
	struct gui *slider_fov;

	struct gui *animation1_button;
	struct gui *animation2_button;
	struct gui *animation3_button;
	struct gui *animation4_button;
	struct gui *animation_speed_label;
	struct gui *animation_speed_slider;
	struct gui *animation_scale_label;
	struct gui *animation_scale_slider;
};

static void set_menu(struct model *model, struct menu *menu)
{
	if (model)
	{
		if (model->mesh->name)
		{
			gui_set_text(menu->model_value, model->mesh->name);
		}
		else
		{
			gui_set_text(menu->model_value, "NULL");
		}

		if (model->base_material.map_kd)
		{
			gui_set_text(menu->texture_value, model->base_material.map_kd->name);
		}
		else
		{
			gui_set_text(menu->texture_value, "");
		}

		if (model->base_material.map_bump)
		{
			gui_set_text(menu->norm_value, model->base_material.map_bump->name);
		}
		else
		{
			gui_set_text(menu->norm_value, "");
		}

		gui_set_text_int(menu->x_value, (int)model->x);
		gui_set_text_int(menu->y_value, (int)model->y);
		gui_set_text_int(menu->z_value, (int)model->z);

		gui_set_slider_position(menu->slider_roty, model->theta_y / (2 * PI));
		gui_set_slider_position(menu->slider_r, model->base_material.ka[0]);
		gui_set_slider_position(menu->slider_g, model->base_material.ka[1]);
		gui_set_slider_position(menu->slider_b, model->base_material.ka[1]);
		gui_set_slider_position(menu->slider_sp, model->base_material.ks[0]);
		gui_set_slider_position(menu->slider_ns, model->base_material.ns / 50);
	}
	else
	{
		gui_set_text(menu->model_value, "");
		gui_set_text(menu->texture_value, "");
		gui_set_text(menu->norm_value, "");
		gui_set_text(menu->x_value, "");
		gui_set_text(menu->y_value, "");
		gui_set_text(menu->z_value, "");

		gui_set_slider_position(menu->slider_roty, 0);
		gui_set_slider_position(menu->slider_r, 0);
		gui_set_slider_position(menu->slider_g, 0);
		gui_set_slider_position(menu->slider_b, 0);
		gui_set_slider_position(menu->slider_sp, 0);
		gui_set_slider_position(menu->slider_ns, 0);
	}
}

int main()
{
	//Load Textures
	char texture_directory[MAXIMUM_PATH] = {0};
	str_cat(texture_directory, os_file_get_directory(), sizeof(texture_directory));
	str_cat(texture_directory, "textures", sizeof(texture_directory));
	struct texture *placeholder_texture = alloc_calloc(1, sizeof(*placeholder_texture));;
	texture_create(placeholder_texture, 15, 15, "placeholder");
	struct dynarray textures;
	dynarray_create(&textures, sizeof(struct texture *), NULL);
	dynarray_push(&textures, &placeholder_texture); //Ensuring that there is always at least one texture
	texture_load_directory(texture_directory, &textures);

	//Create Cameras
	struct camera cams[1] = {0};
	camera_create(&cams[0], 899, 511, 0.5f, 30, 90, CAMERA_PERSPECTIVE);
	
	struct os_window wnd;
	struct texture *ico = texture_find(&textures, "ico");
	os_window_create(&wnd, "Software Renderer", cams[0].w, cams[0].h, ico->pixels, ico->w, ico->h);

	int running = 1;
	int frames = 0;
	int time = 0;

	struct mesh *graph = alloc_calloc(1, sizeof(*graph));
	//graph_create_2(graph, 0, 2 * PI, 0, 2 * PI, 2 * PI / 30.0f, 2 * PI / 30.0f, 1, 1, "graph"); //torus
	//graph_create_2(graph, 0, 2 * PI, 0, PI, 2 * PI / 10.0f, PI / 10.0f, 0, 1, "placeholder"); //sphere
	//graph_create_2(graph, -2 * PI, 2 * PI, -2 * PI, 2 * PI, 4 * PI / 50.0f, 4 * PI / 50.0f, 1, 1, "placeholder", 0); //wave
	//graph_create_2(graph, 0, 2 * PI, 0, PI, 2 * PI / 5.0f, PI / 5.0f, 0, 1); //sphere_low
	//graph_create_2(graph, -5, 5, -5, 5, 0.1f, 0.1f, 1, 1, "placeholder"); //plane
	//graph_create_2(graph, -7, 7, -7, 7, 0.1f, 0.1f, 1, 1, "wave", 0); //wave
	graph_create_square(graph, -1, 1, -1, 1, 2, 2, 1, "placeholder"); //Square

	float animation_speed = 30.0f;
	float animation_scale = 1.0f;

	unsigned char lighting = 0;
	unsigned char lighting_point = 0;
	unsigned char shadow = 0;
	unsigned char animation = 0;

	//Load Models
	char model_directory[MAXIMUM_PATH] = {0};
	str_cat(model_directory, os_file_get_directory(), sizeof(model_directory));
	str_cat(model_directory, "models", sizeof(model_directory));
	struct dynarray models;
	dynarray_create(&models, sizeof(struct mesh *), NULL);
	dynarray_push(&models, &graph);
	mesh_load_directory(model_directory, &models, &textures);
	//NOTE: cannot add more to models after this point. Consider allocating an array using malloc instead of using dynarray

	//Create Scene
	struct dynarray scene;
	dynarray_create(&scene, sizeof(struct model), NULL);

	struct light_dir light_dirs[1] = {0};
	//light_dir_create(&lights[0], 0, -1, 1, 0.1f, 0.1f, 0.1f, 1, 1, 1);
	light_dir_create(&light_dirs[0], 1, 0, 0, 0.3f, 0.3f, 0.3f, 1, 1, 1);
	//light_dir_create(&light_dirs[0], 1, -1, 1, 0.1f, 0.1f, 0.1f, 1, 1, 1);

	struct light_point light_points[1] = {0};
	light_point_create(&light_points[0], 0.1f, 0.1f, 0.1f, 1, 1, 1, 0.005f, 0.07f, 1.0f, mesh_find(&models, "sphere_low_poly"));
	light_point_set_x(&light_points[0], -40);
	light_point_set_z(&light_points[0], -5);

	//Load Font
	struct font font;
	font_create(&font, texture_find(&textures, "font"));

	//	
	struct model test1;
	model_create(&test1, mesh_find(&models, "suzanne"), NULL);
	model_set_z(&test1, 0);
	model_set_y(&test1, 1);
	dynarray_push(&scene, &test1);

	struct model test2;
	model_create(&test2, mesh_find(&models, "cube"), NULL);
	model_set_z(&test2, 5);
	dynarray_push(&scene, &test2);

	struct model test3;
	model_create(&test3, mesh_find(&models, "low_poly"), NULL);
	model_set_z(&test3, -10);
	model_set_y(&test3, -10);
	dynarray_push(&scene, &test3);

	struct model test4;
	model_create(&test4, mesh_find(&models, "square"), NULL);
	model_set_z(&test4, 10);
	//model_set_theta_x(&test4, -PI/2);
	model_set_scale(&test4, 15);
	test4.base_material.kd[0] = 0;
	test4.base_material.ka[0] = 0;
	test4.base_material.kd[1] = 0.25f;
	test4.base_material.ka[1] = 0.25f;
	test4.base_material.kd[2] = 1.0f;
	test4.base_material.ka[2] = 1.0f;
	test4.base_material.ns = 150;
	test4.base_material.ks[0] = 1.0f;
	test4.base_material.ks[1] = 1.0f;
	test4.base_material.ks[2] = 1.0f;
	dynarray_push(&scene, &test4);

	struct model test5;
	model_create(&test5, mesh_find(&models, "square"), NULL);
	test5.base_material.map_bump = texture_find(&textures, "NormalMap2");
	model_set_x(&test5, -40);
	dynarray_push(&scene, &test5);

	struct model test6;
	model_create(&test6, mesh_find(&models, "wave"), NULL);
	model_set_x(&test6, 40);
	dynarray_push(&scene, &test6);

	struct model test7;
	model_create(&test7, mesh_find(&models, "square"), texture_find(&textures, "checkerboard"));
	model_set_z(&test7, 5);
	model_set_y(&test7, 6);
	dynarray_push(&scene, &test7);
	//

	struct menu menu = {0};

	menu.gui_root = gui_create(&font, NULL, 0, 0, cams[0].w, cams[0].h, 0, GUI_ROOT);

	menu.owner_label = gui_create(&font, menu.gui_root, 10, 305, 155, 15, 25, GUI_LABEL);
	menu.FPS_label = gui_create(&font, menu.gui_root, 10, 5, 26, 15, 10, GUI_LABEL);
	menu.FPS_value = gui_create(&font, menu.gui_root, 36, 5, 40, 15, 10, GUI_LABEL);

	menu.tab = gui_create(&font, menu.gui_root, 10, 285, 200, 15, 0, GUI_TAB);
	menu.window_scene = gui_add_tab(menu.tab, " Scene", 40, 260);
	menu.window_new = gui_add_tab(menu.tab, " New", 30, 120);
	menu.window_light = gui_add_tab(menu.tab, "  Light", 40, 160);
	menu.window_camera = gui_add_tab(menu.tab, "Camera", 40, 40);
	menu.window_animation = gui_add_tab(menu.tab, "Animation", 50, 140);

	menu.model_label = gui_create(&font, menu.window_scene, 5, 225, 40, 15, 10, GUI_LABEL);
	menu.model_value = gui_create(&font, menu.window_scene, 55, 225, 100, 15, 50, GUI_LABEL);
	menu.x_value = gui_create(&font, menu.window_scene, 55, 165, 30, 15, 50, GUI_LABEL);
	menu.y_value = gui_create(&font, menu.window_scene, 90, 165, 30, 15, 50, GUI_LABEL);
	menu.z_value = gui_create(&font, menu.window_scene, 125, 165, 30, 15, 50, GUI_LABEL);
	menu.text_label = gui_create(&font, menu.window_scene, 5, 185, 40, 15, 10, GUI_LABEL);
	menu.texture_value = gui_create(&font, menu.window_scene, 55, 185, 100, 15, 50, GUI_LABEL);
	menu.norm_label = gui_create(&font, menu.window_scene, 5, 205, 40, 15, 10, GUI_LABEL);
	menu.norm_value = gui_create(&font, menu.window_scene, 55, 205, 100, 15, 50, GUI_LABEL);
	menu.delete_button = gui_create(&font, menu.window_scene, 55, 145, 100, 15, 50, GUI_BUTTON);
	menu.left_arrow_button = gui_create(&font, menu.window_scene, 20, 165, 25, 15, 50, GUI_BUTTON);
	menu.right_arrow_button = gui_create(&font, menu.window_scene, 165, 165, 25, 15, 50, GUI_BUTTON);
	menu.clear_button = gui_create(&font, menu.window_scene, 55, 125, 100, 15, 50, GUI_BUTTON);
	menu.slider_roty = gui_create(&font, menu.window_scene, 55, 105, 100, 15, 10, GUI_SLIDER);
	menu.slider_roty_label = gui_create(&font, menu.window_scene, 5, 105, 40, 15, 10, GUI_LABEL);
	menu.slider_r = gui_create(&font, menu.window_scene, 55, 85, 100, 15, 10, GUI_SLIDER);
	menu.slider_r_label = gui_create(&font, menu.window_scene, 5, 85, 40, 15, 10, GUI_LABEL);
	menu.slider_g = gui_create(&font, menu.window_scene, 55, 65, 100, 15, 10, GUI_SLIDER);
	menu.slider_g_label = gui_create(&font, menu.window_scene, 5, 65, 40, 15, 10, GUI_LABEL);
	menu.slider_b = gui_create(&font, menu.window_scene, 55, 45, 100, 15, 10, GUI_SLIDER);
	menu.slider_b_label = gui_create(&font, menu.window_scene, 5, 45, 40, 15, 10, GUI_LABEL);
	menu.slider_sp = gui_create(&font, menu.window_scene, 55, 25, 100, 15, 10, GUI_SLIDER);
	menu.slider_sp_label = gui_create(&font, menu.window_scene, 5, 25, 40, 15, 10, GUI_LABEL);
	menu.slider_ns = gui_create(&font, menu.window_scene, 55, 5, 100, 15, 10, GUI_SLIDER);
	menu.slider_ns_label = gui_create(&font, menu.window_scene, 5, 5, 40, 15, 10, GUI_LABEL);

	menu.new_model_label = gui_create(&font, menu.window_new, 5, 85, 40, 15, 10, GUI_LABEL);
	menu.new_model_value = gui_create(&font, menu.window_new, 55, 85, 100, 15, 50, GUI_TEXTBOX);
	menu.new_x_value = gui_create(&font, menu.window_new, 55, 25, 30, 15, 50, GUI_TEXTBOX);
	menu.new_y_value = gui_create(&font, menu.window_new, 90, 25, 30, 15, 50, GUI_TEXTBOX);
	menu.new_z_value = gui_create(&font, menu.window_new, 125, 25, 30, 15, 50, GUI_TEXTBOX);
	menu.new_text_label = gui_create(&font, menu.window_new, 5, 45, 40, 15, 10, GUI_LABEL);
	menu.new_texture_value = gui_create(&font, menu.window_new, 55, 45, 100, 15, 50, GUI_TEXTBOX);
	menu.new_norm_label = gui_create(&font, menu.window_new, 5, 65, 40, 15, 10, GUI_LABEL);
	menu.new_norm_value = gui_create(&font, menu.window_new, 55, 65, 100, 15, 50, GUI_TEXTBOX);
	menu.new_create_button = gui_create(&font, menu.window_new, 55, 5, 100, 15, 50, GUI_BUTTON);

	menu.check_light_label = gui_create(&font, menu.window_light, 5, 125, 40, 15, 10, GUI_LABEL);
	menu.check_light = gui_create(&font, menu.window_light, 55, 125, 15, 15, 0, GUI_CHECKBOX);
	menu.check_shadow_label = gui_create(&font, menu.window_light, 5, 105, 40, 15, 10, GUI_LABEL);
	menu.check_shadow = gui_create(&font, menu.window_light, 55, 105, 15, 15, 0, GUI_CHECKBOX);
	menu.check_point_label = gui_create(&font, menu.window_light, 5, 65, 40, 15, 10, GUI_LABEL);
	menu.check_point = gui_create(&font, menu.window_light, 55, 65, 15, 15, 0, GUI_CHECKBOX);
	menu.slider_light_label = gui_create(&font, menu.window_light, 5, 85, 40, 15, 10, GUI_LABEL);
	menu.slider_light = gui_create(&font, menu.window_light, 55, 85, 100, 15, 10, GUI_SLIDER);
	menu.slider_light_x_label = gui_create(&font, menu.window_light, 5, 45, 40, 15, 10, GUI_LABEL);
	menu.slider_light_x = gui_create(&font, menu.window_light, 55, 45, 100, 15, 10, GUI_SLIDER);
	menu.slider_light_y_label = gui_create(&font, menu.window_light, 5, 25, 40, 15, 10, GUI_LABEL);
	menu.slider_light_y = gui_create(&font, menu.window_light, 55, 25, 100, 15, 10, GUI_SLIDER);
	menu.slider_light_z_label = gui_create(&font, menu.window_light, 5, 5, 40, 15, 10, GUI_LABEL);
	menu.slider_light_z = gui_create(&font, menu.window_light, 55, 5, 100, 15, 10, GUI_SLIDER);

	menu.animation1_button = gui_create(&font, menu.window_animation, 5, 105, 100, 15, 50, GUI_BUTTON);
	menu.animation2_button = gui_create(&font, menu.window_animation, 5, 85, 100, 15, 50, GUI_BUTTON);
	menu.animation3_button = gui_create(&font, menu.window_animation, 5, 65, 100, 15, 50, GUI_BUTTON);
	menu.animation4_button = gui_create(&font, menu.window_animation, 5, 45, 100, 15, 50, GUI_BUTTON);
	menu.animation_speed_label = gui_create(&font, menu.window_animation, 5, 25, 40, 15, 10, GUI_LABEL);
	menu.animation_speed_slider = gui_create(&font, menu.window_animation, 55, 25, 100, 15, 10, GUI_SLIDER);
	menu.animation_scale_label = gui_create(&font, menu.window_animation, 5, 5, 40, 15, 10, GUI_LABEL);
	menu.animation_scale_slider = gui_create(&font, menu.window_animation, 55, 5, 100, 15, 10, GUI_SLIDER);

	menu.slider_fov_label = gui_create(&font, menu.window_camera, 5, 5, 40, 15, 10, GUI_LABEL);
	menu.slider_fov = gui_create(&font, menu.window_camera, 55, 5, 100, 15, 10, GUI_SLIDER);

	gui_set_text(menu.FPS_label, "FPS:");
	gui_set_text(menu.owner_label, "Gabriel Alexander Bawden");

	gui_set_text(menu.model_label, "Model:");
	gui_set_text(menu.text_label, "Text:");
	gui_set_text(menu.norm_label, "Norm:");
	gui_set_text(menu.delete_button, "           Delete");
	gui_set_text(menu.left_arrow_button, " ←");
	gui_set_text(menu.right_arrow_button, "  →");
	gui_set_text(menu.clear_button, "            Clear");
	gui_set_text(menu.slider_roty_label, "rot_y:");
	gui_set_text(menu.slider_r_label, "r:");
	gui_set_text(menu.slider_g_label, "g:");
	gui_set_text(menu.slider_b_label, "b:");
	gui_set_text(menu.slider_sp_label, "sp:");
	gui_set_text(menu.slider_ns_label, "ns:");

	gui_set_text(menu.new_model_label, "Model:");
	gui_set_text(menu.new_text_label, "Text:");
	gui_set_text(menu.new_norm_label, "Norm:");
	gui_set_text(menu.new_create_button, "           Create");

	gui_set_text(menu.check_light_label, "Light:");
	gui_set_text(menu.check_shadow_label, "Shad:");
	gui_set_text(menu.check_point_label, "Point:");
	gui_set_text(menu.slider_light_label, "rot_y:");
	gui_set_text(menu.slider_light_x_label, "pos_x:");
	gui_set_text(menu.slider_light_y_label, "pos_y:");
	gui_set_text(menu.slider_light_z_label, "pos_z:");
	gui_set_slider_position(menu.slider_roty, 0);
	gui_set_slider_position(menu.slider_light_x, 0.5f);
	gui_set_slider_position(menu.slider_light_y, 0.5f);
	gui_set_slider_position(menu.slider_light_z, 0.5f);

	gui_set_text(menu.slider_fov_label, "FOV:");
	gui_set_slider_position(menu.slider_fov, (cams[0].fov - 40) / 80);

	gui_set_text(menu.animation1_button, "      No Animation");
	gui_set_text(menu.animation2_button, "        X-Direction");
	gui_set_text(menu.animation3_button, "        Z-Direction");
	gui_set_text(menu.animation4_button, "       XZ-Direction");
	gui_set_text(menu.animation_speed_label, "Speed:");
	gui_set_text(menu.animation_scale_label, "Scale:");
	gui_set_slider_position(menu.animation_speed_slider, (animation_speed - 10) / 50.0f);
	gui_set_slider_position(menu.animation_scale_slider, (animation_scale / 2.0f));

	light_dir_set_theta_y(&light_dirs[0], PI / 2.2f);
	gui_set_slider_position(menu.slider_light, light_dirs[0].theta_y / (2 * PI));

	gui_set_state(menu.check_light, lighting);

	gui_hide(menu.owner_label);
	gui_hide(menu.gui_root);

	float speed = 0.1f;

	struct dynarray events;
	dynarray_create(&events, sizeof(struct gui_event), NULL);
	
	size_t current_index = 0;

	if (dynarray_len(&scene) > 0)
	{
		struct model *current_model = dynarray_get(&scene, current_index);

		set_menu(current_model, &menu);
	}
	else
	{
		set_menu(NULL, &menu);
	}

	struct texture shadtest;
	texture_create(&shadtest, 1024, 1024, "shadow");

	int mouse_previous_x = (int)wnd.w / 2;
	int mouse_previous_y = (int)wnd.h / 2;
	os_mouse_set_position(&wnd, mouse_previous_x, mouse_previous_y);
	os_mouse_hide();

	while (running)
	{
		for (struct os_event evt = {0}; os_window_get_event(&evt); memset(&evt, 0, sizeof(evt)))
		{
			if (evt.wnd == &wnd)
			{
				switch (evt.event)
				{
				case OS_EVENT_KEY_UP:
				{
					if (!menu.gui_root->active_child)
					{
						switch (evt.id)
						{
						case 'M':
						{
							//gui_show(&guis, owner_label);
							menu.owner_label->visible = !menu.owner_label->visible;

							break;
						}
						case OS_KEY_ESCAPE:
						{
							menu.gui_root->visible = !menu.gui_root->visible;

							if (menu.gui_root->visible)
							{
								os_mouse_show();
							}
							else
							{
								mouse_previous_x = (int)wnd.w / 2;
								mouse_previous_y = (int)wnd.h / 2;

								os_mouse_set_position(&wnd, mouse_previous_x, mouse_previous_y);

								os_mouse_hide();
							}

							break;
						}
						case OS_KEY_ENTER:
						{
							camera_set_x(&cams[0], 0);
							camera_set_y(&cams[0], 0);
							camera_set_z(&cams[0], 0);

							camera_set_theta_x(&cams[0], 0);
							camera_set_theta_y(&cams[0], 0);
							camera_set_theta_z(&cams[0], 0);

							break;
						}
						}
					}

					break;
				}
				case OS_EVENT_LEFT_MOUSE_DOWN:
				{
					if (menu.gui_root->visible)
					{
						menu.gui_root->active_child = 0;

						gui_mouse_down_children(menu.gui_root, evt.id, evt.data, &events, menu.gui_root);
					}

					break;
				}
				case OS_EVENT_LEFT_MOUSE_UP:
				{
					if (menu.gui_root->visible)
					{
						menu.gui_root->active_child = 0;

						gui_mouse_up_children(menu.gui_root, &events, menu.gui_root);
					}

					break;
				}
				case OS_EVENT_RIGHT_MOUSE_UP:
				{
					if (menu.gui_root->visible)
					{
					}

					break;
				}
				case OS_EVENT_MOUSE_MOVE:
				{
					int mouse_x = evt.id;
					int mouse_y = evt.data;

					if (menu.gui_root->visible)
					{
						gui_mouse_move_children(menu.gui_root, mouse_x, mouse_y, &events);
					}
					else
					{
						int buffer = 100;
						int left = 0 + buffer;
						int right = ((int)wnd.w - 1) - buffer;
						int bottom = 0 + buffer;
						int top = ((int)wnd.h - 1) - buffer;

						if (mouse_x < left || mouse_x > right)
						{
							mouse_previous_x = ((int)wnd.w / 2) + (mouse_x - mouse_previous_x);
							mouse_x = (int)wnd.w / 2;
							os_mouse_set_position(&wnd, mouse_x, mouse_y);
						}

						if (mouse_y < bottom || mouse_y > top)
						{
							mouse_previous_y = ((int)wnd.h / 2) + (mouse_previous_y - mouse_y);
							mouse_y = (int)wnd.h / 2;
							os_mouse_set_position(&wnd, mouse_x, mouse_y);
						}

						float theta_x = cams[0].theta_x + (speed / 20) * (mouse_y - mouse_previous_y);
						float theta_y = cams[0].theta_y + (speed / 20) * (mouse_previous_x - mouse_x);

						if (theta_x <= PI / 2 && theta_x >= -PI / 2)
						{
							camera_set_theta_x(&cams[0], theta_x);
						}

						camera_set_theta_y(&cams[0], theta_y);

						mouse_previous_x = mouse_x;
						mouse_previous_y = mouse_y;
					}

					break;
				}
				case OS_EVENT_TYPE:
				{
					if (menu.gui_root->visible)
					{
						gui_type_children(menu.gui_root, evt.wide);
					}

					break;
				}
				case OS_EVENT_LOSE_FOCUS:
				{
					memset(wnd.keystates, 0, sizeof(wnd.keystates));

					break;
				}
				case OS_EVENT_RESIZE:
				{
					camera_resize(&cams[0], evt.id, evt.data);

					break;
				}
				}
			}

			switch (evt.event)
			{
			case OS_EVENT_END:
			{
				running = 0;

				os_window_destroy(&wnd);
				dynarray_destroy(&textures);
				dynarray_destroy(&models);
				dynarray_destroy(&scene);
				gui_destroy_children(menu.gui_root);
				font_destroy(&font);

				os_process_exit(0);

				break;
			}
			}
		}

		while (dynarray_len(&events))
		{
			struct gui_event evt;
			dynarray_pop(&events, &evt);

			switch (evt.event)
			{
			case GUI_EVENT_BUTTON_PRESS:
			{
				if (evt.id == menu.new_create_button)
				{
					struct mesh *model = mesh_find(&models, gui_get_text(menu.new_model_value));
					
					struct texture *tex = texture_find(&textures, gui_get_text(menu.new_texture_value));
 
					struct texture *norm = texture_find(&textures, gui_get_text(menu.new_norm_value));

					if (str_equal(tex->name, "placeholder"))
					{
						tex = NULL;
					}

					if (str_equal(norm->name, "placeholder"))
					{
						norm = NULL;
					}

					if(!model->texture_coordinates)
					{
						tex = NULL;
						norm = NULL;
					}

					struct model model_new;
					model_create(&model_new, model, tex);
					model_set_x(&model_new, (float)str_str_to_int(gui_get_text(menu.new_x_value)));
					model_set_y(&model_new, (float)str_str_to_int(gui_get_text(menu.new_y_value)));
					model_set_z(&model_new, (float)str_str_to_int(gui_get_text(menu.new_z_value)));
					model_new.base_material.map_bump = norm;
					dynarray_push(&scene, &model_new);

					if (dynarray_len(&scene) == 1)
					{
						current_index = 0;

						struct model *current_model = dynarray_get(&scene, current_index);

						set_menu(current_model, &menu);
					}

					break;
				}
				if (evt.id == menu.delete_button)
				{
					dynarray_remove(&scene, current_index);

					if (current_index == dynarray_len(&scene))
					{
						current_index--;
					}

					if (dynarray_len(&scene) == 0)
					{
						set_menu(NULL, &menu);
					}
					else
					{
						struct model *current_model = dynarray_get(&scene, current_index);

						set_menu(current_model, &menu);
					}

					break;
				}
				if (evt.id == menu.clear_button)
				{
					dynarray_clear(&scene);

					current_index = 0;

					set_menu(NULL, &menu);

					break;
				}
				if (evt.id == menu.left_arrow_button)
				{
					if (current_index > 0)
					{
						current_index--;

						struct model *current_model = dynarray_get(&scene, current_index);

						set_menu(current_model, &menu);
					}
				}
				if (evt.id == menu.right_arrow_button)
				{
					if (current_index < dynarray_len(&scene) - 1)
					{
						current_index++;

						struct model *current_model = dynarray_get(&scene, current_index);

						set_menu(current_model, &menu);
					}
				}
				if (evt.id == menu.animation1_button)
				{
					animation = 0;
				}
				if (evt.id == menu.animation2_button)
				{
					animation = 1;
				}
				if (evt.id == menu.animation3_button)
				{
					animation = 2;
				}
				if (evt.id == menu.animation4_button)
				{
					animation = 3;
				}

				break;
			}
			case GUI_EVENT_CHECKBOX_PRESS:
			{
				if (evt.id == menu.check_light)
				{
					lighting = (unsigned char)evt.data;

					break;
				}
				if (evt.id == menu.check_shadow)
				{
					shadow = (unsigned char)evt.data;

					break;
				}
				if (evt.id == menu.check_point)
				{
					lighting_point = (unsigned char)evt.data;

					break;
				}

				break;
			}
			case GUI_EVENT_SLIDER_MOVE:
			{
				if (evt.id == menu.slider_light)
				{
					float theta_y = 2 * PI * evt.data_float;

					light_dir_set_theta_y(&light_dirs[0], theta_y);
				}
				if (evt.id == menu.slider_fov)
				{
					camera_set_fov(&cams[0], 80 * evt.data_float + 40);
				}
				if (evt.id == menu.slider_roty)
				{
					model_set_theta_y(&((struct model *)scene.array)[current_index], 2 * PI * evt.data_float);
				}
				if (evt.id == menu.slider_r)
				{
					((struct model *)scene.array)[current_index].base_material.ka[0] = evt.data_float;
					((struct model *)scene.array)[current_index].base_material.kd[0] = evt.data_float;
				}
				if (evt.id == menu.slider_g)
				{
					((struct model *)scene.array)[current_index].base_material.ka[1] = evt.data_float;
					((struct model *)scene.array)[current_index].base_material.kd[1] = evt.data_float;
				}
				if (evt.id == menu.slider_b)
				{
					((struct model *)scene.array)[current_index].base_material.ka[2] = evt.data_float;
					((struct model *)scene.array)[current_index].base_material.kd[2] = evt.data_float;
				}
				if (evt.id == menu.slider_sp)
				{
					for (size_t i = 0; i < 3; i++)
					{
						((struct model *)scene.array)[current_index].base_material.ks[i] = evt.data_float;
					}
				}
				if (evt.id == menu.slider_ns)
				{
					((struct model *)scene.array)[current_index].base_material.ns = 50 * evt.data_float;
				}
				if (evt.id == menu.slider_light_x)
				{
					light_point_set_x(&light_points[0], -40 + 5 * (2 * evt.data_float - 1));
				}
				if (evt.id == menu.slider_light_y)
				{
					light_point_set_y(&light_points[0], 5 * (2 * evt.data_float - 1));
				}
				if (evt.id == menu.slider_light_z)
				{
					light_point_set_z(&light_points[0], -10 * evt.data_float);
				}
				if (evt.id == menu.animation_speed_slider)
				{
					animation_speed = (50 * evt.data_float) + 10;
				}
				if (evt.id == menu.animation_scale_slider)
				{
					animation_scale = (2 * evt.data_float);
				}

				break;
			}
			}
		}

		if (!menu.gui_root->active_child) //Check if any textboxes are currently selected
		{
			if (wnd.keystates['W'])
			{
				camera_key_x(&cams[0], speed);
			}
			if (wnd.keystates['S'])
			{
				camera_key_x(&cams[0], -speed);
			}
			if (wnd.keystates['D'])
			{
				camera_key_z(&cams[0], speed);
			}
			if (wnd.keystates['A'])
			{
				camera_key_z(&cams[0], -speed);
			}
			if (wnd.keystates['I'])
			{
				float theta_x = cams[0].theta_x + (speed / 2);

				if (theta_x <= PI / 2)
				{
					camera_set_theta_x(&cams[0], theta_x);
				}
			}
			if (wnd.keystates['K'])
			{
				float theta_x = cams[0].theta_x - (speed / 2);

				if (theta_x >= -PI / 2)
				{
					camera_set_theta_x(&cams[0], theta_x);
				}
			}
			if (wnd.keystates['J'])
			{
				float theta_y = cams[0].theta_y + speed / 2;

				camera_set_theta_y(&cams[0], theta_y);
			}
			if (wnd.keystates['L'])
			{
				float theta_y = cams[0].theta_y - speed / 2;

				camera_set_theta_y(&cams[0], theta_y);
			}
			if (wnd.keystates[' '])
			{
				camera_key_y(&cams[0], speed);
			}
			if (wnd.keystates[OS_KEY_LSHIFT])
			{
				camera_key_y(&cams[0], -speed);
			}
		}

		if (frames % 50 == 0)
		{
			int FPS = (int)(50 / (((os_time_tick_count() - time)) / 1000.0f));

			time = os_time_tick_count();

			gui_set_text_int(menu.FPS_value, FPS);
		}

		struct mesh *wave = mesh_find(&models, "wave");

		if (!str_equal(wave->name, "placeholder"))
		{
			graph_wave(wave, frames, animation_speed, animation_scale, animation);
		}

		camera_update(&cams[0]);

		light_dir_update(&light_dirs[0], &cams[0], shadow);
		light_point_update(&light_points[0], cams[0].transformation);

		render_draw(&light_points[0].light_model, &cams[0], light_dirs, light_points, 1, 0, 0);

		for (size_t i = 0; i < dynarray_len(&scene); i++)
		{
			model_update(&((struct model *)scene.array)[i]);

			if (shadow)
			{
				render_draw_shadow(&((struct model *)scene.array)[i], &light_dirs[0].cam, 1);
			}
		}

		for (size_t i = 0; i < dynarray_len(&scene); i++)
		{
			render_draw(&((struct model *)scene.array)[i], &cams[0], light_dirs, light_points, 1, lighting * (1 + lighting_point), shadow);
		}

		gui_render_children(menu.gui_root, &cams[0]);

		os_window_draw(&wnd, cams[0].pixels, cams[0].w, cams[0].h);

		frames++;
	}

	return 0;
}