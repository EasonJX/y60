#ifndef HELLO_WORLD_INCLUDED
#define HELLO_WORLD_INCLUDED

#include <string>

const char * greet();
void greet2();
void greet_n(int n);
std::string greet_more_args(const std::string & msg, const int n);

struct foo {
    foo();
    ~foo();
    const char * bar();
    const char * foobar();
};

#endif // HELLO_WORLD_INCLUDED
