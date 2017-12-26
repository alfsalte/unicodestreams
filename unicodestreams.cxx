
#include <iostream>
#include <fstream>

#include "unicodestreams.hxx"

namespace {

inline
bool
is_valid_utf32(char32_t c)
{
  if (c > 0x10ffff)
    return false;
  if ((c & 0xfffe) == 0xfffe)
    return false;
  return c > 0xdfff || c < 0xd800;
}

inline
bool
is_valid_utf16_follow(char16_t c)
{
  return c >= 0xdc00 && c <= 0xdfff;
}

inline
bool
is_valid_utf8_follow(unsigned char c)
{
  return c >= 0x80 && c <= 0xbf;
}

const int arr__[5] = { 0, 0, 0x80, 0x800, 0x10000 };

std::istream::int_type
get_utf8(std::istream * is)
{
  typedef std::istream::int_type int_type;
  typedef alf::unicodestreams::status_type status_type;

  int_type w, k, c, j;
  char ch;

  if (is == 0)
    return -(int_type)status_type::NO_STREAM;

  if (! *is)
    return -(int_type)status_type::BAD_STREAM;

  if (! is->get(ch)) {
    if (is->rdstate() & std::ios_base::eofbit)
      return -(int_type)status_type::EOF_STREAM;
    return -(int_type)status_type::BAD_STREAM;
  }
  c = std::istream::traits_type::to_int_type(ch);
  if (c < 0x80)
    return c;
  if (c < 0xc0)
    return -(int_type)status_type::NO_LEAD;
  if (c < 0xe0) {
    j = k = 2;
    w = c & 0x1f;
  } else if (c < 0xf0) {
    j = k = 3;
    w = c & 0x0f;
  } else if (c < 0xf8) {
    j = k = 4;
    w = c & 0x07;
  } else
    return -(int_type)status_type::NOT_UTF8;
  while (--j) {
    if (! is->get(ch))
      return -(int_type)status_type::BAD_STREAM;
    c = (int_type)(unsigned char)ch;
    if (c < 0x80 || c >= 0xc0)
      return -(int_type)status_type::NOT_UTF8;
    w = (w << 6) | (c & 0x3f);
  }
  if (! is_valid_utf32(char32_t(w)))
    return -(int_type)status_type::NOT_UNICODE;
  if (w < arr__[k])
    return -(int_type)status_type::NOT_UTF8;
  return w;

} // end of function get_utf8

std::ostream::int_type
put_utf8(std::ostream * os, std::ostream::int_type c)
{
  typedef alf::unicodestreams::status_type status_type;
  typedef std::ostream::int_type int_type;

  if (os == 0)
    return -(int_type)status_type::NO_STREAM;
  if (! *os)
    return -(int_type)status_type::BAD_STREAM;
  if (! is_valid_utf32(char32_t(c)))
    return -(int_type)status_type::NOT_UNICODE;
  if (c < 0x80) {
    if (! os->put(c))
      return -(int_type)status_type::BAD_STREAM;
    return c;
  }
  if (c < 0x800) {
    if (! os->put(char(0xc0 | (c >> 6))))
      return -(int_type)status_type::BAD_STREAM;
    if (! os->put(char(0x80 | (c & 0x3f))))
      return -(int_type)status_type::BAD_STREAM;
    return c;
  }
  if (c < 0x10000) {
    if (! os->put(char(0xe0 | (c >> 12))))
      return -(int_type)status_type::BAD_STREAM;
    if (! os->put(char(0x80 | ((c >> 6) & 0x3f))))
      return -(int_type)status_type::BAD_STREAM;
    if (! os->put(char(0x80 | (c & 0x3f))))
      return -(int_type)status_type::BAD_STREAM;
    return c;
  }
  if (! os->put(char(0xf0 | (c >> 18))))
    return -(int_type)status_type::BAD_STREAM;
  if (! os->put(char(0x80 | ((c >> 12) & 0x3f))))
    return -(int_type)status_type::BAD_STREAM;
  if (! os->put(char(0x80 | ((c >> 6) & 0x3f))))
    return -(int_type)status_type::BAD_STREAM;
  if (! os->put(char(0x80 | (c & 0x3f))))
    return -(int_type)status_type::BAD_STREAM;
  return c;

} // end of function put_utf8

std::basic_istream<char16_t>::int_type
get_u16(std::basic_istream<char16_t> * is)
{
  
  typedef alf::unicodestreams::status_type status_type;
  typedef std::basic_istream<char16_t>::int_type int_type;

  int w, c;
  char16_t ch;

  if (is == 0)
    return -(int_type)status_type::NO_STREAM;

  if (! *is)
    return -(int_type)status_type::BAD_STREAM;

  if (! is->get(ch)) {
    if (is->rdstate() & std::ios_base::eofbit)
      return -(int_type)status_type::EOF_STREAM;
    return -(int_type)status_type::BAD_STREAM;
  }
  c = std::basic_istream<char16_t>::traits_type::to_int_type(ch);
  if (c < 0xd800 || c >= 0xe000) {
    if (! is_valid_utf32(char32_t(c)))
      return -(int_type)status_type::NOT_UNICODE;
    return c;
  }
  if (c >= 0xdc00)
    return -(int_type)status_type::NO_LEAD;
  w = (c & 0x3ff) << 10;
  if (! is->get(ch))
    return -(int_type)status_type::BAD_STREAM;
  c = std::basic_istream<char16_t>::traits_type::to_int_type(ch);
  if (c < 0xdc00 || c >= 0xe000)
    return -(int_type)status_type::NO_FOLLOW;
  w |= (c & 0x3ff);
  w += 0x10000;
  if (! is_valid_utf32(char32_t(w)))
    return -(int_type)status_type::NOT_UNICODE;
  return w;

} // end of function get_u16

std::basic_ostream<char16_t>::int_type
put_u16(std::basic_ostream<char16_t> * os,
	std::basic_ostream<char16_t>::int_type c)
{

  typedef alf::unicodestreams::status_type status_type;
  typedef std::basic_ostream<char16_t>::int_type int_type;

  if (os == 0)
    return -(int_type)status_type::NO_STREAM;
  if (! *os)
    return -(int_type)status_type::BAD_STREAM;
  if (! is_valid_utf32(char32_t(c)))
    return -(int_type)status_type::NOT_UNICODE;
  if (c < 0x10000) {
    if (! os->put(char16_t(c)))
      return -(int_type)status_type::BAD_STREAM;
    return c;
  }
  int k = c - 0x10000;
  if (! os->put(0xd800 | (k >> 10)))
    return -(int_type)status_type::BAD_STREAM;
  if (! os->put(0xdc00 | (k & 0x3ff)))
    return -(int_type)status_type::BAD_STREAM;
  return c;

} // end of function put_u16
  
std::basic_istream<char32_t>::int_type
get_u32(std::basic_istream<char32_t> * is)
{

  typedef alf::unicodestreams::status_type status_type;
  typedef std::basic_istream<char32_t>::int_type int_type;

  int w, c;
  char32_t ch;

  if (is == 0)
    return -(int_type)status_type::NO_STREAM;

  if (! *is)
    return -(int_type)status_type::BAD_STREAM;

  if (! is->get(ch)) {
    if (is->rdstate() & std::ios_base::eofbit)
      return -(int_type)status_type::EOF_STREAM;
    return -(int_type)status_type::BAD_STREAM;
  }
  c = std::basic_istream<char32_t>::traits_type::to_int_type(ch);
  if (! is_valid_utf32(char32_t(c)))
    return -(int_type)status_type::NOT_UNICODE;
  return c;

} // end of function get_u32

std::basic_ostream<char32_t>::int_type
put_u32(std::basic_ostream<char32_t> * os,
	std::basic_ostream<char32_t>::int_type c)
{
  typedef alf::unicodestreams::status_type status_type;
  typedef std::basic_ostream<char32_t>::int_type int_type;

  if (os == 0)
    return -(int_type)status_type::NO_STREAM;
  if (! *os)
    return -(int_type)status_type::BAD_STREAM;
  if (! is_valid_utf32(char32_t(c)))
    return -(int_type)status_type::NOT_UNICODE;
  if (! os->put(c))
    return -(int_type)status_type::BAD_STREAM;
  return c;

} // end of function put_u32

// s is either LE or BE
// check endianess of hardware and set swap_state based on
// expected input to be LE/BE according to s.
alf::unicodestreams::u32_swap_state_type
u32_check(alf::unicodestreams::u32_swap_state_type s)
{
  typedef alf::unicodestreams::u32_swap_state_type u32_swap_state_type;

  union {
    unsigned int v;
    char b[4];
  };

  v = 0x01020304;
  switch (s) {
  case u32_swap_state_type::LE:
    switch (b[0]) {
    case 1:
      // 1234 -> 4321
      return u32_swap_state_type::v4321;
    case 2:
      // 2143 -> 4321
      return u32_swap_state_type::v3412;
    case 3:
      // 3412 -> 4321
      return u32_swap_state_type::v2143;
    case 4:
      // 4321 -> 4321
      return u32_swap_state_type::v1234;
    }
    break;
  case u32_swap_state_type::BE:
    switch(b[0]) {
    case 1: // 1234 -> 1234
      return u32_swap_state_type::v1234;
    case 2: // 2143 -> 1234
      return u32_swap_state_type::v2143;
    case 3: // 3412 -> 1234
      return u32_swap_state_type::v3412;
    case 4: // 4321 -> 1234
      return u32_swap_state_type::v4321;
    }
    break;
  }
  return u32_swap_state_type::None;
}

// s is either LE or BE
// check endianess of hardware and set swap_state based on
// expected input to be LE/BE according to s.
alf::unicodestreams::u16_swap_state_type
u16_check(alf::unicodestreams::u16_swap_state_type s)
{
  typedef alf::unicodestreams::u16_swap_state_type u16_swap_state_type;

  union {
    unsigned short int v;
    char b[2];
  };

  v = 0x0102;
  switch (s) {
  case u16_swap_state_type::LE:
    switch (b[0]) {
    case 1:
      // 12 -> 21
      return u16_swap_state_type::v21;
    case 2:
      // 21 -> 21
      return u16_swap_state_type::v12;
    }
    break;
  case u16_swap_state_type::BE:
    switch(b[0]) {
    case 1: // 12 -> 12
      return u16_swap_state_type::v12;
    case 2: // 21 -> 12
      return u16_swap_state_type::v21;
    }
    break;
  }
  return u16_swap_state_type::None;
}

}; // end of anonymous namespace

