#pragma once
#include <limits>
#include <cstddef>
#include "pointer_iterator.h"
namespace tignear::stdex {
	template <class Value>
	class array_view {
	public:
		using value_type = Value;
		using pointer = value_type* ;
		using const_pointer = const value_type*;
		using reference = value_type & ;
		using const_reference = const value_type&;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;
		using iterator_type = pointer_iterator<const value_type>;
		using const_iterator_type = iterator_type;
		constexpr const static auto npos = std::numeric_limits<size_t>::max();
	private:
		const_pointer ptr = nullptr;
		size_type m_size = 0;
	public:
		constexpr array_view()noexcept = default;
		template <size_t Size>
		constexpr array_view(const std::array<Value,Size>& mirror)noexcept {
			ptr = mirror.data();
			m_size = Size;
		}
		constexpr array_view(const value_type array[])noexcept :ptr(ptr), m_size(size) {
			ptr = array;
			m_size = _countof(array);
		}
		constexpr array_view(const_pointer ptr,size_t size)noexcept:ptr(ptr),m_size(size) {
			
		}
		constexpr array_view& operator=(const array_view& from)noexcept = default;
		constexpr size_type size() const noexcept {
			return m_size;
		}
		constexpr iterator_type begin()const noexcept {
			return iterator_type(ptr);
		}
		constexpr const_iterator_type cbegin()const noexcept {
			return const_iterator_type(ptr);
		}
		constexpr iterator_type end()const noexcept {
			return iterator_type(ptr + size());
		}
		constexpr const_iterator_type cend()const noexcept {
			return const_iterator_type(ptr+size());
		}
		constexpr std::reverse_iterator<iterator_type> rbegin()const noexcept {
			return std::reverse_iterator<iterator_type>(ptr);
		}
		constexpr std::reverse_iterator<iterator_type> rcbegin()const noexcept {
			return std::reverse_iterator<const_iterator_type>(ptr);
		}
		constexpr std::reverse_iterator<iterator_type> rend()const noexcept {
			return std::reverse_iterator<iterator_type>(ptr + size());
		}
		constexpr std::reverse_iterator<iterator_type> rcend()const noexcept {
			return std::reverse_iterator<const_iterator_type>(ptr + size());
		}

		constexpr const_reference operator[](size_type pos) const {
			return *(ptr + pos);
		}
		constexpr const_reference at(size_type pos) const {
			if (pos >= size()) {
				throw std::out_of_range();
			}
			return this[pos];
		}
		constexpr const_reference front()const {
			return *ptr;
		}
		constexpr const_reference back() const {
			return *(ptr + size());
		}
		constexpr const_pointer data() const noexcept {
			return ptr;
		}
		[[nodiscard]] constexpr bool empty() const noexcept {
			return size() == 0;
		}
		constexpr void remove_prefix(size_type n) {
			ptr += n;
		}
		constexpr void remove_suffix(size_type n) {
			size -= n;
		}
		constexpr void swap(array_view& v) noexcept {
			return std::swap(*this, v);//std::swap
		}
		size_type copy(value_type* dest,
			size_type count,
			size_type pos = 0) const {
			return memcpy(dest, ptr + pos, count);
		}
		constexpr array_view substr(size_type pos = 0, size_type count = npos) const {
			if (pos > size()) {
				throw std::out_of_range();
			}
			return array_view(ptr+pos,std::min(count,size()-pos));
		}
		constexpr size_type find(value_type target, size_type pos = 0) const noexcept {
			for (size_type i = pos; i < size(); ++i) {
				if (ptr[i] == target) {
					return i;
				}
			}
			return npos;
		}
		constexpr size_type rfind(value_type target, size_type pos = npos) const noexcept {
			auto end = std::crend(*this);
			pos = std::min(pos, size()-1);
			for (auto itr = std::crbegin(*this); itr != end;++itr) {
				if (*itr == target) {
					return pos;
				}
				--pos;
			}
			return npos;
		}
		constexpr size_type find_first_not_of(value_type target, size_type pos = 0) const noexcept {
			for (auto i = pos; i < size(); ++i) {
				if (target != ptr[i]) {
					return i;
				}
			}
			return npos;
		}
	};
}