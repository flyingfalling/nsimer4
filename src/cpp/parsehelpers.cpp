#include <parsehelpers.h>


/*vector<string> tokenize_string( const string& src, const string& delim, const bool& include_empty_repeats )
{
  vector<string> retval;
  boost::char_separator<char> sep( delim.c_str() );
  boost::tokenizer<boost::char_separator<char>> tokens(src, sep);
  for(const auto& t : tokens)
    {
      retval.push_back( t );
    }
  return retval;
  }*/

//#include <boost/regex.hpp>
//#include <boost/algorithm/string/regex.hpp>

vector<string> tokenize_string( const string& src, const string& delim, const bool& include_empty_repeats )
{

  vector<string> result;
  string tmp=src;
  size_t pos = 0;
  string token;

  //I assume this includes empty repeats...
  while ((pos = tmp.find(delim)) != std::string::npos)
    {
      token = tmp.substr(0, pos);
      result.push_back(token);
      tmp.erase(0, pos + delim.length());
    }

  //need to push back last in case there were none?
  result.push_back( tmp );
  
  return result;
}

bool string_prefix_match( const std::string& orig, const std::string& prefix )
{
  if( orig.compare(0, prefix.size(), prefix) == 0 )
    {
      return true;
    }
  return false;
}

std::string remove_prefix( const std::string& str, const std::string& prefix )
{
  if( string_prefix_match( str, prefix ) == true ) 
    {
      size_t last = str.find_last_of( prefix );
      std::string noprefix = str.substr( last+1, str.size() );
      return noprefix;
    }
  else
    {
      fprintf(stderr, "ERROR in remove_prefix: requested prefix to remove [%s] is not a prefix of me [%s]!!\n", prefix.c_str(), str.c_str());
      exit(1);
    }
}

bool checknumeric( const string& s, real_t& ret )
{
  bool ok=false;
  std::istringstream iss( s );
  
  iss >> ret;

  //check that iss is all done! If it successfully parsed it, we good?
  if( iss.fail() )
    {
      iss.clear();
      string failedstring;
      iss >> failedstring;

      ok = false;
    }
  else
    {
      ok = true;
    }

  //Do I need to try to read "next" to read end of string? Will it return fail????
  string wat;
  iss >> wat;
  if( !iss.eof() )
    {
      fprintf(stderr, "REV: error, checknumeric, didn't finish parse of input string to numeric [%s], read in unexpected [%s]?!?!?!?!\n", s.c_str(), wat.c_str());
      exit(1);
    }

  return ok;
}

//REV: should only read until SPACE (i.e. 10382.283 should be FAIL, not 10382...)
bool checknumericint( const string& s, size_t& ret )
{
  bool ok=false;
  std::istringstream iss( s );
  
  iss >> ret;
  
  //check that iss is all done! If it successfully parsed it, we good?
  if( iss.fail() )
    {
      iss.clear();
      string failedstring;
      iss >> failedstring;
      
      ok = false;
    }
  else
    {
      ok = true;
    }
  
  //Do I need to try to read "next" to read end of string? Will it return fail????
  string wat;
  iss >> wat;
  if( !iss.eof() )
    {
      fprintf(stderr, "REV: error, checknumeric, didn't finish parse of input string to numeric INT [%s], read in unexpected [%s]?!?!?!?!\n", s.c_str(), wat.c_str());
      exit(1);
    }

  return ok;
}

string CAT( const vector<string>& args, const string& sep  )
{
  if(args.size() == 0)
    {
      return "";
    }
  else if(args.size() == 1)
    {
      return args[0];
    }
  else
    {
      string s=args[0];
      for(size_t a=1; a<args.size(); ++a )
	{
	  s += sep + args[a];
	}
      return s;
    }
}

//EOF parsehelpers.h
