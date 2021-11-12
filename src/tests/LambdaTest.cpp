#include <iostream>
#define TESTING 

// #include <assert.h>

using namespace std;

int main() {

    auto f = []() { cout << "this is the msg" << endl; };

    f();

    return 0;

}
