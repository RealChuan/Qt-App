// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#pragma once

#include "predicates.h"

#include <qcompilerdetection.h> // for Q_REQUIRED_RESULT

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

#include <QHash>
#include <QObject>
#include <QSet>
#include <QStringList>

#include <memory>
#include <optional>
#include <type_traits>

namespace Utils {

/////////////////////////
// anyOf
/////////////////////////
template<typename T, typename F>
auto anyOf(const T &container, F predicate) -> bool;
template<typename T, typename R, typename S>
auto anyOf(const T &container, R (S::*predicate)() const) -> bool;
template<typename T, typename R, typename S>
auto anyOf(const T &container, R S::*member) -> bool;

/////////////////////////
// count
/////////////////////////
template<typename T, typename F>
auto count(const T &container, F predicate) -> int;

/////////////////////////
// allOf
/////////////////////////
template<typename T, typename F>
auto allOf(const T &container, F predicate) -> bool;

/////////////////////////
// erase
/////////////////////////
template<typename T, typename F>
void erase(T &container, F predicate);
template<typename T, typename F>
auto eraseOne(T &container, F predicate) -> bool;

/////////////////////////
// contains
/////////////////////////
template<typename T, typename F>
auto contains(const T &container, F function) -> bool;
template<typename T, typename R, typename S>
auto contains(const T &container, R (S::*function)() const) -> bool;
template<typename C, typename R, typename S>
auto contains(const C &container, R S::*member) -> bool;

/////////////////////////
// findOr
/////////////////////////
template<typename C, typename F>
Q_REQUIRED_RESULT auto findOr(const C &container,
                                                typename C::value_type other,
                                                F function) -> typename C::value_type;
template<typename T, typename R, typename S>
Q_REQUIRED_RESULT auto findOr(const T &container,
                                                typename T::value_type other,
                                                R (S::*function)() const) -> typename T::value_type;
template<typename T, typename R, typename S>
Q_REQUIRED_RESULT auto findOr(const T &container,
                                                typename T::value_type other,
                                                R S::*member) -> typename T::value_type;

/////////////////////////
// findOrDefault
/////////////////////////
template<typename C, typename F>
Q_REQUIRED_RESULT typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value,
                                            typename C::value_type>
findOrDefault(const C &container, F function);
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value,
                                            typename C::value_type>
findOrDefault(const C &container, R (S::*function)() const);
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value,
                                            typename C::value_type>
findOrDefault(const C &container, R S::*member);

/////////////////////////
// indexOf
/////////////////////////
template<typename C, typename F>
Q_REQUIRED_RESULT auto indexOf(const C &container, F function) -> int;

/////////////////////////
// maxElementOr
/////////////////////////
template<typename T>
auto maxElementOr(const T &container, typename T::value_type other) -> typename T::value_type;

/////////////////////////
// filtered
/////////////////////////
template<typename C, typename F>
Q_REQUIRED_RESULT auto filtered(const C &container, F predicate) -> C;
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT auto filtered(const C &container, R (S::*predicate)() const) -> C;

/////////////////////////
// partition
/////////////////////////
// Recommended usage:
// C hit;
// C miss;
// std::tie(hit, miss) = Utils::partition(container, predicate);
template<typename C, typename F>
Q_REQUIRED_RESULT std::tuple<C, C> partition(const C &container, F predicate);
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT std::tuple<C, C> partition(const C &container, R (S::*predicate)() const);

/////////////////////////
// filteredUnique
/////////////////////////
template<typename C>
Q_REQUIRED_RESULT auto filteredUnique(const C &container) -> C;

/////////////////////////
// qobject_container_cast
/////////////////////////
template<class T, template<typename> class Container, typename Base>
auto qobject_container_cast(const Container<Base> &container) -> Container<T>;

/////////////////////////
// static_container_cast
/////////////////////////
template<class T, template<typename> class Container, typename Base>
auto static_container_cast(const Container<Base> &container) -> Container<T>;

/////////////////////////
// sort
/////////////////////////
template<typename Container>
inline void sort(Container &container);
template<typename Container, typename Predicate>
inline void sort(Container &container, Predicate p);
template<typename Container, typename R, typename S>
inline void sort(Container &container, R S::*member);
template<typename Container, typename R, typename S>
inline void sort(Container &container, R (S::*function)() const);

/////////////////////////
// reverseForeach
/////////////////////////
template<typename Container, typename Op>
inline void reverseForeach(const Container &c, const Op &operation);

/////////////////////////
// toReferences
/////////////////////////
template<template<typename...> class ResultContainer, typename SourceContainer>
auto toReferences(SourceContainer &sources);
template<typename SourceContainer>
auto toReferences(SourceContainer &sources);

/////////////////////////
// toConstReferences
/////////////////////////
template<template<typename...> class ResultContainer, typename SourceContainer>
auto toConstReferences(const SourceContainer &sources);
template<typename SourceContainer>
auto toConstReferences(const SourceContainer &sources);

/////////////////////////
// take
/////////////////////////
template<class C, typename P>
Q_REQUIRED_RESULT std::optional<typename C::value_type> take(C &container, P predicate);
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT auto take(C &container, R S::*member) -> decltype(auto);
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT auto take(C &container, R (S::*function)() const) -> decltype(auto);

/////////////////////////
// setUnionMerge
/////////////////////////
// Works like std::set_union but provides a merge function for items that match
// !(a > b) && !(b > a) which normally means that there is an "equal" match.
// It uses iterators to support move_iterators.
template<class InputIt1, class InputIt2, class OutputIt, class Merge, class Compare>
auto setUnionMerge(InputIt1 first1,
                       InputIt1 last1,
                       InputIt2 first2,
                       InputIt2 last2,
                       OutputIt d_first,
                       Merge merge,
                       Compare comp) -> OutputIt;
template<class InputIt1, class InputIt2, class OutputIt, class Merge>
auto setUnionMerge(
    InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutputIt d_first, Merge merge) -> OutputIt;
template<class OutputContainer, class InputContainer1, class InputContainer2, class Merge, class Compare>
auto setUnionMerge(InputContainer1 &&input1,
                              InputContainer2 &&input2,
                              Merge merge,
                              Compare comp) -> OutputContainer;
template<class OutputContainer, class InputContainer1, class InputContainer2, class Merge>
auto setUnionMerge(InputContainer1 &&input1, InputContainer2 &&input2, Merge merge) -> OutputContainer;

/////////////////////////
// usize / ssize
/////////////////////////
template<typename Container>
std::make_unsigned_t<typename Container::size_type> usize(Container container);
template<typename Container>
std::make_signed_t<typename Container::size_type> ssize(Container container);

/////////////////////////
// setUnion
/////////////////////////
template<typename InputIterator1, typename InputIterator2, typename OutputIterator, typename Compare>
auto set_union(InputIterator1 first1,
                         InputIterator1 last1,
                         InputIterator2 first2,
                         InputIterator2 last2,
                         OutputIterator result,
                         Compare comp) -> OutputIterator;
template<typename InputIterator1, typename InputIterator2, typename OutputIterator>
auto set_union(InputIterator1 first1,
                         InputIterator1 last1,
                         InputIterator2 first2,
                         InputIterator2 last2,
                         OutputIterator result) -> OutputIterator;

/////////////////////////
// transform
/////////////////////////
// function without result type deduction:
template<typename ResultContainer, // complete result container type
         typename SC,              // input container type
         typename F>               // function type
Q_REQUIRED_RESULT auto transform(SC &&container, F function) -> decltype(auto);

// function with result type deduction:
template<template<typename> class C, // result container type
         typename SC,                // input container type
         typename F,                 // function type
         typename Value = typename std::decay_t<SC>::value_type,
         typename Result = std::decay_t<std::result_of_t<F(Value &)>>,
         typename ResultContainer = C<Result>>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, F function);
