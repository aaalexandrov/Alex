#include <stdio.h>
#include <string.h>
#include <xcb/xcb.h>

const char WindowName[] = "My first XCB window";

int main(int argc, char argv[argc + 1])
{
	printf("Hi XCB!\n");

	int screenNum;
	xcb_connection_t *connection = xcb_connect(NULL, &screenNum);

	xcb_setup_t const* const setup = xcb_get_setup(connection);
	xcb_screen_iterator_t screenIter = xcb_setup_roots_iterator(setup);
	for (int i = 0; i < screenNum; ++i) {
		xcb_screen_next(&screenIter);
	}

	xcb_screen_t *screen = screenIter.data;

	xcb_window_t window = xcb_generate_id(connection);
	uint32_t windowMask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	uint32_t windowVal[] = {screen->white_pixel, XCB_EVENT_MASK_EXPOSURE};

	xcb_create_window(
		connection,
		XCB_COPY_FROM_PARENT,
		window,
		screen->root,
		100, 100, 500, 500,
		5,
		XCB_WINDOW_CLASS_INPUT_OUTPUT,
		screen->root_visual,
		windowMask, windowVal);

	xcb_change_property(connection, XCB_PROP_MODE_REPLACE, window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8, strlen(WindowName), WindowName);

	/*
	xcb_gcontext_t gc = xcb_generate_id(connection);
	uint32_t gcMask = XCB_GC_BACKGROUND;
	uint32_t gcVal[1] = { screen->white_pixel };
	xcb_create_gc(connection, gc, window, gcMask, gcVal);
	*/

	xcb_map_window(connection, window);
	xcb_flush(connection);

	xcb_generic_event_t *e;
	while ((e = xcb_wait_for_event(connection))) {

	}

	xcb_disconnect(connection);

	return 0;
}
