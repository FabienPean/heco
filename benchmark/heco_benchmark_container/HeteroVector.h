// Adapted from https://github.com/hosseinmoein/DataFrame
/*
Copyright (c) 2019-2022, Hossein Moein
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
* Neither the name of Hossein Moein and/or the DataFrame nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL Hossein Moein BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <unordered_map>
#include <functional>
#include <vector>
// ----------------------------------------------------------------------------

namespace hmdf
{

// This class implements a heterogeneous vector. Its design and implementation
// are partly inspired by Andy G's Blog at:
// https://gieseanw.wordpress.com/2017/05/03/a-true-heterogeneous-container/
//
struct  HeteroVector  {

public:

    using size_type = size_t;

    HeteroVector() = default;
    HeteroVector(const HeteroVector &that) = delete;
    HeteroVector(HeteroVector &&that) = delete;

    ~HeteroVector() { clear(); }

    HeteroVector &operator= (const HeteroVector &rhs) = delete;
    HeteroVector &operator= (HeteroVector &&rhs) = delete;

    //Fix type when accessing underlying container
    template<typename U, typename T = std::decay_t<U>>
    std::vector<T> &get_vector();
    template<typename U, typename T = std::decay_t<U>>
    const std::vector<T> &get_vector() const;

    template<typename T>
    void push_back(const T &v);
    template<typename T, class... Args>
    void emplace_back (Args &&... args);
    template<typename T, typename ITR, class... Args>
    void emplace (ITR pos, Args &&... args);

    template<typename T>
    void reserve (size_type r)  { get_vector<T>().reserve (r); }
    template<typename T>
    void shrink_to_fit () { get_vector<T>().shrink_to_fit (); }

    template<typename T>
    size_type size () const { return (get_vector<T>().size()); }

    // Fix absent definition
    void clear() {
        for (auto& c : clear_functions_)
            c(*this);
    };

    template<typename T>
    void erase(size_type pos);

    template<typename T>
    void resize(size_type count);
    template<typename T>
    void resize(size_type count, const T &v);

    template<typename T>
    void pop_back();

    template<typename T>
    bool empty() const noexcept;

    template<typename T>
    T &at(size_type idx);
    template<typename T>
    const T &at(size_type idx) const;
    // Added for convenience to benchmark
    template<class T, typename U = std::decay_t<T>>
    U& get(size_t i) {
        return get_vector<T>()[i];
    }

    template<typename T>
    T &back();
    template<typename T>
    const T &back() const;

    template<typename T>
    T &front();
    template<typename T>
    const T &front() const;

    template<typename T>
    using iterator = typename std::vector<T>::iterator;
    template<typename T>
    using const_iterator = typename std::vector<T>::const_iterator;
    template<typename T>
    using reverse_iterator = typename std::vector<T>::reverse_iterator;
    template<typename T>
    using const_reverse_iterator =
        typename std::vector<T>::const_reverse_iterator;

    template<typename T>
    inline iterator<T>
    begin() noexcept { return (get_vector<T>().begin()); }

    template<typename T>
    inline iterator<T>
    end() noexcept { return (get_vector<T>().end()); }

    template<typename T>
    inline const_iterator<T>
    begin () const noexcept { return (get_vector<T>().begin()); }

    template<typename T>
    inline const_iterator<T>
    end () const noexcept { return (get_vector<T>().end()); }

    template<typename T>
    inline reverse_iterator<T>
    rbegin() noexcept { return (get_vector<T>().rbegin()); }

    template<typename T>
    inline reverse_iterator<T>
    rend() noexcept { return (get_vector<T>().rend()); }

    template<typename T>
    inline const_reverse_iterator<T>
    rbegin () const noexcept { return (get_vector<T>().rbegin()); }

    template<typename T>
    inline const_reverse_iterator<T>
    rend () const noexcept { return (get_vector<T>().rend()); }

private:

    template<typename T>
    inline static std::unordered_map<const HeteroVector *, std::vector<T>>
        vectors_ {  };

    std::vector<std::function<void(HeteroVector &)>>    clear_functions_;
    std::vector<std::function<void(const HeteroVector &,
                                   HeteroVector &)>>    copy_functions_;
    std::vector<std::function<void(HeteroVector &,
                                   HeteroVector &)>>    move_functions_;

    // Visitor stuff
    //
    template<typename T, typename U>
    void visit_impl_help_ (T &visitor);
    template<typename T, typename U>
    void visit_impl_help_ (T &visitor) const;

    template<typename T, typename U>
    void sort_impl_help_ (T &functor);

    template<typename T, typename U>
    void change_impl_help_ (T &functor);
    template<typename T, typename U>
    void change_impl_help_ (T &functor) const;

    // Specific visit implementations
    //
    template<class T, template<class...> class TLIST, class... TYPES>
    void visit_impl_ (T &&visitor, TLIST<TYPES...>);
    template<class T, template<class...> class TLIST, class... TYPES>
    void visit_impl_ (T &&visitor, TLIST<TYPES...>) const;

    template<class T, template<class...> class TLIST, class... TYPES>
    void sort_impl_ (T &&functor, TLIST<TYPES...>);

    template<class T, template<class...> class TLIST, class... TYPES>
    void change_impl_ (T &&functor, TLIST<TYPES...>);
    template<class T, template<class...> class TLIST, class... TYPES>
    void change_impl_ (T &&functor, TLIST<TYPES...>) const;

public:

    template<typename... Ts>
    struct type_list  {   };

    template<typename... Ts>
    struct visitor_base  { using types = type_list<Ts ...>; };

    template<typename T>
    void visit (T &&visitor)  {

        visit_impl_ (visitor, typename std::decay_t<T>::types { });
    }
    template<typename T>
    void visit (T &&visitor) const  {

        visit_impl_ (visitor, typename std::decay_t<T>::types { });
    }
    template<typename T>
    void sort (T &&functor)  {

        sort_impl_ (functor, typename std::decay_t<T>::types { });
    }
    template<typename T>
    void change (T &&functor)  {

        change_impl_ (functor, typename std::decay_t<T>::types { });
    }
    template<typename T>
    void change (T &&functor) const  {

        change_impl_ (functor, typename std::decay_t<T>::types { });
    }
};

} // namespace hmdf

// ----------------------------------------------------------------------------

#include <algorithm>

// ----------------------------------------------------------------------------

namespace hmdf
{

    template<typename U, typename T>
    std::vector<T>& HeteroVector::get_vector() {

        auto    iter = vectors_<T>.find(this);

        // don't have it yet, so create functions for copying and destroying
        if (iter == vectors_<T>.end()) {
            clear_functions_.emplace_back(
                [](HeteroVector& hv) { vectors_<T>.erase(&hv); });

            // if someone copies me, they need to call each
            // copy_function and pass themself
            copy_functions_.emplace_back(
                [](const HeteroVector& from, HeteroVector& to) {
                    vectors_<T>[&to] = vectors_<T>[&from];
                });

            move_functions_.emplace_back(
                [](HeteroVector& from, HeteroVector& to) {
                    vectors_<T>[&to] = std::move(vectors_<T>[&from]);
                });

            iter = vectors_<T>.emplace(this, std::vector<T>()).first;
        }

        return (iter->second);
    }

    // ----------------------------------------------------------------------------

    template<typename U, typename T>
    const std::vector<T>& HeteroVector::get_vector() const {

        return (const_cast<HeteroVector*>(this)->get_vector<T>());
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    void HeteroVector::push_back(const T& v) { get_vector<T>().push_back(v); }

    // ----------------------------------------------------------------------------

    template<typename T, class... Args>
    void HeteroVector::emplace_back(Args&&... args) {

        get_vector<T>().emplace_back(std::forward<Args>(args)...);
    }

    // ----------------------------------------------------------------------------

    template<typename T, typename ITR, class... Args>
    void HeteroVector::emplace(ITR pos, Args&&... args) {

        get_vector<T>().emplace(pos, std::forward<Args>(args)...);
    }

    // ----------------------------------------------------------------------------

    template<typename T, typename U>
    void HeteroVector::visit_impl_help_(T& visitor) {

        auto    iter = vectors_<U>.find(this);

        if (iter != vectors_<U>.end())
            for (auto&& element : iter->second)
                visitor(element);
    }

    // ----------------------------------------------------------------------------

    template<typename T, typename U>
    void HeteroVector::visit_impl_help_(T& visitor) const {

        const auto  citer = vectors_<U>.find(this);

        if (citer != vectors_<U>.end())
            for (auto&& element : citer->second)
                visitor(element);
    }

    // ----------------------------------------------------------------------------

    template<typename T, typename U>
    void HeteroVector::sort_impl_help_(T& functor) {

        auto    iter = vectors_<U>.find(this);

        if (iter != vectors_<U>.end())
            std::sort(iter->second.begin(), iter->second.end(), functor);
    }

    // ----------------------------------------------------------------------------

    template<typename T, typename U>
    void HeteroVector::change_impl_help_(T& functor) {

        auto    iter = vectors_<U>.find(this);

        if (iter != vectors_<U>.end())
            functor(iter->second);
    }

    // ----------------------------------------------------------------------------

    template<typename T, typename U>
    void HeteroVector::change_impl_help_(T& functor) const {

        const auto  citer = vectors_<U>.find(this);

        if (citer != vectors_<U>.end())
            functor(citer->second);
    }

    // ----------------------------------------------------------------------------

    template<class T, template<class...> class TLIST, class... TYPES>
    void HeteroVector::visit_impl_(T&& visitor, TLIST<TYPES...>) {

        // (..., visit_impl_help_<std::decay_t<T>, TYPES>(visitor)); // C++17
        using expander = int[];
        (void)expander { 0, (visit_impl_help_<T, TYPES>(visitor), 0) ... };
    }

    // ----------------------------------------------------------------------------

    template<class T, template<class...> class TLIST, class... TYPES>
    void HeteroVector::visit_impl_(T&& visitor, TLIST<TYPES...>) const {

        // (..., visit_impl_help_<std::decay_t<T>, TYPES>(visitor)); // C++17
        using expander = int[];
        (void)expander { 0, (visit_impl_help_<T, TYPES>(visitor), 0) ... };
    }

    // ----------------------------------------------------------------------------

    template<class T, template<class...> class TLIST, class... TYPES>
    void HeteroVector::sort_impl_(T&& functor, TLIST<TYPES...>) {

        using expander = int[];
        (void)expander { 0, (sort_impl_help_<T, TYPES>(functor), 0) ... };
    }

    // ----------------------------------------------------------------------------

    template<class T, template<class...> class TLIST, class... TYPES>
    void HeteroVector::change_impl_(T&& functor, TLIST<TYPES...>) {

        using expander = int[];
        (void)expander { 0, (change_impl_help_<T, TYPES>(functor), 0) ... };
    }

    // ----------------------------------------------------------------------------

    template<class T, template<class...> class TLIST, class... TYPES>
    void HeteroVector::change_impl_(T&& functor, TLIST<TYPES...>) const {

        using expander = int[];
        (void)expander { 0, (change_impl_help_<T, TYPES>(functor), 0) ... };
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    void HeteroVector::erase(size_type pos) {

        auto& vec = get_vector<T>();

        vec.erase(vec.begin() + pos);
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    void HeteroVector::resize(size_type count) {

        get_vector<T>().resize(count);
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    void HeteroVector::resize(size_type count, const T& v) {

        get_vector<T>().resize(count, v);
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    void HeteroVector::pop_back() { get_vector<T>().pop_back(); }

    // ----------------------------------------------------------------------------

    template<typename T>
    bool HeteroVector::empty() const noexcept {

        return (get_vector<T>().empty());
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    T& HeteroVector::at(size_type idx) {

        return (get_vector<T>().at(idx));
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    const T& HeteroVector::at(size_type idx) const {

        return (get_vector<T>().at(idx));
    }

    // ----------------------------------------------------------------------------

    template<typename T>
    T& HeteroVector::back() { return (get_vector<T>().back()); }

    // ----------------------------------------------------------------------------

    template<typename T>
    const T& HeteroVector::back() const { return (get_vector<T>().back()); }

    // ----------------------------------------------------------------------------

    template<typename T>
    T& HeteroVector::front() { return (get_vector<T>().front()); }

    // ----------------------------------------------------------------------------

    template<typename T>
    const T& HeteroVector::front() const { return (get_vector<T>().front()); }

} // namespace hmdf

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:

