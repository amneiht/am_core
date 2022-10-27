/*
 * tree_help.c
 *
 *  Created on: Oct 17, 2022
 *      Author: amneiht
 */

#include "local_core.h"

enum tree_state {
	move_left, move_right, move_up, move_end
};
ACORE_LOCAL void _acore_tree_view(pj_rbtree *tree, tree_handle *handle,
		void *data) {
	pj_rbtree_node *node = tree->root;
	pj_rbtree_node *old = NULL;
	pj_rbtree_node *root = node;
	pj_rbtree_node *null_node = tree->null;
	enum tree_state state = move_left;
	// stat serach
	const pj_str_t *key;
	while (state != move_end && node != null_node) {
		key = node->key;
		switch (state) {
		case move_left:
			if (node->right == null_node && node->left == null_node) {
				// leaf node
				handle(node, data);
				old = node;
				node = node->parent;
				state = move_up;
			} else if (node->left != null_node) {
				state = move_left;
				node = node->left;
			} else {
				state = move_right;
				node = node->right;
			}
			break;
		case move_right:
			if (node->right == null_node && node->left == null_node) {
				// leaf node
				handle(node, data);
				old = node;
				node = node->parent;
				state = move_up;
			} else if (node->left != null_node) {
				state = move_left;
				node = node->left;
			} else {
				state = move_right;
				node = node->right;
			}
			break;
		case move_up:
			if (old != node->right && node->right != null_node) {
				state = move_right;
				node = node->right;
			} else {
				handle(node, data);
				old = node;
				node = node->parent;
				state = move_up;
			}
			break;
		default:
			break;
		}
		if (node == root && state == move_up) {
			if (node->right == node->left)
				state = move_end;
			else if (old == node->left && node->right == null_node)
				state = move_end;
			else if (old == node->right)
				state = move_end;
			if (state == move_end) {
				handle(node, data);
			}
		}
	}
	(void) key;
}
