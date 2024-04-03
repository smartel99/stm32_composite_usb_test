/**
 * @file    memory.cpp
 * @author  Samuel Martel
 * @date    2024-04-02
 * @brief
 *
 * @copyright
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 * You should have received a copy of the GNU General Public License along with this program. If
 * not, see <a href=https://www.gnu.org/licenses/>https://www.gnu.org/licenses/</a>.
 */
#include <FreeRTOS.h>
#include <portable.h>

#include <cstddef>
#include <new>
#include <string_view>

#include <logging/logger.h>

namespace {
constexpr std::string_view s_tag = "MEM";
}

#if defined(__cpp_exceptions) && __cpp_exceptions == 199711L
void* operator new(std::size_t sz)
{
    if (sz == 0) {
        ++sz;    // avoid std::malloc(0) which may return nullptr on success
    }

    if (void* ptr = pvPortMalloc(sz); ptr != nullptr) {
        LOGT(s_tag, "new(size_t), size = %d, ptr = %p", sz, ptr);
        return ptr;
    }

    throw std::bad_alloc {};    // required by [new.delete.single]/3
}
void operator delete(void* ptr) noexcept
{
    LOGT(s_tag, "delete(void*), ptr = %p", ptr);
    vPortFree(ptr);
}
void operator delete(void* ptr, std::size_t sz) noexcept
{
    LOGT(s_tag, "delete(void*, size_t), ptr = %p, size = %d", ptr, sz);
    vPortFree(ptr);
}

void* operator new[](std::size_t sz)
{
    if (sz == 0) {
        ++sz;    // avoid std::malloc(0) which may return nullptr on success
    }

    if (void* ptr = pvPortMalloc(sz); ptr != nullptr) {
        LOGT(s_tag, "new[](size_t), size = %d, ptr = %p", sz, ptr);
        return ptr;
    }

    throw std::bad_alloc {};    // required by [new.delete.single]/3
}
void operator delete[](void* ptr) noexcept
{
    LOGT(s_tag, "delete[](void*), ptr = %p", ptr);
    vPortFree(ptr);
}
void operator delete[](void* ptr, std::size_t sz) noexcept
{
    LOGT(s_tag, "delete[](void*, size_t), ptr = %p, size = %d", ptr, sz);
    vPortFree(ptr);
}

#    if defined(__cpp_aligned_new) && __cpp_aligned_new == 201606L
void* operator new([[maybe_unused]] std::size_t count, [[maybe_unused]] std::align_val_t al)
{
    LOGE(s_tag, "new(size_t, std::align_val_t) not supported!");
    throw std::bad_alloc {};
}
void operator delete([[maybe_unused]] void* ptr, [[maybe_unused]] std::align_val_t al) noexcept
{
    LOGE(s_tag, "delete(void*, std::align_val_t) not supported!");
}
void operator delete([[maybe_unused]] void*            ptr,
                     [[maybe_unused]] std::size_t      sz,
                     [[maybe_unused]] std::align_val_t al) noexcept
{
    LOGE(s_tag, "delete(void*, size_t, std::align_val_t) not supported!");
}

void* operator new[](std::size_t count, std::align_val_t al)
{
    LOGE(s_tag, "new[](size_t, std::align_val_t) not supported!");
    throw std::bad_alloc {};
}
void operator delete[]([[maybe_unused]] void* ptr, [[maybe_unused]] std::align_val_t al) noexcept
{
    LOGE(s_tag, "delete[](void*, std::align_val_t) not supported!");
}
void operator delete[]([[maybe_unused]] void*            ptr,
                       [[maybe_unused]] std::size_t      sz,
                       [[maybe_unused]] std::align_val_t al) noexcept
{
    LOGE(s_tag, "delete[](void*, size_t, std::align_val_t) not supported!");
}
#    endif
#endif

void* operator new(std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    if (sz == 0) {
        ++sz;    // avoid std::malloc(0) which may return nullptr on success
    }

    void* ptr = pvPortMalloc(sz);
    LOGT(s_tag, "new(size_t, std::nothrow_t), size = %d, ptr = %p", sz, ptr);
    return ptr;
}
void operator delete(void* ptr, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGT(s_tag, "delete(void*, std::nothrow_t), ptr = %p", ptr);
    vPortFree(ptr);
}
void operator delete(void* ptr, std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGT(s_tag, "delete(void*, size_t, std::nothrow_t), ptr = %p, size = %d", ptr, sz);
    vPortFree(ptr);
}

void* operator new[](std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    if (sz == 0) {
        ++sz;    // avoid std::malloc(0) which may return nullptr on success
    }

    void* ptr = pvPortMalloc(sz);
    LOGT(s_tag, "new[](size_t, std::nothrow_t), size = %d, ptr = %p", sz, ptr);
    return ptr;
}
void operator delete[](void* ptr, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGT(s_tag, "delete[](void*, std::nothrow_t), ptr = %p", ptr);
    vPortFree(ptr);
}
void operator delete[](void* ptr, std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGT(s_tag, "delete[](void*, size_t, std::nothrow_t), ptr = %p, size = %d", ptr, sz);
    vPortFree(ptr);
}

#if defined(__cpp_aligned_new) && __cpp_aligned_new == 201606L
void* operator new([[maybe_unused]] std::size_t           count,
                   [[maybe_unused]] std::align_val_t      al,
                   [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGE(s_tag, "new(size_t, std::align_val_t, std::nothrow_t) not supported!");
    return nullptr;
}
void operator delete([[maybe_unused]] void*                 ptr,
                     [[maybe_unused]] std::align_val_t      al,
                     [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGE(s_tag, "delete(void*, std::align_val_t, std::nothrow_t) not supported!");
}
void operator delete([[maybe_unused]] void*                 ptr,
                     [[maybe_unused]] std::size_t           sz,
                     [[maybe_unused]] std::align_val_t      al,
                     [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGE(s_tag, "delete(void*, size_t, std::align_val_t, std::nothrow_t) not supported!");
}

void* operator new[](std::size_t count, std::align_val_t al, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGE(s_tag, "new[](size_t, std::align_val_t, std::nothrow_t) not supported!");
    return nullptr;
}
void operator delete[]([[maybe_unused]] void*                 ptr,
                       [[maybe_unused]] std::align_val_t      al,
                       [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGE(s_tag, "delete[](void*, std::align_val_t, std::nothrow_t) not supported!");
}
void operator delete[]([[maybe_unused]] void*                 ptr,
                       [[maybe_unused]] std::size_t           sz,
                       [[maybe_unused]] std::align_val_t      al,
                       [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    LOGE(s_tag, "delete[](void*, size_t, std::align_val_t, std::nothrow) not supported!");
}
#endif
