#include "include/docksys.h"
#include "i3_docking.h"
#include <stdlib.h>
#include <stdbool.h>
#include <pd_common.h>
#include <pd_docking.h>

struct DummyData {
	int data;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void* create_instance(int x, int y, int width, int height) {
	docksys_create(x, y, width, height);
	return malloc(sizeof(struct DummyData));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void destroy_instance(void* data) {
	free(data);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_callbacks(void* instance, PDDockingCallbacks* callbacks) {
	(void)instance;
	(void)callbacks;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void save_state(void* instance, const char* filename) {
	(void)instance;
	(void)filename;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void load_state(void* instance, const char* filename) {
	(void)instance;
	(void)filename;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void update(void* instance) {
	(void)instance;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void set_mouse(void* instance, void* user_data, float x, float y, bool left_down) {
	(void)instance;
	(void)user_data;
	(void)x;
	(void)y;
	(void)left_down;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void split(void* instance, void* user_data, PDDockingSplitDir dir, PDDockHandle handle) {
	(void)instance;
	(void)user_data;
	(void)dir;
	(void)handle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static PD_EXPORT PDDocking plugin = {
	"i3_docking_system",

	create_instance,
	destroy_instance,

	docksys_set_callbacks,
	split,

	//docksys_horizontal_split,
	//docksys_vertical_split,

	docksys_close_con,
	docksys_update_size,
	docksys_set_mouse,

	save_state,
	load_state,

	update,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PD_EXPORT void InitPlugin(RegisterPlugin* registerPlugin, void* private_data) {
    registerPlugin(PD_DOCKING_API_VERSION, &plugin, private_data);
}

