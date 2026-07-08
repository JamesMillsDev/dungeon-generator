#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <iterator>
#include <ranges>

#include "Maths/Alias.h"

using std::initializer_list;

template<typename T, int64 GROWTH = 16>
class TArray
{
public:
	struct Iterator
	{
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;

	private:
		pointer m_ptr;

	public:
		Iterator();
		Iterator(pointer ptr);

	public:
		reference operator*() const;
		pointer operator->();

		Iterator& operator++();
		Iterator& operator--();

		Iterator operator++(int);
		Iterator operator--(int);

		bool operator==(const Iterator& rhs) const;
		bool operator!=(const Iterator& rhs) const;

	};

	struct View : std::ranges::view_interface<View>
	{
	public:
		T value;

	public:
		Iterator begin() const;
		Iterator end() const;

	};

private:
	int64 m_capacity;
	int64 m_count;

	T* m_data;

public:
	TArray();
	TArray(const initializer_list<T>& initialData);

	TArray(const TArray& rhs);
	TArray(TArray&& rhs) noexcept;
	~TArray();

public:
	void Add(T item);
	void Insert(T item, int64 index);

	void Remove(T item);
	void RemoveAt(int64 index);

	void Clear();

	[[nodiscard]] bool Contains(T item);
	[[nodiscard]] bool IsEmpty() const;

	[[nodiscard]] int64 Find(T item) const;
	[[nodiscard]] int64 Capacity() const;
	[[nodiscard]] int64 Count() const;

	T At(int64 index) const;
	T Front() const;
	T Back() const;

	T* Data() noexcept;
	const T* Data() const noexcept;

	void Resize(uint64 newSize);

	Iterator begin();
	Iterator end();
	[[nodiscard]] uint64 size();
	[[nodiscard]] bool empty();

	Iterator begin() const;
	Iterator end() const;
	[[nodiscard]] uint64 size() const;
	[[nodiscard]] bool empty() const;

private:
	void Expand();

public:
	TArray& operator=(const TArray& rhs);
	TArray& operator=(TArray&& rhs) noexcept;

	T& operator[](int64 index);
	const T& operator[](int64 index) const;

