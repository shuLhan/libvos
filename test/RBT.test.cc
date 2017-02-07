//
// Copyright 2009-2017 M. Shulhan (ms@kilabit.info). All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "test.hh"
#include "../RBT.hh"

using vos::Object;
using vos::BNode;
using vos::TreeNode;
using vos::RBT;

Buffer* b = NULL;

int expect_red_nodes(RBT* rbt, const char* exp)
{
	if (!exp) {
		return 0;
	}

	const char* got = rbt->get_red_node_chars();

	int s = strcmp(exp, got);

	if (s == 0) {
		return 0;
	}

	printf("RBT expecting red nodes: %s\n", exp);
	printf("RBT                 got: %s\n", got);

	return 1;
}

void do_insert(RBT* rbt, const char v, const char* exp_red_nodes)
{
	TreeNode* node = NULL;
	Buffer tree_before;
	Buffer tree_after;
	int s = 0;

	b = new Buffer();
	b->copy_raw(&v, 1);
	node = new TreeNode(b);

	tree_before.copy_raw(rbt->chars());

	rbt->insert(node);

	tree_after.copy_raw(rbt->chars());

	s = expect_red_nodes(rbt, exp_red_nodes);
	if (s) {
		printf("RBT: tree before:\n%s\n", tree_before.chars());
		printf("RBT: insert: %s\n", node->chars());
		printf("RBT: tree after :\n%s\n", tree_after.chars());
	}
}

void tree_populate(RBT* rbt)
{
	const char* exps[] = {
			"\0"
		,	"b"
		,	"a c"
		,	"d"
		,	"c e"
		,	"d f"
		,	"d e g"
		,	"b f h"
		,	"b f g i"
		,	"h j"
		,	"h i k"
		,	"f j l"
		,	"f j k m"
		,	"h l n"
		,	"h l m o"
		,	"h j n p"
		,	"h j n o q"
		,	"d l p r"
		,	"d l p q s"
		,	"d l n r t"
		,	"d l n r s u"
		,	"p t v"
		,	"p t u w"
		,	"p r v x"
		,	"p r v w y"
		,	"l t x z"
		};
	int exp_idx = 0;
	char v = 'a';

	while (v <= 'z') {
		do_insert(rbt, v++, exps[exp_idx++]);
		assert(rbt->is_balance());
	}
}

void do_remove(RBT* rbt, const char node_item, const char* exp)
{
	Buffer tree_before;
	Buffer tree_after;
	TreeNode* node = NULL;
	TreeNode* got = NULL;
	int s;

	b->reset();
	b->appendc(node_item);

	node = rbt->find(b);

	if (!node) {
		return;
	}

	tree_before.copy_raw(rbt->chars());

	got = rbt->remove(node);
	delete got;

	tree_after.copy_raw(rbt->chars());

	if (exp) {
		s = expect_red_nodes(rbt, exp);
		if (s == 0) {
			return;
		}

		printf("RBT: before:\n%s\n", tree_before.chars());
		printf("RBT: remove: %c\n", node_item);
		printf("RBT: after :\n%s\n", tree_after.chars());

		exit(1);
	} else {
		if (! rbt->is_balance()) {
			printf("RBT: before:\n%s\n", tree_before.chars());
			printf("RBT: remove: %c\n", node_item);
			printf("RBT: after :\n%s\n", tree_after.chars());
		}
	}
}

