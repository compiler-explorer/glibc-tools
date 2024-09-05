#include <cstdio>
#include <cstdlib>

char *buffer;

int cause_a_segfault() {
    puts("trying to cause a segfault");
    return (int)buffer[9];
}

int main(int argc, const char *argv[]) {
    puts("hello, world");

    auto x = cause_a_segfault();

    return x + 1;
}