	static_assert(std::bidirectional_iterator<Iterator>);

};

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::Iterator::Iterator()
	: m_ptr{ nullptr }
{

}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::Iterator::Iterator(pointer ptr)
	: m_ptr{ ptr }
{

}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator::reference TArray<T, GROWTH>::Iterator::operator*() const
{
	return *m_ptr;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator::pointer TArray<T, GROWTH>::Iterator::operator->()
{
	return m_ptr;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator& TArray<T, GROWTH>::Iterator::operator++()
{
	++m_ptr;
	return *this;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator& TArray<T, GROWTH>::Iterator::operator--()
{
	--m_ptr;
	return *this;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::Iterator::operator++(int)
{
	Iterator tmp = *this;
	++(*this);
	return tmp;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::Iterator::operator--(int)
{
	Iterator tmp = *this;
	--(*this);
	return tmp;
}

template <typename T, int64 GROWTH>
bool TArray<T, GROWTH>::Iterator::operator==(const Iterator& rhs) const
{
	return m_ptr == rhs.m_ptr;
}

template <typename T, int64 GROWTH>
bool TArray<T, GROWTH>::Iterator::operator!=(const Iterator& rhs) const
{
	return m_ptr != rhs.m_ptr;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::View::begin() const
{
	return std::ranges::begin(value);
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::View::end() const
{
	return std::ranges::end(value);
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::TArray()
	: m_capacity{ GROWTH }, m_count{ 0 }, m_data{ new T[m_capacity]{} }
{

}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::TArray(const initializer_list<T>& initialData)
	: m_capacity{ GROWTH }, m_count{ 0 }, m_data{ new T[m_capacity]{} }
{
	for (uint64 i = 0; i < initialData.size(); ++i)
	{
		Add(*(initialData.begin() + i));
	}
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::TArray(const TArray& rhs)
	: m_capacity{ rhs.m_capacity }, m_count{ rhs.m_count }, m_data{ new T[m_capacity]{} }
{
	for (uint64 i = 0; i < m_capacity; ++i)
	{
		m_data[i] = rhs.m_data[i];
	}
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::TArray(TArray&& rhs) noexcept
	: m_capacity{ rhs.m_capacity }, m_count{ rhs.m_count }, m_data{ rhs.m_data }
{
	rhs.m_capacity = 0;
	rhs.m_count = 0;
	rhs.m_data = nullptr;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::~TArray()
{
	m_capacity = 0;
	m_count = 0;
	delete[] m_data;
}

template <typename T, int64 GROWTH>
void TArray<T, GROWTH>::Add(T item)
{
	if (m_count + 1 >= m_capacity)
	{
		Expand();
	}

	m_data[m_count++] = item;
}

template <typename T, int64 GROWTH>
void TArray<T, GROWTH>::Insert(T item, int64 index)
{
	if (m_count + 1 >= m_capacity)
	{
		Expand();
	}

	memmove(&m_data[index + 1], &m_data[index], (m_capacity - index - 1) * sizeof(T));
	m_data[index] = item;
	m_count++;
}

template <typename T, int64 GROWTH>
void TArray<T, GROWTH>::Remove(T item)
{
	int64 index = Find(item);
	if (index == -1)
	{
		return;
	}

	RemoveAt(index);
}

template <typename T, int64 GROWTH>
void TArray<T, GROWTH>::RemoveAt(int64 index)
{
	if (m_count == 0)
	{
		return;
	}

	memmove(&m_data[index], &m_data[index + 1], (m_count - index - 1) * sizeof(T));
	m_count--;
}

template <typename T, int64 GROWTH>
void TArray<T, GROWTH>::Clear()
{
	m_count = 0;
}

template <typename T, int64 GROWTH>
bool TArray<T, GROWTH>::Contains(T item)
{
	return Find(item) != -1;
}

template <typename T, int64 GROWTH>
bool TArray<T, GROWTH>::IsEmpty() const
{
	return m_count == 0;
}

template <typename T, int64 GROWTH>
int64 TArray<T, GROWTH>::Find(T item) const
{
	for (int64 i = 0; i < m_count; ++i)
	{
		if (m_data[i] == item)
		{
			return i;
		}
	}

	return -1;
}

template <typename T, int64 GROWTH>
int64 TArray<T, GROWTH>::Capacity() const
{
	return m_capacity;
}

template <typename T, int64 GROWTH>
int64 TArray<T, GROWTH>::Count() const
{
	return m_count;
}

template <typename T, int64 GROWTH>
T TArray<T, GROWTH>::At(int64 index) const
{
	return this->operator[](index);
}

template <typename T, int64 GROWTH>
T TArray<T, GROWTH>::Front() const
{
	return At(0);
}

template <typename T, int64 GROWTH>
T TArray<T, GROWTH>::Back() const
{
	return At(m_count - 1);
}

template <typename T, int64 GROWTH>
T* TArray<T, GROWTH>::Data() noexcept
{
	return m_data;
}

template <typename T, int64 GROWTH>
const T* TArray<T, GROWTH>::Data() const noexcept
{
	return m_data;
}

template <typename T, int64 GROWTH>
void TArray<T, GROWTH>::Resize(uint64 newSize)
{
	uint64 copySize = newSize < m_capacity ? newSize : m_capacity;

	T* newData = new T[newSize];
	std::copy(m_data, m_data + copySize, newData);
	delete[] m_data;
	m_data = newData;

	m_capacity = newSize;
	m_count = m_capacity;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::begin()
{
	return Iterator{ &m_data[0] };
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::end()
{
	return Iterator{ &m_data[m_count] };
}

template <typename T, int64 GROWTH>
uint64 TArray<T, GROWTH>::size()
{
	return m_count;
}

template <typename T, int64 GROWTH>
bool TArray<T, GROWTH>::empty()
{
	return m_count == 0;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::begin() const
{
	return Iterator{ &m_data[0] };
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>::template Iterator TArray<T, GROWTH>::end() const
{
	return Iterator{ &m_data[m_count] };
}

template <typename T, int64 GROWTH>
uint64 TArray<T, GROWTH>::size() const
{
	return static_cast<uint64>(Count());
}

template <typename T, int64 GROWTH>
bool TArray<T, GROWTH>::empty() const
{
	return IsEmpty();
}

template <typename T, int64 GROWTH>
void TArray<T, GROWTH>::Expand()
{
	T* newData = new T[m_capacity + GROWTH];
	std::copy(m_data, m_data + m_capacity, newData);
	delete[] m_data;
	m_data = newData;
	m_capacity += GROWTH;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>& TArray<T, GROWTH>::operator=(const TArray& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	m_capacity = rhs.m_capacity;
	m_count = rhs.m_count;
	m_data = new T[m_capacity];
	for (uint64 i = 0; i < m_capacity; ++i)
	{
		m_data[i] = rhs.m_data[i];
	}

	return *this;
}

template <typename T, int64 GROWTH>
TArray<T, GROWTH>& TArray<T, GROWTH>::operator=(TArray&& rhs) noexcept
{
	if (this == &rhs)
	{
		return *this;
	}

	m_capacity = rhs.m_capacity;
	m_count = rhs.m_count;
	m_data = rhs.m_data;

	rhs.m_capacity = 0;
	rhs.m_count = 0;
	rhs.m_data = nullptr;

	return *this;
}

template <typename T, int64 GROWTH>
T& TArray<T, GROWTH>::operator[](int64 index)
{
	assert(index < m_capacity && index >= 0);
	return m_data[index];
}

template <typename T, int64 GROWTH>
const T& TArray<T, GROWTH>::operator[](int64 index) const
{
	assert(index < m_count && index >= 0);
	return m_data[index];
}