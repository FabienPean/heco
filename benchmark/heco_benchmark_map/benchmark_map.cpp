#include <tuple>
#include <vector>
#include <benchmark/benchmark.h>


//*****************************************************************************
// Utils
//*****************************************************************************
#include <unordered_map>
#include <boost/container/flat_map.hpp>
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <tsl/hopscotch_map.h>
#include <tsl/robin_map.h>
#include <tsl/sparse_map.h>
#include <sparsepp/spp.h>


template<typename T>
using rm_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

template<typename T>
struct Itself {
    using type = T;
};

using typeid_t = std::uint32_t;

class TypeCounter {
    static inline typeid_t i = 0;
public:
    template<typename T>
    static inline const auto id = i++;
};
template<typename T> inline auto typeid_v = TypeCounter::id<rm_cvref_t<T>>;

//*****************************************************************************
// Test containers
//*****************************************************************************

template<template<typename,typename> typename Map>
struct HeterogeneousContainer {
    using ptr_t = std::unique_ptr<void, void(*)(void*)>;
    Map<typeid_t, ptr_t> data;

    template<typename... Ts>
    void insert(Ts&&... ts) {
        data.reserve(data.size() + sizeof...(Ts));
        auto emplace = [&](auto&& t, auto&& v) {
            using T = decltype(v);
            using U = rm_cvref_t<T>;
            data.emplace(t, ptr_t( new U{ std::forward<T>(v) }, [](void* instance) { delete static_cast<U*>(instance); } ) );
        };
        (emplace(typeid_v<Ts>, std::forward<Ts>(ts)), ...);
    }

    template<typename T, typename U = rm_cvref_t<T>>
    U& get() {
        return *static_cast<U*>(data.at(typeid_v<U>).get());
    }
};

//*****************************************************************************
// Test functions
//*****************************************************************************
template<typename HC>
static void hc_create(benchmark::State& state, Itself<HC>) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(HC());
    }
}
template<typename HC, typename... Args1, typename... Args2>
static void hc_1by1_insert(benchmark::State& state, Itself<HC>, std::tuple<Args1...> to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        HC c;
        (c.insert(Args1{}), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename HC, typename... Args1, typename... Args2>
static void hc_bulk_insert(benchmark::State& state, Itself<HC>, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        HC c;
        c.insert(Args1{}...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename HC, typename... Args1, typename... Args2>
static void hc_get(benchmark::State& state, Itself<HC>, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    HC c;
    std::apply([&](auto&&... args) { c.insert(args...); }, to_insert);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.template get<Args2>()), ...);
    }
}

#include<tuple>
#include<vector>
#include<string>
#include<unordered_map>
struct T0 { double m0[2]{}; bool m1[2]{}; };
struct T1 { double m0[2]{}; double m1[2]{}; std::unordered_map<int, long int> m3{}; };
struct T2 { char m0[2]{}; char m1[1]{}; std::string m3{}; };
struct T3 { char m0[2]{}; bool m1[3]{}; std::vector<double> m3{}; };
struct T4 { int m0[2]{}; bool m1[2]{}; };
struct T5 { int m0[3]{}; char m1[3]{}; };
struct T6 { double m0[3]{}; int m1[1]{}; std::vector<double> m3{}; };
struct T7 { bool m0[3]{}; double m1[3]{}; std::vector<double> m3{}; };
struct T8 { double m0[2]{}; char m1[3]{}; std::string m3{}; };
struct T9 { bool m0[1]{}; char m1[1]{}; std::unordered_map<int, long int> m3{}; };
struct T10 { double m0[1]{}; bool m1[2]{}; };
struct T11 { bool m0[2]{}; int m1[3]{}; std::vector<double> m3{}; };
struct T12 { int m0[3]{}; char m1[3]{}; std::string m3{}; };
struct T13 { double m0[1]{}; double m1[2]{}; std::unordered_map<int, long int> m3{}; };
struct T14 { int m0[1]{}; char m1[1]{}; std::vector<double> m3{}; };
struct T15 { int m0[2]{}; bool m1[1]{}; std::unordered_map<int, long int> m3{}; };
struct T16 { char m0[1]{}; bool m1[3]{}; };
struct T17 { int m0[3]{}; char m1[1]{}; };
struct T18 { int m0[2]{}; double m1[3]{}; std::string m3{}; };
struct T19 { bool m0[2]{}; bool m1[2]{}; std::string m3{}; };
struct T20 { char m0[1]{}; bool m1[3]{}; std::string m3{}; };
struct T21 { bool m0[3]{}; char m1[2]{}; std::unordered_map<int, long int> m3{}; };
struct T22 { int m0[2]{}; bool m1[1]{}; std::unordered_map<int, long int> m3{}; };
struct T23 { char m0[1]{}; bool m1[1]{}; std::vector<double> m3{}; };
struct T24 { char m0[3]{}; bool m1[2]{}; std::vector<double> m3{}; };
struct T25 { int m0[1]{}; bool m1[3]{}; std::vector<double> m3{}; };
struct T26 { char m0[1]{}; bool m1[2]{}; std::unordered_map<int, long int> m3{}; };
struct T27 { bool m0[3]{}; bool m1[3]{}; };
struct T28 { bool m0[2]{}; bool m1[1]{}; std::vector<double> m3{}; };
struct T29 { int m0[2]{}; double m1[1]{}; std::vector<double> m3{}; };
struct T30 { double m0[1]{}; bool m1[3]{}; };
struct T31 { char m0[2]{}; int m1[3]{}; };
using to_insert_1 = std::tuple<T0>;
using to_insert_2 = std::tuple<T0, T1>;
using to_insert_3 = std::tuple<T0, T1, T2>;
using to_insert_4 = std::tuple<T0, T1, T2, T3>;
using to_insert_5 = std::tuple<T0, T1, T2, T3, T4>;
using to_insert_6 = std::tuple<T0, T1, T2, T3, T4, T5>;
using to_insert_7 = std::tuple<T0, T1, T2, T3, T4, T5, T6>;
using to_insert_8 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7>;
using to_insert_9 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8>;
using to_insert_10 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>;
using to_insert_11 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>;
using to_insert_12 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11>;
using to_insert_13 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12>;
using to_insert_14 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13>;
using to_insert_15 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14>;
using to_insert_16 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15>;
using to_insert_17 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16>;
using to_insert_18 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17>;
using to_insert_19 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18>;
using to_insert_20 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19>;
using to_insert_21 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20>;
using to_insert_22 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21>;
using to_insert_23 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22>;
using to_insert_24 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23>;
using to_insert_25 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24>;
using to_insert_26 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25>;
using to_insert_27 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26>;
using to_insert_28 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27>;
using to_insert_29 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28>;
using to_insert_30 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29>;
using to_insert_31 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30>;
using to_insert_32 = std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9, T10, T11, T12, T13, T14, T15, T16, T17, T18, T19, T20, T21, T22, T23, T24, T25, T26, T27, T28, T29, T30, T31>;
using to_get_1 = std::tuple<T10>;
using to_get_2 = std::tuple<T3, T13>;
using to_get_3 = std::tuple<T16, T10, T21>;
using to_get_4 = std::tuple<T0, T7, T28, T11>;
using to_get_5 = std::tuple<T24, T30, T26, T19, T22>;
using to_get_6 = std::tuple<T14, T16, T4, T17, T22, T0>;
using to_get_7 = std::tuple<T4, T5, T10, T23, T1, T17, T8>;
using to_get_8 = std::tuple<T28, T15, T24, T31, T11, T19, T9, T21>;
using to_get_9 = std::tuple<T0, T8, T22, T9, T12, T23, T13, T20, T2>;
using to_get_10 = std::tuple<T30, T12, T22, T10, T5, T7, T27, T20, T14, T31>;
using to_get_11 = std::tuple<T30, T26, T1, T12, T27, T22, T18, T13, T24, T21, T28>;
using to_get_12 = std::tuple<T0, T10, T14, T2, T8, T22, T5, T30, T16, T15, T17, T19>;
using to_get_13 = std::tuple<T4, T2, T15, T10, T9, T26, T14, T1, T13, T6, T17, T20, T31>;
using to_get_14 = std::tuple<T3, T0, T12, T21, T13, T10, T31, T6, T26, T22, T24, T29, T16, T19>;
using to_get_15 = std::tuple<T25, T12, T3, T19, T20, T6, T9, T8, T22, T5, T30, T15, T31, T2, T0>;
using to_get_16 = std::tuple<T0, T28, T25, T30, T3, T27, T8, T4, T20, T16, T24, T22, T11, T31, T29, T26>;
using to_get_17 = std::tuple<T5, T2, T1, T6, T21, T8, T17, T10, T11, T18, T30, T19, T20, T15, T14, T13, T24>;
using to_get_18 = std::tuple<T20, T13, T12, T18, T9, T0, T4, T26, T8, T10, T23, T11, T2, T22, T1, T29, T24, T5>;
using to_get_19 = std::tuple<T19, T18, T11, T12, T17, T4, T9, T3, T15, T23, T7, T1, T26, T5, T16, T2, T20, T29, T13>;
using to_get_20 = std::tuple<T2, T19, T13, T3, T29, T17, T15, T26, T10, T24, T28, T25, T22, T21, T30, T1, T9, T23, T11, T31>;
using to_get_21 = std::tuple<T15, T10, T20, T18, T12, T25, T30, T2, T27, T24, T6, T7, T1, T28, T0, T3, T19, T9, T21, T14, T16>;
using to_get_22 = std::tuple<T27, T13, T31, T2, T11, T7, T8, T18, T24, T5, T30, T6, T28, T3, T29, T0, T16, T14, T12, T10, T19, T1>;
using to_get_23 = std::tuple<T10, T25, T8, T6, T20, T1, T31, T29, T19, T4, T3, T26, T14, T12, T11, T17, T23, T22, T9, T7, T2, T28, T21>;
using to_get_24 = std::tuple<T24, T31, T21, T29, T28, T10, T26, T15, T25, T20, T30, T6, T17, T19, T7, T0, T27, T16, T13, T5, T23, T8, T2, T4>;
using to_get_25 = std::tuple<T11, T24, T18, T9, T22, T28, T25, T15, T2, T31, T16, T1, T23, T7, T4, T21, T29, T0, T12, T13, T19, T5, T20, T10, T14>;
using to_get_26 = std::tuple<T5, T24, T28, T16, T29, T1, T18, T2, T21, T30, T19, T25, T13, T6, T9, T17, T20, T15, T23, T12, T22, T26, T11, T3, T0, T8>;
using to_get_27 = std::tuple<T14, T11, T9, T16, T18, T8, T10, T2, T15, T27, T30, T13, T12, T20, T1, T5, T4, T7, T29, T31, T17, T0, T21, T24, T6, T25, T3>;
using to_get_28 = std::tuple<T7, T5, T21, T22, T4, T25, T11, T13, T1, T19, T14, T12, T29, T24, T3, T15, T28, T0, T17, T9, T27, T2, T10, T31, T23, T30, T8, T18>;
using to_get_29 = std::tuple<T2, T7, T28, T1, T19, T22, T14, T30, T20, T10, T24, T3, T25, T9, T4, T12, T27, T21, T8, T13, T17, T15, T0, T6, T31, T5, T16, T29, T11>;
using to_get_30 = std::tuple<T20, T4, T30, T1, T28, T29, T15, T8, T0, T16, T21, T18, T22, T6, T7, T2, T23, T13, T25, T31, T27, T17, T19, T9, T10, T12, T24, T3, T14, T26>;
using to_get_31 = std::tuple<T28, T9, T23, T0, T25, T14, T13, T21, T26, T29, T15, T10, T8, T2, T11, T19, T3, T18, T22, T30, T5, T12, T17, T24, T16, T27, T31, T4, T1, T7, T6>;
using to_get_32 = std::tuple<T13, T7, T23, T0, T9, T11, T22, T29, T19, T31, T4, T5, T14, T3, T15, T27, T8, T26, T25, T24, T21, T12, T30, T18, T28, T17, T1, T16, T20, T10, T2, T6>;

//BENCHMARK_CAPTURE(hc_create, 1, Itself<HeterogeneousContainer<std::unordered_map>>{});

BENCHMARK_CAPTURE(hc_1by1_insert, 1_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_1{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 2_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_2{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 3_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_3{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 4_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_4{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 5_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_5{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 6_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_6{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 7_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_7{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 8_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_8{}, to_get_1{});

BENCHMARK_CAPTURE(hc_get, 1_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_1{});
BENCHMARK_CAPTURE(hc_get, 2_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_2{});
BENCHMARK_CAPTURE(hc_get, 3_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_3{});
BENCHMARK_CAPTURE(hc_get, 4_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_4{});
BENCHMARK_CAPTURE(hc_get, 5_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_5{});
BENCHMARK_CAPTURE(hc_get, 6_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_6{});
BENCHMARK_CAPTURE(hc_get, 7_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_7{});
BENCHMARK_CAPTURE(hc_get, 8_unordered, Itself<HeterogeneousContainer<std::unordered_map>>{}, to_insert_32{}, to_get_8{});

BENCHMARK_CAPTURE(hc_1by1_insert, 1_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_1{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 2_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_2{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 3_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_3{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 4_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_4{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 5_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_5{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 6_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_6{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 7_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_7{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 8_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_8{}, to_get_1{});

BENCHMARK_CAPTURE(hc_get, 1_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_1{});
BENCHMARK_CAPTURE(hc_get, 2_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_2{});
BENCHMARK_CAPTURE(hc_get, 3_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_3{});
BENCHMARK_CAPTURE(hc_get, 4_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_4{});
BENCHMARK_CAPTURE(hc_get, 5_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_5{});
BENCHMARK_CAPTURE(hc_get, 6_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_6{});
BENCHMARK_CAPTURE(hc_get, 7_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_7{});
BENCHMARK_CAPTURE(hc_get, 8_boost, Itself<HeterogeneousContainer<boost::container::flat_map>>{}, to_insert_32{}, to_get_8{});

BENCHMARK_CAPTURE(hc_1by1_insert, 1_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_1{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 2_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_2{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 3_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_3{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 4_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_4{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 5_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_5{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 6_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_6{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 7_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_7{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 8_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_8{}, to_get_1{});

BENCHMARK_CAPTURE(hc_get, 1_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_1{});
BENCHMARK_CAPTURE(hc_get, 2_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_2{});
BENCHMARK_CAPTURE(hc_get, 3_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_3{});
BENCHMARK_CAPTURE(hc_get, 4_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_4{});
BENCHMARK_CAPTURE(hc_get, 5_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_5{});
BENCHMARK_CAPTURE(hc_get, 6_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_6{});
BENCHMARK_CAPTURE(hc_get, 7_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_7{});
BENCHMARK_CAPTURE(hc_get, 8_abseil, Itself<HeterogeneousContainer<absl::flat_hash_map>>{}, to_insert_32{}, to_get_8{});

//BENCHMARK_CAPTURE(hc_1by1_insert, 1_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_1{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_1by1_insert, 2_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_2{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_1by1_insert, 3_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_3{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_1by1_insert, 4_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_4{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_1by1_insert, 5_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_5{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_1by1_insert, 6_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_6{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_1by1_insert, 7_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_7{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_1by1_insert, 8_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_8{}, to_get_1{});
//
//BENCHMARK_CAPTURE(hc_get, 1_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_1{});
//BENCHMARK_CAPTURE(hc_get, 2_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_2{});
//BENCHMARK_CAPTURE(hc_get, 3_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_3{});
//BENCHMARK_CAPTURE(hc_get, 4_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_4{});
//BENCHMARK_CAPTURE(hc_get, 5_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_5{});
//BENCHMARK_CAPTURE(hc_get, 6_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_6{});
//BENCHMARK_CAPTURE(hc_get, 7_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_7{});
//BENCHMARK_CAPTURE(hc_get, 8_abseil_btree, Itself<HeterogeneousContainer<absl::btree_map>>{}, to_insert_32{}, to_get_8{});

BENCHMARK_CAPTURE(hc_1by1_insert, 1_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_1{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 2_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_2{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 3_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_3{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 4_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_4{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 5_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_5{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 6_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_6{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 7_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_7{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 8_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_8{}, to_get_1{});

BENCHMARK_CAPTURE(hc_get, 1_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_1{});
BENCHMARK_CAPTURE(hc_get, 2_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_2{});
BENCHMARK_CAPTURE(hc_get, 3_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_3{});
BENCHMARK_CAPTURE(hc_get, 4_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_4{});
BENCHMARK_CAPTURE(hc_get, 5_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_5{});
BENCHMARK_CAPTURE(hc_get, 6_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_6{});
BENCHMARK_CAPTURE(hc_get, 7_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_7{});
BENCHMARK_CAPTURE(hc_get, 8_robin, Itself<HeterogeneousContainer<tsl::robin_map>>{}, to_insert_32{}, to_get_8{});

BENCHMARK_CAPTURE(hc_1by1_insert, 1_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_1{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 2_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_2{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 3_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_3{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 4_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_4{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 5_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_5{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 6_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_6{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 7_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_7{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 8_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_8{}, to_get_1{});

BENCHMARK_CAPTURE(hc_get, 1_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_1{});
BENCHMARK_CAPTURE(hc_get, 2_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_2{});
BENCHMARK_CAPTURE(hc_get, 3_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_3{});
BENCHMARK_CAPTURE(hc_get, 4_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_4{});
BENCHMARK_CAPTURE(hc_get, 5_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_5{});
BENCHMARK_CAPTURE(hc_get, 6_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_6{});
BENCHMARK_CAPTURE(hc_get, 7_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_7{});
BENCHMARK_CAPTURE(hc_get, 8_hopscotch, Itself<HeterogeneousContainer<tsl::hopscotch_map>>{}, to_insert_32{}, to_get_8{});

BENCHMARK_CAPTURE(hc_1by1_insert, 1_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_1{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 2_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_2{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 3_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_3{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 4_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_4{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 5_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_5{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 6_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_6{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 7_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_7{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 8_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_8{}, to_get_1{});

BENCHMARK_CAPTURE(hc_get, 1_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_1{});
BENCHMARK_CAPTURE(hc_get, 2_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_2{});
BENCHMARK_CAPTURE(hc_get, 3_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_3{});
BENCHMARK_CAPTURE(hc_get, 4_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_4{});
BENCHMARK_CAPTURE(hc_get, 5_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_5{});
BENCHMARK_CAPTURE(hc_get, 6_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_6{});
BENCHMARK_CAPTURE(hc_get, 7_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_7{});
BENCHMARK_CAPTURE(hc_get, 8_sparse, Itself<HeterogeneousContainer<tsl::sparse_map>>{}, to_insert_32{}, to_get_8{});

BENCHMARK_CAPTURE(hc_1by1_insert, 1_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_1{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 2_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_2{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 3_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_3{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 4_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_4{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 5_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_5{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 6_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_6{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 7_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_7{}, to_get_1{});
BENCHMARK_CAPTURE(hc_1by1_insert, 8_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_8{}, to_get_1{});

BENCHMARK_CAPTURE(hc_get, 1_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_1{});
BENCHMARK_CAPTURE(hc_get, 2_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_2{});
BENCHMARK_CAPTURE(hc_get, 3_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_3{});
BENCHMARK_CAPTURE(hc_get, 4_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_4{});
BENCHMARK_CAPTURE(hc_get, 5_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_5{});
BENCHMARK_CAPTURE(hc_get, 6_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_6{});
BENCHMARK_CAPTURE(hc_get, 7_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_7{});
BENCHMARK_CAPTURE(hc_get, 8_spp, Itself<HeterogeneousContainer<spp::sparse_hash_map>>{}, to_insert_32{}, to_get_8{});

BENCHMARK_MAIN();
