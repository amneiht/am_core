/*
 * test.h
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */

#ifndef TEST_H_
#define TEST_H_


struct test_api {
	void (*print)(void);
	void (*pow)(int i);
};
extern void test_init(pj_pool_t *pool);
extern void test_close();
#endif /* TEST_H_ */
