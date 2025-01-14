#include <libdevice/keys.h>

#define KEY_NAMES_ENTRY(__key_name, __key_number) [__key_number] = #__key_name,

static const char *KEYS_NAMES[] = {KEY_LIST(KEY_NAMES_ENTRY)};

const char *key_to_string(key_t key)
{
    if (key < __KEY_COUNT)
    {
        return KEYS_NAMES[key];
    }
    else
    {
        return "overflow";
    }
}

bool key_is_valid(key_t key)
{
    return key > 0 && key < __KEY_COUNT;
}