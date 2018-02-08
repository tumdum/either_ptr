#include "either_ptr.hpp"
#include <cstring>
#include <string>
#include <iostream>
#include <type_traits>

struct StdStringer 
{
    template <typename T>
    std::string left(const T* p) const { return std::to_string(*p); }
    template <typename T>
    std::string right(const T* p) const { return std::to_string(*p); }

    std::string left(const std::string* p) const { return *p; }
    std::string right(const std::string* p) const { return *p; }
};

template <int M>
struct Mult
{
    template <typename T>
    void left(T* p) const { *p *= M; }
    template <typename T>
    void right(T* p) const { *p *= M; }

    void left(std::string* p) const { *p = *p + " * " + std::to_string(M); }
    void right(std::string* p) const { left(p); }
};

struct RightHeapDeleter
{
    template <typename T>
    void left(T* ) const { }
    template <typename T>
    void right(T* p) const { delete p; }
};

template <typename T>
T to(const int i) { return i; }
template <>
std::string to<std::string>(const int i) { return std::to_string(i); }

template <typename L, typename R>
void test()
{
    using TPtr = tmd::either_ptr<L, R, RightHeapDeleter>;
    static_assert(sizeof(TPtr) == sizeof(void*), "");
    L i = to<L>(10);
    R* p = new R{to<R>(42)};
    const auto s1 = TPtr::left(&i);
    const auto s2 = TPtr::right(p);

    assert(StdStringer{}.left(&i) == s1.visit(StdStringer{}));
    assert(StdStringer{}.right(p) == s2.visit(StdStringer{}));

    std::cout << "s1: " << s1.visit(StdStringer{}) << std::endl;
    std::cout << "s2: " << s2.visit(StdStringer{}) << std::endl;

    s1.visit(Mult<10>{});
    s2.visit(Mult<10>{});

    assert(StdStringer{}.left(&i) == s1.visit(StdStringer{}));
    assert(StdStringer{}.right(p) == s2.visit(StdStringer{}));

    std::cout << "s1: " << s1.visit(StdStringer{}) << std::endl;
    std::cout << "s2: " << s2.visit(StdStringer{}) << std::endl;
}

int main() {
    test<short, short>();
    test<int, int>();
    test<double, double>();
    test<std::string, std::string>();

    test<std::string, int>();
    test<int, std::string>();
}
