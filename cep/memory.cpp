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

namespace {
void* myNew(std::size_t sz)
{
    if (sz == 0) {
        ++sz;    // avoid std::malloc(0) which may return nullptr on success
    }

    void* ptr = pvPortMalloc(sz);
    if (ptr == nullptr) {
        std::new_handler handler = std::get_new_handler();
        while (handler != nullptr && ptr == nullptr) {
            handler();
            ptr     = pvPortMalloc(sz);
            handler = std::get_new_handler();
        }
    }
    return ptr;
}
}    // namespace


//----------------------------------------------------------------------------------------------------------------------
// THROWING NEW

void* operator new(std::size_t sz)
{
    void* ptr = myNew(sz);
    if (ptr == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions == 199711L
        throw std::bad_alloc {};    // required by [new.delete.single]/3
#else
        std::terminate();    // -fno-exception used.
#endif
    }
    return ptr;
}
void operator delete(void* ptr) noexcept
{
    vPortFree(ptr);
}
void operator delete(void* ptr, std::size_t sz) noexcept
{
    vPortFree(ptr);
}

void* operator new[](std::size_t sz)
{
    void* ptr = myNew(sz);
    if (ptr == nullptr) {
#if defined(__cpp_exceptions) && __cpp_exceptions == 199711L
        throw std::bad_alloc {};    // required by [new.delete.single]/3
#else
        std::terminate();    // -fno-exception used.
#endif
    }
    return ptr;
}
void operator delete[](void* ptr) noexcept
{
    vPortFree(ptr);
}
void operator delete[](void* ptr, std::size_t sz) noexcept
{
    vPortFree(ptr);
}

//----------------------------------------------------------------------------------------------------------------------
// THROWING NEW (custom alignment)

#if defined(__cpp_aligned_new) && __cpp_aligned_new == 201606L
void* operator new([[maybe_unused]] std::size_t count, [[maybe_unused]] std::align_val_t al)
{
#    if defined(__cpp_exceptions) && __cpp_exceptions == 199711L
    throw std::bad_alloc {};    // required by [new.delete.single]/3
#    else
    std::terminate();    // -fno-exception used.
#    endif
}
void operator delete([[maybe_unused]] void* ptr, [[maybe_unused]] std::align_val_t al) noexcept
{
}
void operator delete([[maybe_unused]] void*            ptr,
                     [[maybe_unused]] std::size_t      sz,
                     [[maybe_unused]] std::align_val_t al) noexcept
{
}

void* operator new[](std::size_t count, std::align_val_t al)
{
#    if defined(__cpp_exceptions) && __cpp_exceptions == 199711L
    throw std::bad_alloc {};    // required by [new.delete.single]/3
#    else
    std::terminate();    // -fno-exception used.
#    endif
}
void operator delete[]([[maybe_unused]] void* ptr, [[maybe_unused]] std::align_val_t al) noexcept
{
}
void operator delete[]([[maybe_unused]] void*            ptr,
                       [[maybe_unused]] std::size_t      sz,
                       [[maybe_unused]] std::align_val_t al) noexcept
{
}
#endif

//----------------------------------------------------------------------------------------------------------------------
// NON-THROWING NEW

void* operator new(std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    void* ptr = myNew(sz);
    return ptr;
}
void operator delete(void* ptr, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    vPortFree(ptr);
}
void operator delete(void* ptr, std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    vPortFree(ptr);
}

void* operator new[](std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    void* ptr = myNew(sz);
    return ptr;
}
void operator delete[](void* ptr, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    vPortFree(ptr);
}
void operator delete[](void* ptr, std::size_t sz, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    vPortFree(ptr);
}

//----------------------------------------------------------------------------------------------------------------------
// NON-THROWING NEW (custom alignment)

#if defined(__cpp_aligned_new) && __cpp_aligned_new == 201606L
void* operator new([[maybe_unused]] std::size_t           count,
                   [[maybe_unused]] std::align_val_t      al,
                   [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    return nullptr;
}
void operator delete([[maybe_unused]] void*                 ptr,
                     [[maybe_unused]] std::align_val_t      al,
                     [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
}
void operator delete([[maybe_unused]] void*                 ptr,
                     [[maybe_unused]] std::size_t           sz,
                     [[maybe_unused]] std::align_val_t      al,
                     [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
}

void* operator new[](std::size_t count, std::align_val_t al, [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
    return nullptr;
}
void operator delete[]([[maybe_unused]] void*                 ptr,
                       [[maybe_unused]] std::align_val_t      al,
                       [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
}
void operator delete[]([[maybe_unused]] void*                 ptr,
                       [[maybe_unused]] std::size_t           sz,
                       [[maybe_unused]] std::align_val_t      al,
                       [[maybe_unused]] const std::nothrow_t& tag) noexcept
{
}
#endif
