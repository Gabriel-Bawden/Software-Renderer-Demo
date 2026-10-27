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

	struct gui *label_owner;
	struct gui *label_fps;
	struct gui *textbox_fps;

	struct gui *tab;
	struct gui *window_scene;
	struct gui *window_new;
	struct gui *window_light;
	struct gui *window_camera;
	struct gui *window_animation;
	struct gui *window_test;

	struct gui *label_model_current;
	struct gui *textbox_model_current;
	struct gui *textbox_x_current;
	struct gui *textbox_y_current;
	struct gui *textbox_z_current;
	struct gui *label_texture_current;
	struct gui *textbox_texture_current;
	struct gui *label_normal_current;
	struct gui *textbox_normal_current;
	struct gui *button_delete;
	struct gui *button_left_arrow;
	struct gui *button_right_arrow;
	struct gui *button_clear;
	struct gui *slider_roty;
	struct gui *label_roty;
	struct gui *slider_r;
	struct gui *label_r;
	struct gui *slider_g;
	struct gui *label_g;
	struct gui *slider_b;
	struct gui *label_b;
	struct gui *slider_sp;
	struct gui *label_sp;
	struct gui *slider_ns;
	struct gui *label_ns;

	struct gui *label_model_new;
	struct gui *textbox_model_new;
	struct gui *textbox_x_new;
	struct gui *textbox_y_new;
	struct gui *textbox_z_new;
	struct gui *label_texture_new;
	struct gui *textbox_texture_new;
	struct gui *label_normal_new;
	struct gui *textbox_normal_new;
	struct gui *create_button_new;

	struct gui *label_light;
	struct gui *checkbox_light;
	struct gui *label_shadow;
	struct gui *checkbox_shadow;
	struct gui *label_light_point;
	struct gui *checkbox_light_point;
	struct gui *label_light_roty;
	struct gui *slider_light_roty;
	struct gui *label_light_point_x;
	struct gui *slider_light_point_x;
	struct gui *label_light_point_y;
	struct gui *slider_light_point_y;
	struct gui *label_light_point_z;
	struct gui *slider_light_point_z;

	struct gui *label_fov;
	struct gui *slider_fov;

	struct gui *button_animation1;
	struct gui *button_animation2;
	struct gui *button_animation3;
	struct gui *button_animation4;
	struct gui *label_animation_speed;
	struct gui *slider_animation_speed;
	struct gui *label_animation_scale;
	struct gui *slider_animation_scale;
};