///////////////////////////////////////
// u32

// for reading.
alf::unicodestreams::u32streambuf::u32streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u32streambuf::u32streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for both.
alf::unicodestreams::u32streambuf::
u32streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

alf::unicodestreams::u32streambuf::~u32streambuf()
{
  if (os_)
    overflow(traits_type::eof());
}

// we do not buffer anything, so rather than fill a buffer we just
// read one char and return it - leaving the buffer empty.

// virtual
alf::unicodestreams::u32streambuf::int_type
alf::unicodestreams::u32streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u32streambuf::int_type
alf::unicodestreams::u32streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u32streambuf::streambuf *
alf::unicodestreams::u32streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u32streambuf::int_type
alf::unicodestreams::u32streambuf::get()
{
  if (status_ != status_type::OK)
    return traits_type::eof();
  int_type c = get_u32(is_);
  if (c < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() == ibufe) {
    traits_type::move(ibufb, ibufe - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  this->setg(this->egptr(), this->gptr(), this->egptr() + 1);
  return traits_type::to_int_type(*gptr() = c);
}

alf::unicodestreams::u32streambuf::int_type
alf::unicodestreams::u32streambuf::put(int_type __c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();

  int_type k = 0;
  if (! traits_type::eq_int_type(__c, traits_type::eof())) {
    if ((k = put_u32(os_, __c)) < 0)
      return err_status((status_type)-k);
    return __c;
  }
  return traits_type::not_eof(__c);
}

////////////////////////////////
// u32u16

// for reading.
alf::unicodestreams::u32u16streambuf::u32u16streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u32u16streambuf::u32u16streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for both.
alf::unicodestreams::u32u16streambuf::
u32u16streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

alf::unicodestreams::u32u16streambuf::~u32u16streambuf()
{
  if (os_)
    overflow(traits_type::eof());
}

// virtual
alf::unicodestreams::u32u16streambuf::int_type
alf::unicodestreams::u32u16streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u32u16streambuf::int_type
alf::unicodestreams::u32u16streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u32u16streambuf::streambuf *
alf::unicodestreams::u32u16streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u32u16streambuf::int_type
alf::unicodestreams::u32u16streambuf::get()
{
  if (status_ != status_type::OK)
    return traits_type::eof();
  int_type c = get_u16(is_);
  if (c < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() == ibufe) {
    traits_type::move(ibufb, ibufe - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  this->setg(this->egptr(), this->gptr(), this->egptr() + 1);
  return traits_type::to_int_type(*gptr() = c);
}

alf::unicodestreams::u32u16streambuf::int_type
alf::unicodestreams::u32u16streambuf::put(int_type c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();
  if (os_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *os_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof()))
    return traits_type::not_eof(c);
  if (! is_valid_utf32(c))
    return err_status(status_type::NOT_UNICODE);
  int_type k = put_u16(os_, c);
  if (k < 0)
    return err_status((status_type)-k);
  return c;
}

////////////////////////////////
// u32utf8

// for reading.
alf::unicodestreams::u32utf8streambuf::u32utf8streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u32utf8streambuf::u32utf8streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for both.
alf::unicodestreams::u32utf8streambuf::
u32utf8streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

alf::unicodestreams::u32utf8streambuf::~u32utf8streambuf()
{
  if (os_)
    overflow(traits_type::eof());
}

// virtual
alf::unicodestreams::u32utf8streambuf::int_type
alf::unicodestreams::u32utf8streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u32utf8streambuf::int_type
alf::unicodestreams::u32utf8streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u32utf8streambuf::streambuf *
alf::unicodestreams::u32utf8streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u32utf8streambuf::int_type
alf::unicodestreams::u32utf8streambuf::get()
{
  if (status_ != status_type::OK)
    return traits_type::eof();
  int_type c = get_utf8(is_);
  if (c < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() == ibufe) {
    traits_type::move(ibufb, ibufe - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  this->setg(this->egptr(), this->gptr(), this->egptr() + 1);
  return traits_type::to_int_type(*gptr() = c);
}

alf::unicodestreams::u32utf8streambuf::int_type
alf::unicodestreams::u32utf8streambuf::put(int_type c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();
  if (traits_type::eq_int_type(c, traits_type::eof()))
    return traits_type::not_eof(c);
  int_type k = put_utf8(os_, c);
  if (k < 0)
    return err_status((status_type)-c);
  return c;
}

////////////////////////////////
// u16u32

// for reading.
alf::unicodestreams::u16u32streambuf::u16u32streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u16u32streambuf::u16u32streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
// for both.
alf::unicodestreams::u16u32streambuf::
u16u32streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::u16u32streambuf::int_type
alf::unicodestreams::u16u32streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u16u32streambuf::int_type
alf::unicodestreams::u16u32streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u16u32streambuf::streambuf *
alf::unicodestreams::u16u32streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u16u32streambuf::int_type
alf::unicodestreams::u16u32streambuf::get()
{
  int_type c, a;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if ((c = get_u32(is_)) < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() + 1 == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  if (c < 0x10000)
    return traits_type::to_int_type(*gptr() = c);

  c -= 0x10000;
  *gptr() = a = 0xd800 | (c >> 10);
  gptr()[1] = 0xdc00 | (c & 0x3ff);
  this->setg(this->eback(), this->gptr(), this->gptr() + 2);
  return traits_type::to_int_type(*gptr());

}

alf::unicodestreams::u16u32streambuf::int_type
alf::unicodestreams::u16u32streambuf::put(int_type c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof())) {
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    return traits_type::not_eof(c);
  }

  if (c < 0 || c >= 0x10000)
    return err_status(status_type::NOT_UTF16);

  int_type b;
  if (c < 0xd800 || c >= 0xe000) {
    if (! is_valid_utf32(char32_t(c)))
      return err_status(status_type::NOT_UNICODE);
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    b = put_u32(os_, c);
    if (b < 0)
      return err_status((status_type)-b);
    return c;
  }
  // c in 0xd800..0xdfff
  if (c < 0xdc00) {
    // c in 0xd800..0xdbff
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    pbuf = traits_type::to_char_type(c);
    return c;
  }
  // c in 0xdc00..dfff
  if (pbuf == 0)
    return err_status(status_type::NO_LEAD);
  int_type w = (((pbuf & 0x3ff) << 10) | (c & 0x3ff)) + 0x10000;
  pbuf = 0;
  b = put_u32(os_, w);
  if (b < 0)
    return err_status((status_type)-b);
  return c;
}

////////////////////////////////
// u16

// for reading.
alf::unicodestreams::u16streambuf::u16streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u16streambuf::u16streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
// for both.
alf::unicodestreams::u16streambuf::
u16streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::u16streambuf::int_type
alf::unicodestreams::u16streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u16streambuf::int_type
alf::unicodestreams::u16streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u16streambuf::streambuf *
alf::unicodestreams::u16streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u16streambuf::int_type
alf::unicodestreams::u16streambuf::get()
{
  int_type c, a;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if ((c = get_u16(is_)) < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() + 1 == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  if (c < 0x10000)
    return traits_type::to_int_type(*gptr() = c);

  c -= 0x10000;
  *gptr() = a = 0xd800 | (c >> 10);
  gptr()[1] = 0xdc00 | (c & 0x3ff);
  this->setg(this->eback(), this->gptr(), this->gptr() + 2);
  return traits_type::to_int_type(*gptr());

}

alf::unicodestreams::u16streambuf::int_type
alf::unicodestreams::u16streambuf::put(int_type c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof())) {
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    return traits_type::not_eof(c);
  }

  if (c < 0 || c >= 0x10000)
    return err_status(status_type::NOT_UTF16);

  int_type b;
  if (c < 0xd800 || c >= 0xe000) {
    if (! is_valid_utf32(char32_t(c)))
      return err_status(status_type::NOT_UNICODE);
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    b = put_u16(os_, c);
    if (b < 0)
      return err_status((status_type)-b);
    return c;
  }
  // c in 0xd800..0xdfff
  if (c < 0xdc00) {
    // c in 0xd800..0xdbff
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    pbuf = traits_type::to_char_type(c);
    return c;
  }
  // c in 0xdc00..dfff
  if (pbuf == 0)
    return err_status(status_type::NO_LEAD);
  int_type w = (((pbuf & 0x3ff) << 10) | (c & 0x3ff)) + 0x10000;
  pbuf = 0;
  b = put_u16(os_, w);
  if (b < 0)
    return err_status((status_type)-b);
  return c;
}

////////////////////////////////
// u16utf8

// for reading.
alf::unicodestreams::u16utf8streambuf::u16utf8streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u16utf8streambuf::u16utf8streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
// for both.
alf::unicodestreams::u16utf8streambuf::
u16utf8streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  pbuf = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::u16utf8streambuf::int_type
alf::unicodestreams::u16utf8streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u16utf8streambuf::int_type
alf::unicodestreams::u16utf8streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u16utf8streambuf::streambuf *
alf::unicodestreams::u16utf8streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u16utf8streambuf::int_type
alf::unicodestreams::u16utf8streambuf::get()
{
  int_type c, a;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if ((c = get_utf8(is_)) < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() + 1 == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  if (c < 0x10000)
    return traits_type::to_int_type(*gptr() = c);

  c -= 0x10000;
  *gptr() = a = 0xd800 | (c >> 10);
  gptr()[1] = 0xdc00 | (c & 0x3ff);
  this->setg(this->eback(), this->gptr(), this->gptr() + 2);
  return traits_type::to_int_type(*gptr());

}

alf::unicodestreams::u16utf8streambuf::int_type
alf::unicodestreams::u16utf8streambuf::put(int_type c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();
  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_ )
    return err_status(status_type::BAD_STREAM);
  int_type a, b;
  char16_t ch;

  if (traits_type::eq_int_type(c, traits_type::eof())) {
    // eof, shut down.. .make sure buf has nothing.
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    return traits_type::not_eof(c);
  }
  if (c >= 0xfffe) {
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    return err_status(status_type::NOT_UNICODE);
  }
  if (c < 0xd800 || c >= 0xe000) {
    // single code.
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    ch = traits_type::to_char_type(c);
    if (! is_valid_utf32(ch))
      return err_status(status_type::NOT_UNICODE);
    a = put_utf8(os_, c);
    if (a < 0)
      return err_status((status_type)-a);
    return c;
  }
  if (c < 0xdc00) {
    if (pbuf)
      return err_status(status_type::NO_FOLLOW);
    pbuf = c;
    return c;
  }
  // c in 0xdc00..0xdfff range.
  b = pbuf;
  pbuf = 0;
  if (b < 0xd800 || b >= 0xdc00)
    return err_status(status_type::NO_LEAD);
  a = (((b & 0x3ff) << 10) | (c & 0x3ff)) + 0x10000;
  if (! is_valid_utf32(traits_type::to_char_type(a)))
      return err_status(status_type::NOT_UNICODE);
  int_type k = put_utf8(os_, a);
  if (k < 0)
    return err_status((status_type)-k);
  return c;
}

////////////////////////////////
// utf8u32

// for reading.
alf::unicodestreams::utf8u32streambuf::utf8u32streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::utf8u32streambuf::utf8u32streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
// for both.
alf::unicodestreams::utf8u32streambuf::
utf8u32streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::utf8u32streambuf::int_type
alf::unicodestreams::utf8u32streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::utf8u32streambuf::int_type
alf::unicodestreams::utf8u32streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::utf8u32streambuf::streambuf *
alf::unicodestreams::utf8u32streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::utf8u32streambuf::int_type
alf::unicodestreams::utf8u32streambuf::get()
{
  int_type c, a;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if ((c = get_u32(is_)) < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() + 3 == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }

  char_type * base = this->gptr();
  char_type * p = base;
  int_type ret = 0;
  if (c < 0x80) {
    *p++ = char_type(ret = c);
  } else if (c < 0x800) {
    *p++ = char_type(ret = 0xc0 | (c >> 6));
    *p++ = char_type(0x80 | (c & 0x3f));
  } else if (c < 0x10000) {
    *p++ = char_type(ret = 0xe0 | (c >> 12));
    *p++ = char_type(0x80 | ((c >> 6) & 0x3f));
    *p++ = char_type(0x80 | (c & 0x3f));
  } else {
    *p++ = char_type(ret = 0xf0 | (c >> 18));
    *p++ = char_type(0x80 | ((c >> 12) & 0x3f));
    *p++ = char_type(0x80 | ((c >> 6) & 0x3f));
    *p++ = char_type(0x80 | (c & 0x3f));
  }
  this->setg(ibufb, base, p);
  return traits_type::to_int_type(*this->gptr());

}

alf::unicodestreams::utf8u32streambuf::int_type
alf::unicodestreams::utf8u32streambuf::put(int_type c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof())) {
    if (och_need)
      return err_status(status_type::NO_FOLLOW);
    return traits_type::not_eof(c);
  }

  if (c < 0 || c > 0xf7)
    return err_status(status_type::NOT_UTF8);

  if (c < 0x80) {
    if (och_need)
      return err_status(status_type::NO_FOLLOW);
    return c;
  }
  int_type b;
  if (c < 0xc0) {
    if (och_need == 0)
      return err_status(status_type::NO_LEAD);
    och = (och << 6) | (c & 0x3f);
    if (--och_need == 0) {
      if (! is_valid_utf32(char32_t(och)))
	return err_status(status_type::NOT_UNICODE);
      if (och < och_min)
	return err_status(status_type::NOT_UTF8);
      b = put_u32(os_, och);
      if (b < 0)
	return err_status((status_type)-b);
      return c;
    }
  }
  if (och_need)
    return err_status(status_type::NO_FOLLOW);
  if (c < 0xe0) {
    // two byte code - minimum 0x80.
    och_min = 0x80;
    och_need = 1;
    och = c & 0x1f;
    return c;
  }
  if (c < 0xf0) {
    // three byte code - minimum 0x800
    och_min = 0x800;
    och_need = 2;
    och = c & 0x0f;
    return c;
  }
  och_min = 0x10000;
  och_need = 3;
  och = c & 0x07;
  return c;

}

////////////////////////////////
// utf8u16

// for reading.
alf::unicodestreams::utf8u16streambuf::utf8u16streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::utf8u16streambuf::utf8u16streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
// for both.
alf::unicodestreams::utf8u16streambuf::
utf8u16streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::utf8u16streambuf::int_type
alf::unicodestreams::utf8u16streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::utf8u16streambuf::int_type
alf::unicodestreams::utf8u16streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::utf8u16streambuf::streambuf *
alf::unicodestreams::utf8u16streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::utf8u16streambuf::int_type
alf::unicodestreams::utf8u16streambuf::get()
{
  int_type c, a;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if ((c = get_u16(is_)) < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() + 3 == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }

  char_type * base = this->gptr();
  char_type * p = base;
  int_type ret = 0;
  if (c < 0x80) {
    *p++ = char_type(ret = c);
  } else if (c < 0x800) {
    *p++ = char_type(ret = 0xc0 | (c >> 6));
    *p++ = char_type(0x80 | (c & 0x3f));
  } else if (c < 0x10000) {
    *p++ = char_type(ret = 0xe0 | (c >> 12));
    *p++ = char_type(0x80 | ((c >> 6) & 0x3f));
    *p++ = char_type(0x80 | (c & 0x3f));
  } else {
    *p++ = char_type(ret = 0xf0 | (c >> 18));
    *p++ = char_type(0x80 | ((c >> 12) & 0x3f));
    *p++ = char_type(0x80 | ((c >> 6) & 0x3f));
    *p++ = char_type(0x80 | (c & 0x3f));
  }
  this->setg(ibufb, base, p);
  return traits_type::to_int_type(*this->gptr());

}

alf::unicodestreams::utf8u16streambuf::int_type
alf::unicodestreams::utf8u16streambuf::put(int_type c)
{

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof())) {
    if (och_need)
      return err_status(status_type::NO_FOLLOW);
    return traits_type::not_eof(c);
  }

  if (c < 0 || c > 0xf7)
    return err_status(status_type::NOT_UTF8);

  if (c < 0x80) {
    if (och_need)
      return err_status(status_type::NO_FOLLOW);
    return c;
  }
  int_type b;
  if (c < 0xc0) {
    if (och_need == 0)
      return err_status(status_type::NO_LEAD);
    och = (och << 6) | (c & 0x3f);
    if (--och_need == 0) {
      if (! is_valid_utf32(char32_t(och)))
	return err_status(status_type::NOT_UNICODE);
      if (och < och_min)
	return err_status(status_type::NOT_UTF8);
      b = put_u16(os_, och);
      if (b < 0)
	return err_status((status_type)-b);
      return c;
    }
  }
  if (och_need)
    return err_status(status_type::NO_FOLLOW);
  if (c < 0xe0) {
    // two byte code - minimum 0x80.
    och_min = 0x80;
    och_need = 1;
    och = c & 0x1f;
    return c;
  }
  if (c < 0xf0) {
    // three byte code - minimum 0x800
    och_min = 0x800;
    och_need = 2;
    och = c & 0x0f;
    return c;
  }
  och_min = 0x10000;
  och_need = 3;
  och = c & 0x07;
  return c;

}

