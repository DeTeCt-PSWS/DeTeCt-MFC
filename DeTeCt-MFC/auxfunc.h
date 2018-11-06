#pragma once
#include <string>
#include <sstream>

#define DBOUT(s) { \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}