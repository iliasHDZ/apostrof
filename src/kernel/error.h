#pragma once

typedef void (*ERROR_HANDLER)(const char*);

void error(const char* err);

void error_register(ERROR_HANDLER handler);