////////////////////////////////
// utf8

// for reading.
alf::unicodestreams::utf8streambuf::utf8streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::utf8streambuf::utf8streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
// for both.
alf::unicodestreams::utf8streambuf::
utf8streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och_need = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::utf8streambuf::int_type
alf::unicodestreams::utf8streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::utf8streambuf::int_type
alf::unicodestreams::utf8streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::utf8streambuf::streambuf *
alf::unicodestreams::utf8streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::utf8streambuf::int_type
alf::unicodestreams::utf8streambuf::get()
{
  int_type c, a;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if ((c = get_utf8(is_)) < 0) {
    if (c == -(int_type)status_type::EOF_STREAM)
      return traits_type::eof();
    return err_status((status_type)-c);
  }
  if (this->egptr() + 3 == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }

  char_type * base = this->gptr();
  char_type * p = base;
  int_type ret = 0;
  if (c < 0x80) {
    *p++ = char_type(ret = c);
  } else if (c < 0x800) {
    *p++ = char_type(ret = 0xc0 | (c >> 6));
    *p++ = char_type(0x80 | (c & 0x3f));
  } else if (c < 0x10000) {
    *p++ = char_type(ret = 0xe0 | (c >> 12));
    *p++ = char_type(0x80 | ((c >> 6) & 0x3f));
    *p++ = char_type(0x80 | (c & 0x3f));
  } else {
    *p++ = char_type(ret = 0xf0 | (c >> 18));
    *p++ = char_type(0x80 | ((c >> 12) & 0x3f));
    *p++ = char_type(0x80 | ((c >> 6) & 0x3f));
    *p++ = char_type(0x80 | (c & 0x3f));
  }
  this->setg(ibufb, base, p);
  return traits_type::to_int_type(*this->gptr());

}

alf::unicodestreams::utf8streambuf::int_type
alf::unicodestreams::utf8streambuf::put(int_type c)
{

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof())) {
    if (och_need)
      return err_status(status_type::NO_FOLLOW);
    return traits_type::not_eof(c);
  }

  if (c < 0 || c > 0xf7)
    return err_status(status_type::NOT_UTF8);

  if (c < 0x80) {
    if (och_need)
      return err_status(status_type::NO_FOLLOW);
    return c;
  }
  int_type b;
  if (c < 0xc0) {
    if (och_need == 0)
      return err_status(status_type::NO_LEAD);
    och = (och << 6) | (c & 0x3f);
    if (--och_need == 0) {
      if (! is_valid_utf32(char32_t(och)))
	return err_status(status_type::NOT_UNICODE);
      if (och < och_min)
	return err_status(status_type::NOT_UTF8);
      b = put_utf8(os_, och);
      if (b < 0)
	return err_status((status_type)-b);
      return c;
    }
  }
  if (och_need)
    return err_status(status_type::NO_FOLLOW);
  if (c < 0xe0) {
    // two byte code - minimum 0x80.
    och_min = 0x80;
    och_need = 1;
    och = c & 0x1f;
    return c;
  }
  if (c < 0xf0) {
    // three byte code - minimum 0x800
    och_min = 0x800;
    och_need = 2;
    och = c & 0x0f;
    return c;
  }
  och_min = 0x10000;
  och_need = 3;
  och = c & 0x07;
  return c;

}

