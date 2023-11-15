#ifndef _STRING_VIEW
#define _STRING_VIEW 1
#include <algorithm>
#include <string.h>
//#include <bits/c++config.h>
#define _GLIBCXX14_CONSTEXPR constexpr
#define _GLIBCXX17_CONSTEXPR constexpr


/**
 *  @class basic_string_view <string_view>
 *  @brief  A non-owning reference to a string.
 *
 *  @ingroup strings
 *  @ingroup sequences
 *
 *  @tparam _CharT  Type of character
 *  @tparam _Traits  Traits for character type, defaults to
 *                   char_traits<_CharT>.
 *
 *  A basic_string_view looks like this:
 *
 *  @code
 *    _CharT*    _M_str
 *    size_t     _M_len
 *  @endcode
 */
template<typename _CharT>
class basic_string_view
{
	public:
	// types

	using value_type		= _CharT;
	using pointer		    = value_type*;
	using const_pointer	    = const value_type*;
	using reference		    = value_type&;
	using const_reference	= const value_type&;
	using const_iterator	= const value_type*;
	using iterator		    = const_iterator;
	using size_type		    = size_t;
	//using difference_type	= ptrdiff_t;

	static constexpr size_type npos = size_type(-1);
	const _CharT* _M_str;
	size_t	      _M_len;

	// [string.view.cons], construction and assignment

	constexpr
	basic_string_view() noexcept
	:_M_str{nullptr},_M_len{0}
	{}
	__attribute__((__nonnull__)) constexpr
	basic_string_view(const _CharT* __str) noexcept
	:_M_str{__str},_M_len{__builtin_strlen(__str)}
	{}
	constexpr
	basic_string_view(const _CharT* __str, size_type __len) noexcept
	:_M_str{__str},_M_len{__len}
	{}
	constexpr basic_string_view(const basic_string_view&) noexcept = default;
	constexpr basic_string_view& operator=(const basic_string_view&) noexcept = default;

	constexpr void
	swap(basic_string_view& __sv) noexcept
	{auto __tmp = *this;*this = __sv;__sv = __tmp;}

// [string.view.iterators], iterator support

	constexpr const_iterator begin() const noexcept {return this->_M_str;}
	constexpr const_iterator end() const noexcept   {return this->_M_str+this->_M_len;}
	constexpr const_iterator cbegin() const noexcept{return this->_M_str;}
	constexpr const_iterator cend() const noexcept  {return this->_M_str+this->_M_len;}

// [string.view.capacity], capacity

	[[nodiscard]] constexpr size_type size() const noexcept {return this->_M_len;}
	[[nodiscard]] constexpr size_type length() const noexcept { return _M_len; }
	[[nodiscard]] constexpr bool empty() const noexcept {return this->_M_len==0;}

// [string.view.access], element access

	constexpr const_reference
	operator[](size_type __pos) const noexcept
	{
		// TODO: Assert to restore in a way compatible with the constexpr.
		// __glibcxx_assert(__pos < this->_M_len);
		return *(this->_M_str + __pos);
	}

	constexpr const_reference
	at(size_type __pos) const noexcept {return *(this->_M_str + __pos);}
	constexpr const_reference front() const noexcept {return *this->_M_str;}
	constexpr const_reference back()  const noexcept {return *(this->_M_str + this->_M_len - 1);}
	constexpr const_pointer   data()  const noexcept {return this->_M_str;}

// [string.view.modifiers], modifiers:

	constexpr void
	remove_prefix(size_type __n) noexcept
	{
		__n = std::min(__n,_M_len);
		this->_M_str += __n;
		this->_M_len -= __n;
	}

	constexpr void
	remove_suffix(size_type __n) noexcept
	{ this->_M_len -= std::min(__n,this->_M_len); }

	void clean() noexcept
	{this->_M_len = 0;}


// [string.view.ops], string operations:

	constexpr basic_string_view
	substr(size_type __pos = 0, size_type __n = npos) const noexcept(false)
	{
		__pos = std::min(size(), __pos);
		const size_type __rlen = std::min(__n, _M_len - __pos);
		return basic_string_view{_M_str + __pos, __rlen};
	}

	[[nodiscard]] constexpr
	bool operator == (const basic_string_view s) const
	{return this->compare(s);}

	constexpr
	basic_string_view& operator ++ ()
	{
		if(0<_M_len)
		{
			this->_M_str++;
			this->_M_len--;
		}
		return *this;
	}

	[[nodiscard]] constexpr
	basic_string_view& operator += (const size_type s) const
	{remove_prefix(s);return *this;}

// [string.view.ops], string comaparition:

	// returns 1 if are __str == this, otherwise 0
	_GLIBCXX14_CONSTEXPR int
	compare(const basic_string_view __str) const noexcept
	{
		// calculate size diffrent
		if (this->_M_len == __str._M_len)
		{
			return memcmp(this->_M_str, __str._M_str, std::min(this->_M_len, __str._M_len)) == 0;
		} else
			return 0;
	}

	constexpr int
	compare(size_type __pos1, size_type __n1, basic_string_view __str) const
	{ return this->substr(__pos1, __n1).compare(__str); }

