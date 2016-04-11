//REV: 4 Apr 2016
//generator.h
//generator class for user to make generators to generate actual models (implemented symmodels)




#pragma once

#include <symmodel.h>

struct generator;


struct genfunct_t
{
  vector<string> lines;
  //gencmdstore cmds;
  cmdstore cmds;
  
  std::shared_ptr<generator> gen;

  void addlocalfunct(const string& fname, const string& f )
  {
    cmds.addlocal( fname, f );
  }

  void execute( const size_t& myidx )
  {
    for(size_t l=0; l<lines.size(); ++l)
      {
	vector<elemptr> trace;
	elemptr tmp( gen->model, myidx );
	trace.push_back(tmp);
	DOCMD( lines[l], trace, cmds );
      }
  }

  void add( const string& s )
  {
    lines.push_back( s );
  }

genfunct_t() // const std::shared_ptr<generator>& g )
//: gen(g )
  {
  }
}; //end struct genfunct_t


struct generator
  :
  public std::enable_shared_from_this<generator>

{
  //Takes source model (variable?) I guess.
  void generate()
  {
    //genfunct has access to THIS (i.e. this generator). So it can access: model, and varstogen. Good.
    //RULES: I can only add new var at the base level.
    //They will have side effects...
    genfunct.execute();
  }

  //either has a pointer to a corresp, or a model?

  genfunct_t genfunct; //parent model is what? the model that it generates? heh.
  
  std::shared_ptr<symmodel> model;
  
  //vector<string> varstogen;

  void add_to_genfunct( const string& s )
  {
    if( !genfunct.gen )
      {
	genfunct.gen = shared_from_this();
      }

    genfunct.add( s );
  }
  
  //List of variables that will be simultaneously generated.
  //I can find them in model...note some may be connections.
  //generator( const vector<string>& genlist, const std::shared_ptr<symmodel>& m )
 generator()// const std::shared_ptr<symmodel>& m )
    // : model( m )
  {
  }
  
};

