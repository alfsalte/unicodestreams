#ifndef __ALF_UNICODESTREAMS_HXX__
#define __ALF_UNICODESTREAMS_HXX__

// This provide the following stream classes and the corresponding
// streambuf classes:
//
// some are prefixed by xxxyyy where xxx is one of u32, u16, utf8
// and yyy are one of u32, u16, utf8, iso8859_1_
// This means that you attach a stream that reads the yyy codes
// for utf8 and iso8859_1_ that would be char and it would be a
// std::istream/std::ostream for it. For u32 it would be char32_t
// and for u16 it would be char16_t.
// You read the xxx part so your stream inherit from the appropriate
// stream for xxx.
//
// Some are simply prefixed by xxx - these are attached to a reader
// of the same kind but simply act like a filter that ensures that
// it is valid codes that are read/written.
//
// xxxyyystreambuf, xxxstreambuf - the streambuf to use.
// You normally never use that directly.
//
// xxxyyy{i,o,io}stream - the proper stream.
//
// Example: Read utf8 from a file as UTF-32 (unicode).
//
// fstream F("My file");
// // reading directly from F would give you UTF-8 bytes.
// u32utf8istream G(F); // reading from G give you char32_t UTF-32 chars.
//
// Do not normally mix reading from F and G - only read from G. If you have a
// file that contain mixed UTF-8 and other encoding, read from F until you get
// UTF-8, make sure you are in a situation where the next byte to read from F
// is the first UTF-8 byte. Then read from G and make sure you stop when you
// read the last UTF-8 text. Then continue to read from F again and so on.
// 
// u32utf8streambuf  -- used by u32utf8{i,o,io}stream
// u32utf8istream    -- read a char32_t stream, source is an utf8 char stream.
// u32utf8ostream    -- write a char32_t stream - output is utf8 char stream.
// u32utfiostream
// u32u16streambuf
// u32u16istream
// u32u16ostream
// u32u16iostream
// u16utf8streambuf
// u16utf8istream -- read a char16_t stream - source is utf8 char stream.
// u16utf8ostream -- write a char16_t stream, output is utf8 char stream.
// u16utf8iostream
// utf8u32istream -- read an utf8 char stream, source is char32_t stream.
// utf8u32ostream/buf -- write a utf8 char stream, output is char32_t stream.
// utf8u16istream/buf -- read an utf8 char stream, source is char16_t utf16.
// utf8u16ostream/buf -- write an utf8 char stream, output to utf16 char16_t.
// u16u32istream/buf -- read a char16_t stream, source is char32_t stream.
// u16u32ostream/buf -- write a char16_t stream, output is char32_t stream.
// u32u16istream/buf -- read a char32_t stream, source is char16_t stream.
// u32u16ostream/buf -- write a char32_t stream, output is char16_t stream.
// u32iso8859_1_istream/buf -- read char32_t from iso-8859-1 stream (char)
// u32iso8859_1_ostream/buf -- write char32_t to iso-8859-1 stream (char). 

// For the latter, if you write a char that doesn't exist in the iso-8859-1
// set it is ignored and the stream is set in a fail state.

// For the others if you write any char not in the unicode set it is
// ignored and the stream is set in a fail state.

// iostreams that allow you to both read and write are also possible
// and is bound to either a pair of istream/ostream or a single iostream.
// (see constructors for these).

