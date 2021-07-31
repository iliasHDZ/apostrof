#include "error.h"

ERROR_HANDLER err_handler = 0;

void error(const char* err) {
    if (err_handler != 0) err_handler(err);
}

void error_register(ERROR_HANDLER handler) {
    err_handler = handler;
}