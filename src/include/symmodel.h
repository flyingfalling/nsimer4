//REV: 4 Apr 2016
//symmodel.h
//Symbolic model struct and helpers.

//REV: Alternate idea
// Do everything as classes etc. and make user build like that. Definitely easier/typesafe...and more control over CUDA compilation?
// allow direct specification of dx/dt?

//TODO: add way to specify things to do at end/beginning of turn (for example, setting V[t-1] to V[t].)
//TODO: add way to automatically determine dependencies among variables for update purposes (based on read/write during each update line of each model).
//TODO: make options so that I can do "even" updates i.e. spike schedulers.

#pragma once

#include <commontypes.h>
#include <fparser.h>
#include <parsehelpers.h>
#include <generator.h>

#include <elemptr.h>
#include <cmdstore.h>
#include <corresp.h>
#include <symvar.h>
#include <global_store.h>
#include <nsimer4utils.h>



#include <sys/types.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <string>

#include <algorithm>
#include <memory>

#include <random>

using std::vector;
using std::string;

//forward decl struct symmodel
struct symmodel;
struct cmdstore;
struct elemptr;
struct varptr;
struct generator;

struct corresp;




struct hole
{
  string name;
  std::shared_ptr<symmodel> parent;
  //string type;
  //vector<string> members; //E.g. names (global) of models that have filled this hole?
  vector< std::shared_ptr<symmodel> > members;
  vector< bool > external;
  //Have pointer to connection struct?
  
hole( const string& n, const std::shared_ptr<symmodel>& p )
: name(n), parent(p)
  {
  }

  //What if it is a sub-model we are adding? Does it matter?
  void add( const std::shared_ptr<symmodel>& h );

}; //end struct hole





struct updatefunct_t
{
  vector<string> lines;
  std::shared_ptr<cmdstore> cmds; //REV: will this cause problems when cloning? lolw ill point to same cmdstore?
  std::shared_ptr<symmodel> model;
  
  
updatefunct_t( const std::shared_ptr<symmodel>& m )
: model( m )
  {
    cmds = std::make_shared<cmdstore>( );
  }
  
  //dummy (nothing)
  updatefunct_t()
  {
    cmds = std::make_shared<cmdstore>( );
  }
  
  void add( const string& s )
  {
    lines.push_back( s );
  }

  //REV: where do globals come from? I assume they are part of like, a "simulation". So...just at the root level model I guess? Does each model have its own globals?
  //Globals are like...tmp variables?
  //Can be set/found automatically for each update? I guess...
  void execute( const size_t& myidx, global_store& globals );
};



struct symmodel
  :
  public std::enable_shared_from_this<symmodel>

