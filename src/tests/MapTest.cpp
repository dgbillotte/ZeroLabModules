#include <iostream>
// #include "Grain.hpp"
// #include "AllPassFilter.hpp"
// #include <assert.h>
#include <map>
#include <string>
int DUMP = true;
// #include "dump.hpp"

typedef std::shared_ptr<int> intPtr;
typedef std::map<std::string, intPtr> intDict;
typedef std::pair<std::string, intPtr> strIntPair;

int main() {

    intDict m;

    m.insert(strIntPair("foo", intPtr(new int(5))));

    intPtr ip = m["foo"];

    std::cout << "retrieved value: " << *ip << std::endl;

    return 0;

}
