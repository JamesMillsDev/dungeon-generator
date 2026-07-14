#pragma once

#include <iostream>

#include "Maths/Alias.h"

using std::istream;
using std::ostream;

class String
{
private:
	char* m_contents;
	uint64 m_length;
	uint64 m_capacity;

public:
	String();

	String(const char* str);

	String(const String& rhs);

	String(String&& rhs) noexcept;

	~String();

public:
	[[nodiscard]] uint64 Length() const;

	[[nodiscard]] uint64 Capacity() const;

	[[nodiscard]] char* Data() const;

	[[nodiscard]] const char* CStr() const;

	[[nodiscard]] int64 Find(const String& str) const;

	[[nodiscard]] int64 RFind(const String& str) const;

	[[nodiscard]] int64 FindFirstOf(char c) const;

	[[nodiscard]] int64 FindLastOf(char c) const;

	[[nodiscard]] String SubString(uint64 i, uint64 length = 0);

	void Replace(char find, char replace);

	void Insert(char insert, uint64 index);

	void Replace(const String& find, const String& replace);

	void Insert(const String& insert, uint64 index);

	void Clear();

public:
	bool operator==(const String& rhs) const;

	bool operator!=(const String& rhs) const;

	String& operator=(String rhs);

	String& operator=(const char* rhs);

	String& operator=(String&& rhs) noexcept;

	String operator+(const String& rhs) const;

	String& operator+=(const String& rhs);

	String operator+(char c) const;

	String& operator+=(char c);

	String operator-(const String& rhs) const;

	String& operator-=(const String& rhs);

	char& operator[](uint64 index);

	const char& operator[](uint64 index) const;

	friend ostream& operator<<(ostream& stream, const String& str);

	friend istream& operator>>(istream& stream, String& str);

};