namespace alf {

namespace unicodestreams {

class u32istream;
class u32ostream;
class u32iostream;
class u32utf8istream;
class u32utf8ostream;
class u32utf8iostream;
class u32u16istream;
class u32u16ostream;
class u32u16iostream;
class u16u32istream;
class u16u32ostream;
class u16u32iostream;
class u16utf8istream;
class u16utf8ostream;
class u16utf8iostream;
class utf8u32istream;
class utf8u32ostream;
class utf8u32iostream;
class utf8u16istream;
class utf8u16ostream;
class utf8u16iostream;
class u32iso8859_1_istream;
class u32iso8859_1_ostream;
class u32iso8859_1_iostream;

class u32streambuf;
class u32u16streambuf;
class u32utf8streambuf;
class u16u32streambuf;
class u16streambuf;
class u16utf8streambuf;
class utf8u32streambuf;
class utf8u16streambuf;
class utf8streambuf;
class u32iso8859_1_streambuf;
class u16iso8859_1_streambuf;
class utf8iso8859_1_streambuf;

//////////////////////////////
// status_type

enum class status_type : int {
  OK,
  EOF_STREAM,
  NO_STREAM, // we don't have any external stream.
  BAD_STREAM, // external stream has some error.
  BAD_INPUT, // external input is faulty
  NO_LEAD,   // a code require another code before it.
  NO_FOLLOW, // a code require another code after it.
  NOT_UNICODE, // This isn't unicode (UTF-32).
  NOT_UTF8, // This isn't UTF-8.
  NOT_UTF16, // This isn't UTF-16.
  NOT_ISO_8859_1, // This isn't ISO 8859-1.
  NO_BOM, // byte order mark is missing.
};
    

///////////////////////////////////
// u32_swap_state_type

// The meaning is for example that 3412 means that on input
// we get a value such as 0xuuvvwwzz we transform it
// to user as 0xwwzzuuvv. On writing we employ the opposite so
// that 0xwwzzuuvv becomes 0xuuvvwwzz as written to file.
// note that each of these schemes are their own inverse, so
// the same state works for both input and output.
// The state FChar can be set initially to let us determine
// the state, first char read is assumed to be 0xffef and
// we find the correct transformation and replaces Fchar with
// that found transformation.
// Note that None implies no transformation at all as does v1234.
// Note that on an LE machine reading BE code or BE machine reading
// LE text will both result in a v4321 state so the states here
// do not refer to LE or BE.
// LE and BE are to be used to read LE/BE code regardless of machine's
// native endianness. They both change to v1234 or v4321 according
// to if the machine itself is the same endianess as you ask for or not.
enum class u32_swap_state_type : unsigned short {
  None = 0,
  FChar = 1, // let first char determine (presumably 0xffef BOM).
  LE = 2, // read/write little endian
  BE = 3, // read/write big endian.
  v1234 = 1234, // no transformation.
  v2143 = 2143, // swapping the bytes within each 16 bit unit.
  v3412 = 3412, // swapping the 16 bit units but not the bytes.
  v4321 = 4321, // byte swap in the traditional sense.
};

// The meaning is for example that 21 means that on input
// we get a value such as 0xuuvv we transform it to user as 0xvvuu
// On writing we do the same so that 0xvvuu becomes written as 0xuuvv
// to file.
// note that each of these schemes are their own inverse, so
// the same state works for both input and output.
// The state FChar can be set initially to let us determine
// the state, first char read is assumed to be 0xffef and
// we find the correct transformation and replaces Fchar with
// that found transformation.
// Note that None implies no transformation at all as does v21
// Note that on an LE machine reading BE code or BE machine reading
// LE text will both result in a v4321 state so the states here
// do not refer to LE or BE.
// LE and BE are to be used to read LE/BE code regardless of machine's
// native endianness. They both change to v1234 or v4321 according
// to if the machine itself is the same endianess as you ask for or not.
enum class u16_swap_state_type : unsigned short {
  None = 0,
  FChar = 1, // let first char determine (presumably 0xffef BOM).
  LE = 2, // read/write little endian
  BE = 3, // read/write big endian.
  v12 = 12, // no transformation.
  v21 = 21, // swapping the bytes within each 16 bit unit.
};



///////////////////////////////////
// u32

// u32

// source/destination and what we read/write is both char32_t.
// check that it is valid unicode.
class u32streambuf : public std::basic_streambuf<char32_t> {

  typedef char32_t char_type;
  typedef char32_t ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef u32streambuf streambuf;

public:

  u32streambuf(src_stream & is); // for reading.
  u32streambuf(dst_stream & os); // for writing
  u32streambuf(src_stream & is, dst_stream & os); // for both.
  ~u32streambuf();

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  src_stream * src_stream_() { return is_; }
  dst_stream * dst_stream_() { return os_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

private:

  enum { EBACK = 16, IBUFSZ = 64 };

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  char_type ibuf[IBUFSZ];

}; // end of class u32streambuf

class u32istream : public std::basic_istream<char32_t> {