{
  updatefunct_t updatefunct;

  std::shared_ptr<symmodel> parent;
  
  string name="__ERROR_MODEL_NAME_UNSET";
  string localname="__ERROR_MODEL_LOCALNAME_UNSET";
  vector<string> type;
  
  //vector<symvar> vars;
  vector< std::shared_ptr<symvar> > vars;
  
  vector< std::shared_ptr<symmodel> > models;
  //vector<string> modelnames;
  //vector<string> modeltypes;

  vector<hole> holes;

  std::vector< std::shared_ptr<corresp> > correspondences;

  size_t modelsize = 0; //All models have size? It is shared by all submodels and parent models.

  std::shared_ptr<generator> gen;

  /*
  global_store globalparams; //should never be accessed for model, should only be convenience at highest level circuit. Shoudl really make derivative with thisl...

  global_store getglobals()
  { return globalparams; }

  void addfparam( const string& lname, const vector<real_t>& val )
  { globalparams.addfparam( lname, val ); }

  void addiparam( const string& lname, const vector<size_t>& val )
  { globalparams.addiparam( lname, val ); }
  */
  
  void setgenformodel( const string& modelname, const generator& g );
  
  void notify_size_change( const size_t& i )
  {
    //some variable was pushed to...so size must have changed. Must update my size...
    auto top = get_toplevel_model();
    size_t currsize = top->modelsize;
    if( currsize < i )
      {
	top->modelsize = i;
      }
  }
    
  bool checkready()
  {
    bool ready=true;
    if( !checkcorrready() )
      {
	ready = false;
      }
    if( !checkvarsready() )
      {
	ready = false;
      }
    return ready;
  }
  
  bool checkcorrready();
  
  bool checkvarsready();
  
  //REV: fuck, it must be a CONN or we're fucked? ;)
  void fill_corresp( const std::shared_ptr<corresp>& hiscorr );
  
  void notify_filled_corresp( const std::shared_ptr<symmodel>& targ );
  
  void addcorresp( const std::shared_ptr<symmodel>& targ );
    
  std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symvar>& s );
  
  std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& targ );
  
  bool check_same_toplevel_model( const std::shared_ptr<symmodel>& targmodel )
  {
    if( get_toplevel_model() == targmodel->get_toplevel_model() )
      {
	return true;
      }
    return false;
  }
  
  std::shared_ptr<symmodel> get_toplevel_model()
    {
      if( parent && parent->parent )
	{
	  return parent->get_toplevel_model();
	}
      else
	{
	  return shared_from_this();
	}
    }
  
  
  
  std::shared_ptr<symmodel> makederived( const string& _myname, const string& _mytypes )
    {
      auto newmodel = clone();
      newmodel->name = _myname;
      newmodel->addtypes( _mytypes );

      return newmodel;
    }
  
  //REV: Whoa, ghetto. Better to have user make functions that "Derive" from it. But then...meh they're derived.
  //Make a shared pointer from my data, including all parent etc. reset
  std::shared_ptr<symmodel> clone();
  
  static std::shared_ptr<symmodel> Create( const string& s,  const string& t, const string& lname="__ERROR_MODEL_LOCALNAME_UNSET" )
  {
    auto ptr = std::make_shared<symmodel>( s, t, lname );
    //ptr->set_updatefunct( ptr );
    //REV: haha this will actually work? Don't need to make stack object symmodel tmp(s, t)?
    return ptr;
  }
  
 symmodel( const string& s, const string& t, const string& lname )
   : name( s), type( parsetypes( t ) ), localname( lname )
  {
    //can't do this because not constructed self yet for this.
    //updatefunct = updatefunct_t( shared_from_this() );
    //addtypes( t );
  }

  
  
  void set_updatefunct( const updatefunct_t& uf )
  {
    updatefunct = uf; //copy
    updatefunct.model = shared_from_this();
  }
  
  void add_to_updatefunct( const string& s )
  {
    //If my updatefunct is still null
    if( !updatefunct.model )
      {
	updatefunct.model = shared_from_this();
      }
    
    updatefunct.add(s);
  }
  
  void addvar( const string& s, const string& t )
  {
    vars.push_back( std::make_shared<symvar>(s, t, shared_from_this() ) ); //std::shared_ptr<symmodel>(this)) );
  }


  void addfvars( const string& s, const string& t, const vector<real_t>& f );

  void addivars( const string& s, const string& t, const vector<size_t>& i );
  
  void addhole( const string& s )
  {
    holes.push_back( hole(s, shared_from_this()) ); //std::shared_ptr<symmodel>(this) ) );
  }

  void addtypes( const string& t )
  {
    vector<string> parsed = parsetypes( t );
    for(size_t p=0; p<parsed.size(); ++p)
      {
	type.push_back( parsed[p] );
      }
      
  }

  
  



  
  void addmodel( const std::shared_ptr<symmodel>& m, const string& _localname )
  {
    string newmodelname;
    
    auto realmodel = get_containing_model( _localname, newmodelname );

    auto modelclone = m->clone();
    
    realmodel->models.push_back( modelclone );
  
    modelclone->parent = realmodel;
    modelclone->localname = newmodelname;
  
  }

  //REV; problem, this does not iterate inside models into hierch, i.e. MODEL/BLAH/HOLE
  hole& gethole( const string& h )
    {
      //needs to recursively find the hole?
      vector<size_t> holeidx = find_hole( h );
      if(holeidx.size() == 1)
	{
	  return holes[ holeidx[0] ];
	}
      else
	{
	  fprintf(stderr, "REV: ERROR in gethole [%s], no such hole exists in model [%s]\n", h.c_str(), buildpath().c_str() );
	  exit(1);
	}
    }


  vector<size_t> find_hole( const string& h )
    {
      vector<size_t> r;
      for(size_t n=0; n<holes.size(); ++n )
	{
	  if( holes[n].name.compare( h ) == 0 )
	    {
	      r.push_back(n);
	    }
	}
      return r;
    }

  vector<size_t> find_model( const string& h )
    {
      vector<size_t> r;
      for(size_t n=0; n<models.size(); ++n )
	{
	  if( models[n]->localname.compare(h) == 0 )
	    {
	      r.push_back(n);
	    }
	}
      return r;
    }

  void fillemptymodels( );
			 
  
  void fillhole( const string& h, const std::shared_ptr<symmodel>& modeltofillwith )
  {
    vector<string> parsed = parse( h );
    fillhole( parsed, modeltofillwith );
  }
  
  
  //REV: corresp are only added when I "fill hole" with other!!!!
  //I should do it whenver I "add corresp"
  //Furthermore, whenever I "fill corresp" make sure it copies back ;)
  void fillhole( const vector<string>& h, const std::shared_ptr<symmodel>& modeltofillwith )
  {
    vector<string> parsed = h;
    if( parsed.size() == 0 )
      {
	fprintf(stderr, "ERROR in fillhole, parsed size is zero, hole doesn't exist?\n"); //, hole.c_str());
      }
    else if( parsed.size() == 1 )
      {
	string holename = parsed[0];
	vector<size_t> holeidxs = find_hole( holename );
	if(holeidxs.size() == 1 )
	  {
	    size_t holeidx = holeidxs[0];
	    //fill it
	    holes[ holeidx ].add( modeltofillwith );

	    //REV: ADD CORRESP FROM THIS
	    addcorresp( modeltofillwith ); //or I could have lots of corresp pointers sitting around...
	    //NOTE NEED TO PUSH TO OTHER SIDE TOO!
	    //Done in add corresp
	    //modeltofillwith->addcorresp( shared_from_this() );
	  }
	else
	  {
	    //REV: Ugh, keep around what the starting point was?
	    fprintf(stderr, "ERROR in fill hole, hole (holeidxs.size() != 1), [%s] doesn't exist (in model [%s])\n", holename.c_str(), localname.c_str());
	    exit(1);
	    
	  }
      }
    else
      {
	string submodel = parsed[0];
	vector<size_t> locs = find_model( submodel ); //REV: this will "look up the hierarchy" fuck...
	
	if(locs.size() == 1)
	  {
	    size_t mloc = locs[0];
	    parsed.erase( parsed.begin() );
	    
	    models[ mloc ]->fillhole( parsed, modeltofillwith );
	    
	  }
	else
	  {
	    fprintf(stderr, "ERROR in fillhole, submodel of current model doesn't exist [%s]\n", submodel.c_str() );
	    exit(1);
	  }
      }
  } //end fillhole
  

  //OK, so it tries to resolve it now within the circuit. This is good. If we just
  //fed a "free" local model, it wouldn't know to try to e.g. push it back or whatever.
  void fillhole( const string& h, const string& modeltofillwith )
  {
    std::shared_ptr<symmodel> model = get_model( modeltofillwith );
    fillhole( h, model );
    
  }

  bool is_submodel( const std::shared_ptr<symmodel>& s );
  
  //check if model of type, is CONJUNCTION of all types
  bool model_is_of_type( const string& t )
  {
    
    vector<string> types = parsetypes( t );
    for(size_t x=0; x<types.size(); ++x)
      {
	bool found=false;
	for(size_t modt=0; modt<type.size(); ++modt)
	  {
	    if( type[modt].compare( types[x] ) == 0 )
	      {
		found=true;
	      }
	  }
	if( !found ) //found at least one type that I am not.
	  {
	    return false;
	  }
      }
    return true;
  }
  
  vector< std::shared_ptr<symmodel> > find_models_of_type( const string& t )
    {
      vector< std::shared_ptr<symmodel> > ret;
      for(size_t m=0; m<models.size(); ++m)
	{
	  //checks localtype? Or actual model type?
	  if( models[m]->model_is_of_type( t ) == true )
	    {
	      ret.push_back( models[m] );
	    }
	}
      return ret;
    }
    
  
  //Will this search all "holes" too?? Or only sub-models...? And only one layer down? This will look at "type" of model
  void fillhole_fromtype( const string& hole, const string& modeltofillfrom, const string& t )
  {
    //Does same thing, but fills it with all models of a given type
    //does so by iterating through all models (not all holes!) and filling from type.

    std::shared_ptr<symmodel> model = get_model( modeltofillfrom );
    
    vector< std::shared_ptr<symmodel> > modelsoftype = model->find_models_of_type( t );

    for( size_t m=0; m<modelsoftype.size(); ++m )
      {
	fillhole( hole, modelsoftype[m] );
      }
    
    //Search models of target model, for those of TYPE, and fill hole.
  }
  
  

  //REV: search for hole too?
  std::shared_ptr<symmodel> get_containing_model( const string& n, string& varname )
    {
      vector<string> parsed = parse( n );
      if( parsed.size() == 0 )
	{
	  fprintf(stderr, "Error trying to get containing model for variable path [%s]\n", n.c_str() );
	  exit(1);
	}
      varname = parsed[ parsed.size()-1 ];
      parsed.pop_back();

      return get_model( parsed );
    }
  
  
  std::shared_ptr<symmodel> get_model( const string& n )
    {
      vector<string> parsed = parse( n );
      return get_model( parsed );
    }

  void enum_sub_models()
  {
    fprintf(stderr, "--Submodels of model [%s]\n", localname.c_str() );
    for(size_t m=0; m<models.size(); ++m)
      {
	fprintf(stdout, "[%s] (modelname [%s])\n", models[m]->localname.c_str(), models[m]->name.c_str() );
      }
  }


  //elemptr get_containing_model_widx( const string& unparsed, const size_t& idx, const vector<elemptr>& trace, string& varname )
  elemptr get_containing_model_widx( const string& unparsed, const vector<size_t>& idx, const vector<elemptr>& trace, string& varname )
  {
    vector<string> parsed = parse( unparsed );
    return get_containing_model_widx( parsed, idx, trace, varname );
  }
  
  //elemptr get_containing_model_widx( const vector<string>& parsed, const size_t& idx, const vector<elemptr>& trace, string& varname )
  elemptr get_containing_model_widx( const vector<string>& parsed, const vector<size_t>& idx, const vector<elemptr>& trace, string& varname )
  {
    if(parsed.size() < 1)
      {
	fprintf(stderr, "REV: error in get containing model w idx, parsed size < 1\n");
	exit(1);
      }
    vector<string> popped = parsed;
    varname = popped[ popped.size()-1 ];
    popped.pop_back();
    
    return get_model_widx( popped, idx, trace );
    //I now need to find that model, which better contain var...
    //Note, will looking for a var bubble back up? Only in var name? Or..?
  }
  
  //elemptr get_model_widx( const string& unparsed, const size_t& idx, const vector<elemptr>& trace )
  elemptr get_model_widx( const string& unparsed, const vector<size_t>& idx, const vector<elemptr>& trace )
  {
    vector<string> parsed = parse( unparsed );
    return get_model_widx( parsed, idx, trace );
  }
  


  elemptr get_model_widx( const vector<string>& parsed, const vector<size_t>& idx, const vector<elemptr>& trace );
  
  //REV: This finds "variable" inside a model? Or it finds model?
  //REV; This "BUBBLES" all the way up to root! Note, it should only bubble to root-1
  //If model name is "", will it bubble up? No it will just return this heh.
  std::shared_ptr<symmodel> get_model( const vector<string>& parsed )
    {
      if( parsed.size() == 0 ) //< 1 )
	{
	  return shared_from_this(); //std::shared_ptr< symmodel > ( this );
	}
      else //if ( parsed.size() >= 1 )
	{
	  string submodel = parsed[0];
	  vector<size_t> locs = find_model( submodel );
	  vector<size_t> hlocs = find_hole( submodel );
	  vector<string> nparsed( parsed.begin()+1, parsed.end() );
	
	  if( locs.size() >= 1 )
	    {
	      if(locs.size() > 1 )
		{
		  fprintf(stderr, "WTf found more than one [%s]\n", submodel.c_str() );
		  exit(1);
		}
	      size_t mloc = locs[0];
	    
	      return ( models[ mloc ]->get_model( nparsed ) );
	    }
	  else if( hlocs.size() == 1 )
	    {
	      size_t hloc = hlocs[0];
	    
	      if( holes[ hloc ].members.size() != 1 )
		{
		  fprintf(stderr, "ERROR in get_model, getting [%s] from HOLE, but hole [%s] has size [%lu], but it should be 1\n", submodel.c_str(), holes[hloc].name.c_str(), holes[hloc].members.size() );
		  exit(1);
		}
	      return ( holes[ hloc ].members[0]->get_model( nparsed ) ); //REV: It is external at some point fuck!!!!!!!!!!! I can check whether the returned "containing" model is
	      //external or not after the fact... but I need to always make sure it sends back the containing model with it I guess?
	    }
	  else
	    {
	      //Go up and redo.
	      //If root, stop
	      if( parent && !(parent->parent))
		{
		  fprintf(stderr, "REV: get_model, find model, model doesn't exist...[%s] (local model [%s]) (NOTE BUBBLED UP WHOLE HIERARCHY)\n", submodel.c_str(), localname.c_str());
		  enum_sub_models();
		  exit(1);
		}
	      else if( parent && parent->parent )
		{
		  return parent->get_model( nparsed );
		}
	      else
		{
		  fprintf(stderr, "Whoa, weird thing, parent->parent but not parent? Or something?\n");
		  exit(1);
		}
	    }
	}
    } //end get_model( vect<str> )
  
    

  vector<size_t> get_varloc( const string& s );

  bool is_toplevel()
  {
    if(parent && parent->parent)
      {
	return false;
      }
    return true;
  }

  std::shared_ptr<symmodel> get_root()
    {
      std::shared_ptr<symmodel> top = get_toplevel_model();
      if( top->parent )
	{
	  return top->parent;
	}
      else
	{
	  fprintf(stderr, "REV: something is fucked, top level is not top level, in get root\n");
	  exit(1);
	}
    }
  
  size_t get_modelsize()
  {
    auto toplevel = get_toplevel_model();
    return toplevel->modelsize;
  }
  
  void update( global_store& globals );
  
  //REV: meh these should be shared ptrs too?
  //std::shared_ptr<symvar> getvar_widx( const string& s, const size_t& idx, const vector<elemptr>& trace )
  std::shared_ptr<symvar> getvar_widx( const string& s, const vector<size_t>& idx, const vector<elemptr>& trace );
  

  
  void prefixprint( size_t depth=0 )
  {
    for(size_t d=0; d<depth; ++d)
      {
	fprintf(stdout, "-");
      }
  }

  //Checks all variables referenced in update function, and ensures they exist (and gives location they exist at)
  //ONLY UPDATES MINE, DOES NOT RECURSE.
  void check_update_funct( global_store& globals)
  {
    //literally runs it and tries to read each variable ;)
    //If it can't find it, it exits...
    updatefunct.execute( 0, globals );
  }

  string buildpath( )
  {
    vector<string> path;
    path.push_back( localname );
    
    std::shared_ptr<symmodel> model = parent;
    while( model )
      {
	path.push_back( model->localname );
	model = model->parent;
      }

    string finalpath="";
    for(size_t c=path.size()-1 ; ( c>=0 && c<path.size() ) ; --c)
      {
	finalpath = finalpath + "/" + path[c];
      }

    return finalpath;
  }


  //Need to know which variables are referenced in my update function!!!!! So I know that I can actually find them and that they exist in the target models!!!!
  //Variables "read" in update function
  //Variables "written" in update function
  //Verified location. Since I only reference "foreign models", I need to know which ones I will actually read/write and ensure they exist ;)

  
  //Enumerates all (at a global scope) variables, and recursively does it for all my sub-models
  //All holes which are filled by models, have their global scope printed too.
  void check_and_enumerate( size_t depth , bool checkupdate );
  
}; //end STRUCT SYMMODEL



