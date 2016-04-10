//REV: insteresting problem, since I am making "types" (items) it is literally getting a pointer to it rather than copying it.
//If I just had an object, it would do the full object copy.
//So, either I need the "constructor" for the object instances to build the appropriate structure
//E.g. BUILD_ADEX( xxx ), which makes a new symmodel with correct submodels
//*OR* I need to have a deep-copy constructor, which not only copies, but also constructs new instances of all submodels?
//In other words, recursively call "deep copy"
//How will that work? When I call it, will the vector<symmodelptr> also deep copy same "pointer" locations, or will it make a new xyzpos etc.?

//Note, when it goes and constructs the "children", it needs to set to me.



//REV: At any rate, first, make sure that all correspondences are checked ;)
//How about for "recursive" access of guys? Will that ever happen?


//OK, finally, now whenever I am accessing a variable, I always access it VIA CORRESPONDENCE. Problem is, it goes "my model" to "its model". Am I "inside" a model now?
//The only time it should matter, is when I am accessing via holes. If I access it directly, it is literally an identity. So, I always, when I read a variable, will
//access it via model->getcorresp( get_containing_model( targvarname ) )



//OK, now I get corresopndences when I read/write variables.
//However, final problem is to actually use the correspondence.
//To use it, I must know
//1) index in "calling" model

//That is all?
//I.e. in 'actual' updates, I use an index to call with, but don't care what it is haha.
//Normally, I execute "for all members in model".
//It only matters for "read/write" variables.
//Oh shit, I could literally make a symmodel for every? Nah.
//Which has its own id? haha...
//No, just pass it through.


//REV: OK, final situation, we are passing indices and it automatically gets models, which is fine.
//It still doesn't access with corresps, because corresps always return VECTOR<IDXS> or VECTOR<VALS>

//In case where it is a one-to-many this will not work, as it is not clear what to get?
//Base case is to get only a single value?
//Obviously, we can just access vect[0] to get "main" value.
//However, in some cases, user will be iterating through it.
//Those will (by default?) be iterating through a HOLE!!!!!!!!!!!!!!
//i.e. READFORALL





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


typedef std::function< real_t( const string&, std::shared_ptr<symmodel>&, const cmdstore&, const size_t&, const size_t& ) > cmd_functtype;


vector<string> parse( const string& name);
vector<string> parsetypes( const string& name);


struct symvar
{
  string name;
  string type;
  
  //real_t valu;
  std::vector<real_t> valu;
  
  size_t read=false;
  size_t written=false;

  std::shared_ptr<symmodel> parent;

  bool init=false;
  
  real_t& get( const size_t& idx )
  {
    
  }
  
  real_t getvalu( const size_t& idx )
  {
    if( !init )
      {
	return 0;
      }
    
    if( idx >= valu.size() )
      {
	fprintf(stderr, "In symvar, getvalu, idx [%lu] > size of valu array [%lu], var name [%s] in containing model [%s]\n", idx, valu.size(), name.c_str(), parent->buildpath().c_str());
	exit(1);
      }

    return valu[idx];
  }

  void setvalu( const size_t& idx, const real_t& val )
  {
    if( !init )
      {
	//do nothing
      }
    
    if( idx >= valu.size() )
      {
	fprintf(stderr, "In symvar, setvalu, idx [%lu] > size of valu array [%lu], var name [%s] in containing model [%s]\n", idx, valu.size(), name.c_str(), parent->buildpath().c_str());
	exit(1);
      }

    valu[idx] = val;
  }
  
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

std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& curr, const std::shared_ptr<symmodel>& targ )
{
  std::shared_ptr<corresp> tmp;
  
  bool gotit = curr->getcorresp( targ, tmp );
  if(!gotit)
    {
      fprintf(stderr, "ERROR in some function execution, could not find required corresp between models [%s] and [%s]\n", curr->buildpath().c_str(), targ->buildpath().c_str() );
      exit(1);
    }
  
  return tmp;
}

std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& targ, const vector<elemptr>& trace )
{
  elemptr lastguy = get_curr_model(); //trace[trace.size()-1];
  
  return getcorresp( lastguy.model, targ );
}

