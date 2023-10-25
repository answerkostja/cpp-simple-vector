#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <utility>
#include "array_ptr.h"


class ReserveProxyObj {
private:
	size_t capacity_;
public:
	ReserveProxyObj(size_t capacity) :
		capacity_(capacity) {
	}
	size_t GetCapacity() const {
		return capacity_;
	}
};

inline ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return ReserveProxyObj(capacity_to_reserve);
}

template<typename Type>
class SimpleVector {

public:
	using Iterator = Type*;
	using ConstIterator = const Type*;

	
	SimpleVector() noexcept = default;

	explicit SimpleVector(size_t size) :
		items_(size), size_(size), capacity_(size) {
		Fill(begin(), end(), Type{});
	}



	SimpleVector(size_t size, const Type& value) :
		items_(size), size_(size), capacity_(size) {
		Fill(begin(), end(), value);
	}


	SimpleVector(std::initializer_list<Type> init) :
		items_(init.size()), size_(init.size()), capacity_(init.size()) {
		std::move(init.begin(), init.end(), begin());
	}

	SimpleVector(ReserveProxyObj reserve) :
		items_(reserve.GetCapacity()), size_(0), capacity_(
			reserve.GetCapacity()) {
	}

	SimpleVector(const SimpleVector& other) {
		if (other.GetSize() <= capacity_) {
			size_ = other.GetSize();
			std::copy(other.begin(), other.end(), begin());
		}
		else {
			SimpleVector<Type> tmp(other.GetSize());
			if (other.GetSize() > 0) {
				std::copy(other.begin(), other.end(), tmp.begin());
			}
			swap(tmp);
		}

	}

	SimpleVector(SimpleVector&& other) :
		items_(other.GetCapacity()), size_(other.GetSize()), capacity_(
			other.GetCapacity()) {
		std::move(other.begin(), other.end(), begin());
		other.Clear();

	}

	SimpleVector& operator=(const SimpleVector& rhs) {
		if (this != &rhs) {
			SimpleVector tmp(rhs);
			swap(tmp);
		}
		return *this;
	}


	void PushBack(const Type& item) {

		if (size_ < capacity_) {
			size_++;
			items_[size_ - 1] = item;
		}

		else {
			size_t new_size = size_ + 1;
			Resize(new_size);
			items_[size_ - 1] = item;
		}

	}

	void PushBack(Type&& item) {
		Insert(end(), std::move(item));
	}


	Iterator Insert(ConstIterator pos, const Type& value) {
		size_t npos = pos - cbegin();
		if (capacity_ == 0) {
			SimpleVector<Type> tmp(1);
			tmp.items_[0] = value;
			swap(tmp);
		}
		else if (size_ < capacity_) {
			std::move(begin() + npos, end(), begin() + npos + 1);
			items_[npos] = value;
			size_++;
		}
		else {
			SimpleVector<Type> tmp(capacity_ * 2);
			tmp.Resize(size_ + 1);
			std::move(cbegin(), pos, tmp.begin());
			tmp.items_[npos] = value;
			std::move(pos, cend(), tmp.begin() + npos + 1);
			swap(tmp);
		}
		return begin() + npos;
	}

	Iterator Insert(Iterator pos, Type&& value) {
		size_t npos = pos - cbegin();
		if (capacity_ == 0) {
			ArrayPtr<Type> temp(1);
			temp[0] = std::move(value);
			items_.swap(temp);
			size_ = 1;
			capacity_ = 1;
		}
		else if (size_ < capacity_) {
			std::move(begin() + npos, end(), begin() + npos + 1);
			items_[npos] = std::move(value);
			size_++;
		}
		else {
			ArrayPtr<Type> temp(capacity_ * 2);

			for (size_t i = 0; i < capacity_ * 2; ++i) {
				temp[i] = std::move(Type{});
			}
			SimpleVector<Type> tmp;
			tmp.items_.swap(temp);
			std::move(begin(), pos, tmp.begin());
			tmp.items_[npos] = std::move(value);
			std::move(pos, end(), tmp.begin() + npos + 1);
			tmp.size_ = ++size_;
			tmp.capacity_ = capacity_ * 2;
			swap(tmp);
		}

		return begin() + npos;
	}

