#include "arrayList.h"
#include <stdlib.h>

static void _Grow(ArrayList *arr) {
    arr->bufferLength *= 2;
    arr->buffer = realloc(arr->buffer, arr->bufferLength * sizeof(size_t));
}

void ArrayList_Destroy(ArrayList *arr) {
    free(arr->buffer);
    arr->count = 0;
    arr->bufferLength = 0;
}

void ArrayList_CreateAndClone(const ArrayList *const src, ArrayList *dest) {
    ArrayList_Create(dest, src->count + 10);
    for (size_t i = 0; i < src->count; i++) {
        dest->buffer[i] = src->buffer[i];
    }
}

void ArrayList_Create(ArrayList *arr, size_t initialSize) {
    arr->count = 0;
    arr->bufferLength = initialSize;
    arr->buffer = malloc(sizeof(size_t) * arr->bufferLength);
}

bool ArrayList_Contains(ArrayList *arr, size_t n) {
    for (size_t i = 0; i < arr->count; i++) {
        if (arr->buffer[i] == n) {
            return true;
        }
    }
    return false;
}

void ArrayList_Append(ArrayList *arr, size_t n) {
    if (arr->count >= arr->bufferLength) {
        _Grow(arr);
    }
    arr->buffer[arr->count] = n;
    arr->count++;
}