  typedef char32_t char_type;
  typedef char32_t src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef u32streambuf streambuf;

public:

  u32istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32istream

class u32ostream : public std::basic_ostream<char32_t> {

  typedef char32_t char_type;
  typedef char32_t dst_char_type;
  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u32streambuf streambuf;

public:

  u32ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32ostream

class u32iostream : public std::basic_iostream<char32_t> {

  typedef char32_t char_type;
  typedef char_type src_char_type;
  typedef char_type dst_char_type;
  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u32streambuf streambuf;

public:

  u32iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32iostream

// u32u16

// source is char16_t, read char32_t (underflow).
// destination is char16_t, write char32_t (overflow).
class u32u16streambuf : public std::basic_streambuf<char32_t> {

  typedef char32_t char_type;
  typedef char16_t ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef u32u16streambuf streambuf;

public:

  u32u16streambuf(src_stream & is); // for reading.
  u32u16streambuf(dst_stream & os); // for writing
  u32u16streambuf(src_stream & is, dst_stream & os); // for both.
  ~u32u16streambuf();

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  char_type ibuf[IBUFSZ];

}; // end of class u32u16streambuf

class u32u16istream : public std::basic_istream<char32_t> {

  typedef char32_t char_type;
  typedef char16_t src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef u32u16streambuf streambuf;

public:

  u32u16istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32u16istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32u16istream

class u32u16ostream : public std::basic_ostream<char32_t> {

  typedef char32_t char_type;
  typedef char16_t dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u32u16streambuf streambuf;

public:

  u32u16ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32u16ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32u16ostream

class u32u16iostream : public std::basic_iostream<char32_t> {

  typedef char32_t char_type;
  typedef char16_t src_char_type;
  typedef char16_t dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u32u16streambuf streambuf;

public:

  u32u16iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32u16iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32u16iostream

////////////////////////////
// u32utf8

// source is char, read char32_t (underflow).
// destination is char, write char32_t (overflow).
class u32utf8streambuf : public std::basic_streambuf<char32_t> {

  typedef char32_t char_type;
  typedef char ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::istream src_stream;
  typedef std::ostream dst_stream;
  typedef u32utf8streambuf streambuf;

public:

  u32utf8streambuf(src_stream & is); // for reading.
  u32utf8streambuf(dst_stream & os); // for writing
  u32utf8streambuf(src_stream & is, dst_stream & os); // for both.
  ~u32utf8streambuf();

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  char_type ibuf[IBUFSZ];

}; // end of class u32utf8streambuf

class u32utf8istream : public std::basic_istream<char32_t> {

  typedef char32_t char_type;
  typedef char src_char_type;
  typedef std::basic_istream<char32_t> base_type;
  typedef std::istream src_stream;
  typedef u32utf8streambuf streambuf;

public:

  u32utf8istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32utf8istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32utf8istream

class u32utf8ostream : public std::basic_ostream<char32_t> {

  typedef char32_t char_type;
  typedef char dst_char_type;
  typedef std::basic_ostream<char32_t> base_type;
  typedef std::ostream dst_stream;
  typedef u32utf8streambuf streambuf;

public:

  u32utf8ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32utf8ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32utf8ostream

class u32utf8iostream : public std::basic_iostream<char32_t> {

  typedef std::basic_iostream<char32_t> base_type;
  typedef std::istream src_stream;
  typedef std::ostream dst_stream;
  typedef u32utf8streambuf streambuf;

public:

  u32utf8iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32utf8iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32utf8iostream

//////////////////////////////////////
// u16u32

// source is char32_t, read char16_t (underflow).
// destination is char32_t, write char16_t (overflow).
class u16u32streambuf : public std::basic_streambuf<char16_t> {

  typedef char16_t char_type;
  typedef char32_t ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef u16u32streambuf streambuf;

public:

  u16u32streambuf(src_stream & is); // for reading.
  u16u32streambuf(dst_stream & os); // for writing
  u16u32streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };
  //enum { EBACK = 28, BUFSZ = 2 }; // 30 char buffer + pbuf for us.

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  int_type pbuf;
  char_type ibuf[IBUFSZ];

}; // end of class u16u32streambuf

class u16u32istream : public std::basic_istream<char16_t> {

  typedef char16_t char_type;
  typedef char32_t src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef u16u32streambuf streambuf;

public:

  u16u32istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16u32istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16u32istream

class u16u32ostream : public std::basic_ostream<char16_t> {

  typedef char16_t char_type;
  typedef char32_t dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16u32streambuf streambuf;

public:

  u16u32ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16u32ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16u32ostream

class u16u32iostream : public std::basic_iostream<char16_t> {

  typedef char16_t char_type;
  typedef char32_t src_char_type;
  typedef char32_t dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16u32streambuf streambuf;

public:

  u16u32iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16u32iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16u32iostream

//////////////////////////////////////
// u16

// source/destination is char16_t, read/write char16_t (underflow/overflow).
class u16streambuf : public std::basic_streambuf<char16_t> {

  typedef char16_t char_type;
  typedef char16_t ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef u16streambuf streambuf;

public:

  u16streambuf(src_stream & is); // for reading.
  u16streambuf(dst_stream & os); // for writing
  u16streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };
  //enum { EBACK = 28, BUFSZ = 2 }; // 30 char buffer + pbuf for us.

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  int_type pbuf; // put buffer has only one char.
  char_type ibuf[IBUFSZ];

}; // end of class u16streambuf

class u16istream : public std::basic_istream<char16_t> {

  typedef char16_t char_type;
  typedef char16_t src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef u16streambuf streambuf;

public:

  u16istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16istream

class u16ostream : public std::basic_ostream<char16_t> {

  typedef char16_t char_type;
  typedef char16_t dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16streambuf streambuf;

public:

  u16ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16ostream

class u16iostream : public std::basic_iostream<char16_t> {

  typedef char16_t char_type;
  typedef char16_t src_char_type;
  typedef char16_t dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16streambuf streambuf;

public:

  u16iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16iostream

//////////////////////////////////////
// u16utf8

// source is char, read char16_t (underflow).
// destination is char, write char16_t (overflow).
class u16utf8streambuf : public std::basic_streambuf<char16_t> {

  typedef char16_t char_type;
  typedef char ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef u16utf8streambuf streambuf;

public:

  u16utf8streambuf(src_stream & is); // for reading.
  u16utf8streambuf(dst_stream & os); // for writing
  u16utf8streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };
  //enum { EBACK = 28, BUFSZ = 2 }; // 30 char + pbuf buffer for us.

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  int_type pbuf; // put buffer has only one char.
  char_type ibuf[IBUFSZ];

}; // end of class u16utf8streambuf

class u16utf8istream : public std::basic_istream<char16_t> {

  typedef char16_t char_type;
  typedef char src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef u16utf8streambuf streambuf;

public:

  u16utf8istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16utf8istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16utf8istream

class u16utf8ostream : public std::basic_ostream<char16_t> {

  typedef char16_t char_type;
  typedef char dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16utf8streambuf streambuf;

public:

  u16utf8ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16utf8ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16utf8ostream

class u16utf8iostream : public std::basic_iostream<char16_t> {

  typedef char16_t char_type;
  typedef char src_char_type;
  typedef char dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16utf8streambuf streambuf;

public:

  u16utf8iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16utf8iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16utf8iostream

//////////////////////////////////////
// utf8u32

// source is char32_t, read utf8 (underflow).
// destination is char32_t, write utf8 (overflow).
class utf8u32streambuf : public std::basic_streambuf<char> {

  typedef char char_type;
  typedef char32_t ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef utf8u32streambuf streambuf;

public:

  utf8u32streambuf(src_stream & is); // for reading.
  utf8u32streambuf(dst_stream & os); // for writing
  utf8u32streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };
  //enum { EBACK = 24, BUFSZ = 4 }; // 32 char buffer for us.

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  int_type  och;
  int_type  och_need;
  int_type  och_min;
  char_type ibuf[IBUFSZ];

}; // end of class utf8u32streambuf

class utf8u32istream : public std::basic_istream<char> {

  typedef char char_type;
  typedef char32_t src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef utf8u32streambuf streambuf;

public:

  utf8u32istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8u32istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8u32istream

class utf8u32ostream : public std::basic_ostream<char> {

  typedef char char_type;
  typedef char32_t dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8u32streambuf streambuf;

public:

  utf8u32ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8u32ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8u32ostream

class utf8u32iostream : public std::basic_iostream<char> {

  typedef char char_type;
  typedef char32_t src_char_type;
  typedef char32_t dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8u32streambuf streambuf;

public:

  utf8u32iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8u32iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8u32iostream

//////////////////////////////////////
// utf8u16

// source is char16_t, read utf8 (underflow).
// destination is char16_t, write utf8 (overflow).
class utf8u16streambuf : public std::basic_streambuf<char> {

  typedef char char_type;
  typedef char16_t ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef utf8u16streambuf streambuf;

public:

  utf8u16streambuf(src_stream & is); // for reading.
  utf8u16streambuf(dst_stream & os); // for writing
  utf8u16streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };
  //enum { EBACK = 28, BUFSZ = 4 }; // 32 char buffer for us.

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  int_type  och;
  int_type  och_need;
  int_type  och_min;
  char_type ibuf[IBUFSZ];

}; // end of class utf8u16streambuf

class utf8u16istream : public std::basic_istream<char> {

  typedef char char_type;
  typedef char16_t src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef utf8u16streambuf streambuf;

public:

  utf8u16istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8u16istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8u16istream

class utf8u16ostream : public std::basic_ostream<char> {

  typedef char char_type;
  typedef char16_t dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8u16streambuf streambuf;

public:

  utf8u16ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8u16ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8u16ostream

class utf8u16iostream : public std::basic_iostream<char> {

  typedef char char_type;
  typedef char16_t src_char_type;
  typedef char16_t dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8u16streambuf streambuf;

public:

  utf8u16iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8u16iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8u16iostream

//////////////////////////////////////
// utf8

// source/destination is utf8, read/write utf8 (underflow/overflow).
class utf8streambuf : public std::basic_streambuf<char> {

  typedef char char_type;
  typedef char ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef utf8streambuf streambuf;

public:

  utf8streambuf(src_stream & is); // for reading.
  utf8streambuf(dst_stream & os); // for writing
  utf8streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };
  //enum { EBACK = 28, BUFSZ = 4 }; // 32 char buffer for us.

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  int_type  och;
  int_type  och_need;
  int_type  och_min;
  char_type ibuf[IBUFSZ];

}; // end of class utf8streambuf

class utf8istream : public std::basic_istream<char> {

  typedef char char_type;
  typedef char src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef utf8streambuf streambuf;

public:

  utf8istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8istream

class utf8ostream : public std::basic_ostream<char> {

  typedef char char_type;
  typedef char dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8streambuf streambuf;

public:

  utf8ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8ostream

class utf8iostream : public std::basic_iostream<char> {

  typedef char char_type;
  typedef char src_char_type;
  typedef char dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8streambuf streambuf;

public:

  utf8iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8iostream

//////////////////////////////////////
// u32iso8859_1_

// source is char, read char32_t (underflow).
// destination is char, write char32_t (overflow).
class u32iso8859_1_streambuf : public std::basic_streambuf<char32_t> {

  typedef char32_t char_type;
  typedef char ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef u32iso8859_1_streambuf streambuf;

public:

  u32iso8859_1_streambuf(src_stream & is); // for reading.
  u32iso8859_1_streambuf(dst_stream & os); // for writing
  u32iso8859_1_streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  char_type ibuf[IBUFSZ];

}; // end of class u32iso8859_1_streambuf

class u32iso8859_1_istream : public std::basic_istream<char32_t> {

