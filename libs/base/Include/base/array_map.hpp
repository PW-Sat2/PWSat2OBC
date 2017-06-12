#ifndef LIBS_BASE_INCLUDE_BASE_ARRAY_MAP_HPP_
#define LIBS_BASE_INCLUDE_BASE_ARRAY_MAP_HPP_

template <typename Input, std::size_t Count, typename Mapper, typename Output = decltype(std::declval<Mapper>()(std::declval<Input&>()))>
std::array<Output, Count> Map(Input (&input)[Count], Mapper map)
{
    std::array<Output, Count> result;
    for (std::size_t i = 0; i < Count; i++)
    {
        result[i] = map(input[i]);
    }

    return result;
}

template <typename Input, std::size_t Count, typename Mapper, typename Output = decltype(std::declval<Mapper>()(std::declval<Input&>()))>
std::array<Output, Count> Map(std::array<Input, Count>& input, Mapper map)
{
    std::array<Output, Count> result;
    for (std::size_t i = 0; i < Count; i++)
    {
        result[i] = map(input[i]);
    }

    return result;
}

#endif /* LIBS_BASE_INCLUDE_BASE_ARRAY_MAP_HPP_ */
