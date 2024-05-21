#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct ArrayList {
    size_t *buffer;
    size_t count;
    size_t bufferLength;
} ArrayList;

void ArrayList_Create(ArrayList * arr, size_t initialSize);

void ArrayList_Destroy(ArrayList *arr);

void ArrayList_CreateAndClone(const ArrayList *const src, ArrayList *dest);

bool ArrayList_Contains(ArrayList *arr, size_t n);

void ArrayList_Append(ArrayList *arr, size_t n);
