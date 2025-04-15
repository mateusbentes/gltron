// nebu_system_keynames.h
#ifndef NEBU_SYSTEM_KEYNAMES_H
#define NEBU_SYSTEM_KEYNAMES_H

#define CUSTOM_KEY_COUNT 4 * 24

typedef struct {
    int key;
    char *name;
} Key;

typedef struct {
    Key key[1024]; // Assuming a maximum of 1024 custom keys
} KeySet;

typedef struct {
    Key key[CUSTOM_KEY_COUNT];
} custom_keynames;

extern custom_keynames custom_keys;

#endif // NEBU_SYSTEM_KEYNAMES_H
