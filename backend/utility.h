#ifndef _UTILITY_H_
#define _UTILITY_H_

struct StrCmp
{
   bool operator()(const char* a, const char* b)
   {
      return strcmp(a, b) < 0;
   }
};

#endif
