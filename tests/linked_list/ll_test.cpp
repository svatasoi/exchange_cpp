#define BOOST_TEST_MODULE LinkedListTest

#include <iostream>
#include <string>
#include <boost/test/included/unit_test.hpp>
#include "test_container.cpp"
#include "linked_list/linked_list.cpp"

using namespace std;

BOOST_AUTO_TEST_CASE(llist_seq_test) {
	auto db = new LinkedList<string, string>(&compare_str, "", "");
    auto db_int = new LinkedList<int, string>(&compare_int, 0, "");
	
    test_str_container(db);
    test_int_container(db_int);
    
    delete db_int;
    delete db;
}

BOOST_AUTO_TEST_CASE(llist_parallel_test) {
	auto db_ii = new LinkedList<int, int>(&compare_int, -1, -1);
	test_int_container_parallel(db_ii);
	delete db_ii;
}