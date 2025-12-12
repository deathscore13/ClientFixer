#pragma once

// call method
template <typename T, uintptr_t offset>
__forceinline static T __call(void* ptr)
{
    static_assert(offset % sizeof(uintptr_t) == 0);
    return reinterpret_cast<T>((*reinterpret_cast<uintptr_t**>(ptr))[offset / sizeof(uintptr_t)]);
}

// get property
template <typename T, uintptr_t offset>
__forceinline static T& __property(void* ptr)
{
    return *reinterpret_cast<T*>(reinterpret_cast<uintptr_t*>(ptr) + offset * sizeof(uintptr_t));
}
