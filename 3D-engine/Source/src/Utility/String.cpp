#include "Utility/String.h"

#include <cassert>
#include <string>
#include <utility>

#include "Maths/Maths.h"

#include "Utility/Collections/TList.h"

using std::string;

constexpr int CAPACITY_GROWTH = 128;

String::String()
	: m_contents{ new char[CAPACITY_GROWTH] }, m_length{ 0 }, m_capacity{ CAPACITY_GROWTH }
{}

String::String(const char* str)
	: m_contents{ new char[strlen(str) + 1] },
	m_length{ strlen(str) }, m_capacity{ m_length + CAPACITY_GROWTH }
{
	for (uint64 i = 0; i < m_length; ++i)
	{
		m_contents[i] = str[i];
	}
}

String::String(const String& rhs)
	: m_contents{ new char[rhs.m_capacity] }, m_length{ rhs.m_length }, m_capacity{ rhs.m_capacity }
{
	for (uint64 i = 0; i < m_length; ++i)
	{
		m_contents[i] = rhs.m_contents[i];
	}
}

String::String(String&& rhs) noexcept
	: m_contents{ rhs.m_contents }, m_length{ rhs.m_length }, m_capacity{ rhs.m_capacity }
{
	rhs.m_contents = nullptr;
	rhs.m_length = 0;
	rhs.m_capacity = 0;
}

String::~String()
{
	delete[] m_contents;
	m_contents = nullptr;

	m_capacity = 0;
	m_length = 0;
}

uint64 String::Length() const
{
	return m_length;
}

uint64 String::Capacity() const
{
	return m_capacity;
}

char* String::Data() const
{
	return m_contents;
}

const char* String::CStr() const
{
	return m_contents;
}

int64 String::Find(const String& str) const
{
	for (int64 i = 0; std::cmp_less(i, m_length); ++i)
	{
		if (m_contents[i] == str[0])
		{
			bool found = true;
			for (int64 j = 0; std::cmp_less(j, str.Length()) || std::cmp_less(i + j, m_length); ++j)
			{
				if (m_contents[i + j] != str[j])
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				return i;
			}
		}
	}

	return -1;
}

int64 String::RFind(const String& str) const
{
	for (int64 i = static_cast<int64>(m_length); i >= 0; ++i)
	{
		if (m_contents[i] == str[0])
		{
			bool found = true;
			for (int64 j = 0; std::cmp_less(j, str.Length()) || std::cmp_less(i + j, m_length); ++j)
			{
				if (m_contents[i + j] != str[j])
				{
					found = false;
					break;
				}
			}

			if (found)
			{
				return i;
			}
		}
	}

	return -1;
}

int64 String::FindFirstOf(const char c) const
{
	for (int64 i = 0; std::cmp_less(i, m_length); ++i)
	{
		if (m_contents[i] == c)
		{
			return i;
		}
	}

	return -1;
}

int64 String::FindLastOf(const char c) const
{
	for (int64 i = static_cast<int64>(m_length); i >= 0; --i)
	{
		if (m_contents[i] == c)
		{
			return i;
		}
	}

	return -1;
}

int64 String::FindFirstNotOf(const char c) const
{
	for (int64 i = 0; i < std::cmp_less(i, m_length); ++i)
	{
		if (m_contents[i] != c)
		{
			return i;
		}
	}

	return -1;
}

int64 String::FindLastNotOf(const char c) const
{
	for (int64 i = static_cast<int64>(m_length); i >= 0; --i)
	{
		if (m_contents[i] != c)
		{
			return i;
		}
	}

	return -1;
}

String String::SubString(const uint64 i, const uint64 length) const
{
	TList<char> characters;
	const uint64 endIndex = Maths::Min(i + length, i + (m_length - length));

	for (uint64 index = i; index < endIndex; ++index)
	{
		characters.Add(m_contents[i]);
	}
	
	return characters.Data();
}

void String::Replace(const char find, const char replace) const
{
	for (uint64 i = 0; i < m_length; ++i)
	{
		if (m_contents[i] == find)
		{
			m_contents[i] = replace;
		}
	}
}