static void set_menu(struct model *model, struct menu *menu)
{
	if (model)
	{
		if (model->mesh->name)
		{
			gui_set_text(menu->textbox_model_current, model->mesh->name);
		}
		else
		{
			gui_set_text(menu->textbox_model_current, "NULL");
		}

		if (model->base_material.map_kd)
		{
			gui_set_text(menu->textbox_texture_current, model->base_material.map_kd->name);
		}
		else
		{
			gui_set_text(menu->textbox_texture_current, "");
		}

		if (model->base_material.map_bump)
		{
			gui_set_text(menu->textbox_normal_current, model->base_material.map_bump->name);
		}
		else
		{
			gui_set_text(menu->textbox_normal_current, "");
		}

		gui_set_text_int(menu->textbox_x_current, (int)model->x);
		gui_set_text_int(menu->textbox_y_current, (int)model->y);
		gui_set_text_int(menu->textbox_z_current, (int)model->z);

		gui_set_slider_position(menu->slider_roty, model->theta_y / (2 * PI));
		gui_set_slider_position(menu->slider_r, model->base_material.ka[0]);
		gui_set_slider_position(menu->slider_g, model->base_material.ka[1]);
		gui_set_slider_position(menu->slider_b, model->base_material.ka[2]);
		gui_set_slider_position(menu->slider_sp, model->base_material.ks[0]);
		gui_set_slider_position(menu->slider_ns, model->base_material.ns / 50);
	}
	else
	{
		gui_set_text(menu->textbox_model_current, "");
		gui_set_text(menu->textbox_texture_current, "");
		gui_set_text(menu->textbox_normal_current, "");
		gui_set_text(menu->textbox_x_current, "");
		gui_set_text(menu->textbox_y_current, "");
		gui_set_text(menu->textbox_z_current, "");

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
	light_dir_create(&light_dirs[0], 1, 0, 0, 0.1f, 0.1f, 0.1f, 1, 1, 1);
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

	menu.label_owner = gui_create(&font, menu.gui_root, 10, 305, 155, 15, 25, GUI_LABEL);
	menu.label_fps = gui_create(&font, menu.gui_root, 10, 5, 26, 15, 10, GUI_LABEL);
	menu.textbox_fps = gui_create(&font, menu.gui_root, 36, 5, 40, 15, 10, GUI_LABEL);

	menu.tab = gui_create(&font, menu.gui_root, 10, 285, 200, 15, 0, GUI_TAB);
	menu.window_scene = gui_add_tab(menu.tab, " Scene", 40, 245);
	menu.window_new = gui_add_tab(menu.tab, " New", 30, 105);
	menu.window_light = gui_add_tab(menu.tab, "  Light", 40, 145);
	menu.window_camera = gui_add_tab(menu.tab, "Camera", 40, 25);
	menu.window_animation = gui_add_tab(menu.tab, "Animation", 50, 125);

	menu.label_model_current = gui_create(&font, menu.window_scene, 5, 225, 45, 15, 10, GUI_LABEL);
	menu.textbox_model_current = gui_create(&font, menu.window_scene, 60, 225, 100, 15, 50, GUI_LABEL);
	menu.textbox_x_current = gui_create(&font, menu.window_scene, 60, 165, 30, 15, 50, GUI_LABEL);
	menu.textbox_y_current = gui_create(&font, menu.window_scene, 95, 165, 30, 15, 50, GUI_LABEL);
	menu.textbox_z_current = gui_create(&font, menu.window_scene, 130, 165, 30, 15, 50, GUI_LABEL);
	menu.label_texture_current = gui_create(&font, menu.window_scene, 5, 185, 45, 15, 10, GUI_LABEL);
	menu.textbox_texture_current = gui_create(&font, menu.window_scene, 60, 185, 100, 15, 50, GUI_LABEL);
	menu.label_normal_current = gui_create(&font, menu.window_scene, 5, 205, 45, 15, 10, GUI_LABEL);
	menu.textbox_normal_current = gui_create(&font, menu.window_scene, 60, 205, 100, 15, 50, GUI_LABEL);
	menu.button_delete = gui_create(&font, menu.window_scene, 60, 145, 100, 15, 50, GUI_BUTTON);
	menu.button_left_arrow = gui_create(&font, menu.window_scene, 25, 165, 25, 15, 50, GUI_BUTTON);
	menu.button_right_arrow = gui_create(&font, menu.window_scene, 170, 165, 25, 15, 50, GUI_BUTTON);
	menu.button_clear = gui_create(&font, menu.window_scene, 60, 125, 100, 15, 50, GUI_BUTTON);
	menu.slider_roty = gui_create(&font, menu.window_scene, 60, 105, 100, 15, 10, GUI_SLIDER);
	menu.label_roty = gui_create(&font, menu.window_scene, 5, 105, 45, 15, 10, GUI_LABEL);
	menu.slider_r = gui_create(&font, menu.window_scene, 60, 85, 100, 15, 10, GUI_SLIDER);
	menu.label_r = gui_create(&font, menu.window_scene, 5, 85, 45, 15, 10, GUI_LABEL);
	menu.slider_g = gui_create(&font, menu.window_scene, 60, 65, 100, 15, 10, GUI_SLIDER);
	menu.label_g = gui_create(&font, menu.window_scene, 5, 65, 45, 15, 10, GUI_LABEL);
	menu.slider_b = gui_create(&font, menu.window_scene, 60, 45, 100, 15, 10, GUI_SLIDER);
	menu.label_b = gui_create(&font, menu.window_scene, 5, 45, 45, 15, 10, GUI_LABEL);
	menu.slider_sp = gui_create(&font, menu.window_scene, 60, 25, 100, 15, 10, GUI_SLIDER);
	menu.label_sp = gui_create(&font, menu.window_scene, 5, 25, 45, 15, 10, GUI_LABEL);
	menu.slider_ns = gui_create(&font, menu.window_scene, 60, 5, 100, 15, 10, GUI_SLIDER);
	menu.label_ns = gui_create(&font, menu.window_scene, 5, 5, 45, 15, 10, GUI_LABEL);

	menu.label_model_new = gui_create(&font, menu.window_new, 5, 85, 45, 15, 10, GUI_LABEL);
	menu.textbox_model_new = gui_create(&font, menu.window_new, 60, 85, 100, 15, 50, GUI_TEXTBOX);
	menu.textbox_x_new = gui_create(&font, menu.window_new, 60, 25, 30, 15, 50, GUI_TEXTBOX);
	menu.textbox_y_new = gui_create(&font, menu.window_new, 95, 25, 30, 15, 50, GUI_TEXTBOX);
	menu.textbox_z_new = gui_create(&font, menu.window_new, 130, 25, 30, 15, 50, GUI_TEXTBOX);
	menu.label_texture_new = gui_create(&font, menu.window_new, 5, 45, 45, 15, 10, GUI_LABEL);
	menu.textbox_texture_new = gui_create(&font, menu.window_new, 60, 45, 100, 15, 50, GUI_TEXTBOX);
	menu.label_normal_new = gui_create(&font, menu.window_new, 5, 65, 45, 15, 10, GUI_LABEL);
	menu.textbox_normal_new = gui_create(&font, menu.window_new, 60, 65, 100, 15, 50, GUI_TEXTBOX);
	menu.create_button_new = gui_create(&font, menu.window_new, 60, 5, 100, 15, 50, GUI_BUTTON);

	menu.label_light = gui_create(&font, menu.window_light, 5, 125, 45, 15, 10, GUI_LABEL);
	menu.checkbox_light = gui_create(&font, menu.window_light, 60, 125, 15, 15, 0, GUI_CHECKBOX);
	menu.label_shadow = gui_create(&font, menu.window_light, 5, 105, 45, 15, 10, GUI_LABEL);
	menu.checkbox_shadow = gui_create(&font, menu.window_light, 60, 105, 15, 15, 0, GUI_CHECKBOX);
	menu.label_light_point = gui_create(&font, menu.window_light, 5, 65, 45, 15, 10, GUI_LABEL);
	menu.checkbox_light_point = gui_create(&font, menu.window_light, 60, 65, 15, 15, 0, GUI_CHECKBOX);
	menu.label_light_roty = gui_create(&font, menu.window_light, 5, 85, 45, 15, 10, GUI_LABEL);
	menu.slider_light_roty = gui_create(&font, menu.window_light, 60, 85, 100, 15, 10, GUI_SLIDER);
	menu.label_light_point_x = gui_create(&font, menu.window_light, 5, 45, 45, 15, 10, GUI_LABEL);
	menu.slider_light_point_x = gui_create(&font, menu.window_light, 60, 45, 100, 15, 10, GUI_SLIDER);
	menu.label_light_point_y = gui_create(&font, menu.window_light, 5, 25, 45, 15, 10, GUI_LABEL);
	menu.slider_light_point_y = gui_create(&font, menu.window_light, 60, 25, 100, 15, 10, GUI_SLIDER);
	menu.label_light_point_z = gui_create(&font, menu.window_light, 5, 5, 45, 15, 10, GUI_LABEL);
	menu.slider_light_point_z = gui_create(&font, menu.window_light, 60, 5, 100, 15, 10, GUI_SLIDER);

	menu.label_fov = gui_create(&font, menu.window_camera, 5, 5, 45, 15, 10, GUI_LABEL);
	menu.slider_fov = gui_create(&font, menu.window_camera, 60, 5, 100, 15, 10, GUI_SLIDER);

	menu.button_animation1 = gui_create(&font, menu.window_animation, 60, 105, 100, 15, 50, GUI_BUTTON);
	menu.button_animation2 = gui_create(&font, menu.window_animation, 60, 85, 100, 15, 50, GUI_BUTTON);
	menu.button_animation3 = gui_create(&font, menu.window_animation, 60, 65, 100, 15, 50, GUI_BUTTON);
	menu.button_animation4 = gui_create(&font, menu.window_animation, 60, 45, 100, 15, 50, GUI_BUTTON);
	menu.label_animation_speed = gui_create(&font, menu.window_animation, 5, 25, 45, 15, 10, GUI_LABEL);
	menu.slider_animation_speed = gui_create(&font, menu.window_animation, 60, 25, 100, 15, 10, GUI_SLIDER);
	menu.label_animation_scale = gui_create(&font, menu.window_animation, 5, 5, 45, 15, 10, GUI_LABEL);
	menu.slider_animation_scale = gui_create(&font, menu.window_animation, 60, 5, 100, 15, 10, GUI_SLIDER);

	gui_set_text(menu.label_fps, "FPS:");
	gui_set_text(menu.label_owner, "Gabriel Alexander Bawden");

	gui_set_text(menu.label_model_current, "Model:");
	gui_set_text(menu.label_texture_current, "Texture:");
	gui_set_text(menu.label_normal_current, "Normal:");
	gui_set_text(menu.button_delete, "           Delete");
	gui_set_text(menu.button_left_arrow, " ←");
	gui_set_text(menu.button_right_arrow, "  →");
	gui_set_text(menu.button_clear, "            Clear");
	gui_set_text(menu.label_roty, "Rot_Y:");
	gui_set_text(menu.label_r, "Color R:");
	gui_set_text(menu.label_g, "Color G:");
	gui_set_text(menu.label_b, "Color B:");
	gui_set_text(menu.label_sp, "Spec:");
	gui_set_text(menu.label_ns, "Ns:");

	gui_set_text(menu.label_model_new, "Model:");
	gui_set_text(menu.label_texture_new, "Texture:");
	gui_set_text(menu.label_normal_new, "Normal:");
	gui_set_text(menu.create_button_new, "           Create");

	gui_set_text(menu.label_light, "Light:");
	gui_set_text(menu.label_shadow, "Shadow:");
	gui_set_text(menu.label_light_point, "Point:");
	gui_set_text(menu.label_light_roty, "Rot_Y:");
	gui_set_text(menu.label_light_point_x, "Pos_X:");
	gui_set_text(menu.label_light_point_y, "Pos_Y:");
	gui_set_text(menu.label_light_point_z, "Pos_Z:");
	gui_set_slider_position(menu.slider_roty, 0);
	gui_set_slider_position(menu.slider_light_point_x, 0.5f);
	gui_set_slider_position(menu.slider_light_point_y, 0.5f);
	gui_set_slider_position(menu.slider_light_point_z, 0.5f);

	gui_set_text(menu.label_fov, "FOV:");
	gui_set_slider_position(menu.slider_fov, (cams[0].fov - 40) / 80);

	gui_set_text(menu.button_animation1, "      No Animation");
	gui_set_text(menu.button_animation2, "        X-Direction");
	gui_set_text(menu.button_animation3, "        Z-Direction");
	gui_set_text(menu.button_animation4, "       XZ-Direction");
	gui_set_text(menu.label_animation_speed, "Speed:");
	gui_set_text(menu.label_animation_scale, "Scale:");
	gui_set_slider_position(menu.slider_animation_speed, (animation_speed - 10) / 50.0f);
	gui_set_slider_position(menu.slider_animation_scale, (animation_scale / 2.0f));

	light_dir_set_theta_y(&light_dirs[0], PI / 2.2f);
	gui_set_slider_position(menu.slider_light_roty, light_dirs[0].theta_y / (2 * PI));

	gui_set_state(menu.checkbox_light, lighting);

	gui_hide(menu.label_owner);
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
							//gui_show(&guis, label_owner);
							menu.label_owner->visible = !menu.label_owner->visible;

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
				if (evt.id == menu.create_button_new)
				{
					struct mesh *model = mesh_find(&models, gui_get_text(menu.textbox_model_new));
					
					struct texture *tex = texture_find(&textures, gui_get_text(menu.textbox_texture_new));
 
					struct texture *norm = texture_find(&textures, gui_get_text(menu.textbox_normal_new));

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
					model_set_x(&model_new, (float)str_str_to_int(gui_get_text(menu.textbox_x_new)));
					model_set_y(&model_new, (float)str_str_to_int(gui_get_text(menu.textbox_y_new)));
					model_set_z(&model_new, (float)str_str_to_int(gui_get_text(menu.textbox_z_new)));
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
				if (evt.id == menu.button_delete)
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
				if (evt.id == menu.button_clear)
				{
					dynarray_clear(&scene);

					current_index = 0;

					set_menu(NULL, &menu);

					break;
				}
				if (evt.id == menu.button_left_arrow)
				{
					if (current_index > 0)
					{
						current_index--;

						struct model *current_model = dynarray_get(&scene, current_index);

						set_menu(current_model, &menu);
					}
				}
				if (evt.id == menu.button_right_arrow)
				{
					if (current_index < dynarray_len(&scene) - 1)
					{
						current_index++;

						struct model *current_model = dynarray_get(&scene, current_index);

						set_menu(current_model, &menu);
					}
				}
				if (evt.id == menu.button_animation1)
				{
					animation = 0;
				}
				if (evt.id == menu.button_animation2)
				{
					animation = 1;
				}
				if (evt.id == menu.button_animation3)
				{
					animation = 2;
				}
				if (evt.id == menu.button_animation4)
				{
					animation = 3;
				}

				break;
			}
			case GUI_EVENT_CHECKBOX_PRESS:
			{
				if (evt.id == menu.checkbox_light)
				{
					lighting = (unsigned char)evt.data;

					break;
				}
				if (evt.id == menu.checkbox_shadow)
				{
					shadow = (unsigned char)evt.data;

					break;
				}
				if (evt.id == menu.checkbox_light_point)
				{
					lighting_point = (unsigned char)evt.data;

					break;
				}

				break;
			}
			case GUI_EVENT_SLIDER_MOVE:
			{
				if (evt.id == menu.slider_light_roty)
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
				if (evt.id == menu.slider_light_point_x)
				{
					light_point_set_x(&light_points[0], -40 + 5 * (2 * evt.data_float - 1));
				}
				if (evt.id == menu.slider_light_point_y)
				{
					light_point_set_y(&light_points[0], 5 * (2 * evt.data_float - 1));
				}
				if (evt.id == menu.slider_light_point_z)
				{
					light_point_set_z(&light_points[0], -10 * evt.data_float);
				}
				if (evt.id == menu.slider_animation_speed)
				{
					animation_speed = (50 * evt.data_float) + 10;
				}
				if (evt.id == menu.slider_animation_scale)
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
				camera_key_z(&cams[0], speed);
			}
			if (wnd.keystates['S'])
			{
				camera_key_z(&cams[0], -speed);
			}
			if (wnd.keystates['D'])
			{
				camera_key_x(&cams[0], speed);
			}
			if (wnd.keystates['A'])
			{
				camera_key_x(&cams[0], -speed);
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

			gui_set_text_int(menu.textbox_fps, FPS);
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