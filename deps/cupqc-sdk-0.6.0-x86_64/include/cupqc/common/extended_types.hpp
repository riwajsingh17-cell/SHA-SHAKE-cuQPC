#ifndef EXTENDED_TYPES_HPP
#define EXTENDED_TYPES_HPP

#include <cstdint>

namespace cupqc_common {
    using uint128_t = unsigned __int128;

    template<typename Precision>
    struct double_width {};

    template<>
    struct double_width<uint8_t> {
        using type = uint16_t;
    };

    template<>
    struct double_width<uint16_t> {
        using type = uint32_t;
    };

    template<>
    struct double_width<uint32_t> {
        using type = uint64_t;
    };

    template<>
    struct double_width<uint64_t> {
        using type = uint128_t;
    };

    template<typename Precision>
    using double_width_t = typename double_width<Precision>::type;

    template<typename Precision>
    using quad_width_t = double_width_t<double_width_t<Precision>>;
    
} // namespace cupqc
#endif