#include "cma/debug_allocator.hpp"
#include <cassert>

bool fail_cb_called = false;

void fail_cb()
{
    fail_cb_called = true;
}

template<
    typename POCC = std::false_type,
    typename POCCA = std::false_type,
    typename POCMA = std::true_type,
    typename POCS = std::true_type>
using alloc_t = cma::debug_allocator<char, POCC, POCCA, POCMA, POCS, fail_cb>;


void check_fail_cb(bool fail)
{
    assert(fail == fail_cb_called);
    fail_cb_called = false;
}

int main()
{
    // Leak detect
    {
        using alloc = alloc_t<>;
        using traits = std::allocator_traits<alloc>;
        alloc a;

        traits::allocate(a, 100);
    }
    check_fail_cb(true);

    // Double free detect
    {
        using alloc = alloc_t<>;
        using traits = std::allocator_traits<alloc>;
        alloc a;

        char* c = traits::allocate(a, 100);
        traits::deallocate(a, c, 100);
        traits::deallocate(a, c, 100);
    }
    check_fail_cb(true);

    // wrong size on free
    {
        using alloc = alloc_t<>;
        using traits = std::allocator_traits<alloc>;
        alloc a;

        char* c = traits::allocate(a, 100);
        traits::deallocate(a, c, 50);
    }
    check_fail_cb(true);

    // Normal operation
    {
        using alloc = alloc_t<>;
        using traits = std::allocator_traits<alloc>;
        alloc a;

        char* c = traits::allocate(a, 100);
        traits::deallocate(a, c, 100);
    }
    check_fail_cb(false);

    // Free with wrong alloc
    {
        using alloc = alloc_t<>;
        using traits = std::allocator_traits<alloc>;
        alloc a;
        alloc b;

        char* c = traits::allocate(a, 100);
        traits::deallocate(b, c, 100);
    }
    check_fail_cb(true);
}