#ifdef Q_CC_CLANG
// "Matching of template template-arguments excludes compatible templates"
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2016/p0522r0.html (P0522R0)
// in C++17 makes the above match e.g. C=std::vector even though that takes two
// template parameters. Unfortunately the following one matches too, and there is no additional
// partial ordering rule, resulting in an ambiguous call for this previously valid code.
// GCC and MSVC ignore that issue and follow the standard to the letter, but Clang only
// enables the new behavior when given -frelaxed-template-template-args .
// To avoid requiring everyone using this header to enable that feature, keep the old implementation
// for Clang.
template<template<typename, typename> class C, // result container type
         typename SC,                          // input container type
         typename F,                           // function type
         typename Value = typename std::decay_t<SC>::value_type,
         typename Result = std::decay_t<std::result_of_t<F(Value &)>>,
         typename ResultContainer = C<Result, std::allocator<Result>>>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, F function);
#endif

// member function without result type deduction:
template<template<typename...> class C, // result container type
         typename SC,                   // input container type
         typename R,
         typename S>
Q_REQUIRED_RESULT auto transform(SC &&container, R (S::*p)() const) -> decltype(auto);

// member function with result type deduction:
template<typename ResultContainer, // complete result container type
         typename SC,              // input container type
         typename R,
         typename S>
Q_REQUIRED_RESULT auto transform(SC &&container, R (S::*p)() const) -> decltype(auto);

// member without result type deduction:
template<typename ResultContainer, // complete result container type
         typename SC,              // input container
         typename R,
         typename S>
Q_REQUIRED_RESULT auto transform(SC &&container, R S::*p) -> decltype(auto);

// member with result type deduction:
template<template<typename...> class C, // result container
         typename SC,                   // input container
         typename R,
         typename S>