	constexpr int
	compare(size_type __pos1, size_type __n1,
	basic_string_view __str, size_type __pos2, size_type __n2) const
	{ return this->substr(__pos1, __n1).compare(__str.substr(__pos2, __n2)); }

	__attribute__((__nonnull__))
	constexpr int
	compare(const _CharT* __str) const noexcept
	{ return this->compare(basic_string_view{__str}); }

	__attribute__((__nonnull__))
	constexpr int
	compare(size_type __pos1, size_type __n1, const _CharT* __str) const
	{ return this->substr(__pos1, __n1).compare(basic_string_view{__str}); }

	__attribute__((__nonnull__))
	constexpr int
	compare(size_type __pos1, size_type __n1,const _CharT* __str, size_type __n2) const noexcept(false)
	{ return this->substr(__pos1, __n1).compare(basic_string_view(__str, __n2)); }


	constexpr bool
	starts_with(basic_string_view __x) const noexcept
	{ return this->substr(0, __x.size()).compare(__x); }
	constexpr bool
	starts_with(_CharT __x) const noexcept
	{ return !this->empty() && this->front() == __x; }
	constexpr bool
	starts_with(const _CharT* __x) const noexcept
	{ return this->starts_with(basic_string_view(__x)); }

	constexpr bool
	ends_with(basic_string_view __x) const noexcept
	{ return this->size() >= __x.size() && this->compare(this->size() - __x.size(), npos, __x) == 0; }
	constexpr bool
	ends_with(_CharT __x) const noexcept
	{ return !this->empty() && this->back() == __x; }
	constexpr bool
	ends_with(const _CharT* __x) const noexcept
	{ return this->ends_with(basic_string_view(__x)); }



	_GLIBCXX14_CONSTEXPR size_type
	find(const _CharT* __str, size_type __pos, size_type __n) const noexcept
	{
		__glibcxx_requires_string_len(__str, __n);
		if (__n == 0)
			return __pos <= this->_M_len ? __pos : npos;
		if (__n <= this->_M_len)
		{
			for(; __pos <= this->_M_len - __n; ++__pos)
				if( this->_M_str[__pos] == __str[0]
					&& memcmp( this->_M_str+__pos+1,__str+1, __n-1) ==0 )
					return __pos;
		}
		return npos;
	}

	_GLIBCXX14_CONSTEXPR size_type
	find(_CharT __c, size_type __pos) const noexcept
	{
		size_type __ret = npos;
		if (__pos < this->_M_len)
		{
			const size_type __n = this->_M_len - __pos;
			const _CharT* __p = find(this->_M_str + __pos, __n, __c);
			if (__p)
				__ret = __p - this->_M_str;
		}
		return __ret;
	}

	_GLIBCXX14_CONSTEXPR size_type
	rfind(const _CharT* __str, size_type __pos, size_type __n) const noexcept
	{
		__glibcxx_requires_string_len(__str, __n);

		if (__n <= this->_M_len)
		{
		__pos = std::min(size_type(this->_M_len - __n), __pos);
		do {
			if (memcmp(this->_M_str + __pos, __str, __n) == 0)
				return __pos;
		}while (__pos-- > 0);
		}
		return npos;
	}

	_GLIBCXX14_CONSTEXPR size_type
	rfind(_CharT __c, size_type __pos) const noexcept
	{
		size_type __size = this->_M_len;
		if (__size > 0)
		{
			if (--__size > __pos)
				__size = __pos;
			for (++__size; __size-- > 0; )
				if (this->_M_str[__size] == __c)
					return __size;
		}
		return npos;
	}

	_GLIBCXX14_CONSTEXPR size_type
	find_first_of(const _CharT* __str, size_type __pos,
	size_type __n) const noexcept
	{
		__glibcxx_requires_string_len(__str, __n);
		for (; __n && __pos < this->_M_len; ++__pos)
		{
			const _CharT* __p = find(__str, __n,this->_M_str[__pos]);
			if (__p)
				return __pos;
		}
		return npos;
	}

	_GLIBCXX14_CONSTEXPR size_type
	find_last_of(const _CharT* __str, size_type __pos,
	size_type __n) const noexcept
	{
		__glibcxx_requires_string_len(__str, __n);
		size_type __size = this->size();
		if (__size && __n)
		{
			if (--__size > __pos)
				__size = __pos;
			do{
				if (find(__str, __n, this->_M_str[__size]))
					return __size;
			}while (__size-- != 0);
		}
		return npos;
	}

protected:

	static
	_GLIBCXX17_CONSTEXPR
	const _CharT*
	find(const _CharT* __s, size_t __n, const _CharT& __a)
	{
		if (__n == 0)
			return 0;
	#if __cplusplus >= 201703L
		if( __builtin_constant_p(__n)
			&& __builtin_constant_p(__a)
			&& __constant_char_array_p(__s, __n) )
			return find(__s, __n, __a);
	#endif
		return static_cast<const value_type*>(__builtin_memchr(__s, __a, __n));
	}
};

using string_view = basic_string_view<char>;

#endif
