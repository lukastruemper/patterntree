#pragma once

#include <type_traits>

namespace PatternTree {
    template<typename T>
    struct kokkos_rank : public std::integral_constant<std::size_t, 0> {};

    template<typename T>
    struct kokkos_rank<T*> : public std::integral_constant<std::size_t, kokkos_rank<T>::value + 1> {};

    template<typename T>
    concept ONEDIM = (PatternTree::kokkos_rank<T>::value == 1);

    template<typename T>
    concept TWODIM = (PatternTree::kokkos_rank<T>::value == 2);

    template <typename T>
    struct identity
    {
        using type = T;
    };

    template<typename T>
    struct remove_all_pointers : std::conditional_t<
        std::is_pointer_v<T>,
        remove_all_pointers<
            std::remove_pointer_t<T>
        >,
        identity<T>
    >
    {};

    template<typename T>
    using remove_all_pointers_t = typename remove_all_pointers<T>::type;
}