Q_REQUIRED_RESULT auto transform(SC &&container, R S::*p) -> decltype(auto);

// same container types for input and output, const input
// function:
template<template<typename...> class C, // container type
         typename F,                    // function type
         typename... CArgs>             // Arguments to SC
Q_REQUIRED_RESULT auto transform(const C<CArgs...> &container, F function) -> decltype(auto);

// same container types for input and output, const input
// member function:
template<template<typename...> class C, // container type
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT auto transform(const C<CArgs...> &container, R (S::*p)() const) -> decltype(auto);

// same container types for input and output, const input
// members:
template<template<typename...> class C, // container
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT auto transform(const C<CArgs...> &container, R S::*p) -> decltype(auto);

// same container types for input and output, non-const input
// function:
template<template<typename...> class C, // container type
         typename F,                    // function type
         typename... CArgs>             // Arguments to SC
Q_REQUIRED_RESULT auto transform(C<CArgs...> &container, F function) -> decltype(auto);

// same container types for input and output, non-const input
// member function:
template<template<typename...> class C, // container type
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT auto transform(C<CArgs...> &container, R (S::*p)() const) -> decltype(auto);

// same container types for input and output, non-const input
// members:
template<template<typename...> class C, // container
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT auto transform(C<CArgs...> &container, R S::*p) -> decltype(auto);

/////////////////////////////////////////////////////////////////////////////
////////    Implementations    //////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

//////////////////
// anyOf
/////////////////
template<typename T, typename F>
auto anyOf(const T &container, F predicate) -> bool
{
    return std::any_of(std::begin(container), std::end(container), predicate);
}

// anyOf taking a member function pointer
template<typename T, typename R, typename S>
auto anyOf(const T &container, R (S::*predicate)() const) -> bool
{
    return std::any_of(std::begin(container), std::end(container), std::mem_fn(predicate));
}

// anyOf taking a member pointer
template<typename T, typename R, typename S>
auto anyOf(const T &container, R S::*member) -> bool
{
    return std::any_of(std::begin(container), std::end(container), std::mem_fn(member));
}

//////////////////
// count
/////////////////
template<typename T, typename F>
auto count(const T &container, F predicate) -> int
{
    return std::count_if(std::begin(container), std::end(container), predicate);
}

//////////////////
// allOf
/////////////////
template<typename T, typename F>
auto allOf(const T &container, F predicate) -> bool
{
    return std::all_of(std::begin(container), std::end(container), predicate);
}

// allOf taking a member function pointer
template<typename T, typename R, typename S>
auto allOf(const T &container, R (S::*predicate)() const) -> bool
{
    return std::all_of(std::begin(container), std::end(container), std::mem_fn(predicate));
}

// allOf taking a member pointer
template<typename T, typename R, typename S>
auto allOf(const T &container, R S::*member) -> bool
{
    return std::all_of(std::begin(container), std::end(container), std::mem_fn(member));
}

//////////////////
// erase
/////////////////
template<typename T, typename F>
void erase(T &container, F predicate)
{
    container.erase(std::remove_if(std::begin(container), std::end(container), predicate),
                    std::end(container));
}
template<typename T, typename F>
auto eraseOne(T &container, F predicate) -> bool
{
    const auto it = std::find_if(std::begin(container), std::end(container), predicate);
    if (it == std::end(container))
        return false;
    container.erase(it);
    return true;
}

//////////////////
// contains
/////////////////
template<typename T, typename F>
auto contains(const T &container, F function) -> bool
{
    return anyOf(container, function);
}

template<typename T, typename R, typename S>
auto contains(const T &container, R (S::*function)() const) -> bool
{
    return anyOf(container, function);
}

template<typename C, typename R, typename S>
auto contains(const C &container, R S::*member) -> bool
{
    return anyOf(container, std::mem_fn(member));
}

//////////////////
// findOr
/////////////////
template<typename C, typename F>
Q_REQUIRED_RESULT auto findOr(const C &container,
                                                typename C::value_type other,
                                                F function) -> typename C::value_type
{
    typename C::const_iterator begin = std::begin(container);
    typename C::const_iterator end = std::end(container);

    typename C::const_iterator it = std::find_if(begin, end, function);
    return it == end ? other : *it;
}

template<typename T, typename R, typename S>
Q_REQUIRED_RESULT auto findOr(const T &container,
                                                typename T::value_type other,
                                                R (S::*function)() const) -> typename T::value_type
{
    return findOr(container, other, std::mem_fn(function));
}

template<typename T, typename R, typename S>
Q_REQUIRED_RESULT auto findOr(const T &container,
                                                typename T::value_type other,
                                                R S::*member) -> typename T::value_type
{
    return findOr(container, other, std::mem_fn(member));
}

//////////////////
// findOrDefault
//////////////////
// Default implementation:
template<typename C, typename F>
Q_REQUIRED_RESULT typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value,
                                            typename C::value_type>
