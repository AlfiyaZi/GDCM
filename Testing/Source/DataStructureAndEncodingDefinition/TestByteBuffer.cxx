#include "gdcmByteBuffer.h"
#include <iostream>

int TestByteBuffer(int, char *[])
{
  const int size = 128;
  gdcm::ByteBuffer b;
  char buffer[size];

  int r = 0;
  for(int i=-size; i<0; ++i)
    {
    buffer[i+size] = (char)(i);
    }
  memcpy(b.Get(size), buffer, size);
  const char *start = b.GetStart();
  for(int i=size; i>0; --i)
    {
    if( (int)start[size-i] != -i )
      {
      std::cout << (int)start[size-i] << " " << -i << std::endl;
      ++r;
      }
    }

  for(int i=0; i<size; ++i)
    {
    buffer[i] = (char)i;
    }
  memcpy(b.Get(size), buffer, size);

  for(int i=0; i<size; ++i)
    {
    if( (int)start[i] != i )
      {
      std::cout << (int)start[i] << std::endl;
      ++r;
      }
    }
  
  return r;
}
