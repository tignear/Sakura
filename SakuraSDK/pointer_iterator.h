#pragma once
#include <iterator>
namespace tignear::stdex {
	template <class Value>
	class pointer_iterator {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = Value;
		using size_type = size_t;
		using pointer = value_type * ;
		using reference = value_type & ;
		using difference_type = std::ptrdiff_t;
	private:
		Value* const base;
		Value* ptr;
	public:
		constexpr pointer_iterator() {}
		constexpr pointer_iterator(const pointer_iterator&) = default;
		constexpr pointer_iterator(pointer_iterator&&) = default;
		constexpr pointer_iterator& operator=(const pointer_iterator&) = default;
		constexpr pointer_iterator& operator=(pointer_iterator&&) = default;
		constexpr reference operator*() const{
			return *ptr;
		}
		constexpr pointer operator->()const {
			return ptr;
		}
		constexpr reference operator[](size_type index)const{
			return base[index];
		}
		constexpr pointer_iterator& operator++() {
			++ptr;
			return *this;
		}
		constexpr pointer_iterator operator++(int) {
			auto cp = *this;
			++ptr;
			return cp;
		}
		constexpr pointer_iterator& operator--() {
			--ptr;
			return *this;
		}
		constexpr pointer_iterator operator--(int) {
			auto cp = *this;
			--ptr;
			return cp;
		}
		constexpr pointer_iterator& operator+=(size_type i) {
			ptr += i;
			return *this;
		}	
		constexpr pointer_iterator operator+(size_type i)const {
			auto cp = *this;
			cp += i;
			return cp;
		}
		constexpr pointer_iterator& operator-=(size_type i) {
			ptr -= i;
			return *this;
		}
		constexpr pointer_iterator operator-(size_type i)const {
			auto cp = *this;
			cp -= i;
			return cp;
		}
		constexpr bool operator==(const pointer_iterator& other)const {
			return ptr == other.ptr;
		}
		constexpr bool operator!=(const pointer_iterator& other) const {
			return ptr != other.ptr;
		}
		constexpr bool operator>=(const pointer_iterator& other)const {
			return ptr >= other.ptr;
		}
		constexpr bool operator>(const pointer_iterator& other)const {
			return ptr > other.ptr;
		}
		constexpr bool operator<=(const pointer_iterator& other) const {
			return ptr <= other.ptr;
		}
		constexpr bool operator<(const pointer_iterator& other) const {
			return ptr < other.ptr;
		}
	};
	template<class Value>
	using reverse_pointer_iterator = std::reverse_iterator<pointer_iterator<Value>>;
}