	void PopBack() noexcept {
		assert(size_ > 0);
		size_--;
	}

	Iterator Erase(ConstIterator pos) {
		size_t index = pos - cbegin();
		if (!IsEmpty()) {
			std::move(begin() + index + 1, end(), begin() + index);
			--size_;
		}
		return begin() + index;
	}

	void swap(SimpleVector& other) noexcept {
		items_.swap(other.items_);
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
	}

	size_t GetSize() const noexcept {
		return size_;
	}

	size_t GetCapacity() const noexcept {
		return capacity_;
	}

	bool IsEmpty() const noexcept {
		return (size_ == 0);
	}

	Type& operator[](size_t index) noexcept {
		return items_[index];
	}

	const Type& operator[](size_t index) const noexcept {
		return items_[index];
	}

	Type& At(size_t index) {
		if (index >= size_)
			throw std::out_of_range("out_of_range");
		return items_[index];
	}


	const Type& At(size_t index) const {
		if (index >= size_)
			throw std::out_of_range("out_of_range");
		return items_[index];
	}


	void Clear() noexcept {
		size_ = 0;
	}


	void Resize(size_t new_size) {

		if (new_size < size_)
			size_ = new_size;

		if (new_size <= capacity_) {
			FillWithDefault(end(), begin() + new_size);
			size_ = new_size;
		}

		if (new_size > capacity_) {

			size_t new_capacity = std::max(new_size, capacity_ * 2);
			ArrayPtr<Type> new_items(new_capacity);

			FillWithDefault(new_items.Get() + capacity_, new_items.Get() + new_size);
			std::move(begin(), end(), new_items.Get());
			items_.swap(new_items);
			size_ = new_size;
			capacity_ = new_capacity;
		}
	}


	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) {
			//Создаём новый массив
			ArrayPtr<Type> new_items(new_capacity);
			std::move(begin(), end(), new_items.Get());
			items_.swap(new_items);
			capacity_ = new_capacity;
		}
	}

	Iterator begin() noexcept {
		return items_.Get();
	}

	Iterator end() noexcept {
		return items_.Get() + size_;
	}

	ConstIterator begin() const noexcept {
		return items_.Get();
	}

	ConstIterator end() const noexcept {
		return items_.Get() + size_;
	}

	ConstIterator cbegin() const noexcept {
		return items_.Get();
	}

	ConstIterator cend() const noexcept {
		return items_.Get() + size_;
	}
private:
	ArrayPtr<Type> items_;
	size_t size_ = 0;
	size_t capacity_ = 0;

void Fill(Iterator first, Iterator last, Type value)
	{
		while (first != last)
		{
			*first = std::move(value);
			first++;
		}
	}
void FillWithDefault(Iterator first, Iterator last){
while (first != last)
{
      *first = Type{};
       first++;
}
}
};

template<typename Type>
inline bool operator==(const SimpleVector<Type>& lhs,
	const SimpleVector<Type>& rhs) {
	return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs,
	const SimpleVector<Type>& rhs) {
	return (!(lhs == rhs));
}

template<typename Type>
inline bool operator<(const SimpleVector<Type>& lhs,
	const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
		rhs.end());
}

template<typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs,
	const SimpleVector<Type>& rhs) {
	return !(rhs < lhs);
}

template<typename Type>
inline bool operator>(const SimpleVector<Type>& lhs,
	const SimpleVector<Type>& rhs) {
	return rhs < lhs;
}

template<typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs,
	const SimpleVector<Type>& rhs) {
	return !(lhs < rhs);
}