////////////////////////////////
// u32iso8859_1

// for reading.
alf::unicodestreams::u32iso8859_1_streambuf::
u32iso8859_1_streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u32iso8859_1_streambuf::
u32iso8859_1_streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
// for both.
alf::unicodestreams::u32iso8859_1_streambuf::
u32iso8859_1_streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::u32iso8859_1_streambuf::int_type
alf::unicodestreams::u32iso8859_1_streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u32iso8859_1_streambuf::int_type
alf::unicodestreams::u32iso8859_1_streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u32iso8859_1_streambuf::streambuf *
alf::unicodestreams::u32iso8859_1_streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u32iso8859_1_streambuf::int_type
alf::unicodestreams::u32iso8859_1_streambuf::get()
{
  char ch;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_)
    return err_status(status_type::BAD_STREAM);
  if (! is_->get(ch)) {
    if (is_->rdstate() & std::ios_base::eofbit)
      return traits_type::eof();
    return err_status(status_type::BAD_STREAM);
  }

  if (this->egptr() == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }

  return traits_type::to_int_type(*this->gptr() = ch);

}

alf::unicodestreams::u32iso8859_1_streambuf::int_type
alf::unicodestreams::u32iso8859_1_streambuf::put(int_type c)
{
  if (status_ != status_type::OK)
    return traits_type::eof();

  if (os_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *os_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof()))
    return traits_type::not_eof(c);
  if (c < 0 || c > 0xff)
    return err_status(status_type::NOT_ISO_8859_1);
  if (! os_->put(char(c)))
    return err_status(status_type::BAD_STREAM);
  return c;

}