  typedef char32_t char_type;
  typedef char src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef u32iso8859_1_streambuf streambuf;

public:

  u32iso8859_1_istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32iso8859_1_istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32iso8859_1_istream

class u32iso8859_1_ostream : public std::basic_ostream<char32_t> {

  typedef char32_t char_type;
  typedef char dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u32iso8859_1_streambuf streambuf;

public:

  u32iso8859_1_ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32iso8859_1_ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32iso8859_1_ostream

class u32iso8859_1_iostream : public std::basic_iostream<char32_t> {

  typedef char32_t char_type;
  typedef char src_char_type;
  typedef char dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u32iso8859_1_streambuf streambuf;

public:

  u32iso8859_1_iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32iso8859_1_iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u32iso8859_1_iostream

//////////////////////////////////////
// u16iso8859_1_

// source is char, read UTF-16 (char16_t) (underflow).
// destination is char, write utf-16 (char16_t) (overflow).
class u16iso8859_1_streambuf : public std::basic_streambuf<char16_t> {

  typedef char16_t char_type;
  typedef char ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef u16iso8859_1_streambuf streambuf;

public:

  u16iso8859_1_streambuf(src_stream & is); // for reading.
  u16iso8859_1_streambuf(dst_stream & os); // for writing
  u16iso8859_1_streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  char_type ibuf[IBUFSZ];

}; // end of class u16iso8859_1_streambuf

class u16iso8859_1_istream : public std::basic_istream<char16_t> {

  typedef char16_t char_type;
  typedef char src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef u16iso8859_1_streambuf streambuf;

public:

  u16iso8859_1_istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16iso8859_1_istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16iso8859_1_istream

class u16iso8859_1_ostream : public std::basic_ostream<char16_t> {

  typedef char16_t char_type;
  typedef char dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16iso8859_1_streambuf streambuf;

public:

  u16iso8859_1_ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16iso8859_1_ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16iso8859_1_ostream

class u16iso8859_1_iostream : public std::basic_iostream<char16_t> {

  typedef char16_t char_type;
  typedef char src_char_type;
  typedef char dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef u16iso8859_1_streambuf streambuf;

public:

  u16iso8859_1_iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16iso8859_1_iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class u16iso8859_1_iostream

//////////////////////////////////////
// utf8iso8859_1_

// source/destination is iso 8859-1, read/write utf8 (underflow/overflow).
class utf8iso8859_1_streambuf : public std::basic_streambuf<char> {

  typedef char char_type;
  typedef char ext_char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<ext_char_type> src_stream;
  typedef std::basic_ostream<ext_char_type> dst_stream;
  typedef utf8iso8859_1_streambuf streambuf;

public:

  utf8iso8859_1_streambuf(src_stream & is); // for reading.
  utf8iso8859_1_streambuf(dst_stream & os); // for writing
  utf8iso8859_1_streambuf(src_stream & is, dst_stream & os); // for both.

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };
  //enum { EBACK = 28, BUFSZ = 4 }; // 32 char buffer for us.

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  char_type * ibufb;
  char_type * ibufe;
  int_type och;
  char_type ibuf[IBUFSZ];

}; // end of class utf8iso8859_1_streambuf

class utf8iso8859_1_istream : public std::basic_istream<char> {

  typedef char char_type;
  typedef char src_char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef utf8iso8859_1_streambuf streambuf;

public:

  utf8iso8859_1_istream(src_stream & is)
    : base_type(0), isbuf_(is)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8iso8859_1_istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8iso8859_1_istream

class utf8iso8859_1_ostream : public std::basic_ostream<char> {

  typedef char char_type;
  typedef char dst_char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8iso8859_1_streambuf streambuf;

public:

  utf8iso8859_1_ostream(dst_stream & os)
    : base_type(0), isbuf_(os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8iso8859_1_ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8iso8859_1_ostream

class utf8iso8859_1_iostream : public std::basic_iostream<char> {

  typedef char char_type;
  typedef char src_char_type;
  typedef char dst_char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<src_char_type> src_stream;
  typedef std::basic_ostream<dst_char_type> dst_stream;
  typedef utf8iso8859_1_streambuf streambuf;

public:

  utf8iso8859_1_iostream(src_stream & is, dst_stream & os)
    : base_type(0), isbuf_(is, os)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  utf8iso8859_1_iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

private:

  streambuf isbuf_;

}; // end of class utf8iso8859_1_iostream

// incase you need byte swapping for char16_t or char32_t:

class u32bswap_streambuf : public std::basic_streambuf<char32_t> {

  typedef char32_t char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<char_type> src_stream;
  typedef std::basic_ostream<char_type> dst_stream;
  typedef u32bswap_streambuf streambuf;

public:

  typedef u32_swap_state_type swap_state_type;

  // for reading.
  u32bswap_streambuf(src_stream & is,
		     swap_state_type s = swap_state_type::None);

  // for writing
  u32bswap_streambuf(dst_stream & os,
		     swap_state_type s = swap_state_type::None);

  // for both.
  u32bswap_streambuf(src_stream & is, dst_stream & os,
		     swap_state_type s = swap_state_type::None);

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }
  swap_state_type swap_state() const { return swap_state_; }

  swap_state_type set_swap_state(swap_state_type s)
  { swap_state_type t=swap_state_; swap_state_ = s; return t; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };

  int_type get();
  int_type put(int_type c);
  static swap_state_type check(swap_state_type s);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  swap_state_type swap_state_;
  char_type * ibufb;
  char_type * ibufe;
  char_type ibuf[IBUFSZ];

}; // end of class u32bswap_streambuf

class u32bswap_istream : public std::basic_istream<char32_t> {

  typedef char32_t char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<char_type> src_stream;
  typedef u32bswap_streambuf streambuf;

public:

  typedef u32_swap_state_type swap_state_type;

  u32bswap_istream(src_stream & is,
		   swap_state_type s = swap_state_type::None)
    : base_type(0), isbuf_(is, s)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32bswap_istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

  swap_state_type streambuf_swap_state() const { return isbuf_.swap_state(); }

  swap_state_type set_swap_state(swap_state_type s)
  { return isbuf_.set_swap_state(s); }

private:

  streambuf isbuf_;

}; // end of class u32bswap_istream

class u32bswap_ostream : public std::basic_ostream<char32_t> {

  typedef char32_t char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<char_type> dst_stream;
  typedef u32bswap_streambuf streambuf;

public:

  typedef u32_swap_state_type swap_state_type;

  u32bswap_ostream(dst_stream & os, swap_state_type s = swap_state_type::None)
    : base_type(0), isbuf_(os, s)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32bswap_ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

  swap_state_type streambuf_swap_state() const { return isbuf_.swap_state(); }

  swap_state_type set_swap_state(swap_state_type s)
  { return isbuf_.set_swap_state(s); }

private:

  streambuf isbuf_;

}; // end of class u32bswap_ostream

class u32bswap_iostream : public std::basic_iostream<char32_t> {

  typedef char32_t char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<char_type> src_stream;
  typedef std::basic_ostream<char_type> dst_stream;
  typedef u32bswap_streambuf streambuf;

public:

  typedef u32_swap_state_type swap_state_type;

  u32bswap_iostream(src_stream & is, dst_stream & os,
		    swap_state_type s = swap_state_type::None)
    : base_type(0), isbuf_(is, os, s)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u32bswap_iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

  swap_state_type streambuf_swap_state() const { return isbuf_.swap_state(); }

  swap_state_type set_swap_state(swap_state_type s)
  { return isbuf_.set_swap_state(s); }

private:

  streambuf isbuf_;

}; // end of class u32bswap_iostream

////////////////////////////
// u16bswap_

class u16bswap_streambuf : public std::basic_streambuf<char16_t> {

  typedef char16_t char_type;
  typedef std::basic_streambuf<char_type> base_type;
  typedef std::basic_istream<char_type> src_stream;
  typedef std::basic_ostream<char_type> dst_stream;
  typedef u16bswap_streambuf streambuf;

public:

