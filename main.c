#include <allegro5/allegro.h>
#include <allegro5/allegro_physfs.h>
#include <allegro5/allegro_image.h>
#include <physfs.h>
#include <stdio.h>

extern unsigned char builtin_data[];
extern size_t builtin_data_size;

static void print_physfs_error()
{
	PHYSFS_ErrorCode errcode = PHYSFS_getLastErrorCode();
	const char* errstr = PHYSFS_getErrorByCode(errcode);
	fprintf(stderr, "physfs error %u: %s\n", (unsigned)errcode, errstr);
}

int main(int argc, char** argv)
{
	(void)argc;

	// ---
	// Early Initialization

	if (!al_init())
	{
		fputs("al_init failed\n", stderr);
		return 1;
	}

	if (!PHYSFS_init(argv[0]))
	{
		fputs("PHYSFS_init failed\n", stderr);
		return 1;
	}

	if (!al_init_image_addon())
		fputs("al_init_image_addon failed\n", stderr);

	if (!al_install_keyboard())
		fputs("al_install_keyboard failed\n", stderr);

	// ---
	// VFS Initialization

	const bool enable_loose_files = true;
	const bool enable_data_zip = true;
	const bool enable_builtin_data = true;

	// Here we choose to mount three sources so that files can be loaded from:
	//  - The contents of the 'data' directory in the current working directory
	//  - The contents of the file 'data.zip' in the current working directory
	//  - The contents of the built-in data archive compiled in to the program

	// Each source is mounted to the root of the VFS, such that any conflicting
	// filenames will be resolved by the search order, determined by the reverse
	// order in which each source is mounted

	// Register the built-in data archive
	if (enable_builtin_data)
	{
		if (!PHYSFS_mountMemory(
			builtin_data, builtin_data_size, NULL,
			"__builtin_data.zip", NULL, 0
		))
		{
			fputs("Failed to add built-in data archive to search path\n", stderr);
			print_physfs_error();
		}
	}

	// Register 'data.zip' as a search path (data archive)
	if (enable_data_zip)
	{
		if (!PHYSFS_mount("./data.zip", NULL, 0))
		{
			fputs("Failed to add ./data.zip to search path\n", stderr);
			print_physfs_error();
		}
	}

	// Register the filesystem as a search path (loose files)
	if (enable_loose_files)
	{
		if (!PHYSFS_mount("./data/", NULL, 0))
		{
			fputs("Failed to add ./data/ to search path\n", stderr);
			print_physfs_error();
		}
	}

	// After calling this, Allegro's filesystem operations are handled by PhysFS
	al_set_physfs_file_interface();

	// ---
	// Allegro Initialization

	ALLEGRO_DISPLAY* dpy = NULL;
	ALLEGRO_EVENT_QUEUE* q = NULL;
	ALLEGRO_BITMAP* happy_bmp = NULL;

	al_set_new_display_flags(ALLEGRO_GENERATE_EXPOSE_EVENTS);
	dpy = al_create_display(640, 480);

	if (!dpy)
	{
		fputs("al_create_display failed\n", stderr);
		goto cleanup;
	}

	q = al_create_event_queue();

	if (!q)
	{
		fputs("al_create_event_queue failed\n", stderr);
		goto cleanup;
	}

	al_register_event_source(q, al_get_display_event_source(dpy));

	if (al_is_keyboard_installed())
		al_register_event_source(q, al_get_keyboard_event_source());

	// ---
	// Asset loading

	happy_bmp = al_load_bitmap("happy.png");

	if (!happy_bmp)
		fputs("loading happy.png failed\n", stderr);

	// ---
	// Game event loop

	bool running = true;
	bool redraw = true;
	ALLEGRO_EVENT e;

	while (running)
	{
		// --- Drawing
		if (redraw)
		{
			if (happy_bmp)
			{
				al_clear_to_color(al_map_rgb(0, 0, 0));
				al_draw_bitmap(happy_bmp, 50, 50, 0);
			}
			else
			{
				al_clear_to_color(al_map_rgb(255, 0, 0));
			}

			redraw = false;
			al_flip_display();
		}

		// --- Event handling

		al_wait_for_event(q, &e);

		do
		{
			switch (e.type)
			{
				case ALLEGRO_EVENT_DISPLAY_EXPOSE:
					redraw = true;
					break;

				case ALLEGRO_EVENT_KEY_CHAR:
					switch (e.keyboard.keycode)
					{
						case ALLEGRO_KEY_Q:
						case ALLEGRO_KEY_ESCAPE:
							running = false;
					}
					break;
			}
		} while (al_get_next_event(q, &e));
	}

	// ---
	// Cleanup

cleanup:
	if (happy_bmp) al_destroy_bitmap(happy_bmp);
	if (q) al_destroy_event_queue(q);
	if (dpy) al_destroy_display(dpy);

	al_set_standard_file_interface();
	PHYSFS_deinit();
	al_uninstall_keyboard();
	al_uninstall_system();

	return 0;
}
