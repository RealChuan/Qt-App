#pragma once

#include "predicates.h"

#include <QList>
#include <QSet>
#include <QStringList>

#include <algorithm>
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>

namespace Utils {

// ---- 内部辅助 -------------------------------------------------------------

template<typename F, typename T>
decltype(auto) invokeItem(F function, const T &value)
{
    if constexpr (std::is_member_function_pointer_v<F>)
        return (derefItem(value).*function)();
    else
        return function(value);
}

// ---- 查找 -----------------------------------------------------------------

template<std::ranges::range Container, typename Predicate>
bool anyOf(const Container &container, Predicate predicate)
{
    return std::ranges::any_of(container, predicate);
}

template<std::ranges::range Container, typename Predicate>
bool allOf(const Container &container, Predicate predicate)
{
    return std::ranges::all_of(container, predicate);
}

template<std::ranges::range Container, typename R, typename S>
bool allOf(const Container &container, R (S::*function)() const)
{
    return std::ranges::all_of(container, [function](const auto &item) {
        return (derefItem(item).*function)();
    });
}

template<std::ranges::range Container, typename Predicate>
auto findOrDefault(const Container &container, Predicate predicate)
    -> typename Container::value_type
{
    const auto it = std::ranges::find_if(container, predicate);
    return it == std::ranges::end(container) ? typename Container::value_type() : *it;
}

template<std::ranges::range Container, typename T, typename Predicate>
auto findOr(const Container &container, T defaultValue, Predicate predicate)
    -> typename Container::value_type
{
    const auto it = std::ranges::find_if(container, predicate);
    return it == std::ranges::end(container) ? typename Container::value_type(defaultValue) : *it;
}

template<std::ranges::range Container, typename LessThan>
auto maxElementOrDefault(const Container &container, LessThan lessThan)
    -> typename Container::value_type
{
    if (std::ranges::empty(container))
        return {};
    return *std::ranges::max_element(container, lessThan);
}

// ---- 过滤 -----------------------------------------------------------------

template<std::ranges::range Container, typename Predicate>
Container filtered(const Container &container, Predicate predicate)
{
    Container result;
    std::ranges::copy_if(container, std::back_inserter(result), predicate);
    return result;
}

template<std::ranges::range Container, typename R, typename S>
Container filtered(const Container &container, R (S::*function)() const)
{
    return filtered(container, [function](const auto &item) {
        return (derefItem(item).*function)();
    });
}

// ---- 映射 -----------------------------------------------------------------

// 显式指定结果容器类型：transform<QStringList>(container, function)
template<typename ResultContainer, std::ranges::range Container, typename F>
ResultContainer transform(Container &&container, F function)
{
    ResultContainer result;
    std::ranges::transform(std::forward<Container>(container),
                           std::back_inserter(result),
                           [function](const auto &item) { return invokeItem(function, item); });
    return result;
}

// 指定容器模板（默认 QList）：transform<QList>(container, function)
template<template<typename> class C = QList, std::ranges::range Container, typename F>
auto transform(Container &&container, F function)
{
    using Result = std::decay_t<decltype(invokeItem(function, *std::ranges::begin(container)))>;
    return transform<C<Result>>(std::forward<Container>(container), function);
}

// ---- 排序 -----------------------------------------------------------------

template<std::ranges::range Container, typename Predicate>
void sort(Container &container, Predicate predicate)
{
    std::ranges::sort(container, predicate);
}

template<std::ranges::range Container, typename R, typename S>
void sort(Container &container, R (S::*function)() const)
{
    std::ranges::sort(container, [function](const auto &left, const auto &right) {
        return (derefItem(left).*function)() < (derefItem(right).*function)();
    });
}

template<std::ranges::range Container, typename Predicate>
Container sorted(const Container &container, Predicate predicate)
{
    Container result = container;
    std::ranges::sort(result, predicate);
    return result;
}

// ---- 其它 -----------------------------------------------------------------

template<typename Set, typename T>
bool insert(Set &set, const T &value)
{
    if (set.contains(value))
        return false;
    set.insert(value);
    return true;
}

template<std::ranges::range Container, typename Op>
void reverseForeach(const Container &container, const Op &operation)
{
    for (auto it = std::ranges::rbegin(container); it != std::ranges::rend(container); ++it)
        operation(*it);
}

} // namespace Utils
