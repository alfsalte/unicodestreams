
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>

#include "../unicodestreams.hxx"

void do_write(const char * fn, const char32_t * txt)
{
  std::ofstream f(fn);
  alf::unicodestreams::u32utf8ostream g(f);

  g << txt;
}

int do_read(std::basic_istream<char16_t> & is, char16_t * buf, size_t sz)
{
  char16_t * p = buf;
  char16_t * e = buf + sz;
  char16_t ch;
  int ret = 0;
  while (p < e && is.get(ch)) {
    *p++ = ch;
    ++ret;
  }
  return ret;
}

void do_read(const char * fn, const char32_t * txt)
{
  char16_t buf[1024];
  std::ifstream f(fn);
  alf::unicodestreams::u16utf8istream g(f);
  int k = do_read(g, buf, 1024);
  int j = 0;
  int u = 0;
  char32_t c;
  char16_t lead = 0;
  char16_t c16;
  while (j < k) {
    if (txt[u] == 0) {
      std::cout << "too many chars in file" << std::endl;
      std::cout << "txt[" << u << "] = 0x"
		<< std::hex << std::setfill('0') << std::setw(6)
		<< int(txt[u]) << std::endl;
      std::cout << "buf[" << j << "] = 0x"
		<< std::hex << std::setfill('0') << std::setw(4)
		<< int(buf[j]) << std::endl;
      return;
    }
    if ((c16 = buf[j]) < 0xd800 || c16 >= 0xe000) {
      if (lead) {
	std::cout << "character buf[" << (j-1);
	std::cout << "] = 0x"
		  << std::hex << std::setfill('0') << std::setw(4)
		  << int(lead)
		  << " requires a follow code" << std::endl;
	return;
      }
      if (txt[u] != char32_t(c16)) {
	std::cout << "character txt[" << u << "] = 0x"
		  << std::hex << std::setfill('0') << std::setw(6)
		  << int(txt[u]) << std::endl;
	std::cout << "character buf[" << j << "] = 0x"
		  << std::hex << std::setfill('0') << std::setw(4)
		  << int(buf[j]) << std::endl;
	return;
      }
      ++u;
      ++j;
    } else if (c16 < 0xdc00) {
      if (lead) {
	std::cout << "character buf[" << (j-1) << "] = 0x"
		  << std::hex << std::setfill('0') << std::setw(4)
		  << int(lead)
		  << " requires a follow code" << std::endl;
	return;
      }
      lead = c16;
      ++j;
    } else if (lead == 0) {
      std::cout << "character buf[" << j << "] = 0x"
		<< std::hex << std::setfill('0') << std::setw(4)
		<< int(buf[j])
		<< " requires a lead code." << std::endl;
      return;
    } else {
      int w = (((lead & 0x3ff) << 10) | (buf[j] & 0x3ff)) + 0x10000;
      static const char chbuf[] = "character buf[";

      if (txt[u] != char32_t(w)) {
	std::cout << "character txt[" << u;
	std::cout << "] = 0x"
		  << std::hex << std::setfill('0') << std::setw(6)
		  << int(txt[u]) << std::endl;
	std::cout << chbuf << j;
	std::cout << "] = 0x"
		  << std::hex << std::setfill('0') << std::setw(6)
		  << int(buf[j])
		  << std::endl;
	return;
      }
      ++u;
      ++j;
    }
  }
  std::cout << "string read is ok." << std::endl;
}

void do_write_read(const char * fn, const char32_t * txt)
{
  do_write(fn, txt);
  do_read(fn, txt);
}

int main()
{
  do_write_read("test-a.txt",
		U"\ufeffHello there\n\U00002029Blah blah\n");
}
