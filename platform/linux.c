void
os_path_build(struct OsState *os_state)
{
	/* path_exe */
	readlink("/proc/self/exe", os_state->path_exe, PATH_MAX);

	/* path_run */
	cstr_path_parent(os_state->path_run, os_state->path_exe);

	/* path_data */
	cstr_cat(os_state->path_data, os_state->path_run, "data/");

	/* path */
	cstr_path_parent(os_state->path, os_state->path_run);

	/* path_src */
	cstr_cat(os_state->path_src, os_state->path, "src/");
}

void
os_create_window(struct OsContext *os_context)
{
	os_context->display = XOpenDisplay(NULL);
	if(os_context->display == NULL)
	{
		FATAL("%s", "XOPenDisplay.\n");
	}

	os_context->window_root = DefaultRootWindow(os_context->display);

	GLint gl_attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
	XVisualInfo *visual_info = glXChooseVisual(os_context->display, 0, gl_attributes);
	if(visual_info == NULL)
	{
		fprintf(stderr, "ERROR: glXChooseVisual.\n");
		exit(EXIT_FAILURE);
	}

	Colormap colormap = XCreateColormap(os_context->display, os_context->window_root,
					    visual_info->visual, AllocNone);

	XSetWindowAttributes swa = {0};
	swa.colormap = colormap;
	swa.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
		EnterWindowMask | LeaveWindowMask | PointerMotionMask |
		Button1MotionMask | Button2MotionMask | Button3MotionMask | Button4MotionMask | Button5MotionMask |
		ButtonMotionMask | ExposureMask | StructureNotifyMask | ResizeRedirectMask;

	os_context->width = WIDTH;
	os_context->height = HEIGHT;
	os_context->window_main = XCreateWindow(os_context->display, os_context->window_root,
				      0, 0,
				      os_context->width, os_context->height, 0,
				      visual_info->depth, InputOutput, visual_info->visual,
				      CWColormap | CWEventMask, &swa
		);

	XMapWindow(os_context->display, os_context->window_main);
	XStoreName(os_context->display, os_context->window_main, "Grand Theft Desu");

	os_context->gl_context = glXCreateContext(os_context->display, visual_info, NULL, GL_TRUE);
	glXMakeCurrent(os_context->display, os_context->window_main, os_context->gl_context);
}

void
os_memory_alloc(struct OsState *os_state)
{
	os_state->app_memory_size = GB(2);
	os_state->app_memory = mem_alloc(os_state->app_memory_size, false);
}

void
os_memory_free(struct OsState *os_state)
{
	mem_free(os_state->app_memory);
}

void
os_mouse_move(struct OsContext *os_context)
{
	/* IMPLEMENT(lungu): Position */
	/* FIXME(lungu): If you move the mouse too fast or if your sensitivity is too high
	   the cursor either goes out of the window or it hits the edge of the screen */
	XWarpPointer(os_context->display, None, os_context->window_main,
		     0, 0, os_context->width, os_context->height,
		     os_context->width / 2, os_context->height / 2);
}

void
os_mouse_lock(struct OsContext *os_context, struct Input *os_input)
{
	struct Input_Mouse *mouse = &os_input->mice[0];

	v2i cursor_pos;
	int ignored_int;
	Window ignored_window;
	XQueryPointer(os_context->display, os_context->window_main, &ignored_window, &ignored_window,
		      &ignored_int, &ignored_int, &cursor_pos.x, &cursor_pos.y, &ignored_int);

	/* FIXME(lungu) */
	//v2 pos_new = V2();
	if(mouse->locked /* IMPLEMENT && Window is focused */)
	{
		#if 0
		if(!v2_eql(pos_center, pos_new))
		{
			mouse->d = v2_s(pos_new, mouse->p);
			//v2_print(mouse->d, "D: ");
			mouse->moved = 1;

			os_mouse_move(os_context);
			mouse->p = pos_center;
		}
		else
		{
			mouse->moved = 0;
		}
		#endif

		os_mouse_move(os_context);
		if(mouse->visible)
		{
			Cursor cursor;
			Pixmap cursor_pix;
			XColor dummy;

			cursor_pix = XCreateBitmapFromData(os_context->display, os_context->window_main, "", 1, 1);
			cursor = XCreatePixmapCursor(os_context->display,
						     cursor_pix, cursor_pix, &dummy, &dummy, 1, 1);

			XFreePixmap(os_context->display, cursor_pix);
			XDefineCursor(os_context->display, os_context->window_main, cursor);

			mouse->visible = false;
		}
	}
	else
	{
		if(!mouse->visible)
		{
			XUndefineCursor(os_context->display, os_context->window_main);
			mouse->visible = true;
		}
	}
}