  typedef u16_swap_state_type swap_state_type;

  // for reading.
  u16bswap_streambuf(src_stream & is,
		     swap_state_type s = swap_state_type::None);

  // for writing
  u16bswap_streambuf(dst_stream & os,
		     swap_state_type s = swap_state_type::None);

  // for both.
  u16bswap_streambuf(src_stream & is, dst_stream & os,
		     swap_state_type s = swap_state_type::None);

  virtual int_type underflow();
  virtual int_type overflow(int_type __c);
  virtual streambuf * setbuf(char_type * __s, std::streamsize __n);

  status_type status() const { return status_; }
  streambuf & clear_status() { status_ = status_type::OK; return *this; }
  swap_state_type swap_state() const { return swap_state_; }

  swap_state_type set_swap_state(swap_state_type s)
  { swap_state_type t=swap_state_; swap_state_ = s; return t; }

protected:

  enum { EBACK = 16, IBUFSZ = 64 };

  int_type get();
  int_type put(int_type c);

  int_type err_status(status_type s)
  { status_ = s; return traits_type::eof(); }

  src_stream * is_;
  dst_stream * os_;
  status_type status_;
  swap_state_type swap_state_;
  char_type * ibufb;
  char_type * ibufe;
  char_type ibuf[IBUFSZ];

}; // end of class u16bswap_streambuf

class u16bswap_istream : public std::basic_istream<char16_t> {

  typedef char16_t char_type;
  typedef std::basic_istream<char_type> base_type;
  typedef std::basic_istream<char_type> src_stream;
  typedef u16bswap_streambuf streambuf;

public:

  typedef u16_swap_state_type swap_state_type;

  u16bswap_istream(src_stream & is,
		   swap_state_type s = swap_state_type::None)
    : base_type(0), isbuf_(is, s)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16bswap_istream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

  swap_state_type streambuf_swap_state() const { return isbuf_.swap_state(); }

  swap_state_type set_swap_state(swap_state_type s)
  { return isbuf_.set_swap_state(s); }

private:

  streambuf isbuf_;

}; // end of class u16bswap_istream

class u16bswap_ostream : public std::basic_ostream<char16_t> {

  typedef char16_t char_type;

  typedef std::basic_ostream<char_type> base_type;
  typedef std::basic_ostream<char_type> dst_stream;
  typedef u16bswap_streambuf streambuf;

public:

  typedef u16_swap_state_type swap_state_type;

  u16bswap_ostream(dst_stream & os, swap_state_type s = swap_state_type::None)
    : base_type(0), isbuf_(os, s)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16bswap_ostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

  swap_state_type streambuf_swap_state() const { return isbuf_.swap_state(); }

  swap_state_type set_swap_state(swap_state_type s)
  { return isbuf_.set_swap_state(s); }

private:

  streambuf isbuf_;

}; // end of class u16bswap_ostream

class u16bswap_iostream : public std::basic_iostream<char16_t> {

  typedef char16_t char_type;

  typedef std::basic_iostream<char_type> base_type;
  typedef std::basic_istream<char_type> src_stream;
  typedef std::basic_ostream<char_type> dst_stream;
  typedef u16bswap_streambuf streambuf;

public:

  typedef u16_swap_state_type swap_state_type;

  u16bswap_iostream(src_stream & is, dst_stream & os,
		    swap_state_type s = swap_state_type::None)
    : base_type(0), isbuf_(is, os, s)
  { this->init(& isbuf_); }

  status_type streambuf_status() const { return isbuf_.status(); }

  u16bswap_iostream & clear_streambuf_status()
  { isbuf_.clear_status(); return *this; }

  swap_state_type streambuf_swap_state() const { return isbuf_.swap_state(); }

  swap_state_type set_swap_state(swap_state_type s)
  { return isbuf_.set_swap_state(s); }

private:

  streambuf isbuf_;

}; // end of class u16bswap_iostream

}; // end of namespace unicodestreams

}; // end of namespace alf

#endif