////////////////////////////////
// u16iso8859_1_

// for reading.
alf::unicodestreams::u16iso8859_1_streambuf::
u16iso8859_1_streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u16iso8859_1_streambuf::
u16iso8859_1_streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
 // for both.
alf::unicodestreams::u16iso8859_1_streambuf::
u16iso8859_1_streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::u16iso8859_1_streambuf::int_type
alf::unicodestreams::u16iso8859_1_streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u16iso8859_1_streambuf::int_type
alf::unicodestreams::u16iso8859_1_streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u16iso8859_1_streambuf::streambuf *
alf::unicodestreams::u16iso8859_1_streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u16iso8859_1_streambuf::int_type
alf::unicodestreams::u16iso8859_1_streambuf::get()
{
  char ch;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_)
    return err_status(status_type::BAD_STREAM);
  if (! is_->get(ch)) {
    if (is_->rdstate() & std::ios_base::eofbit)
      return traits_type::eof();
    return err_status(status_type::BAD_STREAM);
  }

  if (this->egptr() == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }

  return traits_type::to_int_type(*this->gptr() = ch);

}

alf::unicodestreams::u16iso8859_1_streambuf::int_type
alf::unicodestreams::u16iso8859_1_streambuf::put(int_type c)
{

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (os_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *os_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof()))
    return traits_type::not_eof(c);
  if (c < 0 || c > 0xff)
    return err_status(status_type::NOT_ISO_8859_1);
  if (! os_->put(char16_t(c)))
    return err_status(status_type::BAD_STREAM);
  return c;

}

////////////////////////////////
// utf8iso8859_1_

// for reading.
alf::unicodestreams::utf8iso8859_1_streambuf::
utf8iso8859_1_streambuf(src_stream & is)
  : is_(& is), os_(0), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::utf8iso8859_1_streambuf::
utf8iso8859_1_streambuf(dst_stream & os)
  : is_(0), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
 // for both.
alf::unicodestreams::utf8iso8859_1_streambuf::
utf8iso8859_1_streambuf(src_stream & is, dst_stream & os)
  : is_(& is), os_(& os), status_(status_type())
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  och = 0;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::utf8iso8859_1_streambuf::int_type
alf::unicodestreams::utf8iso8859_1_streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::utf8iso8859_1_streambuf::int_type
alf::unicodestreams::utf8iso8859_1_streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::utf8iso8859_1_streambuf::streambuf *
alf::unicodestreams::utf8iso8859_1_streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::utf8iso8859_1_streambuf::int_type
alf::unicodestreams::utf8iso8859_1_streambuf::get()
{
  char ch;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (is_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *is_)
    return err_status(status_type::BAD_STREAM);
  if (! is_->get(ch)) {
    if (is_->rdstate() & std::ios_base::eofbit)
      return traits_type::eof();
    return err_status(status_type::BAD_STREAM);
  }

  if (this->egptr() + 1 == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }

  char_type * base = this->gptr();
  char_type * p = base;
  if (ch < 0x80) {
    *p++ = char_type(ch);
  } else {
    *p++ = char_type(0xc0 | ((ch >> 6) & 3));
    *p++ = char_type(0x80 | (ch & 0x3f));
  }
  this->setg(ibufb, base, p);
  return traits_type::to_int_type(*this->gptr());

}

alf::unicodestreams::utf8iso8859_1_streambuf::int_type
alf::unicodestreams::utf8iso8859_1_streambuf::put(int_type c)
{

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (os_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *os_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof())) {
    if (och)
      return err_status(status_type::NO_FOLLOW);
    return traits_type::not_eof(c);
  }

  if (c < 0 || c > 0xf7)
    return err_status(status_type::NOT_UTF8);

  if (c < 0x80) {
    if (och)
      return err_status(status_type::NO_FOLLOW);
    return c;
  }
  int_type b;
  if (c < 0xc0) {
    if (och == 0)
      return err_status(status_type::NO_LEAD);

    b = ((och & 0x1f) << 6) | (c & 0x3f);
    och = 0;

    if (b < 0x80)
      return err_status(status_type::NOT_UTF8);
    if (b > 0xff)
      return err_status(status_type::NOT_ISO_8859_1);
    if (! os_->put(char(b)))
      return err_status(status_type::BAD_STREAM);
    return c;
  }
  if (c >= 0xc4)
    return err_status(status_type::NOT_ISO_8859_1);
  if (och)
    return err_status(status_type::NO_FOLLOW);
  return och = c;

}

////////////////////////////////
// u32bswap_

// for reading.
alf::unicodestreams::u32bswap_streambuf::
u32bswap_streambuf(src_stream & is,
		   swap_state_type s /* = swap_state_type::None */)
  : is_(& is), os_(0), status_(status_type()), swap_state_(s)
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u32bswap_streambuf::
u32bswap_streambuf(dst_stream & os,
		   swap_state_type s /* = swap_state_type::None */)
  : is_(0), os_(& os), status_(status_type()), swap_state_(s)
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
 // for both.
alf::unicodestreams::u32bswap_streambuf::
u32bswap_streambuf(src_stream & is, dst_stream & os,
		   swap_state_type s /* = swap_state_type::None */)
  : is_(& is), os_(& os), status_(status_type()), swap_state_(s)
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::u32bswap_streambuf::int_type
alf::unicodestreams::u32bswap_streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u32bswap_streambuf::int_type
alf::unicodestreams::u32bswap_streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u32bswap_streambuf::streambuf *
alf::unicodestreams::u32bswap_streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u32bswap_streambuf::int_type
alf::unicodestreams::u32bswap_streambuf::get()
{
  char32_t ch;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (! is_->get(ch)) {
    if (is_->rdstate() & std::ios_base::eofbit)
      return traits_type::eof();
    return err_status(status_type::BAD_STREAM);
  }
  int_type c = (int_type)ch;
  swap_state_type s;
  while (true) {
    switch (swap_state_) {
    case swap_state_type::None:
    case swap_state_type::v1234: // no swap
      break;
    case swap_state_type::FChar:
      // This is first char, check it for fffe in swapped state:
      switch (c) {
      case 0xfffe0000:
	swap_state_ = swap_state_type::v4321;
	continue;
      case 0x0000fffe:
	swap_state_ = swap_state_type::v2143;
	continue;
      case 0xfeff0000:
	swap_state_ = swap_state_type::v3412;
	continue;
      case 0x0000feff:
	swap_state_ = swap_state_type::v1234;
	continue;
      default:
	return err_status(status_type::NO_BOM);
      }
    case swap_state_type::LE:
      // no swap if we're on a LE machine, swap if BE machine.
    case swap_state_type::BE:
      // swap if we're on a LE machine, no swap if on BE.
      if ((s = u32_check(swap_state_)) == swap_state_type::None)
	throw "Something is very wrong E105";
      swap_state_ = s;
      continue;
    case swap_state_type::v2143:
      c = ((c & 0xff00ff) << 8) | ((c >> 8) & 0xff00ff);
      break;
    case swap_state_type::v3412:
      c = ((c & 0xffff) << 16) | (c >> 16);
      break;
    case swap_state_type::v4321:
      c = (c << 24) | ((c & 0xff00) << 8) | ((c >> 8) & 0xff00) | (c >> 24);
      break;
    }
    break;
  }
  if (this->egptr() == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  return traits_type::to_int_type(*this->gptr() = c);

}

alf::unicodestreams::u32bswap_streambuf::int_type
alf::unicodestreams::u32bswap_streambuf::put(int_type c)
{

  if (status_ != status_type::OK)
    return traits_type::eof();

  int_type a = c;
  swap_state_type s;

  if (os_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *os_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof()))
    return traits_type::not_eof(c);

  while (true) {
    switch (swap_state_) {
    case swap_state_type::None:
    case swap_state_type::v1234:
      break;
    case swap_state_type::FChar:
      // no particular swap state, but if c is not 0xfeff
      // we write out 0xfeff (BOM) before writing c.
      if (c != 0xfeff && ! os_->put(char32_t(0xfeff)))
	return err_status(status_type::BAD_STREAM);
      swap_state_ = swap_state_type::v1234;
      break;
    case swap_state_type::LE:
    case swap_state_type::BE:
      // swap if we're on a LE machine, no swap if on BE.
      if ((s = u32_check(swap_state_)) == swap_state_type::None)
	throw "Something is very wrong E106";
      swap_state_ = s;
      continue;
    case swap_state_type::v2143:
      a = ((a & 0xff00ff) << 8) | ((a >> 8) & 0xff00ff);
      break;
    case swap_state_type::v3412:
      a = ((a & 0xffff) << 16) | (a >> 16);
      break;
    case swap_state_type::v4321:
      a = (a << 24) | ((a & 0xff00) << 8) | ((a >> 8) & 0xff00) | (a >> 24);
      break;
    }
    break;
  }
  if (! os_->put(char32_t(a)))
    return err_status(status_type::BAD_STREAM);
  return c;
}

