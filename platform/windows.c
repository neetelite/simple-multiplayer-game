/* Init functions */
void
os_path_build(void)
{
	/* cstr_path_run */
	char string_path_exe[PATH_MAX];
	GetModuleFileNameA(NULL, string_path_exe, PATH_MAX);

	/* path_exe */
	cstr_fs_from_bs(os_state.path_exe, string_path_exe);

	/* path_run */
	cstr_path_parent(os_state.path_run, os_state.path_exe);

	/* path_data */
	cstr_cat(os_state.path_data, os_state.path_run, "data/");

	/* path */
	cstr_path_parent(os_state.path, os_state.path_run);

	/* path_src */
	cstr_cat(os_state.path_src, os_state.path, "src/");
}

LRESULT CALLBACK
os_window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	LRESULT result = 0;

	switch(message)
	{
		#if 0
		/* IMPLEMENT LEARN(lungu): Top borders like Discord */
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window, &paint);
			i32 x = paint.rcPaint.left;
			i32 y = paint.rcPaint.top;
			i32 height = paint.rcPaint.bottom - paint.rcPaint.top;
			i32 width = paint.rcPaint.right - paint.rcPaint.left;

			os_render(device_context);
			EndPaint(window, &paint);
		} break;
		#endif
		case WM_QUIT:
		case WM_CLOSE:
		case WM_DESTROY:
		{
			os_state.running = 0;
		} break;
		default:
		{
			result = DefWindowProc(window, message, w_param, l_param);
		} break;
	}

	return(result);
}

i32
os_window_create(i32 width, i32 height)
{
	HINSTANCE instance = GetModuleHandle(NULL);

	/* Window Class */
	WNDCLASS window_class = {0};
	window_class.style = CS_HREDRAW | CS_VREDRAW;
	window_class.lpfnWndProc = os_window_procedure;
	window_class.hInstance = instance;
	window_class.hCursor = LoadCursor(0, IDC_ARROW);
	//window_class.hIcon = 0;
	window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	window_class.lpszClassName = "window_class";

	if(RegisterClassA(&window_class) == 0)
	{
		MessageBox(0, "Failed to register a window class.", "ERROR", MB_OK | MB_ICONERROR);
		return(-1);
	}

	/* Window | Context */
	os_context.width = width;
	os_context.height = height;

	char window_name[] = "Grand Theft Desu";
	os_context.window = CreateWindowEx(0, window_class.lpszClassName, window_name,
					   WS_OVERLAPPEDWINDOW | WS_VISIBLE,
					   CW_USEDEFAULT, CW_USEDEFAULT,
					   os_context.width, os_context.height,
					   0, 0, instance, 0);

	if(os_context.window == NULL)
	{
		MessageBox(0, "Failed to create a window .", "ERROR", MB_OK | MB_ICONERROR);
		return(-1);
	}

	os_context.screen_width = GetSystemMetrics(SM_CXSCREEN);
	os_context.screen_height = GetSystemMetrics(SM_CYSCREEN);

	os_context.device_context = GetDC(os_context.window);

	return(1);
}

void
os_memory_alloc(Size size)
{
	os_state.app_memory_size = size;
	os_state.app_memory = mem_alloc(os_state.app_memory_size, false);
}

void
os_memory_free(void)
{
	mem_free(os_state.app_memory);
}

void
os_time_init(struct TimeLimiter *time_limiter, u32 fps_max)
{
	os_state.fps_max = fps_max;
	LARGE_INTEGER performance_counter_frequency_result;

	QueryPerformanceFrequency(&performance_counter_frequency_result);
	os_performance_counter_frequency = performance_counter_frequency_result.QuadPart;

	u32 scheduler_ms = 1;
	bool granular_sleep_bool = timeBeginPeriod(scheduler_ms) == TIMERR_NOERROR;

	time_limiter_init(time_limiter, TIME_NS_FROM_S(fps_max));
}

