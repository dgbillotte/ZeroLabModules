#include <iostream>
// #include "Grain.hpp"
// #include "AllPassFilter.hpp"
// #include <assert.h>
#include <vector>
int DUMP = true;
// #include "dump.hpp"

class Foo {
public:
    int _size;
    float _area;
    int _id;
    static int __nextId;

    Foo(int size) :
        _size(size),
        _area(size*size/3),
        _id(__nextId++)
    {
        std::cout << "Foo(" << _id << ") created" << std::endl;
    }

    ~Foo() {
        std::cout << "Foo(" << _id << ") destructor called" << std::endl;
    }
};
int Foo::__nextId = 0;
typedef std::shared_ptr<Foo> FooPtr;

void dumpFoo(FooPtr& foo) {
    std::cout << "Foo(" << foo->_id << "), share count: " << foo.use_count() << std::endl;
}

FooPtr blah() {
    FooPtr fp = FooPtr(new Foo(3));
    dumpFoo(fp);
    return fp;
}

int main() {

    std::vector<FooPtr> foos;

    FooPtr fp = blah();
    std::cout << "just got fp from blah()" << std::endl;
    dumpFoo(fp);

    std::cout << "pushing onto vector" << std::endl;
    foos.push_back(fp);

    dumpFoo(fp);
  
    std::cout << "popping from vector" << std::endl;
    foos.pop_back();
    dumpFoo(fp);

    // dumpFoo(fp);

    return 0;

}