////////////////////////////////
// u16bswap_

// for reading.
alf::unicodestreams::u16bswap_streambuf::
u16bswap_streambuf(src_stream & is,
		   swap_state_type s /* = swap_state_type::None */)
  : is_(& is), os_(0), status_(status_type()), swap_state_(s)
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// for writing
alf::unicodestreams::u16bswap_streambuf::
u16bswap_streambuf(dst_stream & os,
		   swap_state_type s /* = swap_state_type::None */)
  : is_(0), os_(& os), status_(status_type()), swap_state_(s)
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}
 
 // for both.
alf::unicodestreams::u16bswap_streambuf::
u16bswap_streambuf(src_stream & is, dst_stream & os,
		   swap_state_type s /* = swap_state_type::None */)
  : is_(& is), os_(& os), status_(status_type()), swap_state_(s)
{
  ibufb = ibuf;
  ibufe = ibuf + IBUFSZ;
  this->setg(ibuf, ibuf, ibuf);
  this->setp(0, 0);
}

// virtual
alf::unicodestreams::u16bswap_streambuf::int_type
alf::unicodestreams::u16bswap_streambuf::underflow()
{
  return get();
}

// virtual
alf::unicodestreams::u16bswap_streambuf::int_type
alf::unicodestreams::u16bswap_streambuf::overflow(int_type __c)
{
  return put(__c);
}

// virtual
alf::unicodestreams::u16bswap_streambuf::streambuf *
alf::unicodestreams::u16bswap_streambuf::
setbuf(char_type * __s, std::streamsize __n)
{

  std::streamsize n = this->egptr() - this->eback();
  std::streamsize q = this->gptr() - this->eback();
  std::streamsize m = n - q; // still need to read.
  std::streamsize u = q + m; // chars to keep over to new buffer.
  if (__n > EBACK + 4 && __n >= m) {
    if (q + m > 0) {
      if (n > __n) n = __n;
      if (q + m > n) q = n - m;
      traits_type::move(__s, this->gptr() - q, u = q + m);
    }
    ibufb = __s;
    ibufe = __s + __n;
    this->setg(__s, __s + q, __s + u);
  }
  return this;

}

alf::unicodestreams::u16bswap_streambuf::int_type
alf::unicodestreams::u16bswap_streambuf::get()
{
  char16_t ch;

  if (status_ != status_type::OK)
    return traits_type::eof();

  if (! is_->get(ch)) {
    if (is_->rdstate() & std::ios_base::eofbit)
      return traits_type::eof();
    return err_status(status_type::BAD_STREAM);
  }
  int_type c = (int_type)ch;
  swap_state_type s;
  while (true) {
    switch (swap_state_) {
    case swap_state_type::None:
    case swap_state_type::v12: // no swap
      break;
    case swap_state_type::FChar:
      // This is first char, check it for fffe in swapped state:
      switch (c) {
      case 0xfffe:
	swap_state_ = swap_state_type::v21;
	continue;
      case 0xfeff:
	swap_state_ = swap_state_type::v12;
	continue;
      default:
	return err_status(status_type::NO_BOM);
      }
    case swap_state_type::LE:
      // no swap if we're on a LE machine, swap if BE machine.
    case swap_state_type::BE:
      // swap if we're on a LE machine, no swap if on BE.
      if ((s = u16_check(swap_state_)) == swap_state_type::None)
	throw "Something is very wrong E115";
      swap_state_ = s;
      continue;
    case swap_state_type::v21:
      c = ((c << 8) | (c >> 8)) & 0xffff;
      break;
    }
    break;
  }
  if (this->egptr() == ibufe) {
    traits_type::move(ibufb,  this->gptr() - EBACK, EBACK);
    this->setg(ibufb, ibufb + EBACK, ibufb + EBACK);
  }
  return traits_type::to_int_type(*this->gptr() = c);

}

alf::unicodestreams::u16bswap_streambuf::int_type
alf::unicodestreams::u16bswap_streambuf::put(int_type c)
{

  if (status_ != status_type::OK)
    return traits_type::eof();

  int_type a = c;
  swap_state_type s;

  if (os_ == 0)
    return err_status(status_type::NO_STREAM);
  if (! *os_ )
    return err_status(status_type::BAD_STREAM);
  if (traits_type::eq_int_type(c, traits_type::eof()))
    return traits_type::not_eof(c);

  while (true) {
    switch (swap_state_) {
    case swap_state_type::None:
    case swap_state_type::v12:
      break;
    case swap_state_type::FChar:
      // no particular swap state, but if c is not 0xfeff
      // we write out 0xfeff (BOM) before writing c.
      if (c != 0xfeff && ! os_->put(char16_t(0xfeff)))
	return err_status(status_type::BAD_STREAM);
      swap_state_ = swap_state_type::v12;
      break;
    case swap_state_type::LE:
    case swap_state_type::BE:
      // swap if we're on a LE machine, no swap if on BE.
      if ((s = u16_check(swap_state_)) == swap_state_type::None)
	throw "Something is very wrong E116";
      swap_state_ = s;
      continue;
    case swap_state_type::v21:
      a = ((a << 8) | (a >> 8)) & 0xffff;
      break;
    }
    break;
  }
  if (! os_->put(char16_t(a)))
    return err_status(status_type::BAD_STREAM);
  return c;
}
