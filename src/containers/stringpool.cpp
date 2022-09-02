#include "string.h"
#include "stack.h"

Stack<String::PooledString, true> String::stringPool(16);