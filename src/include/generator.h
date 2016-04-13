//REV: 4 Apr 2016
//generator.h
//generator class for user to make generators to generate actual models (implemented symmodels)




#pragma once

#include <symmodel.h>
#include <cmdstore.h>

struct generator;
struct symmodel;
struct global_store;

struct genfunct_t
{
  vector<string> lines;
  cmdstore cmds;
  //std::shared_ptr<generator> gen;
  
  void addlocal(const string& fname, const string& f )
  {
    cmds.addlocal( fname, f );
  }

  void execute( std::shared_ptr<symmodel>& model, global_store& globals );
  
  void add( const string& s )
  {
    lines.push_back( s );
  }

  void enumerate()
  {
    for(size_t x=0; x<lines.size(); ++x)
      {
	fprintf(stdout, "[%s]\n", lines[x].c_str());
      }
  }
  
  genfunct_t()
  {
  }
}; //end struct genfunct_t



struct generator
{
  genfunct_t genfunct;  
  //std::shared_ptr<symmodel> model;
  
  
  void generate( std::shared_ptr<symmodel>& model, global_store& globals );

  void enumerate()
  {
    genfunct.enumerate();
  }
  
  
  void add( const string& s )
  {
    genfunct.add( s );
  }
  
};