findOrDefault(const C &container, F function)
{
    return findOr(container, typename C::value_type(), function);
}

template<typename C, typename R, typename S>
Q_REQUIRED_RESULT typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value,
                                            typename C::value_type>
findOrDefault(const C &container, R (S::*function)() const)
{
    return findOr(container, typename C::value_type(), std::mem_fn(function));
}

template<typename C, typename R, typename S>
Q_REQUIRED_RESULT typename std::enable_if_t<std::is_copy_assignable<typename C::value_type>::value,
                                            typename C::value_type>
findOrDefault(const C &container, R S::*member)
{
    return findOr(container, typename C::value_type(), std::mem_fn(member));
}

//////////////////
// index of:
//////////////////

template<typename C, typename F>
Q_REQUIRED_RESULT int indexOf(const C &container, F function)
{
    typename C::const_iterator begin = std::begin(container);
    typename C::const_iterator end = std::end(container);

    typename C::const_iterator it = std::find_if(begin, end, function);
    return it == end ? -1 : std::distance(begin, it);
}

//////////////////
// max element
//////////////////

template<typename T>
typename T::value_type maxElementOr(const T &container, typename T::value_type other)
{
    typename T::const_iterator begin = std::begin(container);
    typename T::const_iterator end = std::end(container);

    typename T::const_iterator it = std::max_element(begin, end);
    if (it == end)
        return other;
    return *it;
}

//////////////////
// transform
/////////////////

namespace {
/////////////////
// helper code for transform to use back_inserter and thus push_back for everything
// and insert for QSet<>
//

// SetInsertIterator, straight from the standard for insert_iterator
// just without the additional parameter to insert
template<class Container>
class SetInsertIterator
{
protected:
    Container *container;

public:
    using iterator_category = std::output_iterator_tag;
    using container_type = Container;
    explicit SetInsertIterator(Container &x)
        : container(&x)
    {}
    SetInsertIterator<Container> &operator=(const typename Container::value_type &value)
    {
        container->insert(value);
        return *this;
    }
    SetInsertIterator<Container> &operator=(typename Container::value_type &&value)
    {
        container->insert(std::move(value));
        return *this;
    }
    SetInsertIterator<Container> &operator*() { return *this; }
    SetInsertIterator<Container> &operator++() { return *this; }
    SetInsertIterator<Container> operator++(int) { return *this; }
};

// for QMap / QHash, inserting a std::pair / QPair
template<class Container>
class MapInsertIterator
{
protected:
    Container *container;

public:
    using iterator_category = std::output_iterator_tag;
    using container_type = Container;
    explicit MapInsertIterator(Container &x)
        : container(&x)
    {}
    MapInsertIterator<Container> &operator=(
        const std::pair<const typename Container::key_type, typename Container::mapped_type> &value)
    {
        container->insert(value.first, value.second);
        return *this;
    }
    MapInsertIterator<Container> &operator=(
        const QPair<typename Container::key_type, typename Container::mapped_type> &value)
    {
        container->insert(value.first, value.second);
        return *this;
    }
    MapInsertIterator<Container> &operator*() { return *this; }
    MapInsertIterator<Container> &operator++() { return *this; }
    MapInsertIterator<Container> operator++(int) { return *this; }
};

// inserter helper function, returns a std::back_inserter for most containers
// and is overloaded for QSet<> and other containers without push_back, returning custom inserters
template<typename C>
inline std::back_insert_iterator<C> inserter(C &container)
{
    return std::back_inserter(container);
}

template<typename X>
inline SetInsertIterator<QSet<X>> inserter(QSet<X> &container)
{
    return SetInsertIterator<QSet<X>>(container);
}

template<typename K, typename C, typename A>
inline SetInsertIterator<std::set<K, C, A>> inserter(std::set<K, C, A> &container)
{
    return SetInsertIterator<std::set<K, C, A>>(container);
}

template<typename K, typename H, typename C, typename A>
inline SetInsertIterator<std::unordered_set<K, H, C, A>> inserter(
    std::unordered_set<K, H, C, A> &container)
{
    return SetInsertIterator<std::unordered_set<K, H, C, A>>(container);
}

template<typename K, typename V, typename C, typename A>
inline SetInsertIterator<std::map<K, V, C, A>> inserter(std::map<K, V, C, A> &container)
{
    return SetInsertIterator<std::map<K, V, C, A>>(container);
}

template<typename K, typename V, typename H, typename C, typename A>
inline SetInsertIterator<std::unordered_map<K, V, H, C, A>> inserter(
    std::unordered_map<K, V, H, C, A> &container)
{
    return SetInsertIterator<std::unordered_map<K, V, H, C, A>>(container);
}

template<typename K, typename V>
inline MapInsertIterator<QMap<K, V>> inserter(QMap<K, V> &container)
{
    return MapInsertIterator<QMap<K, V>>(container);
}

template<typename K, typename V>
inline MapInsertIterator<QHash<K, V>> inserter(QHash<K, V> &container)
{
    return MapInsertIterator<QHash<K, V>>(container);
}

// Helper code for container.reserve that makes it possible to effectively disable it for
// specific cases

// default: do reserve
// Template arguments are more specific than the second version below, so this is tried first
template<template<typename...> class C, typename... CArgs, typename = decltype(&C<CArgs...>::reserve)>
void reserve(C<CArgs...> &c, typename C<CArgs...>::size_type s)
{
    c.reserve(s);
}

// containers that don't have reserve()
template<typename C>
void reserve(C &, typename C::size_type)
{}

} // namespace

