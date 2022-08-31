/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */
// test list
#include <acore.h>
#include <acore/acore_list.h>
typedef struct plist {
	ACORE_DECL_LIST_ELEMENT(struct plist)
	;
	pj_int32_t priority;
} plist;
void list_clear(void *data) {
	plist *pp = (plist*) data;
	PJ_LOG(1, (ACORE_NAME,"clear data for list id : %d",pp->id));
}
static pj_int32_t element_cmp(acore_list_element *ele1,
		acore_list_element *ele2) {
	plist *p1 = ele1;
	plist *p2 = ele2;
	if (p1->priority == p2->priority)
		return 0;
	if (p1->priority > p2->priority)
		return 1;
	return -1;
}
pj_int32_t invert_element_cmp(acore_list_element *ele1,
		acore_list_element *ele2) {
	plist *p1 = ele1;
	plist *p2 = ele2;
	if (p1->priority == p2->priority)
		return 0;
	if (p1->priority > p2->priority)
		return -1;
	return 1;
}
plist* new_list_ele(pj_pool_t *pool, int prioty) {
	plist *ld = pj_pool_alloc(pool, sizeof(plist));
	acore_list_init_element(ld);
	ld->priority = prioty;
	ld->id = prioty;
	ld->clear = list_clear;
	ld->user_data = ld;
	return ld;
}
static pj_bool_t print_id(void *list, void *arg) {
	plist *pp = (plist*) list;
	PJ_LOG(1, (ACORE_NAME,"print id is:%d",pp->id));
	return PJ_FALSE;
}
int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("pool", 4096, 4096);

	// test u8n sort list
	acore_list_t *list = acore_list_create(pool, NULL);
	for (int i = 1; i < 11; i++) {
		plist *tmp = new_list_ele(pool, i);
		acore_list_add(list, tmp);
	}
	acore_list_search(list, print_id, NULL);
	puts("ss");
	acore_list_remove_id(list, 7);
	plist *tt = acore_list_element_at(list, 100);
	acore_list_remove(list, tt);
	acore_list_search(list, print_id, NULL);
	acore_list_clear(list);
	acore_list_search(list, print_id, NULL);

	// sort list
	puts("\n\ntest_list with sort");
	list = acore_list_create(pool, element_cmp);
	for (int i = 1; i < 11; i++) {
		plist *tmp = new_list_ele(pool, 30 - i);
		acore_list_add(list, tmp);
	}
	acore_list_search(list, print_id, NULL);
	puts("ss");
	acore_pool_release(pool);
	acore_close();
	return 0;
}
