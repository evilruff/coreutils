#pragma once

#include <tuple>
#include <vector>

template<typename ...Ts>
class MonotonicId {
public:
    MonotonicId(Ts... args) {
        m_items = std::make_tuple(args...);
    }

    MonotonicId(const MonotonicId & other): m_items(other.m_items) {}
    MonotonicId(MonotonicId && other) : m_items(std::move(other.m_items)) {}
    MonotonicId& operator = (const MonotonicId& other) {
	m_items = other.m_items;
	return *this;
    }

    bool operator < (const MonotonicId& other) const {
        return m_items < other.m_items;
    }

    bool operator == (const MonotonicId& other) const {
        return m_items == other.m_items;
    }

    int keysCount() const { return m_items.size(); }
    const std::tuple<Ts...> & key() const { return m_items; };

private:
    std::tuple<Ts...>   m_items;
};