// --------------------------------------------------------------------
// Different containers for input and output:
// --------------------------------------------------------------------

// different container types for input and output, e.g. transforming a QList into a QSet

// function without result type deduction:
template<typename ResultContainer, // complete result container type
         typename SC,              // input container type
         typename F>               // function type
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, F function)
{
    ResultContainer result;
    reserve(result, typename ResultContainer::size_type(container.size()));
    std::transform(std::begin(container), std::end(container), inserter(result), function);
    return result;
}

// function with result type deduction:
template<template<typename> class C, // result container type
         typename SC,                // input container type
         typename F,                 // function type
         typename Value,
         typename Result,
         typename ResultContainer>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, F function)
{
    return transform<ResultContainer>(std::forward<SC>(container), function);
}

#ifdef Q_CC_CLANG
template<template<typename, typename> class C, // result container type
         typename SC,                          // input container type
         typename F,                           // function type
         typename Value,
         typename Result,
         typename ResultContainer>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, F function)
{
    return transform<ResultContainer>(std::forward<SC>(container), function);
}
#endif

// member function without result type deduction:
template<template<typename...> class C, // result container type
         typename SC,                   // input container type
         typename R,
         typename S>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, R (S::*p)() const)
{
    return transform<C>(std::forward<SC>(container), std::mem_fn(p));
}

// member function with result type deduction:
template<typename ResultContainer, // complete result container type
         typename SC,              // input container type
         typename R,
         typename S>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, R (S::*p)() const)
{
    return transform<ResultContainer>(std::forward<SC>(container), std::mem_fn(p));
}

// member without result type deduction:
template<typename ResultContainer, // complete result container type
         typename SC,              // input container
         typename R,
         typename S>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, R S::*p)
{
    return transform<ResultContainer>(std::forward<SC>(container), std::mem_fn(p));
}

// member with result type deduction:
template<template<typename...> class C, // result container
         typename SC,                   // input container
         typename R,
         typename S>
Q_REQUIRED_RESULT decltype(auto) transform(SC &&container, R S::*p)
{
    return transform<C>(std::forward<SC>(container), std::mem_fn(p));
}

// same container types for input and output, const input

// function:
template<template<typename...> class C, // container type
         typename F,                    // function type
         typename... CArgs>             // Arguments to SC
Q_REQUIRED_RESULT decltype(auto) transform(const C<CArgs...> &container, F function)
{
    return transform<C, const C<CArgs...> &>(container, function);
}

// member function:
template<template<typename...> class C, // container type
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT decltype(auto) transform(const C<CArgs...> &container, R (S::*p)() const)
{
    return transform<C, const C<CArgs...> &>(container, std::mem_fn(p));
}

// members:
template<template<typename...> class C, // container
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT decltype(auto) transform(const C<CArgs...> &container, R S::*p)
{
    return transform<C, const C<CArgs...> &>(container, std::mem_fn(p));
}

// same container types for input and output, non-const input

// function:
template<template<typename...> class C, // container type
         typename F,                    // function type
         typename... CArgs>             // Arguments to SC
Q_REQUIRED_RESULT decltype(auto) transform(C<CArgs...> &container, F function)
{
    return transform<C, C<CArgs...> &>(container, function);
}

// member function:
template<template<typename...> class C, // container type
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT decltype(auto) transform(C<CArgs...> &container, R (S::*p)() const)
{
    return transform<C, C<CArgs...> &>(container, std::mem_fn(p));
}

// members:
template<template<typename...> class C, // container
         typename R,
         typename S,
         typename... CArgs> // Arguments to SC
Q_REQUIRED_RESULT decltype(auto) transform(C<CArgs...> &container, R S::*p)
{
    return transform<C, C<CArgs...> &>(container, std::mem_fn(p));
}

// Specialization for QStringList:

template<template<typename...> class C = QList, // result container
         typename F>                            // Arguments to C
Q_REQUIRED_RESULT decltype(auto) transform(const QStringList &container, F function)
{
    return transform<C, const QList<QString> &>(static_cast<QList<QString>>(container), function);
}

// member function:
template<template<typename...> class C = QList, // result container type
         typename R,
         typename S>
