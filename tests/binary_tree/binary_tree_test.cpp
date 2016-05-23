#define BOOST_TEST_MODULE BinaryTreeTest

#include <iostream>
#include <string>
#include <boost/test/included/unit_test.hpp>
#include "test_container.cpp"
#include "binary_tree/binary_tree.cpp"

using namespace std;

BOOST_AUTO_TEST_CASE(bintree_seq_test) {
	auto db = new BinaryTree<string, string>(&compare_str, new BTNode<string, string>("",""));
	auto db_int = new BinaryTree<int, string>(&compare_int, new BTNode<int, string>(0, ""));

    test_str_container(db);
    test_int_container(db_int);
    
    delete db_int;
    delete db;
}

BOOST_AUTO_TEST_CASE(bintree_parallel_test) {   
    auto db_ii = new BinaryTree<int, int>(&compare_int, new BTNode<int, int>(0, 0));
	test_int_container_parallel(db_ii);
	delete db_ii;
}