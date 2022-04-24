#include <apostrof.h>

#include "terminal.h"

int main() {
    if (!term_init())
        return -1;

    term_run("/test_app2");
    return 0;
}