#define BOOST_TEST_MODULE ConcMapTest

#include <iostream>
#include <string>
#include <boost/test/included/unit_test.hpp>
#include "test_container.cpp"
#include "linked_list/linked_list.cpp"
#include "concurrent_map/concurrent_map.cpp"

using namespace std;

#define NUM_BUCKETS 20
inline int silly_hash(int &i) {
    return (i % NUM_BUCKETS + NUM_BUCKETS) % NUM_BUCKETS;
}

BOOST_AUTO_TEST_CASE(cmap_seq_test) {
    LinkedList<int, string> list(&compare_int, 0, "");

	auto map = new ConcurrentMap<int, string, LinkedList<int, string>>(&silly_hash, list, NUM_BUCKETS);
    test_int_container(map);
    delete map;
}

BOOST_AUTO_TEST_CASE(cmap_parallel_test) {
    LinkedList<int, int> list(&compare_int, 0, 0);

    auto map = new ConcurrentMap<int, int, LinkedList<int, int>>(&silly_hash, list, NUM_BUCKETS);
    map->print();
	test_int_container_parallel(map);
    map->print();
	delete map;
}