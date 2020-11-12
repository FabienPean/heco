// Copyright (c) 2020 Fabien Péan
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#include<vector>
#include<cstdint>
#include<cstdlib>
#include<memory>
#include<utility>
#include<algorithm>
#include<boost/align/aligned_allocator.hpp>
#include<iostream>

template<typename T>
using rm_cvref_t = std::remove_cv_t<std::remove_reference_t<T>>;

using typeid_t = std::uint32_t;
class TypeCounter {
    static inline typeid_t i = 0;
public:
    template<typename T>
    static inline const auto id = i++;
};
template<typename T> inline auto typeid_v = TypeCounter::id<T>;

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
            using U = rm_cvref_t<decltype(t)>;
            objects.push_back(
                ptr_t{
                new U{ std::forward<T>(t) }, +[](void* instance) { delete static_cast<U*>(instance); } });
        };
        (construct(std::forward<Ts>(values)), ...);
        // update sparse vector
        const auto max_id = std::max({ typeid_v<Ts>... });
        if (max_id >= sparse.size())
            sparse.resize(max_id + 1, -1);
        const auto ids = std::array{ typeid_v<Ts>... };
        for (int i = ids.size(); i > 0; --i)
            sparse[ids[i - 1]] = dense.size() + i - 1;
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
            i.dtor(&objects[i.offset], nullptr, ACTION::DESTROY);
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

#include<cassert>
#include<string>
#include<iostream>
int main()
{
    auto g = typeid_v<bool>;
    {
        heco_1_sparseset_ptr s;
        s.insert<std::string>("test");
        std::cout << s.get<std::string>() << "\n";

        s.insert<int, double, std::vector<double>>(42, 3.14, { 2.17 });
        std::cout << s.get<std::string>() << "\n";
        for (auto& i : s.sparse)
            std::cout << +i << " "; std::cout << "\n";
        std::cout << std::endl;
        for (auto& i : s.dense)
            std::cout << +i << " "; std::cout << "\n";
        std::cout << std::endl;
        // assert(s.get<double>() == 3.14);
        assert(s.get<int>() == 42);
        // assert(s.get<std::vector<double>>()[0] == 2.17);
        // assert(s.get<std::string>() == "test");
    }
    // {
    //     heco_1_sparseset_bytes s;
    //     s.insert<int,double,std::vector<double>>(42,3.14,{2.17});
    //     std::cout<<s.get<double>()<<"\n";
    //     assert(s.get<double>()==3.14);
    //     assert(s.get<int>()==42);
    //     assert(s.get<std::vector<double>>()[0]==2.17);
    // }

    return 0;
}