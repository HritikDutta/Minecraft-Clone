#pragma once

#include "core/types.h"
#include "containers/string.h"
#include "containers/stringview.h"
#include "containers/darray.h"

void LoadFileToString(const StringView& filepath, String& output);
void LoadFileToBytes(const StringView& filepath, DynamicArray<u8>& output);