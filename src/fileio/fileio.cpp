#include "fileio.h"

#include "core/logging.h"
#include "containers/string.h"
#include "containers/stringview.h"
#include "platform/platform.h"

void LoadFileToString(const StringView& filepath, String& output)
{
    FILE* file = fopen(filepath.cstr(), "rb");
    Assert(file != nullptr);

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    output.Resize(length + 1, true);
    fread(output.cstr(), sizeof(char), length, file);

    fclose(file);
}

void LoadFileToBytes(const StringView& filepath, DynamicArray<u8>& output)
{
    FILE* file = fopen(filepath.cstr(), "rb");
    Assert(file != nullptr);

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    output.Resize(length);
    fread(output.data(), sizeof(u8), length, file);

    fclose(file);

}