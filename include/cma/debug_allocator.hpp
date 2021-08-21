#ifndef CMA_DEBUG_ALLOCATOR_HPP
#define CMA_DEBUG_ALLOCATOR_HPP
#include <string>
#include <map>
#include <memory>

namespace cma
{
    template<void (*fail_cb)()>
    struct debug_allocator_info
    {
        // For debug purposes
        std::string id;

        // Maps allocations to sizes of the array
        std::map<void*, std::size_t> allocations;

        ~debug_allocator_info()
        {
            // When we are deleted, we should have no allocations left
            if (!allocations.empty())
            {
                fail_cb();
            }
        }
    };

    template<
        typename T,
        typename POCC,
        typename POCCA,
        typename POCMA,
        typename POCS,
        void (*fail_cb)()>
    struct debug_allocator
    {
        using dai = debug_allocator_info<fail_cb>;
        // So that copying it extends the life of the "info"
        std::shared_ptr<dai> info;
        using value_type = T;

        std::string id() const
        {
            return info->id;
        }

        debug_allocator() : debug_allocator("default") {}
        debug_allocator(std::string id)
            : info(new dai{std::move(id)}) {}

        template <class U>
            debug_allocator(debug_allocator<U, POCC, POCCA, POCMA, POCS, fail_cb> const& other)
            : info(other.info) {}

        // Allocators move constructor must not actually move (call the copy
        // construct)
        template <class U>
            debug_allocator(debug_allocator<U, POCC, POCCA, POCMA, POCS, fail_cb>&& other)
            : debug_allocator(other) {}

        T* allocate(std::size_t n)
        {
            T* ret = static_cast<T*>(operator new (n*sizeof(value_type)));

            // Keep a record of this
            this->info->allocations[ret] = n;

            return ret;
        }

        void deallocate(T* p, std::size_t n) noexcept
        {
            if (this->info->allocations.find(p) == std::cend(this->info->allocations))
            {
                fail_cb();
                return;
            }
            if (this->info->allocations[p] != n)
            {
                fail_cb();
                return;
            }

            this->info->allocations.erase(p);
            operator delete(p);
        }

        debug_allocator select_on_container_copy_construction() const
        {
            if (POCC::value)
            {
                return *this;
            }
            else
            {
                return debug_allocator();
            }
        }

        using propagate_on_container_copy_assignment = POCCA;
        using propagate_on_container_move_assignment = POCMA;
        using propagate_on_container_swap            = POCS;
    };

    template<
        typename T,
        typename U,
        typename POCC,
        typename POCCA,
        typename POCMA,
        typename POCS,
        void (*fail_cb)()>
    bool operator==(debug_allocator<T, POCC, POCCA, POCMA, POCS, fail_cb> const& x,
            debug_allocator<U, POCC, POCCA, POCMA, POCS, fail_cb> const& y) noexcept
    {
        return x.info == y.info;
    }

    template<
        typename T,
        typename U,
        typename POCC,
        typename POCCA,
        typename POCMA,
        typename POCS,
        void (*fail_cb)()>
    bool operator!=(debug_allocator<T, POCC, POCCA, POCMA, POCS, fail_cb> const& x,
            debug_allocator<U, POCC, POCCA, POCMA, POCS, fail_cb> const& y) noexcept
    {
        return !(x == y);
    }
}

#endif
