#pragma once

#include <commontypes.h>
#include <symmodel.h>

struct symmodel;
struct symvar;
struct varptr;
struct elemptr;


//REV: should globalstore just be a symmodel? Does it need special functions? For purposes of checking if it is in global store or not. Or if two are both in
//global store together. For purpose of finding correspondences.
struct global_store
{
  vector< std::shared_ptr<symmodel> > models;
  
  void addiparam( const string& lname, const vector<size_t>& val );
  void addfparam( const string& lname, const vector<real_t>& val );
  
  void add( std::shared_ptr<symmodel>& m )
  {
    models.push_back( m );
  }
  
  void addempty( const string& localname );
  
  //find model type thing. Only first level ;)

  vector<size_t> modellocs( const string& s );

  vector<size_t> modellocs( const std::shared_ptr<symmodel>& m );

  
  elemptr findmodel( const string& s ); //, const vector<size_t>& idx );
  elemptr findmodel( const std::shared_ptr<symmodel>& m );

  void read_and_reset_all( vector<string>& readstate, vector<string>& writtenstate, vector<string>& pushedstate );
  void set_non_generating();
}; //end GLOBAL STORE