void
os_process_events(struct OsContext *context, struct OsState *state, struct Input *input)
{

	struct Input_Keyboard *keyboard = &input->keyboards[0];
	struct Input_Mouse *mouse = &input->mice[0];

	/* SPEED(lungu): */
	for(i32 i = 0; i < COUNT(keyboard->keys); ++i)
	{
		keyboard->down[i] = keyboard->keys[i];
	}

	for(i32 i = 0; i < COUNT(mouse->buttons); ++i)
	{
		mouse->down[i] = mouse->buttons[i];
	}

	mouse->moved = 0;
	os_mouse_lock(context, input);

	u32 half_width = context->width / 2;
	u32 half_height = context->height / 2;

	XEvent event;
	while(XPending(context->display))
	{
		XNextEvent(context->display, &event);
		switch(event.type)
		{
			#if 1
			case KeyPress:
			case KeyRelease:
			{
				/* INCOMPLETE: Implement sparce storage for keyboard */
				KeySym k = XLookupKeysym(&event.xkey, 0);

				bool shift_down = keyboard->keys[KEY_SHIFT_LEFT] ||
					keyboard->keys[KEY_SHIFT_RIGHT];
				bool alt_down = keyboard->keys[KEY_ALT_LEFT] ||
					keyboard->keys[KEY_ALT_RIGHT];

				/*
				if(keyboard->keys[KEY_CAPSLOCK] && !keyboard->down[KEY_CAPSLOCK])
				{
					if(keyboard->capslock_enabled)
					{
						keyboard->capslock_enabled = false;
					}
					else
					{
						keyboard->capslock_enabled = true;
					}
				}
				printf("CAPS: %d\n", keyboard->capslock_enabled);
				*/

				if('a' <= k <= 'z')
				{
					if(shift_down || keyboard->capslock_enabled)
					{

						if(keyboard->keys[k - 32])
						{
							keyboard->down[k - 32] = true;
						}
						else
						{
							keyboard->down[k - 32] = false;
						}
						keyboard->keys[k] = false;
						keyboard->keys[k - 32] = (event.type == KeyPress);
					}
					else
					{
						if(keyboard->keys[k])
						{
							keyboard->down[k] = true;
						}
						else
						{
							keyboard->down[k] = false;
						}

						keyboard->keys[k] = (event.type == KeyPress);
						keyboard->keys[k - 32] = false;
					}
				}

				/* operating system cases */
				if(alt_down && keyboard->keys[KEY_F4])
				{
					state->running = false;
				}
			} break;
			#endif
			case ButtonPress:
			case ButtonRelease:
			{
				u32 b = event.xbutton.button;
				mouse->buttons[b] = (event.type == ButtonPress);
			} break;
			case MotionNotify:
			{
				/* NOTE - This way is better than "mouse->d = mouse->p - new_pos" */
				mouse->moved = 1;
				v2 new_pos = V2(event.xmotion.x, context->height - event.xmotion.y);

				mouse->d = V2(new_pos.x - half_width,
					      new_pos.y - half_height);

				mouse->p = new_pos;
			} break;
		}
	}
}

void
os_render(struct OsContext *os_context)
{
	glXSwapBuffers(os_context->display, os_context->window_main);
}