elemptr get_curr_model( const vector<elemptr>& trace )
{
  if(trace.size() < 1)
    {
      fprintf(stderr, "REV: error in get_curr_model, trace is length 0\n");
      exit(1);
    }
  return trace[ trace.size()-1 ];
}

bool check_cmd_is_multi( const string& s )
{
  const string mult = "MULTFORALL";
  const string sum = "SUMFORALL";
  if( s.compare( mult ) == 0 || s.comapre( sum ) == 0 )
    {
      return true;
    }
  return false;
}

real_t exec_w_corresp( const std::string& toexec, const std::shared_ptr<symmodel>& m, const vector<elemptr>& trace, const cmdstore& cmds )
{
  //RE-parse toexec, to check what function it is.
  //If it is one of the given functions, then we go.

  vector<string> sanityparse = cmds.doparse( toexec );
  if ( sanityparse.size() != 1 )
    {
      fprintf(stderr, "Sanity parse failed in execwcorresp, it is comma sep\n");
      exit(1);
    }
  
  vector<string> parsed = cmds.fparse( toexec );
  if(parsed.size() < 1 || parsed[0].size() < 1)
    {
      fprintf(stderr, "ERROR: parsed size < 1 (or str len < 1). Exec [%s]\n", toexec.c_str());
      exit(1);
    }

  
  if( check_multi )
    {
      //Exec for all corresp.
      //curr model is already pushed back. We need toarget model)
      size_t arbitrary_idx = 666;
      elemptr tmpelem( m, arbitrary_idx ); //idx is arbitrary
      vector<elemptr> newtrace = trace;
      newtrace.push_back( tmpelem );

      //Note toexec is the whole thing passed, we didn't strip it.
      //we are just handling it in a special way based on #args and pushing back to
      //trace.
      //DOCMD will strip the first part and execute it.
      //In other words, in this csae it is sumforall, with one arg only.
      //So, it will strip SUMFORALL, pass it in, and appropriately handle it
      //as a DOCMDSUMCONNS
      return DOCMD( toexec, newtrace, cmds );
    }
  else //Otherwise, I just execute it. But, in that, I make sure that
    //the correspondence is not fucked up.
    {
      elemptr currmodel = get_curr_model();
      std::shared_ptr<corresp> corr = getcorresp( currmodel.model, m );
      size_t curridx = currmodel.idx;
      vector<size_t> c = corr.getall( curridx );
      if( c.size() != 1 )
	{
	  fprintf(stderr, "REV: SUPER ERROR in exec_w_corresp, trying to execute [%s] but model [%s]->[%s] is one-to-many (or zero) ([%lu])\n", toexec.c_str(), currmodel.model->buildtrace().c_str(), m->buildtrace().c_str(), c.size() );
	  exit(1);
	}
      size_t newidx = c[0];
      vector<elemptr> newtrace = trace;
      elemptr tmpelem( m, newidx );
      newtrace.push_back( tmpelem );

      return DOCMD( toexec, newtrace, cmds );
    }

  fprintf(stderr, "REV: error exec w corresp reached end somehow\n");
  exit(1);
} //end exec_w_corresp


elemptr get_model_widx( const string& parsearg, const vector<elemptr>& trace )
{
  
  elemptr lastguy = get_curr_model(); //trace[trace.size()-1];
  vector<elemptr> newtrace = trace;
  newtrace.pop_back();
  return lastguy.model->get_model_widx( parsearg, lastguy.idx, newtrace );
}

elemptr get_containing_model_widx( const string& parsearg, const vector<elemptr>& trace )
{
  
  elemptr lastguy = get_curr_model( trace );
  vector<elemptr> newtrace = trace;
  newtrace.pop_back();
  return lastguy.model->get_containing_model_widx( parsearg, lastguy.idx, newtrace );
}


//#define FUNCDECL( fname )   real_t fname( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )

//"easiest" way is to simply push-back on the end of the model trace. In that way, I will always use the last guy.
//So, instead of directly pushing, it just uses the last guy.
//Problem is, in the case of the other one, it takes a vector it's trying to find? It's "this one", i.e. always points to "calling" guy.
//So, the first time I call it, which model do I call it with? Ah, the base model I guess? Shit...no, it's called from the model I am updating. Ah...
//So, yea, it takes a vector.

