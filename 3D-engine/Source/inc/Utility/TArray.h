#pragma once

#include <cassert>
#include <initializer_list>
#include <iterator>
#include <ranges>

#include "Maths/Alias.h"

using std::initializer_list;

template<typename T>
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
	int64 m_growth;
	int64 m_count;

	T* m_data;

public:
	TArray(int64 growth = 16);
	TArray(const initializer_list<T>& initialData, int64 growth = 16);

	TArray(const TArray& rhs);
	TArray(TArray&& rhs) noexcept;
	~TArray();

public:
	void Add(T item);
	void Insert(T item, int64 index);

	void Remove(T item);
	void RemoveAt(int64 index);

	void Clear();

	bool Contains(T item);
	bool IsEmpty() const;

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

public:
	TArray& operator=(const TArray& rhs);
	TArray& operator=(TArray&& rhs) noexcept;

	T& operator[](int64 index);
	const T& operator[](int64 index) const;

	static_assert(std::bidirectional_iterator<Iterator>);

};

template <typename T>
TArray<T>::Iterator::Iterator()
	: m_ptr{ nullptr }
{
	
}

template <typename T>
TArray<T>::Iterator::Iterator(pointer ptr)
	: m_ptr{ ptr }
{
	
}

template <typename T>
TArray<T>::template Iterator::reference TArray<T>::Iterator::operator*() const
{
	return *m_ptr;
}

template <typename T>
TArray<T>::template Iterator::pointer TArray<T>::Iterator::operator->()
{
	return m_ptr;
}

template <typename T>
TArray<T>::template Iterator& TArray<T>::Iterator::operator++()
{
	++m_ptr;
	return *this;
}

template <typename T>
TArray<T>::template Iterator& TArray<T>::Iterator::operator--()
{
	--m_ptr;
	return *this;
}

template <typename T>
TArray<T>::template Iterator TArray<T>::Iterator::operator++(int)
{
	Iterator tmp = *this;
	++(*this);
	return tmp;
}

template <typename T>
TArray<T>::template Iterator TArray<T>::Iterator::operator--(int)
{
	Iterator tmp = *this;
	--(*this);
	return tmp;
}

template <typename T>
bool TArray<T>::Iterator::operator==(const Iterator& rhs) const
{
	return m_ptr == rhs.m_ptr;
}

template <typename T>
bool TArray<T>::Iterator::operator!=(const Iterator& rhs) const
{
	return m_ptr != rhs.m_ptr;
}

template <typename T>
TArray<T>::template Iterator TArray<T>::View::begin() const
{
	return std::ranges::begin(value);
}

template <typename T>
TArray<T>::template Iterator TArray<T>::View::end() const
{
	return std::ranges::end(value);
}

template <typename T>
TArray<T>::TArray(int64 growth)
	: m_capacity{ growth }, m_growth{ growth }, m_count{ 0 }, m_data{ new T[m_capacity] }
{

}

template <typename T>
TArray<T>::TArray(const initializer_list<T>& initialData, int64 growth)
	: m_capacity{ growth }, m_growth{ growth }, m_count{ 0 }, m_data{ new T[m_capacity]{} }
{
	for (uint64 i = 0; i < initialData.size(); ++i)
	{
		Add(*(initialData.begin() + i));
	}
}

template <typename T>
TArray<T>::TArray(const TArray& rhs)
	: m_capacity{ rhs.m_capacity }, m_growth{ rhs.m_growth }, m_count{ rhs.m_count }, m_data{ new T[m_capacity]{} }
{
	memcpy_s(m_data, m_capacity * sizeof(T), rhs.m_data, rhs.m_capacity * sizeof(T));
}

template <typename T>
TArray<T>::TArray(TArray&& rhs) noexcept
	: m_capacity{ rhs.m_capacity }, m_growth{ rhs.m_growth }, m_count{ rhs.m_count }, m_data{ rhs.m_data }
{
	rhs.m_capacity = 0;
	rhs.m_growth = 0;
	rhs.m_count = 0;
	rhs.m_data = nullptr;
}

template <typename T>
TArray<T>::~TArray()
{
	m_capacity = 0;
	m_growth = 0;
	m_count = 0;
	delete[] m_data;
}