void test_insert()
{
	RBT rbt(Object::CMP);

	printf("RBT: test insertion\n");

	tree_populate(&rbt);

	do_insert(&rbt, 'a', "a l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'b', "a b l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'c', "a b c l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'd', "a b c d l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'e', "a b c d e l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'f', "a b c d e f l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'g', "a b c d e f g l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'h', "a b c d e f g h l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'i', "a b c d e f g h i l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'j', "a b c d e f g h i j l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'k', "a b c d e f g h i j k l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'l', "a b c d e f g h i j k l l t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'm', "a b c d e f g h i j k l l m t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'n', "a b c d e f g h i j k l l m n t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'o', "a b c d e f g h i j k l l m n o t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'p', "a b c d e f g h i j k l l m n o p t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'q', "a b c d e f g h i j k l l m n o p q t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'r', "a b c d e f g h i j k l l m n o p q r t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 's', "a b c d e f g h i j k l l m n o p q r s t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 't', "a b c d e f g h i j k l l m n o p q r s t t x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'u', "a b c d e f g h i j k l l m n o p q r s t t u x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'v', "a b c d e f g h i j k l l m n o p q r s t t u v x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'w', "a b c d e f g h i j k l l m n o p q r s t t u v w x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'x', "a b c d e f g h i j k l l m n o p q r s t t u v w x x z");
	assert(rbt.is_balance());
	do_insert(&rbt, 'y', "a b c d e f g h i j k l l m n o p q r s t t u v v w y y");
	assert(rbt.is_balance());
	do_insert(&rbt, 'z', "a b c d e f g h i j k l l m n o p q r s t t u v v w y y z");
	assert(rbt.is_balance());
}

//
// Insert from a to z and remove from a to z.
//
void test_remove_ascending()
{
	const char* exps[] = {
			"c f l x z"
		,	"f l x z"
		,	"e l x z"
		,	"l x z"
		,	"g j x z"
		,	"j x z"
		,	"i x z"
		,	"x z"
		,	"k n t x z"
		,	"n t x z"
		,	"m t x z"
		,	"t x z"
		,	"o r x z"
		,	"r x z"
		,	"q x z"
		,	"x z"
		,	"s z"
		,	"z"
		,	"u x z"
		,	"x z"
		,	"w z"
		,	"z"
		,	"\0"
		,	"z"
		,	"\0"
		,	"\0"
		};

	RBT rbt(Object::CMP);
	int exp_idx = 0;
	char v = 'a';

	tree_populate(&rbt);

	printf("RBT: remove tree ascending\n");

	b = new Buffer();
	while (v <= 'z') {
		do_remove(&rbt, v++, exps[exp_idx++]);
		assert(rbt.is_balance());
	}
	delete b;
}

//
// Insert from a to z and remove from z to a.
//
void test_remove_descending()
{
	const char* exps[] = {
			"l t x"
		,	"l t w"
		,	"l t"
		,	"l r u"
		,	"l r"
		,	"l s"
		,	"l"
		,	"n q"
		,	"n"
		,	"o"
		,	"\0"
		,	"d j m"
		,	"d j"
		,	"d k"
		,	"d"
		,	"f i"
		,	"f"
		,	"g"
		,	"\0"
		,	"b e"
		,	"b"
		,	"c"
		,	"\0"
		,	"a"
		,	"\0"
		,	"\0"
		};

	RBT rbt(Object::CMP);
	char v = 'z';
	int exp_idx = 0;

	tree_populate(&rbt);

	printf("RBT: remove tree descending\n");

	b = new Buffer();
	while (v >= 'a') {
		do_remove(&rbt, v--, exps[exp_idx++]);
		assert(rbt.is_balance());
	}
	delete b;
}

//
// Insert from a to z and remove manually.
//
void test_remove_manual_01()
{
	RBT rbt(Object::CMP);

	tree_populate(&rbt);

	printf("RBT: remove tree manual 01\n");

	b = new Buffer();

	do_remove(&rbt, 'x', "l t y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'g', "b e l y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'w', "b e l z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'm', "b e j o z");
	assert(rbt.is_balance());
	do_remove(&rbt, 's', "b e h j o q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'k', "b e h i o q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'j', "b e h o q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'c', "a e h o q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'n', "a e h q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'j', "a d e q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'l', "a d e o q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'i', "a d e q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'e', "a d q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'p', "a f q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'a', "f q v z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'u', "f q y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'q', "f y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'h', "y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'b', "f");
	assert(rbt.is_balance());
	do_remove(&rbt, 'o', "\0");
	assert(rbt.is_balance());
	do_remove(&rbt, 'r', "d y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'z', "d v");
	assert(rbt.is_balance());
	do_remove(&rbt, 'f', "v");
	assert(rbt.is_balance());
	do_remove(&rbt, 't', "\0");
	assert(rbt.is_balance());
	do_remove(&rbt, 'd', "y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'v', "\0");
	assert(rbt.is_balance());
	do_remove(&rbt, 'y', "\0");
	assert(rbt.is_balance());

	delete b;
}

void test_remove_manual_02()
{
	RBT rbt(Object::CMP);

	tree_populate(&rbt);

	printf("RBT: remove tree manual 02\n");

	b = new Buffer();

	do_remove(&rbt, 'c', "a f l x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'k', "a f i n x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'd', "f i n x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'g', "e i n x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'q', "e i n s z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'x', "e i n s");
	assert(rbt.is_balance());
	do_remove(&rbt, 'v', "e i n");
	assert(rbt.is_balance());
	do_remove(&rbt, 'b', "i n");
	assert(rbt.is_balance());
	do_remove(&rbt, 'n', "i o");
	assert(rbt.is_balance());
	do_remove(&rbt, 'p', "i");
	assert(rbt.is_balance());
	do_remove(&rbt, 'h', "a i l u");
	assert(rbt.is_balance());
	do_remove(&rbt, 's', "a i l t y");
	assert(rbt.is_balance());
	do_remove(&rbt, 'u', "a i l y");
	assert(rbt.is_balance());

	do_remove(&rbt, 'w', "a i l z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'm', "a j z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'j', "a l z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'j', "z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'l', "a z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'i', "z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'e', "f t z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'a', "t z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'o', "r z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'r', "z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'z', "\0");
	assert(rbt.is_balance());
	do_remove(&rbt, 'f', "y");
	assert(rbt.is_balance());
	do_remove(&rbt, 't', "\0");
	assert(rbt.is_balance());
	do_remove(&rbt, 'y', "\0");
	assert(rbt.is_balance());

	delete b;
}

void test_remove_manual_03()
{
	RBT rbt(Object::CMP);

	tree_populate(&rbt);

	printf("RBT: remove tree manual 03\n");

	b = new Buffer();

	do_remove(&rbt, 'g', "b e l x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'u', "b e l w z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'e', "b l w z");
	assert(rbt.is_balance());
	do_remove(&rbt, 't', "b q w x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'n', "b o q w z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'b', "c o q w z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'v', "c o q z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'm', "c q z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'a', "q z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'p', "z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'o', "h r x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'l', "d i r x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'j', "d r x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'i', "f r x z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'y', "f r x");
	assert(rbt.is_balance());
	do_remove(&rbt, 'c', "r x");
	assert(rbt.is_balance());
	do_remove(&rbt, 'h', "d r");
	assert(rbt.is_balance());
	do_remove(&rbt, 's', "d");
	assert(rbt.is_balance());
	do_remove(&rbt, 'x', "d k z");
	assert(rbt.is_balance());
	do_remove(&rbt, 'q', "f z");
	assert(rbt.is_balance());

	assert(rbt.is_balance());
	do_remove(&rbt, 'w', "f");
	assert(rbt.is_balance());
	do_remove(&rbt, 'k', "d");
	assert(rbt.is_balance());
	do_remove(&rbt, 'r', "\0");
	assert(rbt.is_balance());
	do_remove(&rbt, 'z', "d");
	assert(rbt.is_balance());
	do_remove(&rbt, 'f', "\0");
	assert(rbt.is_balance());
	do_remove(&rbt, 'd', "\0");
	assert(rbt.is_balance());

	delete b;
}

//
// Insert from a to z and remove randomly.
//
void test_remove_random()
{
	RBT rbt(Object::CMP);

	tree_populate(&rbt);

	printf("RBT: remove random\n");

	b = new Buffer();

	srand((unsigned int) time(NULL));

	while (rbt.get_root_unsafe()) {
		do_remove(&rbt, (char)('a' + (rand() % 26)), NULL);
		assert(rbt.is_balance());
	}

	delete b;
}

void test_random_insert_remove()
{
	int nins = 0;
	RBT rbt(Object::CMP);

	printf("RBT: random insert and remove\n");

	srand((unsigned int) time(NULL));

	while (nins < 30) {
		do_insert(&rbt, (char)('a' + (rand() % 26)), NULL);
		assert(rbt.is_balance());
		nins++;
	}

	b = new Buffer();
	while (rbt.get_root_unsafe()) {
		do_remove(&rbt, (char)('a' + (rand() % 26)), NULL);
		assert(rbt.is_balance());
	}
	delete b;
}

int main()
{
	test_insert();
	test_remove_ascending();
	test_remove_descending();
	test_remove_manual_01();
	test_remove_manual_02();
	test_remove_manual_03();
	test_remove_random();
	test_random_insert_remove();
	test_random_insert_remove();
	test_random_insert_remove();

	return 0;
}