//REV: yea, OK, easiest to just make the last one be the model/idx. Then when i call get_model_widx, I will use the end of that trace to do the calling I guess.
//Or, I could use it as a sanity check. Like, the last guy on me, should always be myself and my index. Note that then I have no need to pass index...
//Could just make global function, that wraps the calls.
#define FUNCDECL( fname )   real_t fname( const string& arg, const vector<elemptr>& trace, const cmdstore& cmds )

FUNCDECL(DOCMD);
FUNCDECL(READ);
FUNCDECL(SET);
FUNCDECL(SUM);
FUNCDECL(MULT);
FUNCDECL(DIV);
FUNCDECL(DIFF);
FUNCDECL(NEGATE);
FUNCDECL(EXP);
FUNCDECL(SUMFORALL);
FUNCDECL(MULTFORALL);

//real_t DOCMD( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t READ( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t SET( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t SUM( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t MULT( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t DIV( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t DIFF( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t NEGATE( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t EXP( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t SUMFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );
//real_t MULTFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds );




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

  void execute( const size_t& myidx )
  {
    for(size_t c=0; c<lines.size(); ++c)
      {
	DOCMD( lines[c], model, cmds, myidx );
      }
  }
};



struct elemptr
{
  std::shared_ptr<symmodel> model;
  size_t idx;

  elemptr( const std::shared_ptr<symmodel>& p, const size_t& i )
  : model( p ), idx( i )
  {
  }
};

//REV: what to do when I make a circuit a submodel of another? Is it possible?
//I guess. So, I guess all guys only go up until there is some "top model" type guy, which is different type or has a flag set.

struct corresp
{
  std::shared_ptr<symmodel> targmodel;

  bool init=false;

  corresp()
  {
  }

  corresp( const std::shared_ptr<symmodel>& t)
  : targmodel( t )
  {
  }
  
  size_t get( const size_t& s, const size_t& offset )
  {
    vector<size_t> g = get( s );
    if( offset >= g.size() )
      {
	fprintf(stderr, "REV: error requested offset larger than size\n");
	exit(1);
      }
    return g[ offset ];
  }
  
  real_t getvar( const size_t& s, const symvar& var, const size_t& offset )
  {
    vector<real_t> gv = getallvar( s, var );
    if(offset > gv.size())
      {
	fprintf(stderr, "REV: ERROR, getvar > offset\n");
	exit(1);
      }

    return gv[ offset ];
  }

  void markinit()
  {
    init=true;
  }
  
  bool initialized()
  {
    return init;
  }
  
  //REV: user never does a raw GET, they only use GETVAR.
  //However, creating new vect is a pain in the ass, so just return IDX directly
  virtual vector<size_t> getall( const size_t& s ) = 0;
  //virtual size_t get( const size_t& s, const size_t& offset ) = 0;
  virtual vector<real_t> getallvar( const size_t& s, const symvar& var ) = 0;
  //virtual real_t getvar( const size_t& s, const symvar& var, const size_t& offset ) = 0;
};


//Are they re-ordered? Possibly...same size?
//ONLY LOCAL MODELS SET, SO EVERYTHING ELSE IS A READ!! :)
struct identity_corresp : public corresp
{
  vector<size_t> getall( const size_t& s )
  {
    return vector<size_t>(1, s);
  }

  
  
  //size_t getvar( const size_t& s, const std::vector<real_t>& var )
  vector<real_t> getallvar( const size_t& s, const symvar& var )
  {
    //size_t idx = get(s); //REV: no need, it's identity.
    if( s >= var.valu.size() )
      {
	fprintf(stderr, "In identity corresp, error s > var size\n");
	exit(1);
      }

    return vector<real_t>(1, var.valu[s] );
  }
  

  identity_corresp( const std::shared_ptr<symmodel>& targ )
    :
  corresp( targ )
    {
      
    }
};



