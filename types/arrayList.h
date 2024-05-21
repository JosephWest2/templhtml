#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct ArrayList ArrayList;

void ArrayList_Create(ArrayList * arr, size_t initialSize);

void ArrayList_Clone(const ArrayList *const src, ArrayList *dest);

bool ArrayList_Contains(ArrayList *arr, size_t n);

void ArrayList_Append(ArrayList *arr, size_t n);
