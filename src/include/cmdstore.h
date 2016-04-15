
#pragma once

#include <commontypes.h>
#include <fparser.h>
#include <parsehelpers.h>

struct elemptr; //defined in symmodel
struct cmdstore; //defined here
struct global_store; //defined in symmodel
struct varptr; //defined in symmodel

typedef std::function< varptr( const string&, const vector<elemptr>&, cmdstore&, global_store& ) > cmd_functtype;

struct cmdstore
{
  vector< string > functnames;
  vector< cmd_functtype > functs;
  
  std::default_random_engine RANDGEN;
  
  vector<string> localfnames;
  vector<string> localfs;

  //REV: static, so all copies of CMD will return the same value I assume...i.e. basically global variable...
  static bool testmode=true;

  bool checktest()
  {
    return testmode;
  }

  void settestdone()
  {
    testmode = false;
  }
		   
  
  vector<size_t> findlocal( const string& fname ) const
  {
    vector<size_t> r;
    for(size_t x=0; x<localfnames.size(); ++x)
      {
	if( localfnames[x].compare( fname )  == 0 )
	  {
	    r.push_back( x );
	  }
      }
    return r;
  }
  
  //bool handlelocal( const string& input, string& output ) //const string& fname, const vector<string>& args )
  string handlelocal( const string& input ) const //const string& fname, const vector<string>& args )
  {
    string ret;
    
    vector<string> parsed = fparse( input );
    if( parsed.size() < 2 )
      {
	//fprintf(stderr, "REV; not a function with args ([%s])?\n", input.c_str());
	//exit(1);
	//return input;
	ret = input;
      }
    else
      {
	string fname = parsed[0];
	vector<string> args( parsed.begin()+1, parsed.end() );
    
	vector<size_t> locs = findlocal( fname );
	fprintf(stdout, "REV: found [%lu] **LOCAL** functions matching funct name [%s]\n", locs.size(), fname.c_str());
	if( locs.size() == 1 )
	  {
	    //output = replace( localfs[ locs[0] ], args );
	    ret = replace( localfs[ locs[0] ], args );
	  }
	else if( locs.size() > 1 )
	  {
	    fprintf(stderr, "REV: error handle local, more than one funct registered under name [%s]\n", fname.c_str() );
	    exit(1);
	  }
	else
	  {
	    ret = input;
	    //return input;
	  }
      }
    fprintf(stdout, "HANDLED LOCAL: Replaced [%s] with [%s]\n", input.c_str(), ret.c_str() );
    return ret;
  }
  
  string replace( const string& s, const vector<string>& arglist ) const
  {
    //Doparse should be 1!!!!
    fprintf(stdout, "Replacing! [%s] target, args:", s.c_str());
    for(size_t x=0; x<arglist.size(); ++x)
      {
	fprintf(stdout, " [%s]", arglist[x].c_str() );
      }
    fprintf(stdout, "\n");
    vector<string> parsed = fparse( s );
    
    if( parsed.size() == 1 )
      {
	for(size_t x=0; x<arglist.size(); ++x)
	  {
	    //Fuck, this is a "converter!!!! 
	    string name="arg" + std::to_string( x );
	    if( parsed[0].compare(name) == 0 )
	      {
		//parsed[0] = arglist[x];
		fprintf(stdout, "Found name [%s], so replacing with corresponding arg [%s]\n", arglist[x].c_str(), name.c_str() );
		return arglist[x];
	      }
	  }
	fprintf(stdout, "REV: what the fuck? Didn't find name [%s]...\n", parsed[0].c_str() );
	exit(1);
      }
    else
      {
	//Taking only the next bit of args?
	fprintf(stdout, "Replacing multiple parts..\n");
	vector<string> argpart( parsed.begin()+1, parsed.end() );
	for( size_t p=0; p<argpart.size(); ++p)
	  {
	    argpart[p] = replace( argpart[p], arglist );
	  }
	string fpart =  parsed[0] ;

	string argpartstr = CAT( argpart, "," );
	string newf = fpart + "(" + argpartstr + ")";
	return newf;
      }
  }
  
    
  void addlocal( const string& fname, const string& f )
  {
    //Check no name clash etc.
    localfnames.push_back(fname);
    localfs.push_back(f);
  }
  
  cmdstore();

  void add( const string& s, cmd_functtype& f )
  {
    functnames.push_back(s);
    functs.push_back(f);
  }
  

  //REV: if it doesnt find it, it is a variable or a number
  bool findfunct( const string& s, cmd_functtype& f ) const
  {
    //const vector<string>::iterator it = std::find( functnames.begin(), functnames.end(), s );
    for(size_t x=0; x<functnames.size(); ++x)
      {
	if( functnames[x].compare( s ) == 0 )
	  {
	    f = functs[ x ];
	    return true;
	  }
      }
    return false;
  }


  //Parses just by commas, but leaves matching parens (i.e. functions) intact.
  //This is just a literal parse of inside of funct? Is there any point in this? Just use the remnants from fparse...
  vector<string> doparse( const string& s ) const 
  {
    //JUST RUN MY PARSER HERE, only first level parse.
    auto f( std::begin( s ));
    auto l( std::end( s ));
    const static fparser::doparser<decltype(f)> p;
    vector<string> result;
    bool ok = fparser::qi::phrase_parse(f, l, p, fparser::qi::space, result );
    
    if(!ok)
      {
	fprintf(stderr, "REV: fparse: invalid input!!! [%s]\n", s.c_str());
	exit(1);
      }
    
    return result;
    
  }
  
  //parses function, i.e. expects only single FNAME( COMMA, ARGS )
  vector<string> fparse( const string& s ) const
  {
    //JUST RUN MY PARSER HERE, only first level parse.
    auto f( std::begin( s ));
    auto l( std::end( s ));
    const static fparser::parser<decltype(f)> p;
    vector<string> result;
    bool ok = fparser::qi::phrase_parse(f, l, p, fparser::qi::space, result );
    
    if(!ok)
      {
	fprintf(stderr, "REV: fparse: invalid input!!! [%s]\n", s.c_str());
	exit(1);
      }
    
    return result;
    
  }
    
};
