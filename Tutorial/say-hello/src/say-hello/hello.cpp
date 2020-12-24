#include "hello.hpp"
#include <iostream>

void hello::say_hello(){
    //Say hello
    std::cout<<"Hello, World from say-hello version-"<<SAY_HELLO_VERSION<<"\n";
}