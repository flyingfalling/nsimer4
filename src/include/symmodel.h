//REV: insteresting problem, since I am making "types" (items) it is literally getting a pointer to it rather than copying it.
//If I just had an object, it would do the full object copy.
//So, either I need the "constructor" for the object instances to build the appropriate structure
//E.g. BUILD_ADEX( xxx ), which makes a new symmodel with correct submodels
//*OR* I need to have a deep-copy constructor, which not only copies, but also constructs new instances of all submodels?
//In other words, recursively call "deep copy"
//How will that work? When I call it, will the vector<symmodelptr> also deep copy same "pointer" locations, or will it make a new xyzpos etc.?

//Note, when it goes and constructs the "children", it needs to set to me.


#pragma once

#include <commontypes.h>
#include <fparser.h>
#include <parsehelpers.h>

#include <sys/types.h>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <string>

#include <algorithm>
#include <memory>

using std::vector;
using std::string;

//forward decl struct symmodel
struct symmodel;
struct cmdstore;

typedef std::function< real_t( const string&, std::shared_ptr<symmodel>&, const cmdstore& ) > cmd_functtype;


vector<string> parse( const string& name);
vector<string> parsetypes( const string& name);


struct symvar
{
  string name;
  string type;
  
  real_t valu;
  
  size_t read=false;
  size_t written=false;

  std::shared_ptr<symmodel> parent;
  
  void reset()
  {
    read=0;
    written=0;
  }

  void readvar()
  {
    ++read;
  }

  void writevar()
  {
    ++written;
  }
  
  //Default is "my location"
symvar( const string& n, const std::shared_ptr<symmodel>& p )
: name(n), parent(p)
  {
  }
  
symvar( const string& n, const string& t, const std::shared_ptr<symmodel>& p  )
: name(n), type(t), parent(p)
  {
  }

}; //end struct symvar


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


//stringify macro
#define STR( _mystring )  #_mystring

