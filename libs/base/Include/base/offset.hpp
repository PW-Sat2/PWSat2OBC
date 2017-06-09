#ifndef LIBS_BASE_INCLUDE_BASE_OFFSET_HPP_
#define LIBS_BASE_INCLUDE_BASE_OFFSET_HPP_

#include <type_traits>

/**
 * @defgroup base_offset Offset calculator
 *
 * Utiltity for defining series of fields and assigning addressed to them
 */

/**
 * @brief Single element
 * @tparam Type used to determine size of this element
 */
template <typename T> struct Element
{
    using Underlying = T;
};

/**
 * @brief Collection of elements in single address-space
 * @tparam Elements Template pack of elements
 */
template <typename... Elements> class Offsets
{
  private:
    /**
     * @brief Finds offset of single element
     * @return Offset
     */
    template <std::size_t Acc, typename Element, typename Head, typename... Rest> static constexpr std::size_t FindOffset()
    {
        if (std::is_same<Element, Head>())
        {
            return Acc;
        }

        return FindOffset<Acc + sizeof(typename Head::Underlying), Element, Rest...>();
    }

    /**
     * @brief Finds offset of single element - final step
     * @return Invalid value
     */
    template <std::size_t Acc, typename Element> static constexpr std::size_t FindOffset()
    {
        return 0xFFFF;
    }

  public:
    /**
     * @brief Finds offset of single element
     * @return Offset
     */
    template <typename Element> static constexpr std::size_t Offset()
    {
        return FindOffset<0, Element, Elements...>();
    }
};

#endif /* LIBS_BASE_INCLUDE_BASE_OFFSET_HPP_ */
