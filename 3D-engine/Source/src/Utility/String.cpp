#include "Utility/String.h"

#include <cassert>
#include <string>

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
	return 0;
}

int64 String::RFind(const String& str) const
{
	return 0;
}

int64 String::FindFirstOf(char c) const
{
	return 0;
}

int64 String::FindLastOf(char c) const
{
	return 0;
}

String String::SubString(uint64 i, uint64 length)
{
	return { };
}

void String::Replace(char find, char replace)
{}

void String::Insert(char insert, uint64 index)
{}

void String::Replace(const String& find, const String& replace)
{}

void String::Insert(const String& insert, uint64 index)
{}

void String::Clear()
{
	m_length = 0;
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
	return *this;
}

String String::operator+(char c) const
{
	return { };
}

String& String::operator+=(char c)
{
	return *this;
}

String String::operator-(const String& rhs) const
{
	return { };
}

String& String::operator-=(const String& rhs)
{
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