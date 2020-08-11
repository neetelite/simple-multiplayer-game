#include <X11/X.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#include <linux/limits.h>

struct OsContext
{
	Display *display;
	Window window_main;
	Window window_root;

	u32 width;
	u32 height;

	u32 screen_width;
	u32 screen_height;

	bool fullscreen;

	GLXContext gl_context;
};

struct OsState
{
	u64 app_memory_size;
	void *app_memory;

	char path[PATH_MAX];
	char path_exe[PATH_MAX];
	char path_run[PATH_MAX];
	char path_src[PATH_MAX];
	char path_data[PATH_MAX];

	bool running;

	f32 fps;
	f32 fps_max;
	u32 frame;

	f32 dt;
};
