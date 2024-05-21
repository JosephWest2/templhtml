#include "arrayList.h"
#include <stdlib.h>

typedef struct ArrayList {
    size_t *buffer;
    size_t count;
    size_t bufferLength;
} ArrayList;

static void _Grow(ArrayList *arr) {
    arr->bufferLength *= 2;
    arr->buffer = realloc(arr->buffer, arr->bufferLength * sizeof(size_t));
}

void ArrayList_Clone(const ArrayList *const src, ArrayList *dest) {

}

void ArrayList_Create(ArrayList *arr, size_t initialSize) {
    arr->count = 0;
    arr->bufferLength = 10;
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