struct conn_corresp : public corresp
{
  std::vector<size_t> getall( const size_t& s )
  {
    if( s >= startidx.size() )
      {
	fprintf(stderr, "ERROR in get in conn_corresp, s >= startidx size\n");
	exit(1);
      }
    
    size_t start = startidx[s];
    size_t size = numidx[s];
    return std::vector<size_t>( correspondence.begin()+start, correspondence.begin()+start+size);
  }


  
  std::vector<real_t> getallvar( const size_t& s, const symvar& var )
  {
    std::vector<size_t> myidxs = get( s );
    std::vector<real_t> ret( myidxs.size() );
    for( size_t x=0; x<myidxs.size(); ++x )
      {
	if( myidxs[x] >= correspondence.size() )
	  {
	    fprintf(stderr, "REV: error, myidxs x > correspondences array size\n");
	    exit(1);
	  }
	
	size_t targ = correspondence[ myidxs[x] ];
	if( targ >= var.valu.size() )
	  {
	    fprintf(stderr, "REV: error, targ idx > var array size\n");
	    exit(1);
	  }
	
	
	ret[x] = var.valu[ targ ];
      }

    return ret;
  }

  
 conn_corresp( const std::shared_ptr<symmodel>& targ )
   : corresp( targ )
  {
  }
  
  std::vector<size_t> startidx;
  std::vector<size_t> numidx;
  std::vector<size_t> correspondence;
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
  
  //vector<symvar> vars;
  vector< std::shared_ptr<symvar> > vars;
  
  vector< std::shared_ptr<symmodel> > models;
  //vector<string> modelnames;
  //vector<string> modeltypes;

  vector<hole> holes;

  std::vector< std::shared_ptr<corresp> > correspondences;
  
  
  //Check that this is not top level?
  //When 'adding' hole, (holes are never added from holes)
  //we directly check corresp, and add if it doesn't exist.
  //What to do for "same" guys? It will return false, but return nothing? Shit.
  //Always returns "identity"? Which is a "type" of derived correspodnence? Fuck?!
  void addcorresp( const std::shared_ptr<symmodel>& targ )
  {
    //check that it does not exist, and add only if it does not.
    //Only operates on "top level" models!!
    std::shared_ptr<corresp> tmp;
    bool exists = getcorresp( targ, tmp );
    
    if( !exists )
      {
	std::shared_ptr<symmodel> thistop = get_toplevel_model();
	std::shared_ptr<symmodel> targtop = targ->get_toplevel_model();
	thistop->correspondences.push_back( std::make_shared<conn_corresp>( targtop ) );
      }
  } //end addcoresp
  
  bool getcorresp( const symvar& s, std::shared_ptr<corresp>& c )
  {
    return getcorresp( s.parent, c );
  }
  
  bool getcorresp( const std::shared_ptr<symmodel>& targ, std::shared_ptr<corresp>& c )
    {
      std::shared_ptr<symmodel> thistop = get_toplevel_model();
      std::shared_ptr<symmodel> targtop = targ->get_toplevel_model();
      
      if( thistop == targtop )
	{
	  c = std::make_shared<identity_corresp>( targtop );
	  return true; //it will try to add it? Fuck...
	}
      for(size_t x=0; x<thistop->correspondences.size(); ++x)
	{
	  if( thistop->correspondences[x]->targmodel == targtop )
	    {
	      c = thistop->correspondences[x];
	      return true;
	    }
	}
      return false;
    } //end getcorresp

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

    //need to make new vars. Make sure parent points correctly.
    //vector<std::shared_ptr<symvar>> oldvars = newmodel->vars;
    //newmodel->vars.clear();
    for(size_t v=0; v<newmodel->vars.size(); ++v )
      {
	//auto newvar = std::make_shared<symvar>( );
	newmodel->vars[v]->parent = newmodel;

	//newmodel->vars[v]->valu.clear(); //clear valu? if it had size, it is fucked.
      }
    
    vector<std::shared_ptr<symmodel>> oldsubmodels = newmodel->models;
    newmodel->models.clear();

    if( correspondences.size() > 0 )
      {
	fprintf(stderr, "REV: ERROR in CLONE, cloning but model clone of correspondences is not implemented! Do it!\n");
	exit(1);
      }
    
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
    vars.push_back( std::make_shared<symvar>(s, t, shared_from_this() ) ); //std::shared_ptr<symmodel>(this)) );
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

  


  //REV fuck ambiguous problem here

