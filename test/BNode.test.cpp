//
// Copyright 2009-2016 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test.hpp"
#include "../BNode.hpp"

using vos::BNode;

Buffer *b = NULL;

//
// Precondition:
// 	NULL <= node => NULL
// Postcondition:
//	NULL <= nodeL <=> nodeL2 <=> node => NULL
//
void test_insert_left(BNode* node)
{
	b = new Buffer();
	b->copy_raw(STR_TEST_1);

	BNode *nodeL = new BNode(b);
	node->insert_left(nodeL);

	//
	// NULL <= nodeL <=> node => NULL
	//

	assert(nodeL == node->_left);
	assert(node->_right == NULL);
	assert(strcmp(STR_TEST_1, node->_left->chars()) == 0);
	assert(strcmp(STR_TEST_0, node->chars()) == 0);

	assert(NULL == nodeL->_left);
	assert(nodeL->_right == node);
	assert(strcmp(STR_TEST_1, nodeL->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeL->_right->chars()) == 0);

	b = new Buffer();
	b->copy_raw(STR_TEST_2);

	BNode *nodeL2 = new BNode(b);
	node->insert_left(nodeL2);

	//
	// NULL <= nodeL <=> nodeL2 <=> node => NULL
	//

	assert(NULL == nodeL->_left);
	assert(nodeL->_right == nodeL2);
	assert(strcmp(STR_TEST_2, nodeL->_right->chars()) == 0);
	assert(strcmp(STR_TEST_1, nodeL->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeL->_right->_right->chars()) == 0);

	assert(nodeL == nodeL2->_left);
	assert(nodeL2->_right == node);
	assert(strcmp(STR_TEST_2, nodeL2->chars()) == 0);
	assert(strcmp(STR_TEST_1, nodeL2->_left->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeL2->_right->chars()) == 0);

	assert(nodeL2 == node->_left);
	assert(node->_right == NULL);
	assert(strcmp(STR_TEST_2, node->_left->chars()) == 0);
	assert(strcmp(STR_TEST_1, node->_left->_left->chars()) == 0);
	assert(strcmp(STR_TEST_0, node->chars()) == 0);
}

//
// Precondition:
//	NULL <= node => NULL
// Postcondition:
//	NULL <= node <=> nodeR2 <=> nodeR => NULL
//
void test_insert_right(BNode *node)
{
	b = new Buffer();
	b->copy_raw(STR_TEST_1);

	BNode *nodeR = new BNode(b);
	node->insert_right(nodeR);

	//
	// NULL <= node <=> nodeR => NULL
	//

	assert(NULL == node->_left);
	assert(node->_right == nodeR);
	assert(strcmp(STR_TEST_1, node->_right->chars()) == 0);
	assert(strcmp(STR_TEST_0, node->chars()) == 0);

	assert(node == nodeR->_left);
	assert(nodeR->_right == NULL);
	assert(strcmp(STR_TEST_1, nodeR->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeR->_left->chars()) == 0);

	// TEST INSERT RIGHT 2

	b = new Buffer();
	b->copy_raw(STR_TEST_2);

	BNode *nodeR2 = new BNode(b);
	node->insert_right(nodeR2);

	//
	// NULL <= node <=> nodeR2 <=> nodeR => NULL
	//

	assert(NULL == node->_left);
	assert(node->_right == nodeR2);
	assert(strcmp(STR_TEST_2, node->_right->chars()) == 0);
	assert(strcmp(STR_TEST_1, node->_right->_right->chars()) == 0);
	assert(strcmp(STR_TEST_0, node->chars()) == 0);

	assert(node == nodeR2->_left);
	assert(nodeR2->_right == nodeR);
	assert(strcmp(STR_TEST_2, nodeR2->chars()) == 0);
	assert(strcmp(STR_TEST_1, nodeR2->_right->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeR2->_left->chars()) == 0);
}

//
// Precondition:
//	NULL <= node <=> nodeR2 <=> nodeR => NULL
// Postcondition:
//	NULL <= nodeL <=> nodeL2 <=> node <=> nodeR2 <=> nodeR => NULL
void test_push_left(BNode *node)
{
	b = new Buffer();
	b->copy_raw(STR_TEST_2);

	BNode *nodeL2 = new BNode(b);

	node->push_left(nodeL2);

	//
	// NULL <= nodeL2 <=> node <=> nodeR2 <=> nodeR => NULL
	//

	assert(node->_left == nodeL2);
	assert(nodeL2->_left == NULL);
	assert(nodeL2->_right == node);

	assert(strcmp(STR_TEST_2, nodeL2->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeL2->_right->chars()) == 0);

	b = new Buffer();
	b->copy_raw(STR_TEST_1);

	BNode* nodeL = new BNode(b);

	node->push_left(nodeL);

	//
	// NULL <= nodeL <=> nodeL2 <=> node <=> nodeR2 <=> nodeR => NULL
	//

	assert(nodeL2->_left == nodeL);
	assert(nodeL->_left == NULL);
	assert(nodeL->_right == nodeL2);

	assert(strcmp(STR_TEST_2, nodeL->_right->chars()) == 0);
	assert(strcmp(STR_TEST_1, nodeL->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeL->_right->_right->chars()) == 0);
}

