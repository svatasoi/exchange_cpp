#pragma once

#include <string>
#include <thread>
#include <vector>
#include <boost/test/unit_test.hpp>
#include "linked_list/linked_list.cpp"

inline int compare_str(string& s1, string& s2) {
	return s1.compare(s2);
}

inline int compare_int(int& n, int& m) {
	if (n == m) return 0;
	else if (n < m) return -1;
	else return 1;
}

void test_str_container(IContainer<string, string> *cont) {
    BOOST_CHECK(cont->add("yo", "sup"));
	BOOST_CHECK(cont->add("wassup", "data"));
    BOOST_CHECK(cont->add("wassup", "data2") == 0); //already present
	BOOST_CHECK(cont->add("def", "w"));
	BOOST_CHECK(cont->add("elephant", "ele"));
	BOOST_CHECK(cont->add("zika", "zo"));
	BOOST_CHECK(cont->remove("elephant"));
}

void test_int_container(IContainer<int, string> *cont) {
    string s;
    BOOST_CHECK(cont->add(1, "sup"));
	BOOST_CHECK(cont->add(3, "data"));
	BOOST_CHECK(cont->add(4, "w"));
	BOOST_CHECK(cont->add(-2, "ele"));
	BOOST_CHECK(cont->add(-4, "zo"));
    
    BOOST_CHECK(cont->query(4, &s));
    BOOST_CHECK(s.compare("w") == 0);
    
	BOOST_CHECK(cont->add(2, "da"));
	BOOST_CHECK(cont->remove(3));
}

int ops_per_thread = 1000;
int num_threads    =  100;
enum op_t { ADD_OP, REMOVE_OP, NUM_OPS };

void do_random_operations_to_db(IContainer<int, int> *db, unsigned int seed) {
	for (int i = 0; i < ops_per_thread; ++i) {
		op_t op = static_cast<op_t>(rand_r(&seed) % NUM_OPS);
		unsigned int index = 1 + rand_r(&seed) % 100;
		switch (op) {
		case ADD_OP:
			{ 
				int data = rand_r(&seed) % 100;
				db->add(index, data);
			}
			break;
		case REMOVE_OP:
			db->remove(index);
			break;
		case NUM_OPS:
		default:
			continue;
		}
	}
}

void test_int_container_parallel(IContainer<int, int> *cont) {
    vector<thread> threads;
	int t = time(NULL);
	for (int i = 0; i < num_threads; ++i) {
		threads.push_back(thread(&do_random_operations_to_db, cont, t + i));
	}
	for (int i = 0; i < num_threads; ++i) {
		threads[i].join();
	}
}