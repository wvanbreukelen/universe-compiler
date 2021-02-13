#pragma once

#include <ctype.h>
#include <stdbool.h>

bool is_digit_str(const char* str) {
    bool digits = true;

    while (*str != '\0') {
        digits &= (bool) isdigit(*str);
        str++;
    }

    return digits;
}