// Precondition:
//	NULL <= nodeL <=> nodeL2 <=> node => NULL
// Postcondition:
//	NULL <= nodeL <=> nodeL2 <=> node => nodeR2 <=> nodeR => NULL
void test_push_right(BNode *node)
{
	b = new Buffer();
	b->copy_raw(STR_TEST_2);

	BNode *nodeR2 = new BNode(b);

	node->push_right(nodeR2);

	//
	// NULL <= nodeL <=> nodeL2 <=> node => nodeR2 => NULL
	//

	assert(node->_right == nodeR2);
	assert(nodeR2->_left == node);
	assert(nodeR2->_right == NULL);

	assert(strcmp(STR_TEST_2, nodeR2->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeR2->_left->chars()) == 0);

	b = new Buffer();
	b->copy_raw(STR_TEST_1);

	BNode *nodeR = new BNode(b);

	node->push_right(nodeR);

	//
	// NULL <= nodeL <=> nodeL2 <=> node => nodeR2 <=> nodeR => NULL
	//

	assert(nodeR2->_right == nodeR);
	assert(nodeR->_left == nodeR2);
	assert(nodeR->_right == NULL);

	assert(strcmp(STR_TEST_2, nodeR->_left->chars()) == 0);
	assert(strcmp(STR_TEST_1, nodeR->chars()) == 0);
	assert(strcmp(STR_TEST_0, nodeR->_left->_left->chars()) == 0);
}

// Precondition:
//	NULL <= nodeL <=> nodeL2 <=> node => nodeR <=> nodeR2 => NULL
// Postcondition:
//	NULL <= node => nodeR <=> nodeR2 => NULL
void test_pop_edge(BNode *node)
{
	BNode* pop = node->pop_left_edge();

	// NULL <= nodeL2 <=> node => nodeR <=> nodeR2 => NULL

	assert(pop->_left == NULL);
	assert(pop->_right == NULL);
	assert(node->_left->_left == NULL);
	assert(strcmp(STR_TEST_1, pop->chars()) == 0);
	delete pop;

	pop = node->pop_left_edge();

	// NULL <= node => nodeR <=> nodeR2 => NULL

	assert(pop->_left == NULL);
	assert(pop->_right == NULL);
	assert(node->_left == NULL);
	assert(strcmp(STR_TEST_2, pop->chars()) == 0);
	delete pop;

	pop = node->pop_left_edge();
	assert(pop == NULL);

	pop = node->pop_right_edge();

	// NULL <= node => nodeR => NULL

	assert(pop->_left == NULL);
	assert(pop->_right == NULL);
	assert(node->_right->_right == NULL);
	assert(strcmp(STR_TEST_1, pop->chars()) == 0);
	delete pop;

	pop = node->pop_right_edge();

	// NULL <= node => NULL;
	assert(pop->_left == NULL);
	assert(pop->_right == NULL);
	assert(node->_right == NULL);
	assert(strcmp(STR_TEST_2, pop->chars()) == 0);
	delete pop;

	pop = node->pop_right_edge();
	assert(pop == NULL);
}

int main()
{
	//
	// TEST INSERT LEFT
	//
	BNode *node1 = new BNode(NULL);

	assert(strcmp("BNode", node1->__cname) == 0);

	assert(NULL == node1->_left);
	assert(NULL == node1->_right);
	assert(NULL == node1->chars());

	b = new Buffer();
	b->copy_raw(STR_TEST_0);

	node1->_item = b;

	assert(NULL == node1->_left);
	assert(NULL == node1->_right);
	assert(strcmp(STR_TEST_0, node1->chars()) == 0);

	test_insert_left(node1);

	//
	// TEST INSERT RIGHT
	//

	BNode *node2 = new BNode(NULL);

	b = new Buffer();
	b->copy_raw(STR_TEST_0);

	node2->_item = b;

	test_insert_right(node2);

	//
	// TEST PUSH LEFT and RIGHT
	//
	test_push_left(node2);
	test_push_right(node1);

	// TEST POP
	test_pop_edge(node1);
	test_pop_edge(node2);

	assert(node1->_left == NULL);
	assert(node1->_right == NULL);
	assert(node1->_left == NULL);
	assert(node1->_right == NULL);
	assert(strcmp(STR_TEST_0, node1->chars()) == 0);
	assert(strcmp(STR_TEST_0, node2->chars()) == 0);

	delete node1;
	delete node2;

	// There should be no memory leak at end when tested using Valgrind.
}

// vi: ts=8 sw=8 tw=78:
