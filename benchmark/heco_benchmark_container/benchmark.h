#include <any>
#include <variant>
#include <tuple>
#include <vector>
#include <boost/align/aligned_allocator.hpp>
#include <HeteroVector.h>
#include <entt_context.hpp>
#include <entt.hpp>
#include <benchmark/benchmark.h>


//*****************************************************************************
// Utils
//*****************************************************************************
#include <tsl/robin_map.h>

template<typename T>
using rm_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

using typeid_t = std::uint32_t;

class TypeCounter {
    static inline typeid_t i = 0;
public:
    template<typename T>
    static inline const auto id = i++;
};
template<typename T> inline auto typeid_v = TypeCounter::id<rm_cvref_t<T>>;

//template<typename K, typename V>
//using map = tsl::robin_map<K,V>;
template<typename K, typename V>
using map = std::unordered_map<K, V>;
//*****************************************************************************
// Test containers
//*****************************************************************************
struct mapany {
    std::unordered_map<typeid_t, std::any> data;
    template<typename... Us, typename... Ts>
    void insert(Ts&&... ts) {
        data.reserve(data.size() + sizeof...(Ts));
        (data.emplace(typeid_v<rm_cvref_t<Ts>>, std::forward<Ts>(ts)), ...);
    }
    template<typename T, typename U = std::remove_reference_t<T>>
    U& get() {
        return std::any_cast<U&>(data.at(typeid_v<U>));
    }
};

struct heco_1_map_ptr {
    map<typeid_t, std::unique_ptr<void, void(*)(void*)>> data;

    template<typename... Ts>
    void insert(Ts&&... ts) {
        data.reserve(data.size() + sizeof...(Ts));
        //(data.emplace(type_id<Ts>(), std::forward<Ts>(ts)), ...);
        (data.emplace(typeid_v<Ts>, std::unique_ptr<void, void(*)(void*)>{ new rm_cvref_t<Ts>{ std::forward<Ts>(ts) }, [](void* instance) { delete static_cast<rm_cvref_t<Ts>*>(instance); } }), ...);
    }
    template<typename T, typename U = std::remove_reference_t<T>>
    U& get() {
        return *static_cast<U*>(data.at(typeid_v<U>).get());
    }
};
#include<iostream>
#include<cstring>
struct heco_1_map_bytes {
    template<typename T, size_t N>
    using aligned_allocator = boost::alignment::aligned_allocator<T, N>;
    constexpr static size_t max_alignment = 64;
    using vector_bytes = std::vector<unsigned char, aligned_allocator<unsigned char, max_alignment>>;
    vector_bytes data;

    using offset_t = std::uint32_t;
    map<typeid_t, offset_t> type_to_offset;

    enum class ACTION { CONSTRUCT, COPY, MOVE, DESTROY };
    using destructor_t = size_t(*)(void*, void*, ACTION);
    map<typeid_t, destructor_t> destructors;

    heco_1_map_bytes() = default;
    heco_1_map_bytes(const heco_1_map_bytes&) = delete;
    heco_1_map_bytes(heco_1_map_bytes&&) = default;
    heco_1_map_bytes& operator=(const heco_1_map_bytes&) = delete;
    heco_1_map_bytes& operator=(heco_1_map_bytes&&) = default;
    ~heco_1_map_bytes() noexcept {
        for (auto&& [tid, destructor] : destructors)
            destructor(&data[type_to_offset[tid]], nullptr, ACTION::DESTROY);
    }

    template<typename T>
    void record_destructor() {
        destructors.emplace(
            typeid_v<T>, 
            +[](void* src, void* tgt, ACTION action) 
            {
                switch (action) {
                case ACTION::CONSTRUCT: new(tgt) T; break;
                case ACTION::DESTROY: std::destroy_at(static_cast<T*>(src)); break;
                case ACTION::COPY: new(tgt) T{ *static_cast<T*>(src) }; break;
                case ACTION::MOVE: new(tgt) T{ std::move(*static_cast<T*>(src)) }; break;
                }
                return sizeof(T); 
            }
        );
    }

    template<typename... T>
    auto insert(T&&... types) {
        const auto offsets = allocate<T...>();           //allocate
        {size_t i = 0; (construct(offsets[i++], types), ...); }//construct
        {size_t i = 0; (type_to_offset.emplace(typeid_v<T>, offsets[i++]), ...); }//bookkeeping
        (record_destructor<T>(), ...);//bookkeeping
        return offsets;
    }

    template<typename... T>
    auto allocate()->std::array<offset_t, sizeof...(T)> {
        static_assert(((alignof(T) <= max_alignment) && ...));
        using namespace std;

        constexpr size_t N = sizeof...(T);
        constexpr array<size_t, N> alignments = { alignof(T)... };
        constexpr array<size_t, N> sizes = { sizeof(T)... };

        array<offset_t, N> output;

        const size_t size_before = data.size();
        uintptr_t ptr_end = uintptr_t(data.data() + size_before);
        size_t to_allocate = 0;
        for (int i = 0; i < N; ++i)
        {
            size_t padding = ((~ptr_end + 1) & (alignments[i] - 1));
            output[i] = size_before + to_allocate + padding;
            to_allocate += padding + sizes[i];
            ptr_end += padding + sizes[i];
        }
        size_t size_after = size_before + to_allocate;
        //data.resize(size_after);//Resizing screw up runtime on Linux "free(): invalid pointer", not moving objects is UB
        vector_bytes tmp(size_after);
        for (auto&& [tid, destructor] : destructors)
            destructor(&data[type_to_offset[tid]], &tmp[type_to_offset[tid]], ACTION::MOVE);
        data = std::exchange(tmp, {});
        return output;
    }

    template<typename Type>
    void construct(offset_t n, Type&& t) {
        using U = rm_cvref_t<Type>;
        new(&data[n]) U{ std::forward<Type>(t) };
    }

    template<typename... T>
    auto get() -> decltype(auto) {
        if constexpr (sizeof...(T) == 1)
            return (get<T>(type_to_offset.at(typeid_v<T>)), ...);
        else
            return std::tuple<T&...>{ get<T>(type_to_offset.at(typeid_v<T>))... };
    }

    template<typename T, typename U = std::remove_reference_t<T>>
    U& get(offset_t n) {
        return *std::launder(reinterpret_cast<U*>(&data[n]));
    }
};

struct heco_1_sparseset_ptr
{
    using ptr_t = std::unique_ptr<void, void(*)(void*)>;
    std::vector<std::uint8_t> sparse;
    std::vector<typeid_t>     dense;
    std::vector<ptr_t>        objects;

    template<typename T, typename... Rest>
    decltype(auto) get()
    {
        using U = std::remove_reference_t<T>;
        if constexpr (sizeof...(Rest) == 0)
            return *static_cast<U*>(objects[sparse[typeid_v<U>]].get());
        else
            return std::forward_as_tuple(get<T>(), get<Rest>()...);
    }

    template<typename... Ts>
    void insert(Ts&& ... values)
    {
        objects.reserve(objects.size() + sizeof...(Ts));
        auto construct = [&](auto&& t) {
            using T = decltype(t);
            using U = std::remove_reference_t<decltype(t)>;
            objects.push_back(
                std::unique_ptr<void, void(*)(void*)>{
                new U{ std::forward<T>(t) }, +[](void* instance) { delete static_cast<U*>(instance); } });
        };
        (construct(std::forward<Ts>(values)), ...);
        // update sparse vector
        const auto max_id = std::max({ typeid_v<Ts>... });
        if (max_id >= sparse.size())
            sparse.resize(max_id + 1, -1);
        const auto ids = std::array{ typeid_v<Ts>... };
        for (int i = ids.size(); i > 0; --i)
            sparse[ids[i - 1]] = i - 1;
        // update dense vector
        dense.reserve(dense.size() + sizeof...(Ts));
        (dense.push_back(typeid_v<Ts>), ...);
    }
};

struct heco_1_sparseset_bytes
{
    template<typename T, size_t N>
    using aligned_allocator = boost::alignment::aligned_allocator<T, N>;
    static constexpr inline auto max_alignment = 64UL;
    using offset_t = std::uint32_t;

    enum class ACTION { CONSTRUCT, COPY, MOVE, DESTROY };
    using destructor_t = size_t(*)(void*, void*, ACTION);
    struct typeid_offset_dtor { typeid_t tag; offset_t offset; destructor_t dtor; };

    std::vector<std::uint8_t> sparse;
    std::vector<typeid_offset_dtor> dense;
    using vector_bytes = std::vector<std::byte, aligned_allocator<std::byte, max_alignment>>;
    vector_bytes objects;

    heco_1_sparseset_bytes() = default;
    heco_1_sparseset_bytes(heco_1_sparseset_bytes&&) = default;
    heco_1_sparseset_bytes(const heco_1_sparseset_bytes&) = delete;
    heco_1_sparseset_bytes& operator=(const heco_1_sparseset_bytes&) = delete;
    heco_1_sparseset_bytes& operator=(heco_1_sparseset_bytes&&) = default;
    ~heco_1_sparseset_bytes() {
        for (auto& i : dense)
            i.dtor(&objects[i.offset],nullptr,ACTION::DESTROY);
    }

    template<typename T>
    auto record_destructor() {
        return +[](void* src, void* tgt, ACTION action)
        {
            switch (action) {
            case ACTION::CONSTRUCT: new(tgt) T; break;
            case ACTION::DESTROY: std::destroy_at(static_cast<T*>(src)); break;
            case ACTION::COPY: new(tgt) T{ *static_cast<T*>(src) }; break;
            case ACTION::MOVE: new(tgt) T{ std::move(*static_cast<T*>(src)) }; break;
            default: break;
            }
            return sizeof(T);
        };
    }

    template<typename T>
    decltype(auto) get() { return do_get<T>(dense[sparse[typeid_v<T>]].offset); }

    template<typename T, typename U = std::remove_reference_t<T>>
    U& do_get(offset_t n) { return *std::launder(reinterpret_cast<U*>(&objects[n])); }

    template<typename... Ts>
    void insert(Ts&&... types) {
        // allocate and construct objects
        const auto offsets = allocate<Ts...>();
        auto construct = [&](offset_t n, auto&& t) {
             using T = decltype(t);
            using U = std::remove_reference_t<T>;
            new(&objects[n]) U{ std::forward<T>(t) };
        };
        {size_t i = 0; (construct(offsets[i++], types), ...); }
        // update sparse vector
        const auto ids = std::array{ typeid_v<Ts>... };
        const auto max_id = *std::max_element(ids.cbegin(), ids.end());
        if (max_id >= sparse.size())
            sparse.resize(max_id + 1, -1);
        for (int i = ids.size(); i > 0; --i)
            sparse[ids[i - 1]] = i - 1;
        // update dense vector
        dense.reserve(dense.size() + sizeof...(Ts));
        {size_t i = 0; (dense.push_back({ typeid_v<Ts>,offsets[i++], record_destructor<Ts>() }), ...); }
    }

    template<typename... T>
    auto allocate()->std::array<offset_t, sizeof...(T)> {
        static_assert(((alignof(T) <= max_alignment) && ...));
        using namespace std;

        constexpr size_t N = sizeof...(T);
        constexpr array<size_t, N> alignments = { alignof(T)... };
        constexpr array<size_t, N> sizes = { sizeof(T)... };

        array<offset_t, N> output;

        const size_t size_before = objects.size();
        uintptr_t ptr_end = uintptr_t(objects.data() + size_before);
        size_t to_allocate = 0;
        for (int i = 0; i < N; ++i)
        {
            size_t padding = ((~ptr_end + 1) & (alignments[i] - 1));
            output[i] = size_before + to_allocate + padding;
            to_allocate += padding + sizes[i];
            ptr_end += padding + sizes[i];
        }
        size_t size_after = size_before + to_allocate;
        // objects.resize(size_after);//Resizing screw up runtime on Linux "free(): invalid pointer", not moving objects is UB
        vector_bytes tmp(size_after);
        for (auto&& [tid, offset, dtor] : dense)
            dtor(&objects[offset], &tmp[offset], ACTION::MOVE);
        objects = std::exchange(tmp, {});
        return output;
    }
};

//*****************************************************************************
// Test functions
//*****************************************************************************
static void vecany_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::vector<std::any>());
    }
}
template<typename... Args1, typename... Args2>
static void vecany_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        std::vector<std::any> c;
        std::apply([&](auto&&... args) { (c.push_back(args), ...); }, to_insert);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void vecany_bulk_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        std::vector<std::any> c;
        auto v = std::apply([&](auto&&... args) { return std::array{ std::any{std::forward<decltype(args)>(args)} ... }; }, to_insert);
        c.insert(c.begin(), begin(v), end(v));
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void vecany_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    std::vector<std::any> c;
    (c.push_back(Args1{}), ...);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(std::find_if(c.begin(), c.end(), [](auto& q) { return std::any_cast<Args2>(&q); })), ...);
    }
}
static void mapany_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(mapany());
    }
}
template<typename... Args1, typename... Args2>
static void mapany_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        mapany c;
        (c.insert(Args1{}), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void mapany_bulk_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        mapany c;
        c.insert(Args1{}...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void mapany_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    mapany c;
    std::apply([&](auto&&... args) { c.insert(args...); }, to_insert);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.get<Args2>()), ...);
        //std::apply([&](auto&&... args) { (benchmark::DoNotOptimize(c.get<decltype(args)>()), ...); }, to_get);
    }
}
static void hc_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(heco_1_map_ptr());
    }
}
template<typename... Args1, typename... Args2>
static void hc_1by1_insert(benchmark::State& state, std::tuple<Args1...> to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        heco_1_map_ptr c;
        (c.insert(Args1{}), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void hc_bulk_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        heco_1_map_ptr c;
        c.insert(Args1{}...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void hc_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    heco_1_map_ptr c;
    std::apply([&](auto&&... args) { c.insert(args...); }, to_insert);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.get<Args2>()), ...);
        //auto r = std::tuple{ c.get<Args2>()... };
        //benchmark::DoNotOptimize(r);
        //std::apply([&](auto&&... args) { (benchmark::DoNotOptimize(c.get<decltype(args)>()), ...); }, to_get);
    }
}

static void ha_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(heco_1_map_bytes());
    }
}
template<typename... Args1, typename... Args2>
static void ha_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    std::apply([&](auto&&... args) { (typeid_v<decltype(args)>, ...); }, to_insert);
    for (auto _ : state) {
        heco_1_map_bytes c;
        (c.insert(Args1{}), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void ha_bulk_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        heco_1_map_bytes c;
        c.insert(Args1{}...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void ha_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    heco_1_map_bytes c;
    c.insert(Args1{}...);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.get<Args2>()), ...);
    }
}

static void hcs_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(heco_1_sparseset_ptr());
    }
}
template<typename... Args1, typename... Args2>
static void hcs_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    std::apply([&](auto&&... args) { (typeid_v<decltype(args)>, ...); }, to_insert);
    for (auto _ : state) {
        heco_1_sparseset_ptr c;
        (c.insert(Args1{}), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void hcs_bulk_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        heco_1_sparseset_ptr c;
        c.insert(Args1{}...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void hcs_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    heco_1_sparseset_ptr c;
    c.insert(Args1{}...);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.get<Args2>()), ...);
    }
}

static void has_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(heco_1_sparseset_bytes());
    }
}
template<typename... Args1, typename... Args2>
static void has_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    std::apply([&](auto&&... args) { (typeid_v<decltype(args)>, ...); }, to_insert);
    for (auto _ : state) {
        heco_1_sparseset_bytes c;
        (c.insert(Args1{}), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void has_bulk_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        heco_1_sparseset_bytes c;
        c.insert(Args1{}...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void has_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    heco_1_sparseset_bytes c;
    c.insert(Args1{}...);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.get<Args2>()), ...);
    }
}

static void andyg_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(hmdf::HeteroVector());
    }
}
template<typename... Args1, typename... Args2>
static void andyg_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        hmdf::HeteroVector c;
        (c.push_back(Args1{}), ...);
    }
}
template<typename... Args1, typename... Args2>
static void andyg_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    hmdf::HeteroVector c;
    std::apply([&](auto&&... args) { (c.push_back(args), ...); }, to_insert);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.get<Args2>(0)), ...);
    }
}
static void entt_ctx_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(entt::Context());
    }
}
template<typename... Args1, typename... Args2>
static void entt_ctx_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    //std::apply([&](auto&&... args) { return std::array{ entt::type_info<decltype(args)>::id()... }; }, to_test);
    for (auto _ : state) {
        entt::Context c;
        (c.set<Args1>(), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void entt_ctx_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    entt::Context c;
    std::apply([&](auto&&... args) { (c.set<rm_cvref_t<decltype(args)>>(args), ...); }, to_insert);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.ctx<Args2>()), ...);
    }
}
static void entt_reg_create(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(entt::registry());
    }
}
template<typename... Args1, typename... Args2>
static void entt_reg_1by1_insert(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        entt::registry c;
        //std::apply([&](auto&&... args) { (c.prepare<rm_cvref_t<decltype(args)>>(), ...); }, to_test);
        auto entity = c.create();
        (c.emplace<Args1>(entity), ...);
        benchmark::DoNotOptimize(c);
    }
}
template<typename... Args1, typename... Args2>
static void entt_reg_get(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    entt::registry c;
    auto entity = c.create();
    (c.emplace<Args1>(entity, Args1{}), ...);
    for (auto _ : state) {
        (benchmark::DoNotOptimize(c.get<Args2>(entity)), ...);
    }
}

template<typename... Args1, typename... Args2>
static void gen_types_entt(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        std::apply([&](auto&&... args) { (benchmark::DoNotOptimize(entt::type_info<rm_cvref_t<decltype(args)>>::id()), ...); }, to_insert);
    }
}
template<typename... Args1, typename... Args2>
static void gen_types_ha(benchmark::State& state, std::tuple<Args1...>to_insert, std::tuple<Args2...> to_get) {
    for (auto _ : state) {
        std::apply([&](auto&&... args) { (benchmark::DoNotOptimize(typeid_v<rm_cvref_t<decltype(args)>>), ...); }, to_insert);
    }
}

//*****************************************************************************
// Loaded benchmarks
//*****************************************************************************
//BENCHMARK(gen_types_entt);
//BENCHMARK(gen_types_ha);
// CREATE
BENCHMARK(mapany_create);
BENCHMARK(hc_create);
BENCHMARK(ha_create);
BENCHMARK(hcs_create);
BENCHMARK(has_create);
BENCHMARK(entt_ctx_create);
BENCHMARK(andyg_create);
BENCHMARK(entt_reg_create);
