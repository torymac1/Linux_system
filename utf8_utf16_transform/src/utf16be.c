#include "utf.h"
#include "debug.h"
#include "wrappers.h"
#include <unistd.h>

int
from_utf16be_to_utf16le(int infile, int outfile)
{
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  size_t bytes_to_write;    //ssize_t bytes_to_write;
  int ret = 0;   //int ret = 0;

  bom = UTF16LE;

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);    //reverse_bytes(&bom, 2);
#endif
  bytes_read=read_to_bigendian(infile, &(buf.upper_bytes), 2);  //newline to read bom
  write_to_bigendian(outfile, &bom, 2);


  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
    debug("buf %X %X", buf.upper_bytes, buf.lower_bytes);
    bytes_to_write = 2; /* utf-32 future compatibility */
    reverse_bytes(&(buf.upper_bytes), 2);
    if(is_lower_surrogate_pair(buf)) {
      debug("buf %X %X", buf.upper_bytes, buf.lower_bytes);
      if((bytes_read = read_to_bigendian(infile, &(buf.lower_bytes), 2) > 0)) {
        break;
      }
      reverse_bytes(&(buf.lower_bytes), 2);
      bytes_to_write += 2;
    }
    //debug("After reverse buf %X %X", buf.upper_bytes, buf.lower_bytes);
    write_to_bigendian(outfile, &buf, bytes_to_write);   //write_to_bigendian(outfile, &buf, bytes_to_write);
  }
  ret = bytes_read;
  return ret;
}



int
from_utf16be_to_utf8(int infile, int outfile)
{
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  //size_t bytes_to_write;    //ssize_t bytes_to_write;
  code_point_t code_point;
  size_t size_of_glyph;
  bom = UTF8;

  utf8_glyph_t utf8_glyph;

  bytes_read=read_to_bigendian(infile, &(buf.upper_bytes), 2);  //newline to read bom
  write_to_bigendian(outfile, &bom, 3);

  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
    reverse_bytes(&(buf.upper_bytes),2);
    debug("buf.upper_bytes %X lower_bytes %X", buf.upper_bytes, buf.lower_bytes);
    if (is_upper_surrogate_pair(buf)){
      bytes_read = read_to_bigendian(infile, &(buf.lower_bytes), 2);
      reverse_bytes(&(buf.lower_bytes),2);
    }
    code_point=utf16_glyph_to_code_point(&buf);
    debug("code_point %X", code_point);
    size_of_glyph=utf8_glyph_size_of_code_point(code_point);
    utf8_glyph = code_point_to_utf8_glyph(code_point, &size_of_glyph);
    write_to_bigendian(outfile, &utf8_glyph, size_of_glyph);
    memset(&buf,0,sizeof(utf16_glyph_t));
    }
  return bytes_read;
}

utf16_glyph_t
code_point_to_utf16be_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  utf16_glyph_t ret;

  memset(&ret, 0, sizeof ret);
  if(is_code_point_surrogate(code_point)) {
    code_point -= 0x10000;
    ret.upper_bytes = (code_point >> 10) + 0xD800;
    ret.lower_bytes = (code_point & 0x3FF) + 0xDC00;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
    reverse_bytes(&ret.lower_bytes, 2);
  #endif
    *size_of_glyph = 4;
  }
  else {
    ret.upper_bytes |= code_point;
  #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    reverse_bytes(&ret.upper_bytes, 2);
  #endif
    *size_of_glyph = 2;
  }
  return ret;
}