elemptr findmodel( const string& s, const vector<elemptr>& trace, global_store& globals );

varptr get_proper_var_widx( const string& varname, const vector<elemptr>& trace, global_store& globals );
void set_proper_var_widx(const string& varname, const vector<elemptr>& trace, global_store& globals, const varptr& vp );
void push_proper_var_widx(const string& varname, const vector<elemptr>& trace, global_store& globals, const varptr& vp, const vector<size_t>& topushascorr );
std::shared_ptr<corresp> getcorresp_forvar( const std::shared_ptr<symmodel>& curr, const std::shared_ptr<symmodel>& targ, const std::shared_ptr<symvar>& var );
std::shared_ptr<corresp> getcorresp_forvar( const std::shared_ptr<symmodel>& targ, const vector<elemptr>& trace, const std::shared_ptr<symvar>& var );
std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& curr, const std::shared_ptr<symmodel>& targ);
std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& targ, const vector<elemptr>& trace );
elemptr get_curr_model( const vector<elemptr>& trace );
varptr exec_w_corresp( const std::string& toexec, const std::shared_ptr<symmodel>& m, const vector<elemptr>& trace, cmdstore& cmds, global_store& globals );
elemptr get_model_widx( const string& parsearg, const vector<elemptr>& trace );
elemptr get_containing_model_widx( const string& parsearg, const vector<elemptr>& trace, string& varname );
string get_containing_model_path( const string& parsearg, string& vartail );