void String::Insert(const char insert, const uint64 index)
{
	if (m_length + 1 >= m_capacity)
	{
		Expand();
	}

	std::memmove(&m_contents[index + 1], &m_contents[index], m_length - 1);
	m_contents[index] = insert;
	m_length++;
}

void String::Replace(const String& find, const String& replace)
{
	int64 index = Find(find);
	while (std::cmp_less(index, m_length))
	{
		if (m_length + replace.Length() > m_capacity)
		{
			Expand();
		}

		std::memmove(&m_contents[index + 1], &m_contents[index], m_length - replace.Length());

		for (int64 i = 0; std::cmp_less(i, replace.Length()); ++i)
		{
			m_contents[index + i] = replace[i];
		}

		index = Find(find);
	}

	m_length = strlen(m_contents);
}

void String::Insert(const String& insert, const uint64 index)
{
	if (m_length + insert.Length() > m_capacity)
	{
		Expand();
	}

	std::memmove(&m_contents[index + 1], &m_contents[index], m_length - insert.Length());
	for (int64 i = 0; std::cmp_less(i, insert.Length()); ++i)
	{
		m_contents[index + i] = insert[i];
	}

	m_length = strlen(m_contents);
}

void String::Clear()
{
	m_length = 0;
}

void String::Expand()
{
	m_capacity += CAPACITY_GROWTH;
	m_contents = static_cast<char*>(realloc(m_contents, m_capacity));
}

bool String::operator==(const String& rhs) const
{
	if (this == &rhs)
	{
		return true;
	}

	return strcmp(m_contents, rhs.m_contents) == 0 && m_capacity == rhs.m_capacity &&
		m_length == rhs.m_length;
}

bool String::operator!=(const String& rhs) const
{
	if (this == &rhs)
	{
		return false;
	}

	return strcmp(m_contents, rhs.m_contents) != 0 || m_capacity != rhs.m_capacity ||
		m_length != rhs.m_length;
}

String& String::operator=(String rhs)
{
	std::swap(m_contents, rhs.m_contents);
	std::swap(m_length, rhs.m_length);
	std::swap(m_capacity, rhs.m_capacity);

	return *this;
}

String& String::operator=(const char* rhs)
{
	const uint64 length = strlen(rhs);
	Clear();

	for (uint64 i = 0; i < length; ++i)
	{
		*this += rhs[i];
	}

	return *this;
}

String& String::operator=(String&& rhs) noexcept
{
	if (this == &rhs)
	{
		return *this;
	}

	m_contents = rhs.m_contents;
	m_length = rhs.m_length;
	m_capacity = rhs.m_capacity;

	rhs.m_contents = nullptr;
	rhs.m_length = 0;
	rhs.m_capacity = 0;

	return *this;
}

String String::operator+(const String& rhs) const
{
	String newStr = *this;
	newStr += rhs;

	return newStr;
}

String& String::operator+=(const String& rhs)
{
	while (m_length + rhs.Length() >= m_capacity)
	{
		Expand();
	}

	for (int64 i = 0; i < std::cmp_less(i, rhs.Length()); ++i)
	{
		m_contents[m_length + i] = rhs[i];
	}

	m_length += rhs.Length();
	return *this;
}

String String::operator+(const char c) const
{
	String newStr = *this;
	newStr += c;

	return newStr;
}

String& String::operator+=(const char c)
{
	if (m_length + 1 >= m_capacity)
	{
		Expand();
	}

	m_contents[m_length++] = c;
	return *this;
}

char& String::operator[](const uint64 index)
{
	assert(index < m_length);
	return m_contents[index];
}

const char& String::operator[](const uint64 index) const
{
	assert(index < m_length);
	return m_contents[index];
}

ostream& operator<<(ostream& stream, const String& str)
{
	stream << str.m_contents;
	return stream;
}

istream& operator>>(istream& stream, String& str)
{
	string data;
	stream >> data;
	str = data.c_str();

	return stream;
}