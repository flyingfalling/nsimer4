
#pragma once

//#define BOOST_SPIRIT_DEBUG
#define BOOST_SPIRIT_USE_PHOENIX_V3
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/foreach.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>


#include <fstream>
#include <string>
#include <cerrno>

//template <typename T>
//typedef std::vector<T> vec;
using std::vector;

typedef std::string string;

namespace fparser
{

  namespace fusion = boost::fusion;
  namespace phoenix = boost::phoenix;
  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;
  
  template < typename It, typename Skipper = qi::space_type >
    struct parser : qi::grammar<It, vector<string>(), Skipper>
    {
      inline parser();
      
      //REV: is second arg what it takes, or what it returns?
      qi::rule< It, vector<string>(), qi::space_type > vecform;
      qi::rule< It, string(), qi::space_type > stringform;
      qi::rule< It, string(), qi::space_type > fname;
      
    };

  template < typename It, typename Skipper = qi::space_type >
    struct doparser : qi::grammar<It, vector<string>(), Skipper>
    {
      inline doparser();
      
      //REV: is second arg what it takes, or what it returns?
      qi::rule< It, vector<string>(), qi::space_type > vecform;
      qi::rule< It, string(), qi::space_type > estringform;
      qi::rule< It, string(), qi::space_type > stringform;
      qi::rule< It, string(), qi::space_type > fname;
      
    };
  
}


#include <fparser.cpp>