struct cmdstore
{
  vector< string > functnames;
  vector< cmd_functtype > functs;


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






real_t DOCMD( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t READ( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t SET( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t SUM( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t MULT( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t DIV( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t DIFF( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t NEGATE( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t EXP( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t SUMFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
real_t MULTFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );




struct updatefunct_t
{
  vector<string> lines;
  cmdstore cmds;
  std::shared_ptr<symmodel> model;

  updatefunct_t( const std::shared_ptr<symmodel>& m )
  : model( m )
  {
  }

  //dummy (nothing)
  updatefunct_t()
  {
  }
  
  void add( const string& s )
  {
    lines.push_back( s );
  }

  void execute()
  {
    for(size_t c=0; c<lines.size(); ++c)
      {
	DOCMD( lines[c], model, cmds );
      }
  }
};



//Normal equals constructors are POINTER equal, i.e. just get pointer to it.
//Only in case of "addmodel" do we invoke DEEP COPY.
struct symmodel
  :
  public std::enable_shared_from_this<symmodel>

{
  updatefunct_t updatefunct;

  std::shared_ptr<symmodel> parent;
  
  string name="__ERROR_MODEL_NAME_UNSET";
  string localname="__ERROR_MODEL_LOCALNAME_UNSET";
  vector<string> type;
  
  vector<symvar> vars;
    
  vector< std::shared_ptr<symmodel> > models;
  //vector<string> modelnames;
  //vector<string> modeltypes;

  vector<hole> holes;

  std::shared_ptr<symmodel> makederived( const string& _myname, const string& _mytypes )
    {
      auto newmodel = clone();
      newmodel->name = _myname;
      newmodel->addtypes( _mytypes );

      return newmodel;
    }
  
  //REV: Whoa, ghetto. Better to have user make functions that "Derive" from it. But then...meh they're derived.
  //Make a shared pointer from my data, including all parent etc. reset
  std::shared_ptr<symmodel> clone()
  {
    //Whoa, ghetto copy of object DATA. Will do deep copy to get contained pointed to data.
    auto newmodel = std::make_shared<symmodel>( *(shared_from_this()) );

    //Overwrite new stuff by making new recursively.
    //updatefunct is same
    newmodel->parent.reset();
    //name is same
    newmodel->localname="__ERROR_MODEL_LOCALNAME_UNSET";
    //type is same

    vector<std::shared_ptr<symmodel>> oldsubmodels = newmodel->models;
    newmodel->models.clear();
    
    for(size_t m=0; m<oldsubmodels.size(); ++m )
      {
	//REV: These are model pointers to OLD model locations. I need to do a deep copy for each one.
	auto newsub = oldsubmodels[m]->clone();

	newmodel->models.push_back( newsub );
	
	newsub->parent = newmodel;
	newsub->localname = oldsubmodels[m]->localname;
      }

    //holes are copied, but hole MEMBERS are not.
    //newmodel->holes.clear();
    for(size_t h=0; h<newmodel->holes.size(); ++h)
      {
	newmodel->holes[h].parent = newmodel;
	newmodel->holes[h].members.clear();
	newmodel->holes[h].external.clear();
      }

    return newmodel;
  }
  
  static std::shared_ptr<symmodel> Create( const string& s,  const string& t, const string& lname="__ERROR_MODEL_LOCALNAME_UNSET" )
  {
    //REV: haha this will actually work? Don't need to make stack object symmodel tmp(s, t)?
    return std::make_shared<symmodel>( s, t, lname );
  }
  
 symmodel( const string& s, const string& t, const string& lname )
   : name( s), type( parsetypes( t ) ), localname( lname )
  {
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
    vars.push_back( symvar(s, t, shared_from_this() ) ); //std::shared_ptr<symmodel>(this)) );
  }

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


  //This asks, is s a submodel of me?
  bool is_submodel( const std::shared_ptr<symmodel>& s )
  {
    std::shared_ptr<symmodel> model = s->parent;
    
    while( model )
      {
	if( model == shared_from_this() )
	  {
	    return true;
	  }
	model = model->parent;
      }
    return false;
  }
  
  
  //Is this filling a hole? E.g. with an external model?
  //At what point do I actually "resolve" all variables/models?
  //Can adding models actually be external? No, they can't. Those must be holes? No, they can be external? If they're connected?
  //What's the point of a hole? Something that MIGHT be external or interanl? OK... And might be of size N.
  //Is pos a hole? It is filled by a local model pos
  //When I specify var...yea it's just single thing.
  //Can holes be for variables too? Or are they model holes? Separate holes? Make all holes just variables? No, models...
  //We need to know when to update. If we only reference variables, we don't know when they've been updated. They must only be parameters?

  
  
  void addmodel( const std::shared_ptr<symmodel>& m, const string& _localname )
  {
    //not a pointer, I assume? Pushes a COPY of it?
    //SHIT
    //models.push_back( m );

    //REV: need to get what would be the containing model! Then push it.

    string newmodelname;

    //REV: Ah, get containing model will return what? Localname will just be gL.
    //So, containing model should be "this"
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
	//if( models[n]->name.compare( h ) == 0 )
	//if( modelnames[n].compare( h ) == 0 )
	if( models[n]->localname.compare(h) == 0 )
	  {
	    r.push_back(n);
	  }
      }
    return r;
  }

  //REV; this could get nasty because I don't know "where" I am referencing it from?
  //Assume it always goes up to "maximum"fd location, i.e. root of all models
  //referenced? Root I'm filling it from...? I.e. raises parents until it is zero,
  //comparing at each point, and then BAM. For now it's just a string, since the
  //target model might not be added yet (hm?). Do I always need to make sure it has
  //been added already so that I can resolve it? That seems much safer...
  //In other words, I literally just get "pointers" to models at this point in
  //time. Much safer ;) But the way it finds the pointer is it is resolved at this
  //point in time.

  void fillhole( const string& h, const std::shared_ptr<symmodel>& modeltofillwith )
  {
    vector<string> parsed = parse( h );
    fillhole( parsed, modeltofillwith );
  }
  

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
	  }
	else
	  {
	    fprintf(stderr, "ERROR in fill hole, hole (holeidxs.size() != 1), [%s] doesn't exist (in model [%s])\n", holename.c_str(), localname.c_str());
	    exit(1);
	  }
      }
    else
      {
	string submodel = parsed[0];
	vector<size_t> locs = find_model( submodel );
	
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
  
  //REV: This finds "variable" inside a model? Or it finds model?
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
	
	if( locs.size() >= 1 )
	  {
	    if(locs.size() > 1 )
	      {
		fprintf(stderr, "WTf found more than one [%s]\n", submodel.c_str() );
		exit(1);
	      }
	    size_t mloc = locs[0];
	    
	    //Strip off the first part.
	    //parsed.erase( parsed.begin() );
	    vector<string> nparsed( parsed.begin()+1, parsed.end() );
	    return ( models[ mloc ]->get_model( nparsed ) );
	  }
	else if( hlocs.size() == 1 )
	  {
	    size_t hloc = hlocs[0];
	    
	    //Strip off the first part.
	    //parsed.erase( parsed.begin() );
	    vector<string> nparsed( parsed.begin()+1, parsed.end() );
	    //return ( models[ mloc ].get_model( nparsed ) );
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
	    fprintf(stderr, "REV: get_model, find model, model doesn't exist...[%s] (local model [%s])\n", submodel.c_str(), localname.c_str());
	    enum_sub_models();
	    exit(1);
	  }
      }
  } //end get_model( vect<str> )
  
    

  size_t get_varloc( const string& s )
  {
    for(size_t x=0; x<vars.size(); ++x)
      {
	if( s.compare( vars[x].name ) == 0 )
	  {
	    return x;
	  }
      }

    fprintf(stderr, "REV: ERROR variable [%s] could not be found in this model [%s]\n", s.c_str(), name.c_str() );
    exit(1);
  }

  //REV: meh these should be shared ptrs too?
  symvar& getvar( const string& s )
  {
    //std::vector<string> parsed = parse( s );
    string varname;
    std::shared_ptr<symmodel> containingmodel = get_containing_model( s, varname );
    size_t loc = containingmodel->get_varloc( varname );
    return vars[ loc ];
  }

  symvar& readvar( const string& s )
  {
    getvar( s ).readvar();
    return getvar(s);
  }

  void setvar( const string& s, const real_t& v )
  {
    getvar( s ).writevar();
    return;
  }

  void prefixprint( size_t depth=0 )
  {
    for(size_t d=0; d<depth; ++d)
      {
	fprintf(stdout, "-");
      }
  }

  //Checks all variables referenced in update function, and ensures they exist (and gives location they exist at)
  //ONLY UPDATES MINE, DOES NOT RECURSE.
  void check_update_funct()
  {
    //literally runs it and tries to read each variable ;)
    //If it can't find it, it exits...
    updatefunct.execute();
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
  void check_and_enumerate( size_t depth = 0 )
  {
    fprintf(stdout, "\n");
    prefixprint( depth );
    fprintf( stdout, "MODEL [%s] (modelnametype [%s])\n", localname.c_str(), name.c_str() );

    prefixprint( depth );
    fprintf( stdout, "=TYPES: ");
    for(size_t t=0; t<type.size(); ++t)
      {
	fprintf(stdout, "[%s]", type[t].c_str() );
      }
    fprintf(stdout, "\n");

    prefixprint( depth );
    //Print HOLES and their filled models?
    //Print "path" to highest parent of every model by tracing parent until NULL?
    fprintf(stdout, "=HOLES:\n");
    for(size_t h=0; h<holes.size(); ++h)
      {
	prefixprint( depth );
	fprintf(stdout, "  [%s], filled by [%lu] members\n", holes[h].name.c_str(), holes[h].members.size() );

	for(size_t m=0; m<holes[h].members.size(); ++m)
	  {
	    prefixprint( depth );
	    fprintf(stdout, "   [%lu]: [%s]\n", m, holes[h].members[m]->buildpath().c_str() );
	  }
      }

    prefixprint( depth );
    fprintf(stdout, "++checking update function for variable reference resolution\n");
    check_update_funct();
    
    prefixprint( depth );
    fprintf(stdout, "=SUBMODELS:\n");
    //prefixprint( depth );
    for(size_t subm=0; subm<models.size(); ++subm)
      {
	size_t jump=5;
	models[subm]->check_and_enumerate( depth+jump );
      }
    

  } //end check_and_enumerate
  
}; //end STRUCT SYMMODEL




//EOF symmodel.h
