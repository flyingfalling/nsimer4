#pragma once

#include <commontypes.h>

#include <sys/types.h>
#include <boost/tokenizer.hpp>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>


using std::string;
using std::vector;

vector<string> tokenize_string( const string& src, const string& delim, const bool& include_empty_repeats=false );

bool string_prefix_match( const std::string& orig, const std::string& prefix );

std::string remove_prefix( const std::string& str, const std::string& prefix );


bool checknumeric( const string& s, real_t& ret );
bool checknumericint( const string& s, size_t& ret );
string CAT( const vector<string>& args, const string& sep  );


//EOF parsehelpers.h
