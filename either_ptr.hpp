#ifndef EITHER_PTR_HPP_
#define EITHER_PTR_HPP_

#include <cstdint>
#include <cassert>
#include <type_traits>

namespace tmd
{

template <typename L, typename R>
struct default_deallocator
{
    void left(L* l) const { delete l; }
    void right(R* r) const { delete r; }
};

template <typename L, typename R, typename D = default_deallocator<L, R>>
struct either_ptr
{
    using Left = L;
    using Right = R;
    using Self = either_ptr<Left, Right, D>;

    static_assert(alignof(L) > 1, "");
    static_assert(alignof(R) > 1, "");

    static Self left(L* l)
    { 
        return Self{(void*)l}; 
    }

    static Self right(R* r)
    { 
        Self self{(void*)r};
        self.m_data.num |= 1UL;
        return self;
    }

    template<typename F>
    auto visit(const F& f) const -> decltype(f.left(static_cast<L*>(nullptr)))
    {
        using LR = decltype(f.left(static_cast<L*>(nullptr)));
        using RR = decltype(f.right(static_cast<R*>(nullptr)));
        static_assert(std::is_same<LR, RR>::value, "left and right have to return same type");

        if (isLeft())
        {
            return f.left(reinterpret_cast<Left*>(m_data.num));
        }
        else
        {
            return f.right(reinterpret_cast<Right*>(m_data.num & ~1UL));
        }
    }

    either_ptr(const either_ptr&) = default;
    either_ptr& operator=(const either_ptr&) = default;

    either_ptr(either_ptr&&) = default;
    either_ptr& operator=(either_ptr&&) = default;

    ~either_ptr()
    {
        visit(D{});
    }

private:
    either_ptr(void* p)
    {
        /* 
         * This assert is not useless, consider what will happen when L or T is
         * set to this type:
         * using AlignedChar = const char __attribute__((aligned (2)));
         * All static asserts will be satisfied yet it will be still possible to
         * pass character literals which can be allocated at addresses that have
         * lowest bit set.
         */
        m_data.ptr = p;
        assert(isLeft());
    }

    bool isLeft() const { return (m_data.num & 1) == 0; }

    union
    {
        void* ptr;
        uintptr_t num;
    } m_data;
};

}

#endif  // EITHER_PTR_HPP_
