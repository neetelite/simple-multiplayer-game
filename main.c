/* TODO: Clean this include stuff */
#include "../../library/standard.h"
#include "../../library/base.h"

#if defined(OS_WINDOWS)
#include "platform/windows.h"
#include "platform/windows.c"
#elif defined(OS_LINUX)
#include "platform/linux.h"
#include "platform/linux.c"
#endif

#include "../../library/graphics.h"

struct OsContext os_context;
struct OsState os_state;

#define LOADER_WHITELIST
#define LOADER_LIST_DAE

#include "include.h"

#include "app.h"
#include "app.c"

#define WIDTH 640
#define HEIGHT 480
#define FPS_MAX 60

i32
main(void)
{
	/* Path */
	os_path_build();

	/* Window */
	os_create_window(WIDTH, HEIGHT);

	/* Memory */
	os_memory_alloc(GB(2));

	/* Graphics */
	os_opengl_init(&os_context);

	/* Timer */
	struct TimeLimiter limiter = {0};
	time_limiter_init(&limiter, TIME_NS_FROM_S(1) / FPS_MAX);

	/* Input */
	/* TODO */

	os_state.running = 1;
	while(os_state.running)
	{
		os_events();
		app_main();
		os_render();

		//time_limiter_tick(&limiter);
		//os_state.dt = limiter.dt_s;
	}

	os_memory_free();
	mem_print();
}