Q_REQUIRED_RESULT decltype(auto) transform(const QStringList &container, R (S::*p)() const)
{
    return transform<C, const QList<QString> &>(static_cast<QList<QString>>(container),
                                                std::mem_fn(p));
}

// members:
template<template<typename...> class C = QList, // result container
         typename R,
         typename S>
Q_REQUIRED_RESULT decltype(auto) transform(const QStringList &container, R S::*p)
{
    return transform<C, const QList<QString> &>(static_cast<QList<QString>>(container),
                                                std::mem_fn(p));
}

//////////////////
// filtered
/////////////////
template<typename C, typename F>
Q_REQUIRED_RESULT C filtered(const C &container, F predicate)
{
    C out;
    std::copy_if(std::begin(container), std::end(container), inserter(out), predicate);
    return out;
}

template<typename C, typename R, typename S>
Q_REQUIRED_RESULT C filtered(const C &container, R (S::*predicate)() const)
{
    C out;
    std::copy_if(std::begin(container), std::end(container), inserter(out), std::mem_fn(predicate));
    return out;
}

//////////////////
// filteredCast
/////////////////
template<typename R, typename C, typename F>
Q_REQUIRED_RESULT R filteredCast(const C &container, F predicate)
{
    R out;
    std::copy_if(std::begin(container), std::end(container), inserter(out), predicate);
    return out;
}

//////////////////
// partition
/////////////////

// Recommended usage:
// C hit;
// C miss;
// std::tie(hit, miss) = Utils::partition(container, predicate);

template<typename C, typename F>
Q_REQUIRED_RESULT std::tuple<C, C> partition(const C &container, F predicate)
{
    C hit;
    C miss;
    reserve(hit, container.size());
    reserve(miss, container.size());
    auto hitIns = inserter(hit);
    auto missIns = inserter(miss);
    for (const auto &i : container) {
        if (predicate(i))
            hitIns = i;
        else
            missIns = i;
    }
    return std::make_tuple(hit, miss);
}

template<typename C, typename R, typename S>
Q_REQUIRED_RESULT std::tuple<C, C> partition(const C &container, R (S::*predicate)() const)
{
    return partition(container, std::mem_fn(predicate));
}

//////////////////
// filteredUnique
/////////////////

template<typename C>
Q_REQUIRED_RESULT C filteredUnique(const C &container)
{
    C result;
    auto ins = inserter(result);

    QSet<typename C::value_type> seen;
    int setSize = 0;

    auto endIt = std::end(container);
    for (auto it = std::begin(container); it != endIt; ++it) {
        seen.insert(*it);
        if (setSize == seen.size()) // unchanged size => was already seen
            continue;
        ++setSize;
        ins = *it;
    }
    return result;
}

//////////////////
// qobject_container_cast
/////////////////
template<class T, template<typename> class Container, typename Base>
Container<T> qobject_container_cast(const Container<Base> &container)
{
    Container<T> result;
    auto ins = inserter(result);
    for (Base val : container) {
        if (T target = qobject_cast<T>(val))
            ins = target;
    }
    return result;
}

//////////////////
// static_container_cast
/////////////////
template<class T, template<typename> class Container, typename Base>
Container<T> static_container_cast(const Container<Base> &container)
{
    Container<T> result;
    reserve(result, container.size());
    auto ins = inserter(result);
    for (Base val : container)
        ins = static_cast<T>(val);
    return result;
}

//////////////////
// sort
/////////////////
template<typename Container>
inline void sort(Container &container)
{
    std::stable_sort(std::begin(container), std::end(container));
}

template<typename Container, typename Predicate>
inline void sort(Container &container, Predicate p)
{
    std::stable_sort(std::begin(container), std::end(container), p);
}

// const lvalue
template<typename Container>
inline Container sorted(const Container &container)
{
    Container c = container;
    sort(c);
    return c;
}

// non-const lvalue
// This is needed because otherwise the "universal" reference below is used, modifying the input
// container.
template<typename Container>
inline Container sorted(Container &container)
{
    Container c = container;
    sort(c);
    return c;
}

// non-const rvalue (actually rvalue or lvalue, but lvalue is handled above)
template<typename Container>
inline Container sorted(Container &&container)
{
    sort(container);
    return std::move(container);
}

// const rvalue
template<typename Container>
inline Container sorted(const Container &&container)
{
    return sorted(container);
}

// const lvalue
template<typename Container, typename Predicate>
inline Container sorted(const Container &container, Predicate p)
{
    Container c = container;
    sort(c, p);
    return c;
}

// non-const lvalue
// This is needed because otherwise the "universal" reference below is used, modifying the input
// container.
template<typename Container, typename Predicate>
inline Container sorted(Container &container, Predicate p)
{
    Container c = container;
    sort(c, p);
    return c;
}

