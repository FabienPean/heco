#include <cstddef>
#include <memory>
#include <type_traits>
#include <cassert>
#include <algorithm>

namespace heco
{
    template <typename T>
    struct vecter_impl;

    template<typename T>
    auto typeid_v() noexcept { return std::uintptr_t(std::destroy_at<T>); }

    template<typename T> struct Itself { using type = T; };

    struct Vtable {
        std::size_t id;
        std::size_t size;
        std::size_t alignment;
        void (*destroy)(void* begin, void* end);
        void (*copy)(void* begin, void* end, void* to);
        void*(*clone)(void* begin, void* end);
        void (*deallocate)(void* begin, void* end);

        template<typename T>
        Vtable(Itself<T>) :
            id{ typeid_v<T>() },
            size{ sizeof(T) },
            alignment{ alignof(T) },
            destroy{ utils::destroy<T> },
            copy{ utils::copy<T> },
            clone{ utils::clone<T> },
            deallocate{ utils::deallocate<T> }
        {}

        template<typename U, typename V = Vtable>
        static inline const auto of = V(Itself<U>{});

    private:
        struct utils {
            template<typename T>
            static void destroy(void* sta, void* end) {
                if (!std::is_trivially_destructible_v<T> && end == nullptr && sta != nullptr)
                    std::destroy_at((T*)sta);
                if (!std::is_trivially_destructible_v<T> && end != nullptr && sta != nullptr)
                    std::destroy((T*)sta, (T*)end);
            }
            template<typename T>
            static void copy(void* sta, void* end, void* other) {
                std::copy((const T*)sta, (const T*)end, (T*)other);
            }
            template<typename T>
            static void* clone(void* sta, void* end) {
                std::size_t n = (const T*)end - (const T*)sta;
                using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
                T* new_start = reinterpret_cast<T*>(new Storage[n]);
                std::copy((const T*)sta, (const T*)end, new_start);
                return new_start;
            };
            template<typename T>
            static void deallocate(void* begin, void* end) {
                std::allocator<T>().deallocate((T*)begin, (T*)end - (T*)begin);
            }
        };
    };

    struct vecter
    {
    protected:
        const Vtable* vtable = &Vtable::of<std::byte>;
        void* _sta = nullptr;
        void* _end = nullptr;
        void* _cap = nullptr;
    public:
        vecter() = default;
        vecter(const Vtable* v) : vtable(v) {};
        vecter(vecter&& other) noexcept :
            vtable(other.vtable),
            _sta(std::exchange(other._sta, nullptr)),
            _end(std::exchange(other._end, nullptr)),
            _cap(std::exchange(other._cap, nullptr))
        {}
        vecter(const vecter& other) :
            vtable(other.vtable),
            _sta(other.vtable->clone(other._sta, other._end)),
            _end(begin() + other.size()),
            _cap(begin() + other.size())
        {}
        vecter& operator=(vecter&& other) {
            if (&other != this)
            {
                vtable->destroy(_sta, _end);
                vtable->deallocate(_sta, _end);
                vtable = other.vtable;
                _sta = std::exchange(other._sta, nullptr);
                _end = std::exchange(other._end, nullptr);
                _cap = std::exchange(other._cap, nullptr);
            }
            return *this;
        }
        vecter& operator=(const vecter& other) {
            if (&other != this)
            {
                vtable->destroy(_sta, _end);
                vtable->deallocate(_sta, _end);
                vtable = other.vtable;
                _sta = other.vtable->clone(other._sta, other._sta);
                _end = begin() + other.size();
                _cap = begin() + other.size();
            }
            return *this;
        }
        ~vecter() {
            if (vtable != nullptr && _sta != nullptr)
                vtable->destroy(_sta, _end);
            if (_sta != nullptr)
                vtable->deallocate(_sta, _end);
        }

        constexpr std::byte* cap() noexcept { return static_cast<std::byte*>(_cap); }
        constexpr const std::byte* cap() const noexcept { return static_cast<std::byte*>(_cap); }
        constexpr std::byte* begin() noexcept { return static_cast<std::byte*>(_sta); }
        constexpr std::byte* end()   noexcept { return static_cast<std::byte*>(_end); }
        constexpr const std::byte* begin() const noexcept { return static_cast<std::byte*>(_sta); }
        constexpr const std::byte* end()   const noexcept { return static_cast<std::byte*>(_end); }
        std::size_t size() const noexcept { return end() - begin(); }
        std::size_t capacity() const noexcept { return cap() - begin(); }

        std::size_t id() const noexcept { return vtable->id; }
        std::size_t size_value() const noexcept { return vtable->size; }
        std::size_t alignment_value() const noexcept { return vtable->alignment; }

        template<typename T>
        static vecter_impl<T> create() { return vecter_impl<T>(); }

        template<typename T>
        auto& vector() {
            assert(typeid_v<T>() == id());
            return *static_cast<vectr::vecter_impl<T>*>(this);
        }
        template<typename T>
        auto& vector(std::size_t i) {
            assert(typeid_v<T>() == id());
            return (*static_cast<vectr::vecter_impl<T>*>(this))[i];
        }
    };

    template<typename T = std::byte>
    struct vecter_impl final : vecter
    {
        using Allocator = std::allocator<T>;
        // types
        using value_type = T;
        using allocator_type = Allocator;
        using pointer = typename std::allocator_traits<Allocator>::pointer;
        using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using iterator = T*;
        using const_iterator = const T*;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        // iterators
        constexpr iterator               cap() noexcept { return static_cast<T*>(_cap); }
        constexpr const_iterator         cap() const noexcept { return static_cast<T*>(_cap); }
        constexpr iterator               begin() noexcept { return static_cast<T*>(_sta); }
        constexpr const_iterator         begin() const noexcept { return static_cast<T*>(_sta); }
        constexpr iterator               end() noexcept { return static_cast<T*>(_end); }
        constexpr const_iterator         end() const noexcept { return static_cast<T*>(_end); }
        constexpr reverse_iterator       rbegin() noexcept { return end() - 1; }
        constexpr const_reverse_iterator rbegin() const noexcept { return end() - 1; }
        constexpr reverse_iterator       rend() noexcept { return begin() - 1; }
        constexpr const_reverse_iterator rend() const noexcept { return begin() - 1; }
        constexpr const_iterator         cbegin() const noexcept { return begin(); }
        constexpr const_iterator         cend() const noexcept { return end(); }
        constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
        constexpr const_reverse_iterator crend() const noexcept { return rend(); }
        // [vecter_impl.capacity], capacity
        bool empty() const noexcept { return size() == 0; }
        size_type size() const noexcept { return end() - begin(); }
        size_type max_size() const noexcept { return std::numeric_limits<difference_type>::max(); }
        size_type capacity() const noexcept { return cap() - begin(); }
        void resize(size_type sz) {
            auto i = [=]() {
                std::size_t i = 0, x = 1;
                while (i < sz)
                    i = 1ULL << x++;
                return i;
            } ();
            auto new_capacity = std::min(i, 256ULL);
            new_capacity = new_capacity == 256 ? std::max(256ULL,std::size_t(1.5 * sz)) : new_capacity;
            reserve(new_capacity);
            _end = std::uninitialized_value_construct_n(end(), sz - size());
        }
        constexpr void resize(size_type sz, const T& c) {
            auto i = [=]() {
                std::size_t i = 0, x = 1;
                while (i < sz)
                    i = 1ULL << x++;
                return i;
            } ();
            auto new_capacity = std::min(i, 256ULL);
            new_capacity = new_capacity == 256 ? std::max(256ULL, std::size_t(1.5 * sz)) : new_capacity;
            reserve(new_capacity);
            _end = std::uninitialized_fill_n(end(), sz - size(), c);
        }
        constexpr void reserve(size_type n) {
            if (n > capacity()) {
                const size_type s = size();
                using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
                T* new_start = reinterpret_cast<T*>(new Storage[n]);
                std::uninitialized_move(begin(), end(), new_start);
                vtable->destroy(_sta, _end);
                vtable->deallocate(_sta, _end);
                _sta = new_start;
                _end = new_start + s;
                _cap = new_start + n;
            }
        }
        constexpr void shrink_to_fit() {
            const size_type s = size();
            if (capacity() > size()) {
                using Storage = std::aligned_storage_t<sizeof(T), alignof(T)>;
                T* new_start = static_cast<T*>(new Storage[s]);
                std::uninitialized_move(begin(), end(), new_start);
                vtable->destroy(_sta, _end);
                vtable->deallocate(_sta, _end);
                _sta = new_start;
                _end = new_start + s;
                _cap = _end;
            }
        }
        // element access
        constexpr reference       operator[](size_type n) { return *(begin() + n); }
        constexpr const_reference operator[](size_type n) const { return *(begin() + n); }
        constexpr reference       at(size_type n) { assert(n < size()); return *(begin() + n); }
        constexpr const_reference at(size_type n) const { assert(n < size()); return *(begin() + n); }
        constexpr reference       front() { return *begin(); }
        constexpr const_reference front() const { return *begin(); }
        constexpr reference       back() { return *(end() - 1); }
        constexpr const_reference back() const { return *(end() - 1); }
        // [vecter_impl.data], data access
        constexpr value_type* data() noexcept { return begin(); }
        constexpr const value_type* data() const noexcept { return begin(); }
        // [vecter_impl.modifiers], modifiers
        template<class... Args> constexpr reference emplace_back(Args&&... args)
        {
            reserve(size() + 1);
            T* obj = nullptr;
            if constexpr (std::is_aggregate_v<T>)
                obj = ::new (_end) T{ std::forward<Args>(args)... };
            else
                obj = ::new (_end) T(std::forward<Args>(args)...);
            _end = end() + 1;
            return *obj;
        }
        template<typename U, typename = std::enable_if_t<std::is_convertible_v<U, T>>>
        constexpr reference push_back(U&& x) {
            reserve(size() + 1);
            T* obj = new(_end) T(std::forward<U>(x));
            _end = end() + 1;
            return *obj;
        }
        //constexpr void push_back(T&& x);
        constexpr T pop_back() {
            assert(_end != nullptr && _end > _sta);
            _end = end() - 1;
            return T(std::move(*end()));
        }
        constexpr void clear() noexcept {
            std::destroy(begin(), end());
            _cap = _end = _sta;
        }
        constexpr void reset() noexcept {
            std::destroy(begin(), end());
            vtable->deallocate(begin(), cap());
            _cap = _end = _sta = nullptr;
        }

        // user-defined
        vecter_impl() : vecter(&Vtable::of<T>) {}
    };
}
