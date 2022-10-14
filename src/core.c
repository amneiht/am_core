/*
 * core.c
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */
#include <acore.h>
extern void* mod_stdio_load();
int main(int argc, char **argv) {
	acore_init();
	mod_stdio_load();
	acore_loop();
	acore_close();
}