// non-const rvalue (actually rvalue or lvalue, but lvalue is handled above)
template<typename Container, typename Predicate>
inline Container sorted(Container &&container, Predicate p)
{
    sort(container, p);
    return std::move(container);
}

// const rvalue
template<typename Container, typename Predicate>
inline Container sorted(const Container &&container, Predicate p)
{
    return sorted(container, p);
}

// pointer to member
template<typename Container, typename R, typename S>
inline void sort(Container &container, R S::*member)
{
    auto f = std::mem_fn(member);
    using const_ref = typename Container::const_reference;
    std::stable_sort(std::begin(container), std::end(container), [&f](const_ref a, const_ref b) {
        return f(a) < f(b);
    });
}

// const lvalue
template<typename Container, typename R, typename S>
inline Container sorted(const Container &container, R S::*member)
{
    Container c = container;
    sort(c, member);
    return c;
}

// non-const lvalue
// This is needed because otherwise the "universal" reference below is used, modifying the input
// container.
template<typename Container, typename R, typename S>
inline Container sorted(Container &container, R S::*member)
{
    Container c = container;
    sort(c, member);
    return c;
}

// non-const rvalue (actually rvalue or lvalue, but lvalue is handled above)
template<typename Container, typename R, typename S>
inline Container sorted(Container &&container, R S::*member)
{
    sort(container, member);
    return std::move(container);
}

// const rvalue
template<typename Container, typename R, typename S>
inline Container sorted(const Container &&container, R S::*member)
{
    return sorted(container, member);
}

// pointer to member function
template<typename Container, typename R, typename S>
inline void sort(Container &container, R (S::*function)() const)
{
    auto f = std::mem_fn(function);
    using const_ref = typename Container::const_reference;
    std::stable_sort(std::begin(container), std::end(container), [&f](const_ref a, const_ref b) {
        return f(a) < f(b);
    });
}

// const lvalue
template<typename Container, typename R, typename S>
inline Container sorted(const Container &container, R (S::*function)() const)
{
    Container c = container;
    sort(c, function);
    return c;
}

// non-const lvalue
// This is needed because otherwise the "universal" reference below is used, modifying the input
// container.
template<typename Container, typename R, typename S>
inline Container sorted(Container &container, R (S::*function)() const)
{
    Container c = container;
    sort(c, function);
    return c;
}

// non-const rvalue (actually rvalue or lvalue, but lvalue is handled above)
template<typename Container, typename R, typename S>
inline Container sorted(Container &&container, R (S::*function)() const)
{
    sort(container, function);
    return std::move(container);
}

// const rvalue
template<typename Container, typename R, typename S>
inline Container sorted(const Container &&container, R (S::*function)() const)
{
    return sorted(container, function);
}

//////////////////
// reverseForeach
/////////////////
template<typename Container, typename Op>
inline void reverseForeach(const Container &c, const Op &operation)
{
    auto rend = c.rend();
    for (auto it = c.rbegin(); it != rend; ++it)
        operation(*it);
}

//////////////////
// toReferences
/////////////////
template<template<typename...> class ResultContainer, typename SourceContainer>
auto toReferences(SourceContainer &sources)
{
    return transform<ResultContainer>(sources, [](auto &value) { return std::ref(value); });
}

template<typename SourceContainer>
auto toReferences(SourceContainer &sources)
{
    return transform(sources, [](auto &value) { return std::ref(value); });
}

//////////////////
// toConstReferences
/////////////////
template<template<typename...> class ResultContainer, typename SourceContainer>
auto toConstReferences(const SourceContainer &sources)
{
    return transform<ResultContainer>(sources, [](const auto &value) { return std::cref(value); });
}

template<typename SourceContainer>
auto toConstReferences(const SourceContainer &sources)
{
    return transform(sources, [](const auto &value) { return std::cref(value); });
}

//////////////////
// take:
/////////////////

template<class C, typename P>
Q_REQUIRED_RESULT std::optional<typename C::value_type> take(C &container, P predicate)
{
    const auto end = std::end(container);

    const auto it = std::find_if(std::begin(container), end, predicate);
    if (it == end)
        return std::nullopt;

    std::optional<typename C::value_type> result = std::make_optional(std::move(*it));
    container.erase(it);
    return result;
}

// pointer to member
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT decltype(auto) take(C &container, R S::*member)
{
    return take(container, std::mem_fn(member));
}

// pointer to member function
template<typename C, typename R, typename S>
Q_REQUIRED_RESULT decltype(auto) take(C &container, R (S::*function)() const)
{
    return take(container, std::mem_fn(function));
}

//////////////////
// setUnionMerge: Works like std::set_union but provides a merge function for items that match
//                !(a > b) && !(b > a) which normally means that there is an "equal" match.
//                It uses iterators to support move_iterators.
/////////////////

