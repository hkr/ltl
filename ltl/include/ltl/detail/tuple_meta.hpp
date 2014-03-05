#ifndef LTL_TUPLE_META_HPP
#define LTL_TUPLE_META_HPP

#include <tuple>

namespace ltl {
namespace detail {
  

template <typename T, typename Tuple>
struct tuple_push_back;

template <typename T, typename... Ts>
struct tuple_push_back<T, std::tuple<Ts...>>
{
    typedef std::tuple<Ts..., T> type;
};
    
template <template <typename> class MetaFunc, typename OutTuple, typename... Ts>
struct tuple_map_types_impl
{
    typedef OutTuple type;
};

template <template <typename> class MetaFunc, typename OutTuple, typename T, typename... Ts>
struct tuple_map_types_impl<MetaFunc, OutTuple, T, Ts...>
{
    typedef typename tuple_push_back<typename MetaFunc<T>::type, OutTuple>::type IntermediateTuple;
    typedef typename tuple_map_types_impl<MetaFunc, IntermediateTuple, Ts...>::type type;
};
    
template <typename InputTuple, typename OutputTuple, template <typename> class MetaFunc, std::size_t Idx = 0>
struct tuple_map_values_impl
{
    static const std::size_t TupleSize = std::tuple_size<InputTuple>::value;
    static void apply(InputTuple& in, OutputTuple& out)
    {
        if (Idx < TupleSize)
        {
            static const std::size_t I = Idx < TupleSize ? Idx : TupleSize - 1;
            std::get<I>(out) = MetaFunc<typename std::tuple_element<I, InputTuple>::type>()(std::get<I>(in));
            tuple_map_values_impl<InputTuple, OutputTuple, MetaFunc, I+1>::apply(in, out);
        }
    }
};
    
template <template <typename> class MetaFunc, typename... Ts>
struct tuple_map
{
    typedef typename tuple_map_types_impl<MetaFunc, std::tuple<>, Ts...>::type type;
    
    type operator()(std::tuple<Ts...>& ts) const
    {
        type result;
        tuple_map_values_impl<std::tuple<Ts...>, type, MetaFunc>::apply(ts, result);
        return result;
    }
};
    
template <typename Tuple, std::size_t Idx = 0>
struct tuple_transform
{
    static const std::size_t TupleSize = std::tuple_size<Tuple>::value;
    
    template <typename Function, typename OutputIterator>
    static OutputIterator apply(Tuple& x, OutputIterator out, Function func)
    {
        if (Idx < TupleSize)
        {
            static const std::size_t I = Idx < TupleSize ? Idx : TupleSize - 1;
            *out++ = func(std::get<I>(x));
            return tuple_transform<Tuple, I+1>::apply(x, out, func);
        }
        return out;
    }
};
    
} // namespace detail
} // namespace ltl


#endif // LTL_TUPLE_META_HPP
