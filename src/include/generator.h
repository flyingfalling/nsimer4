//REV: 4 Apr 2016
//generator.h
//generator class for user to make generators to generate actual models (implemented symmodels)




#pragma once

//#include <symmodel.h>
#include <corresp.h>
#include <cmdstore.h>
#include <global_store.h>

struct generator;
struct symmodel;
struct cmdstore;

struct genfunct_t
{
  vector<string> lines;
  
  std::shared_ptr<cmdstore> cmds;
  
  void addlocal(const string& fname, const string& f );
 
  void execute( std::shared_ptr<symmodel>& model, global_store& globals );

  void execute_line( std::shared_ptr<symmodel> model, global_store& globals, const size_t& line );
  
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
    cmds = std::make_shared<cmdstore>( );
  }
}; //end struct genfunct_t



struct generator
{
  genfunct_t genfunct;  
  
  void generate( std::shared_ptr<symmodel>& model, global_store& globals );

  void enumerate()
  {
    genfunct.enumerate();
  }
  
  
  void add( const string& s )
  {
    genfunct.add( s );
  }
  
  void execute_line(  std::shared_ptr<symmodel> model, global_store& globals, const size_t& line )
  {
    genfunct.execute_line( model, globals, line );
  }
  
};