  //Get containing model is supposed to:
  //get the "next model up" given a string (i.e. bot model might not exist yet). So, if I try to add e.g. adex1/gAMPA/mod3/blah, but mod3 is in adex, not gAMPA, it will still
  //correctly create it in adex1/mod3? Hm. that doesn't seem right. In those cases, it will error out I guess...

  //For finding the containing model of a model that exists, I simply find the model, and then take "parent".

  //For finding the containing model of a variable that exists (?), I literally get that model location...problem is I want to find a variable. So I want to search for
  //it up the hierarchy. So, get_containing_model, would get the "ostensible" containing model, but then would look up the hierarchy from that model for the variable
  //(or in any passed through locations? No). So, at any rate, do I need to "check" that the variable is in containing model? No, because actually getting the variable
  //will be done by iterating up the thread.
  //The problem is, then the containing model would need to change.
  //So, they should be done together. In other words, getting the variable, should also get the correct containing model. So, finding the correct containing model,
  //should always return it. So, make a new funct.
  
  void addmodel( const std::shared_ptr<symmodel>& m, const string& _localname )
  {
    //not a pointer, I assume? Pushes a COPY of it?
    //SHIT
    //models.push_back( m );

    //REV: need to get what would be the containing model! Then push it.

    string newmodelname;

    //REV: Ah, get containing model will return what? Localname will just be gL.
    //So, containing model should be "this"
    //REV: This gets the containing model of the localname (as a VECTOR or VAR). Fuck that, literally just do, find the model, then model->parent type shit.
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
  

  //REV: Only DIRECTLY FILLS *THIS* specified model! I.e. does not "look up the hierarchy" for the hole to fill.
  //
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
	    modeltofillwith->addcorresp( shared_from_this() );
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


  //REV: does this literally get a model, or does it only blah? Ah, usually this will be "get containing model" type thing.
  //Note, a lot of time it will be to get the correspondence.
  
  
  //Do I return the model itself, or do I just append? I guess return the model, much easier. If I go deeper, I need to append to modeltrace, but pop off as I come back.

  //Do same for read var...basically I find the containing model...then bam.
  //Wait, I need to get back a symvar ptr too?
  //Yea, so I get model, idx, and symptr. That's great.
  //Get-containing-model of var type thing. Just strips off end.
  //Then gets the containing model. Then the only thing is to get the var from the model with idx. That is gotten as normal haha.

  //This also checks that variable actually exists in that contained model, rather than iterating up the trace.
  //Once I get the symvar, I can use symvar->parent to get actual containing model ;) OK, great.

  
  elemptr get_containing_model_widx( const string& unparsed, const size_t& idx, const vector<elemptr>& trace )
  {
    vector<string> parsed = parse( unparsed );
    return get_containing_model_widx( parsed, idx, trace );
  }
  
  elemptr get_containing_model_widx( const vector<string>& parsed, const size_t& idx, const vector<elemptr>& trace )
  {
    if(parsed.size() < 1)
      {
	fprintf(stderr, "REV: error in get containing model w idx, parsed size < 1\n");
	exit(1);
      }
    vector<string> popped = parsed;
    string varname = popped[ popped.size()-1 ];
    popped.pop_back();

    return get_model_widx( parsed, idx, trace );
    //I now need to find that model, which better contain var...
    //Note, will looking for a var bubble back up? Only in var name? Or..?
  }
  
  elemptr get_model_widx( const string& unparsed, const size_t& idx, const vector<elemptr>& trace )
  {
    vector<string> parsed = parse( unparsed );
    return get_model_widx( parsed, idx, trace );
  }
  
  //I am searching for the solution to MODEL1/HOLE1/HOLE2/MODEL3, etc., given start idx (beginning of trace)
  elemptr get_model_widx( const vector<string>& parsed, const size_t& idx, const vector<elemptr>& trace )
  {
    //same as get_model, but I always keep index around at each point, and when I return, I return a model and an index

    //I start with it already parsed.
    //If parsed.size() == 0, I simply return this (with an index?)
    if( parsed.size() == 0)
      {
	return elemptr( shared_from_this(), idx );
      }

    //This is the next model I will go into
    string submodel = parsed[0];
    vector<string> remainder( parsed.begin()+1, parsed.end() ); //same as remainder, remainder.erase(0);

    //REV: This is where I should iterate up the tree! This is the issue.
    vector<size_t> mlocs = find_model( submodel );
    vector<size_t> hlocs = find_hole( submodel );
    
    //At this point, we are finding the hole etc. normally.
    if( mlocs.size() >= 1 )
      {
	if( mlocs.size() > 1 )
	  {
	    fprintf(stderr, "WTF found more than one in getmodelwidx\n");
	    exit(1);
	  }
	size_t mloc = mlocs[0];
	//add model to trace? I guess? It is a submodel, so it is not necessary I guess? But it helps it find submodels I guess? Could this cause a problem?

	std::shared_ptr<symmodel> nextmodel = models[mloc];
	
	vector<elemptr> newtrace = trace;
	size_t idx_in_submodel = idx; //no change, b/c submodel.
	newtrace.push_back( elemptr( shared_from_this(), idx ) );
	
	return nextmodel->get_model_widx( nparsed, idx_in_submodel, newtrace );
      }
    else if( hlocs.size() >= 1 )
      {
	if( hlocs.size() > 1)
	  {
	    fprintf(stderr, "WTF more than one HOLE found in getmodelwidx\n");
	    exit(1);
	  }

	size_t hloc = hlocs[0];
	if( holes[ hloc ].members.size() != 1 )
	  {
	    fprintf(stderr, "ERROR in get_model_widx, getting [%s] from HOLE, but hole [%s] has size [%lu], but it should be 1\n", submodel.c_str(), holes[hloc].name.c_str(), holes[hloc].members.size() );
	    exit(1);
	  }

	//REV: At this point we need to be careful. We need to go through a correspondence if it is an external model. Maybe if it is an internal one. Not for now.

	std::shared_ptr<symmodel> nextmodel = holes[hloc].members[0];
	//Will this ever happen? I search submodels first, so probably not?
	//Note, this is not iterating UP the tree to find model names??? Shit...
	//If I do that, what happens? It eventually never finds it?
	if( check_same_toplevel_model( nextmodel ) )
	  {
	    vector<elemptr> newtrace = trace;
	    size_t idx_in_submodel = idx; //no change, b/c submodel.
	    newtrace.push_back( elemptr( shared_from_this(), idx ) );
	    
	    return nextmodel->get_model_widx( nparsed, idx_in_submodel, newtrace );
	  }
	else
	  {
	    //I NEED TO GO THROUGH A CORRESPONDENCE
	    //If I find it, but the corresp has multiple answers for me, then I quit.
	    //Note, index should specify which one to get directly right?
	    //FUCK FUCK NO NO NO IT WONT, bc ill tell it which one to get, and it
	    //will get it directly? No that will work...as long as I dont try to
	    //access it as e.g. BLAH/BLAH/MULTIVAR

	    std::shared_ptr<corresp> mycorresp;
	    bool exists = getcorresp( nextmodel, mycorresp );
	    if( !exists )
	      {
		fprintf(stderr, "REV: getcorresp in get_model_widx, failed, no such corresp exists between [%s] and [%s]\n", buildpath().c_str(), nextmodel->buildpath().c_str());
		exit(1);
	      }
	    
	    
	    //REV; SANITY, if corresp not allocated yet, just return 0.
	    size_t idx_in_submodel = 0;
	    //REV; Don't check this here, check this in the corresp struct? I.e. return dummy data if it is not existing yet (or exit?)
	    if(mycorrep->initialized())
	      {
		vector<size_t> sanity = mycorresp->getall( idx );
		if( sanity.size() != 1 )
		  {
		    fprintf(stderr, "SANITY check for corresp during access failed! Expected corresp for idx [%lu] of model [%s] to have only 1 corresponding element in model [%s], but it had [%lu]\n", idx, buildpath().c_str(), nextmodel->buildpath().c_str(), sanity.size() );
		    exit(1);
		  }
		size_t idx_in_submodel = sanity[0]; //no change, b/c submodel.
	      }

	    vector<elemptr> newtrace = trace;
	    newtrace.push_back( elemptr( shared_from_this(), idx ) );

	    return nextmodel->get_model_widx( nparsed, idx_in_submodel, newtrace );
	  }
      }
    else
      {
	//Else, try to bubble up to ROOT.
	//If I'm at root, I try to find in trace? Fuck...
	//It will try to match every sub-fucking model until it finds one lol.
	//If I try to bubble up to root, do I add it to the stream in the same way?
	//I guess so, same as a submodel heh...
	if( parent && (parent->parent) )
	  {
	    std::shared_ptr<symmodel> nextmodel = parent;
	    
	    vector<elemptr> newtrace = trace;
	    size_t idx_in_submodel = idx; //no change, b/c submodel.
	    newtrace.push_back( elemptr( shared_from_this(), idx ) );
	    
	    return nextmodel->get_model_widx( nparsed, idx_in_submodel, newtrace );
	    
	    nextmodel->get_model_widx( nparsed, idx_in_submodel, newtrace );
	  }
	else if(  parent && !(parent->parent) )
	  {
	    //ONLY if size of trace is zero do we exit?
	    if(trace.size() == 0)
	      {
		fprintf(stderr, "REV: Error, could not find in get_model_widx, even by bubbling up parents, or by jumping back model trace\n");
		exit(1);
		  
	      }
	  }
	else
	  {
	    fprintf(stderr, "REV; this should never happen weird, parent->parent but not parent. Exit\n");
	    exit(1);
	  }
      } //couldn't find in "else" (i.e. not in this model, so try bubbling up parents)

    if(trace.size() == 0)
      {
	fprintf(stderr, "Trace size zero\n");
	exit(1);
      }
    //Move back model and try again?
    vector<elemptr> newtrace = trace;
    size_t idx_in_submodel = trace[ newtrace.size() - 1].idx; //end of trace.
    std::shared_ptr<symmodel> nextmodel = trace[ newtrace.size() - 1].model;
    newtrace.pop_back();

    return nextmodel->get_model_widx( parsed, idx_in_submodel, newtrace );
    
  }
  
  //REV: This finds "variable" inside a model? Or it finds model?
  //REV; This "BUBBLES" all the way up to root! Note, it should only bubble to root-1
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
  
    

  vector<size_t> get_varloc( const string& s )
  {
    vector<size_t> loc;
    for(size_t x=0; x<vars.size(); ++x)
      {
	if( s.compare( vars[x]->name ) == 0 )
	  {
	    loc.push_back(x);
	  }
      }

    return loc;
    //fprintf(stderr, "REV: ERROR variable [%s] could not be found in this model [%s]\n", s.c_str(), name.c_str() );
    //exit(1);
  }

  bool is_toplevel()
  {
    if(parent & parent->parent)
      {
	return false;
      }
    return true;
  }


  
  
  
  //REV: meh these should be shared ptrs too?
  std::shared_ptr<symvar> getvar( const string& s )
  {
    //std::vector<string> parsed = parse( s );
    string varname;
    std::shared_ptr<symmodel> containingmodel = get_containing_model( s, varname );
    vector<size_t> loc = containingmodel->get_varloc( varname );

    if(loc.size() == 0 )
      {
	if( parent )
	  {
	    return parent->getvar( s );
	  }
	else
	  {
	    fprintf(stderr, "GETVAR, could not get variable through model hierarchy. Note I am now at root level so I do not kn ow model name...[%s]\n", s.c_str());
	    exit(1);
	  }
      }
    if(locs.size() > 1 )
      {
	fprintf(stderr, "REV weird more than one var of same name [%s]\n", s.c_str());
	exit(1);
      }

    //actualcontaining = shared_from_this();
    return vars[ loc[0] ];
  }

  std::shared_ptr<symvar> readvar( const string& s )
  {
    //std::shared_ptr<symmodel> tmp;
    getvar( s )->readvar();
    return getvar(s);
  }

  void setvar( const string& s, const real_t& v )
  {
    getvar( s )->writevar();
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
    updatefunct.execute( 0 );
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
    for(size_t subm=0; subm<models.size(); ++subm)
      {
	size_t jump=5;
	models[subm]->check_and_enumerate( depth+jump );
      }
    

  } //end check_and_enumerate
  
}; //end STRUCT SYMMODEL




//EOF symmodel.h