template <typename T>
void TArray<T>::Add(T item)
{
	if (m_count + 1 >= m_capacity)
	{
		Resize(m_capacity + m_growth);
	}

	m_data[m_count++] = item;
}

template <typename T>
void TArray<T>::Insert(T item, int64 index)
{
	if (m_count + 1 >= m_capacity)
	{
		Resize(m_capacity + m_growth);
	}

	memmove(&m_data[index + 1], &m_data[index], (m_capacity - index - 1) * sizeof(T));
	m_data[index] = item;
	m_count++;
}

template <typename T>
void TArray<T>::Remove(T item)
{
	int64 index = Find(item);
	if (index == -1)
	{
		return;
	}

	RemoveAt(index);
}

template <typename T>
void TArray<T>::RemoveAt(int64 index)
{
	if (m_count == 0)
	{
		return;
	}

	memmove(&m_data[index], &m_data[index + 1], (m_count - index - 1) * sizeof(T));
	m_count--;
}

template <typename T>
void TArray<T>::Clear()
{
	m_count = 0;
}

template <typename T>
bool TArray<T>::Contains(T item)
{
	return Find(item) != -1;
}

template <typename T>
bool TArray<T>::IsEmpty() const
{
	return m_count == 0;
}

template <typename T>
int64 TArray<T>::Find(T item) const
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

template <typename T>
int64 TArray<T>::Capacity() const
{
	return m_capacity;
}

template <typename T>
int64 TArray<T>::Count() const
{
	return m_count;
}

template <typename T>
T TArray<T>::At(int64 index) const
{
	return this->operator[](index);
}

template <typename T>
T TArray<T>::Front() const
{
	return At(0);
}

template <typename T>
T TArray<T>::Back() const
{
	return At(m_count - 1);
}

template <typename T>
T* TArray<T>::Data() noexcept
{
	return m_data;
}

template <typename T>
const T* TArray<T>::Data() const noexcept
{
	return m_data;
}

template <typename T>
void TArray<T>::Resize(uint64 newSize)
{
	T* data = static_cast<T*>(malloc(sizeof(T) * newSize));
	memcpy_s(data, m_capacity * sizeof(T), m_data, m_capacity * sizeof(T));
	delete[] m_data;
	m_data = data;

	m_capacity = newSize;
}

template <typename T>
TArray<T>::template Iterator TArray<T>::begin()
{
	return Iterator{ &m_data[0] };
}

template <typename T>
TArray<T>::template Iterator TArray<T>::end()
{
	return Iterator{ &m_data[m_count] };
}

template <typename T>
uint64 TArray<T>::size()
{
	return m_count;
}

template <typename T>
bool TArray<T>::empty()
{
	return m_count == 0;
}

template <typename T>
TArray<T>::template Iterator TArray<T>::begin() const
{
	return Iterator{ &m_data[0] };
}

template <typename T>
TArray<T>::template Iterator TArray<T>::end() const
{
	return Iterator{ &m_data[m_count] };
}

template <typename T>
uint64 TArray<T>::size() const
{
	return static_cast<uint64>(Count());
}

template <typename T>
bool TArray<T>::empty() const
{
	return IsEmpty();
}

template <typename T>
TArray<T>& TArray<T>::operator=(const TArray& rhs)
{
	if (this == &rhs)
	{
		return *this;
	}

	m_capacity = rhs.m_capacity;
	m_growth = rhs.m_growth;
	m_count = rhs.m_count;
	m_data = new T[m_capacity];
	memcpy_s(m_data, m_capacity * sizeof(T), rhs.m_data, rhs.m_capacity * sizeof(T));

	return *this;
}

template <typename T>
TArray<T>& TArray<T>::operator=(TArray&& rhs) noexcept
{
	if (this == &rhs)
	{
		return *this;
	}

	m_capacity = rhs.m_capacity;
	m_growth = rhs.m_growth;
	m_count = rhs.m_count;
	m_data = rhs.m_data;

	rhs.m_capacity = 0;
	rhs.m_growth = 0;
	rhs.m_count = 0;
	rhs.m_data = nullptr;

	return *this;
}

template <typename T>
T& TArray<T>::operator[](int64 index)
{
	assert(index < m_count && index >= 0);
	return m_data[index];
}

template <typename T>
const T& TArray<T>::operator[](int64 index) const
{
	assert(index < m_count && index >= 0);
	return m_data[index];
}