template<class InputIt1, class InputIt2, class OutputIt, class Merge, class Compare>
OutputIt setUnionMerge(InputIt1 first1,
                       InputIt1 last1,
                       InputIt2 first2,
                       InputIt2 last2,
                       OutputIt d_first,
                       Merge merge,
                       Compare comp)
{
    for (; first1 != last1; ++d_first) {
        if (first2 == last2)
            return std::copy(first1, last1, d_first);
        if (comp(*first2, *first1)) {
            *d_first = *first2++;
        } else {
            if (comp(*first1, *first2)) {
                *d_first = *first1;
            } else {
                *d_first = merge(*first1, *first2);
                ++first2;
            }
            ++first1;
        }
    }
    return std::copy(first2, last2, d_first);
}

template<class InputIt1, class InputIt2, class OutputIt, class Merge>
OutputIt setUnionMerge(
    InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, OutputIt d_first, Merge merge)
{
    return setUnionMerge(first1,
                         last1,
                         first2,
                         last2,
                         d_first,
                         merge,
                         std::less<std::decay_t<decltype(*first1)>>{});
}

template<class OutputContainer, class InputContainer1, class InputContainer2, class Merge, class Compare>
OutputContainer setUnionMerge(InputContainer1 &&input1,
                              InputContainer2 &&input2,
                              Merge merge,
                              Compare comp)
{
    OutputContainer results;
    results.reserve(input1.size() + input2.size());

    setUnionMerge(std::make_move_iterator(std::begin(input1)),
                  std::make_move_iterator(std::end(input1)),
                  std::make_move_iterator(std::begin(input2)),
                  std::make_move_iterator(std::end(input2)),
                  std::back_inserter(results),
                  merge,
                  comp);

    return results;
}

template<class OutputContainer, class InputContainer1, class InputContainer2, class Merge>
OutputContainer setUnionMerge(InputContainer1 &&input1, InputContainer2 &&input2, Merge merge)
{
    return setUnionMerge<OutputContainer>(std::forward<InputContainer1>(input1),
                                          std::forward<InputContainer2>(input2),
                                          merge,
                                          std::less<std::decay_t<decltype(*std::begin(input1))>>{});
}

template<typename Container>
std::make_unsigned_t<typename Container::size_type> usize(Container container)
{
    return static_cast<std::make_unsigned_t<typename Container::size_type>>(container.size());
}

template<typename Container>
std::make_signed_t<typename Container::size_type> ssize(Container container)
{
    return static_cast<std::make_signed_t<typename Container::size_type>>(container.size());
}

template<typename Compare>
struct CompareIter
{
    Compare compare;

    explicit constexpr CompareIter(Compare compare)
        : compare(std::move(compare))
    {}

    template<typename Iterator1, typename Iterator2>
    constexpr bool operator()(Iterator1 it1, Iterator2 it2)
    {
        return bool(compare(*it1, *it2));
    }
};

template<typename InputIterator1, typename InputIterator2, typename OutputIterator, typename Compare>
auto set_union_impl(InputIterator1 first1,
                              InputIterator1 last1,
                              InputIterator2 first2,
                              InputIterator2 last2,
                              OutputIterator result,
                              Compare comp) -> OutputIterator
{
    auto compare = CompareIter<Compare>(comp);

    while (first1 != last1 && first2 != last2) {
        if (compare(first1, first2)) {
            *result = *first1;
            ++first1;
        } else if (compare(first2, first1)) {
            *result = *first2;
            ++first2;
        } else {
            *result = *first1;
            ++first1;
            ++first2;
        }
        ++result;
    }

    return std::copy(first2, last2, std::copy(first1, last1, result));
}

template<typename InputIterator1, typename InputIterator2, typename OutputIterator, typename Compare>
auto set_union(InputIterator1 first1,
                         InputIterator1 last1,
                         InputIterator2 first2,
                         InputIterator2 last2,
                         OutputIterator result,
                         Compare comp) -> OutputIterator
{
    return Utils::set_union_impl(first1, last1, first2, last2, result, comp);
}

template<typename InputIterator1, typename InputIterator2, typename OutputIterator>
auto set_union(InputIterator1 first1,
                         InputIterator1 last1,
                         InputIterator2 first2,
                         InputIterator2 last2,
                         OutputIterator result) -> OutputIterator
{
    return Utils::set_union_impl(first1,
                                 last1,
                                 first2,
                                 last2,
                                 result,
                                 std::less<typename InputIterator1::value_type>{});
}

// Replacement for deprecated Qt functionality

template<class T>
auto toSet(const QList<T> &list) -> QSet<T>
{
    return QSet<T>(list.begin(), list.end());
}

template<class T>
auto toList(const QSet<T> &set) -> QList<T>
{
    return QList<T>(set.begin(), set.end());
}

template<class Key, class T>
void addToHash(QHash<Key, T> *result, const QHash<Key, T> &additionalContents)
{
    result->insert(additionalContents);
}

} // namespace Utils
