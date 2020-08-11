struct Game
{
	struct Player player;

	u32 program_pass;

	bool networked;
	struct NetData_Client net_data;
};

struct AppMemory
{
	Size size;
	void *storage;

	bool init;
};

struct AppMemory app_memory;
struct Game *game;
