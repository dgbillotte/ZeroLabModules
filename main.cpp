//
//  main.cpp
//  ZeroLab
//
//  Created by Daniel on 7/20/21.
//  Copyright © 2021 Daniel. All rights reserved.
//

#include <iostream>
using namespace std;

string& str();
void test(string* s); 

struct Foo {
    Foo(const char *s) : _s(*(new string(s))), _id(++__next_id) {
        cout << "constructing a Foo(id:" << _id << ", name:" << _s << " mem: " << &_s << ")" << endl;
    }

    Foo(const Foo &f) : _s(f._s), _id(++__next_id) {
        cout << "copy-constructing a Foo(id:" << _id << ", name:" << _s << " mem: " << &_s << ")" << endl;
    }

    ~Foo() {
        cout << "destroying a Foo(id:" << _id << ", name:" << _s << " mem: " << &_s << ")" << endl;
    }

    void operator =(const Foo &f) {
        _s = f._s;
        cout << "assigning a Foo(id:" << _id << ", name:" << _s << " mem: " << &_s << ")" << endl;
        // return this;
    }

    string& _s;
    int _id;
    static int __next_id;
};
int Foo::__next_id = 0;


Foo& snew(const char *s) {
    Foo *f = new Foo(s);
    Foo &out = *f;
    delete f;
    return out;
}

void other(Foo &f) {
    // cout << "we got an input Foo(id:" << f._s << ")" << endl;
    cout << "other: Foo(id:" << f._id << ", name:" << f._s << " mem: " << &(f._s) << ")" << endl;
    Foo f2 = snew("f2");
    Foo &f3 = snew("f3");
}

int main(int argc, const char * argv[]) {
    cout << "main: start" << endl;
    Foo &f = snew("f1");
    cout << "main: created f1" << endl; 
    other(f); 

    cout << "main: called other()" << endl;

    return 0;
}



