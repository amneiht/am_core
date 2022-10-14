#include <acore.h>

int main(int argc, char **argv) {
	acore_init();
	// register some help funtion
	pj_pool_t *pool = acore_pool_create(NULL, 1024, 1024);
	pj_str_t help = pj_str("help");
	acore_ui_command_ele *ele = acore_ui_create_command2(pool, "pika",
			"pika -i <test> -u<test>",
			NULL, NULL, NULL);

	acore_ui_register_command("amneiht", &ele, 1);
	acore_user_input(&help);
	pj_str_t test = pj_str("log set 5");
	acore_user_input(&test);
	pj_pool_release(pool);
	acore_close();
	return 0;
}
