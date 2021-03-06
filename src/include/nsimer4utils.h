
#pragma once

#include <commontypes.h>
#include <parsehelpers.h>

#include <random>

//stringify macro
#define STR( _mystring )  #_mystring

#define ADDFUNCT(fname)							\
  {									\
    cmd_functtype fa = fname;						\
    add( #fname, fa );							\
  }

vector<string> parse( const string& name);
vector<string> parsetypes( const string& name);
vector<string> parsecorr( const string& name);


vector<real_t> vect_mult( const vector< vector<real_t> >& v );
vector<real_t> vect_sum( const vector< vector<real_t> >& v );
vector<real_t> vect_div(  const vector< vector<real_t> >& v );
vector<real_t> vect_diff(  const vector< vector<real_t> >& v );
vector<real_t> vect_negate( const vector<real_t>& val );
vector<real_t> vect_exp( const vector<real_t>& val );
vector<real_t> vect_normal( const vector<real_t>& meanval, const vector<real_t>& stdval, std::default_random_engine& RANDGEN );
vector<real_t> vect_uniform( const vector<real_t>& minval, const vector<real_t>& maxval, std::default_random_engine& RANDGEN );
vector<real_t> vect_sqrt( const vector<real_t>& val );
vector<real_t> vect_sqr( const vector<real_t>& val );

bool check_idx( const string& varname );

bool check_issize( const string& varname );

//REV: shit, if I parse "/" will it give me "nothing" or will it give me ""/""?
//I'm leaning towards nothing? Heh...
bool check_iscorr( const string& varname, string& premodelname, string& postmodelname );

bool check_cmd_is_multi( const string& s );
