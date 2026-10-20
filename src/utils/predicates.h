#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

namespace Utils {

// 统一处理“容器里存值”和“容器里存指针”两种情况。
template<typename T>
concept NonPointer = !std::is_pointer_v<std::remove_cvref_t<T>>;

template<NonPointer T>
constexpr auto derefItem(const T &value) -> const T &
{
    return value;
}

template<typename T>
constexpr auto derefItem(T *value) -> const T &
{
    return *value;
}

// 通过成员变量或 getter 与给定值比较，生成一元谓词。
template<typename R, typename S, typename T>
auto equal(R S::*member, T value)
{
    return [member, value](const auto &object) { return derefItem(object).*member == value; };
}

template<typename R, typename S, typename T>
auto equal(R (S::*function)() const, T value)
{
    return [function, value](const auto &object) { return (derefItem(object).*function)() == value; };
}

} // namespace Utils