/* Window Functions */
void
os_fullscreen_toggle()
{
	DWORD style = GetWindowLong(os_context.window, GWL_STYLE);
	if(style & WS_OVERLAPPEDWINDOW)
	{
		MONITORINFO monitor_info = {sizeof(monitor_info)};
		if(GetWindowPlacement(os_context.window, &os_context.window_position) &&
		   GetMonitorInfo(MonitorFromWindow(os_context.window, MONITOR_DEFAULTTOPRIMARY), &monitor_info))
		{
			SetWindowLong(os_context.window, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
			SetWindowPos(os_context.window, HWND_TOP,
						 monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
						 monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
						 monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED
				);

			/* TODO(neet): For stretching */
			//os_context.width = os_context.screen_width;
			//os_context.height = os_context.screen_height;
		}
	}
	else
	{
		SetWindowLong(os_context.window, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(os_context.window, &os_context.window_position);
		SetWindowPos(os_context.window, 0, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED
			) ;
	}

	os_context.fullscreen = !os_context.fullscreen;
}

void
os_mouse_set_position(struct Input_Mouse *mouse, v2 pos)
{
	SetCursorPos((i32)pos.x, (i32)pos.y);
}

void
os_events(void)
{

	struct Input_Keyboard *keyboard = &input.keyboards[0];
	struct Input_Mouse *mouse = &input.mice[0];

	for(i32 i = 0; i < COUNT(keyboard->keys); ++i)
	{
		if(keyboard->keys[i]) keyboard->down[i] = 1;
		else keyboard->down[i] = 0;
	}

	for(i32 i = 0; i < COUNT(mouse->buttons); ++i)
	{
		if(mouse->buttons[i]) mouse->down[i] = 1;
		else mouse->down[i] = 0;
	}

	f32 w = 1920;
	f32 h = 1080;

	/* TODO: Should this be here? */
	POINT pos_point;
	GetCursorPos(&pos_point);
	v2 pos_new = V2((f32)pos_point.x, (f32)(h - pos_point.y));
	v2 pos_center = V2(w/2, h/2); /* TODO(lunug): Temporary constants, use middle of game screen */
	if(mouse->locked && GetFocus())
	{ /* SPEED */
		if(!v2_eql(pos_center, pos_new))
		{
			mouse->d = v2_s(pos_new, mouse->p);
			//v2_print(mouse->d, "D: ");
			mouse->moved = 1;

			os_mouse_set_position(mouse, pos_center);
			mouse->p = pos_center;
		}
		else
		{
			mouse->moved = 0;
		}
	}
	else
	{
		/* TODO IMPLEMENT(lungu): */
	}
	if(!mouse->moved) mouse->d = V2_ZERO;

	MSG message;
	while(PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		switch(message.message)
		{
		/* keyboard input */
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			u32 k = (u32)message.wParam;
			bool alt_down = ((message.lParam & (1 << 29)) != 0);
			bool was_down = ((message.lParam & (1 << 30)) != 0);
			bool key_down = ((message.lParam & (1 << 31)) == 0);

			//if(key_down != was_down)
			{
				if(k >= 65 && k <= 90)
				{
					k += 32;
				}

				keyboard->down[k] = was_down;
				keyboard->keys[k] = key_down;

				if(keyboard->keys[KEY_ENTER] && alt_down)
				{
					os_fullscreen_toggle();
				}

				/* TODO(neet): Change this to f4 */
				if(keyboard->keys['q'])
				{
					os_state.running = 0;
				}
			}

			if(k <= 10)
			{
				mouse->buttons[k] = 1;
			}
		} break;

		/* mosue input */
		case WM_LBUTTONDOWN:
		{
			mouse->buttons[BUTTON_LEFT] = 1;
		} break;
		case WM_LBUTTONUP:
		{
			mouse->buttons[BUTTON_LEFT] = 0;
		} break;
		case WM_MBUTTONDOWN:
		{
			mouse->buttons[BUTTON_MIDDLE] = 1;
		} break;
		case WM_MBUTTONUP:
		{
			mouse->buttons[BUTTON_MIDDLE] = 0;
		} break;
		case WM_RBUTTONDOWN:
		{
			mouse->buttons[BUTTON_RIGHT] = 1;
		} break;
		case WM_RBUTTONUP:
		{
			mouse->buttons[BUTTON_RIGHT] = 0;
		} break;
		/* mosue movement */
		#if 0
		case WM_MOUSEMOVE:
		{
			mouse->moved = 1;

			v2 position_new = V2(GET_X_LPARAM(message.lParam),
						 os_context.height - GET_Y_LPARAM(message.lParam) - 1);

			mouse->d = v2_s(position_new, mouse->p);

			mouse->p = position_new;
		} break;
		#endif

		default:
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
		}
	}
}

void
os_render(void)
{
	SwapBuffers(os_context.device_context);
}
