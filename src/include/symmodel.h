//REV: 4 Apr 2016
//symmodel.h
//Symbolic model struct and helpers.

//REV: Alternate idea
// Do everything as classes etc. and make user build like that. Definitely easier/typesafe...and more control over CUDA compilation?
// allow direct specification of dx/dt?

//TODO: add way to specify things to do at end/beginning of turn (for example, setting V[t-1] to V[t].)
//TODO: add way to automatically determine dependencies among variables for update purposes (based on read/write during each update line of each model).
//TODO: make options so that I can do "even" updates i.e. spike schedulers.


//REV: current issue is that during generation, I have trouble with not-yet-existing variables/correspondences, which must be read when determining dependencies.
//Best idea is to simply go through in a dry run, create all temp vars, corresps, etc., but without actually doing anything (i.e. just return artificial values?)
//And without adding any "size" yet. In other words, just read, and add required corresps/models, but don't actually do any pushing.
//In other words, no actual initialization.
//We run into a big problem if some guy tries to read someone without initializing it.
//Assume user has (in general) correctly ordered WITHIN individual generators. Not necessary though...

//So, worst case um, I try to read from a correspondence that does not exist yet...?
//For a variable that doesn't exist yet! In that case, do I always create it (....fuck?).
//No! I don't care! If it doesn't exist, it will return identity or constant!!!!
//but, that's not right. Because it should not be identity/constant in most cases.
//Hm, best idea is to (by default), when I add any model, I recursively add a correspondence to EVERY FUCKING EVERY MODEL, both in globals
//and in the actual guy. That is fine. Let's do that ;) I could delete them afterwords...
//NO, that will cause errors because they are not "active"

//Note, shit, when I generate, those HIERARCY->GLOBAL TMP correspondences will not be dleted, but globals will be locally deleted.
//But, it should never be referenced again.

//OK, so the main issue is the difficulty of possibly trying to reference a not-yet-existing variable (solved but executing all ADDLOCAL first),
//and then the issue of trying to reference a not-yet-existing correspondence (NOT solved by adding it when I actually run the PUSH command, because a totally
//different function may try to read into that variable...before it is pushed back)
//So, before I created myprobs. So other guys will try to read myprobs (via a corresp to it), before it has been pushed. I.e. read before push.
//So, the result is to execute all pushes first. But, some pushes may try to read from a "pushed" guy, which would do the creating.
//So...I'm fucked? If I ever try to read from a correspondence that doesn't exist, create it. If the model the correspondence refers to does not exist,
//Create it? That...that is difficult. Yea, let's assume local ordering is always preserved.
//If so, we should never encounter issues? ok...so just read indiv vars. How far do I need to "read back" before there is a dependency?
//Literally take the "slots" between that and the previous dependency in that one as the stuff to do beforehand.

//OK, do it.

//For each line (?), I literally do the full read of MODEL vars only.
//In terms of set/write/push.
//write/set take precedence over read. I.e. all set/write must happen before all reads to a given variable.
//For each variable, keep track of what line/what gen it was written by. Problem is it will actually try to construct...? which will fuck everything up?
//How about just do everything, and then unalloc it later? That is way easier...
//So, it's not to check that everything is ready...? If source is not init, just do shit? It will never be init in some case? Shit...
//I need to tell it that this is like "test" mode...
//I just need to avoid pushing if that is set, that is all.
//Like just set vars *as if* I had pushed, but don't.
//And each guy will specify individual VARIABLES read/written by each line ;)
//I really don't like the "global" idea ;(

//I can't set a variable at the top level of circuit because then "globals" would not be able to read that.
//I could literally set a global variable, but that is way too ghetto.
//I could set a variable in CMDS, but again...


//Um, static variable? Not threadsafe lol fuck. Best to "pass" it along with something, like CMDS? Always pass same cmds along with everything?
//At any rate...what? I just don't push...I do add corrs though ;) lol. But without filling.

//No filling, no pushing.

//REV: yea...yea...I need a much better way than to have the separate globals...so that I can check together? Just have a static CMD shared by all I guess? yea...


//So, check if generating() == true (in variable or corresp)
//If so, I don't do anything.
//Do I add corresp? I might add it but not fill it...? Filling it is bad? Shit, then when will I fill it? Rofl... when I run it again what will I do?
//How will I undo everything? if I push back the local variable now, when I re-run it, when will it be pushed back? oh, it won't need to be, it is already there?
//Nah, but it might just be a local guy that disappears, so ...

//Will I ever actually build/run it after doing a check? Sure, for sanity check. So, in that case, what? Do I only generate the guys? What happens when I hit an "addvar"
//but it's already added? That's fine, I just ignore it ;)

//OK, at any rate, "adds" are all done first, fine.

//Then, push -> set -> read.
//Whenever something is read or written to variable, we set it. Just do it IN the variable itself...much easier...


#pragma once

#include <commontypes.h>
#include <fparser.h>
#include <parsehelpers.h>
#include <generator.h>

#include <cmdstore.h>

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





vector<string> parse( const string& name);
vector<string> parsetypes( const string& name);




struct varptr
{
  vector<real_t> valu;
  vector<size_t> idx; //necessary?
};



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

}; //end GLOBAL STORE




struct symvar
{
public:
  std::shared_ptr<symmodel> parent;
  string name;
  string type;

private:
  std::vector<real_t> valu;
  std::vector<size_t> ivalu;

  bool init=false;
  bool genmode=true;

  size_t read=0;
  size_t written=0;
  size_t pushed=0;




private:
  bool generating()
  {
    return genmode;
  }

  void donegenerating()
  {
    genmode = false;
  }
  
  bool isconst()
  {
    //only if size is 1????
    if( ivalu.size() == 1 || valu.size() == 1 )
      {
	return true;
      }
    return false;
  }


  bool isint()
  {
    return (ivalu.size() > 0);
  }

  
  real_t getvalu( const size_t& idx );
  vector<real_t> getvalus( const vector<size_t>& idx );

  size_t getivalu( const size_t& idx );
  vector<size_t> getivalus( const vector<size_t>& idx );
  

  void setvalu( const size_t& idx, const real_t& val );
  void setvalus( const vector<size_t>& idx, const vector<real_t>& val );
  
  void setivalu( const size_t& idx, const size_t& val );
  void setivalus( const vector<size_t>& idx, const vector<size_t>& val );

  void addivalu( const size_t& i);
  void addfvalu( const real_t& f);
    
public:
  bool isinit()
  {
    return init;
  }

  void markinit();
  
  varptr vgetvalus( const vector<size_t>& idx );
  void vsetvalus( const vector<size_t>& idx, const varptr& v );
  
  void addivalus( const vector<size_t>& i);
  void addfvalus( const vector<real_t>& f);

  void addvalus( const varptr& vp );
    
  
  void reset()
  {
    read=0;
    written=0;
    pushed=0;
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


struct elemptr
{
  std::shared_ptr<symmodel> model;
  vector<size_t> idx;

  //model is null...
  elemptr()
  {
  }
  
elemptr( const std::shared_ptr<symmodel>& p, const size_t& i )
: model( p ), idx( vector<size_t>(1,i) )
  {
  }

elemptr( const std::shared_ptr<symmodel>& p, const vector<size_t>& i )
: model( p ), idx( i )
  {
  }
};


elemptr findmodel( const string& s, const vector<elemptr>& trace, global_store& globals );

struct corresp
{
public:
  std::shared_ptr<symmodel> targmodel;
  std::shared_ptr<symmodel> parent;

private:
  bool init=false;
  bool genmode=true;
  
  size_t read=0;
  size_t written=0;
  size_t pushed=0;
  
  std::vector<size_t> startidx;
  std::vector<size_t> numidx;
  std::vector<size_t> correspondence;


public:
  void reset()
  {
    read=0;
    written=0;
    pushed=0;
  }
  
  bool generating()
  {
    return genmode;
  }
  
  void donegenerating()
  {
    genmode = false;
  }
  
  
  void push( const size_t& size, const vector<size_t>& topush )
  {
    ++pushed;
    
    startidx.push_back( corrspondence.size() );
    numidx.push_back( topush.size() );
    correpondence.insert( correspondence.end(), topush.begin(), topush.end () );
  }

  void markinit();
    
  bool isinit()
  {
    return init;
  }


  virtual void set( const vector<size_t>& idxs, const vector<size_t> newvals )
  {
    ++written;
    return;
  }

  vector<size_t> getall( const vector<size_t>& s )
  {
    //Only if not init? only if generating?
    if( !isinit() || generating() )
      {
	++read;

	//vector<size_t> tmp( 1, 0 );
	//return tmp;
      }
    
    vector<size_t> ret;
    for(size_t a=0; a<s.size(); ++a)
      {
	size_t idx = s[a];
	vector<size_t> toadd = getall(idx);
	for(size_t t=0; t<toadd.size(); ++t)
	  {
	    ret.push_back( toadd[t] );
	  }
      }
    return ret;
  }

  //Whenver I "would" fill it, I set "pushed" to true...?
  void fill( const vector<size_t>& arg );
  
  corresp()
  {
  }

corresp( const std::shared_ptr<symmodel>& t, const std::shared_ptr<symmodel>& p)
: targmodel( t ), parent(p )
  {
  }
  
    
private:
  virtual vector<size_t> getall( const size_t& s ) = 0;
}; //end struct corresp


//stringify macro
#define STR( _mystring )  #_mystring



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





std::shared_ptr<corresp> getcorresp_forvar( const std::shared_ptr<symmodel>& curr, const std::shared_ptr<symmodel>& targ, const std::shared_ptr<symvar>& var );

std::shared_ptr<corresp> getcorresp_forvar( const std::shared_ptr<symmodel>& targ, const vector<elemptr>& trace, const std::shared_ptr<symvar>& var );

std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& curr, const std::shared_ptr<symmodel>& targ );

std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& targ, const vector<elemptr>& trace );

elemptr get_curr_model( const vector<elemptr>& trace );

bool check_cmd_is_multi( const string& s );

varptr exec_w_corresp( const std::string& toexec, const std::shared_ptr<symmodel>& m, const vector<elemptr>& trace, cmdstore& cmds , global_store& globals);



//REV: trace will contain the most recent index? I guess? Why use trace????
//REV: trae contains last idx?
varptr get_proper_var_widx( const string& varname, const vector<elemptr>& trace, global_store& globals );

void set_proper_var_widx(const string& varname, const vector<elemptr>& trace, global_store& globals, const varptr& vp );

void push_proper_var_widx(const string& varname, const vector<elemptr>& trace, global_store& globals, const varptr& vp, const vector<size_t>& topushascorr );		   




elemptr get_model_widx( const string& parsearg, const vector<elemptr>& trace );

elemptr get_containing_model_widx( const string& parsearg, const vector<elemptr>& trace, string& varname );


string get_containing_model_path( const string& parsearg, string& vartail );

//#define FUNCDECL( fname )   real_t fname( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )

//"easiest" way is to simply push-back on the end of the model trace. In that way, I will always use the last guy.
//So, instead of directly pushing, it just uses the last guy.
//Problem is, in the case of the other one, it takes a vector it's trying to find? It's "this one", i.e. always points to "calling" guy.
//So, the first time I call it, which model do I call it with? Ah, the base model I guess? Shit...no, it's called from the model I am updating. Ah...
//So, yea, it takes a vector.

//REV: yea, OK, easiest to just make the last one be the model/idx. Then when i call get_model_widx, I will use the end of that trace to do the calling I guess.
//Or, I could use it as a sanity check. Like, the last guy on me, should always be myself and my index. Note that then I have no need to pass index...
//Could just make global function, that wraps the calls.
#define FUNCDECL( fname )   varptr fname( const string& arg, const vector<elemptr>& trace, cmdstore& cmds, global_store& globals )

FUNCDECL(DOCMD);
FUNCDECL(READ);
FUNCDECL(SET);
FUNCDECL(SUM);
FUNCDECL(MULT);
FUNCDECL(DIV);
FUNCDECL(DIFF);
FUNCDECL(NEGATE);
FUNCDECL(EXP);
FUNCDECL(GAUSSRAND);
FUNCDECL(UNIFORMRAND);

FUNCDECL(SUMFORALL);
FUNCDECL(MULTFORALL);

FUNCDECL(SUMFORALLHOLES);
FUNCDECL(SUMFORALLCONNS);
FUNCDECL(MULTFORALLHOLES);
FUNCDECL(MULTFORALLCONNS);

FUNCDECL( NEWLOCAL );
FUNCDECL( FORALL );
FUNCDECL( PUSHFORALL );
FUNCDECL( PUSH );


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

  //REV: where do globals come from? I assume they are part of like, a "simulation". So...just at the root level model I guess? Does each model have its own globals?
  //Globals are like...tmp variables?
  //Can be set/found automatically for each update? I guess...
  void execute( const size_t& myidx, global_store& globals )
  {
    for(size_t c=0; c<lines.size(); ++c)
      {
	vector<elemptr> trace;
	elemptr elem( model, myidx );
	trace.push_back( elem );
	DOCMD( lines[c], trace, cmds, globals );
      }
  }
};





//REV: what to do when I make a circuit a submodel of another? Is it possible?
//I guess. So, I guess all guys only go up until there is some "top model" type guy, which is different type or has a flag set.



//Are they re-ordered? Possibly...same size?
//ONLY LOCAL MODELS SET, SO EVERYTHING ELSE IS A READ!! :)
struct identity_corresp : public corresp
{
  vector<size_t> getall( const size_t& s )
  {
    return vector<size_t>(1, s);
  }
  
 identity_corresp( const std::shared_ptr<symmodel>& targ,  const std::shared_ptr<symmodel>& p )
   :
  corresp( targ, p )
    {
      
    }
  
 
};


struct const_corresp : public corresp
{
  vector<size_t> getall( const size_t& s )
  {
    return vector<size_t>(1, 0);
  }
  
 const_corresp( const std::shared_ptr<symmodel>& targ,  const std::shared_ptr<symmodel>& p )
    :
  corresp( targ, p )
  {
    
  }
};


struct conn_corresp : public corresp
{
  std::vector<size_t> getall( const size_t& s )
  {
    //DUMMY. Just return zero...because we have no way of knowing what it will be until it's actually filled
    if( isinit()  == false || generating() )
      {
	return vector<size_t>( 1, 0 );
      }
    else
      {
	if( s >= startidx.size() )
	  {
	    fprintf(stderr, "ERROR in get in conn_corresp, s >= startidx size\n");
	    exit(1);
	  }
	
	size_t start = startidx[s];
	size_t size = numidx[s];
	return vector<size_t>( correspondence.begin()+start, correspondence.begin()+start+size);
      }
  }


  
  void set( const vector<size_t>& idxs, const vector<size_t> newvals )
  {
    if(idxs.size() != newvals.size() )
      {
	fprintf(stderr, "corresp set, size idx neq newvals\n");
	exit(1);
      }
    
    for(size_t x=0; x<idxs.size(); ++x)
      {
	size_t myn = idxs[x];
	size_t myv = newvals[x];

	correspondence[myn] = myv;
	if( startidx[myn] != myn )
	  {
	    fprintf(stderr, "REV trying to SET in corresp, but it is not a LARGE SIDE\n");
	    exit(1);
	  }

	if( numidx[myn] != 1 )
	  {
	    fprintf(stderr, "REV trying to SET in corresp, but it is not SIZE=1 (i.e. it's not LARGE SIDE )\n");
	    exit(1);
	  }
      }
  }
  
  
 conn_corresp( const std::shared_ptr<symmodel>& targ ,const std::shared_ptr<symmodel>& p )
   : corresp( targ, p )
  {
  }
  
  
};


struct gendep
{
  std::shared_ptr<symvar> targvar;
  std::shared_ptr<generator> targen;
  vector<size_t> lines; //line number(s) that generate the dependencies?
};

//I need a way to run "dummy" generation including PUSH/PUSHFORALL to determine
//which set which. Note, generation may include SET, GEN, etc.
//WRITE includes PUSH/SET/PUSHFORALL. But I can't write until it exists.
//So, read, create, and write.


//Note "reset" is another type of generator :)


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

  size_t modelsize = 0; //All models have size? It is shared by all submodels and parent models.

  std::shared_ptr<generator> gen;

  global_store globalparams; //should never be accessed for model, should only be convenience at highest level circuit. Shoudl really make derivative with thisl...

  global_store getglobals()
  { return globalparams; }

  void addfparam( const string& lname, const vector<real_t>& val )
  { globalparams.addfparam( lname, val ); }

  void addiparam( const string& lname, const vector<size_t>& val )
  { globalparams.addiparam( lname, val ); }
  
  
  //REV: where are generators stored? I assume in this model?
  //Or are all generators stored in a single location?
  //But we won't use them again, we need to separate generation of size, generation of values, and
  //reset()
  //If size() not specified, problem, error!
  //If values() gen not specified, it is assumed that size solved that issue? I.e. co-generated?
  //But, then we can't re-draw.
  //If reset() not specified, uh, error!

  void setgenformodel( const string& modelname, const generator& g );
  
  //REV: crap, this will need to be done on the GPU too at generation time fuck -_-;

  //Set init to true...?
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
  
  bool checkcorrready()
  {
    bool corrready=true;
    for(size_t v=0; v<correspondences.size(); ++v)
      {
	if( !correspondences[v]->isinit() )
	  {
	    fprintf(stderr, "WARNING: checkcorrready(): model [%s] to model [%s] correspondence is not ready (var [%s])\n", buildpath().c_str(), correspondences[v]->targmodel->buildpath().c_str(), vars[v]->name.c_str() );
	    corrready=false;
	  }
      }
    //for all vars, for all corresp, are init?
    for( size_t m=0; m<models.size(); ++m)
      {
	if( !models[m]->checkcorrready() )
	  {
	    corrready=false;
	  }
      }
    return corrready;
  }
  
  bool checkvarsready()
  {
    bool varsready=true;
    for(size_t v=0; v<vars.size(); ++v)
      {
	if( !vars[v]->isinit() )
	  {
	    fprintf(stderr, "WARNING: checkvarsready(): model [%s] var [%s] not ready\n", buildpath().c_str(), vars[v]->name.c_str() );
	    varsready=false;
	  }
      }
    //for all vars, for all corresp, are init?
    for( size_t m=0; m<models.size(); ++m)
      {
	if( !models[m]->checkvarsready() )
	  {
	    varsready=false;
	  }
      }
    return varsready;
  }
  
  //REV: fuck, it must be a CONN or we're fucked? ;)
  void fill_corresp( const std::shared_ptr<corresp>& hiscorr )
  {
    //My indices ?????
    vector< vector<size_t> > mycorresps( get_modelsize() );
    
    vector<size_t> mydudes;
    for(size_t n=0; n<hiscorr->correspondence.size(); ++n)
      {
	size_t c=hiscorr->correspondence[n];

	if( c >= mycorresps.size() )
	  {
	    fprintf(stderr, "REV: error in mirroring correspondence...\n");
	    exit(1);
	  }

	mycorresps[c].push_back( n );
      }

    //this is MY parst.
    auto corr = getcorresp( hiscorr->parent );

    corr->correspondence.clear();
    corr->startidx.clear();
    corr->numidx.clear();
    for(size_t x=0; x<get_modelsize(); ++x)
      {
	corr->push( mycorresps[x].size(), mycorresps[x] );
	
	//corr->startidx.push_back( corr->correspondence.size() ); //curr end location.
	// corr->numidx.push_back( mycorresps[x].size() );
	//  corr->correspondence.insert( corr->correspondence.end(),
	//			     mycorresps[x].begin(),
	//			     mycorresps[x].end() );
      }
    
    //corr is now initialized!
    corr->markinit();
    return;
  }
  
  void notify_filled_corresp( const std::shared_ptr<symmodel>& targ )
  {
    //Call this to "mirror" from large-small on othe side.
    auto tmp = getcorresp( targ );
    tmp->markinit(); //I better be init haha;
    targ->fill_corresp( tmp );
    return;
  }
  
  void addcorresp( const std::shared_ptr<symmodel>& targ )
  {
    auto tmp = getcorresp( targ );
    
    if( !tmp )
      {
	std::shared_ptr<symmodel> thistop = get_toplevel_model();
	std::shared_ptr<symmodel> targtop = targ->get_toplevel_model();
	thistop->correspondences.push_back( std::make_shared<conn_corresp>( targtop, thistop ) );
	
	targ->addcorresp( shared_from_this() );
      }
  } //end addcoresp
  
  std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symvar>& s )
  {
    if( s->isconst() )
      {
	//Doesn't matter, just return a const guy or smthing.
	return std::make_shared<const_corresp>( s->parent, shared_from_this() );
      }
    if(! s->parent)
      {
	fprintf(stderr, "REV: error in getcorresp(var), var has no parent!\n");
	exit(1);
      }
    
    return getcorresp( s->parent );
  }
  
  std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& targ )
    {

      std::shared_ptr<corresp> c;
      
      fprintf(stdout, "Looking for corresp between [%s] and [%s]\n", buildpath().c_str(), targ->buildpath().c_str());
      std::shared_ptr<symmodel> thistop = get_toplevel_model();
      std::shared_ptr<symmodel> targtop = targ->get_toplevel_model();

      fprintf(stdout, "Will return...\n");
      if( thistop == targtop )
	{
	  c = std::make_shared<identity_corresp>( targtop, thistop );
	  //return true; //it will try to add it? Fuck...
	}

      else
	{

	  //Does it exist already? Wat?
	  for(size_t x=0; x<thistop->correspondences.size(); ++x)
	    {
	      if( thistop->correspondences[x]->targmodel == targtop )
		{
		  c = thistop->correspondences[x];
		}
	    }
	}
      
      return c;
      
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

    //COPY UPDATE FUNCTD
    newmodel->updatefunct.model = newmodel;

    vector< std::shared_ptr<symvar> > newvararray = newmodel->vars;
    newmodel->vars.clear();
    //VARIABLES ARE NOW SHARED PTRS, I NEED TO MAKE NEW ONES
    for(size_t v=0; v<newvararray.size(); ++v )
      {
	newmodel->vars.push_back( std::make_shared<symvar>( newvararray[v]->name, newvararray[v]->type, newmodel ) );
      }

    //Done
    
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

    if( correspondences.size() > 0 )
      {
	fprintf(stderr, "REV: ERROR in CLONE, cloning but model clone of correspondences is not implemented! Do it!\n");
	exit(1);
      }
    
    return newmodel;
  }
  
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


  void addfvars( const string& s, const string& t, const vector<real_t>& f )
  {
    addvar( s, t );
    vars[ vars.size() - 1 ]->addfvalus( f );
  }

  void addivars( const string& s, const string& t, const vector<size_t>& i )
  {
    addvar( s, t );
    vars[ vars.size() - 1 ]->addivalus( i );
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

  void fillemptymodels( )
  {
    size_t ms=get_modelsize();
    if( ms == 0 )
      {
	fprintf(stderr, "REV: error in symmodel;;fillemptymodels; model size is 0!\n");
	exit(1);
      }

    vector<real_t> tmp( ms, 0 );
    
    //iterate through all models' variables, fill with modelsize zero.
    for(size_t x=0; x<vars.size(); ++x)
      {
	if( !vars[x]->isinit() )
	  {
	    vars[x]->addfvalus( tmp );
	  }
      }

    for(size_t m=0; m<models.size(); ++m )
      {
	models[n]->fillemptymodels();
      }
  }
			 
  
  void fillhole( const string& h, const std::shared_ptr<symmodel>& modeltofillwith )
  {
    vector<string> parsed = parse( h );
    fillhole( parsed, modeltofillwith );
  }
  

  //REV: Only DIRECTLY FILLS *THIS* specified model! I.e. does not "look up the hierarchy" for the hole to fill.
  //


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
  


  elemptr get_model_widx( const vector<string>& parsed, const vector<size_t>& idx, const vector<elemptr>& trace )
  {
    //I start with it already parsed.
    //If parsed.size() == 0, I simply return this (with an index?)
    //The empty string indicates "this" model? No, when I parse it, I need to sep it, so an empty, will return in a zero-parse, of size zero, or nothing?
    //Either way, would result in same, so return ;)
    if( parsed.size() == 0 || parsed[0].compare("") == 0 )
      {
	fprintf(stdout, "FOUND MODEL! [%s]\n", buildpath().c_str() );
	elemptr t = elemptr( shared_from_this(), idx );
	return t;
      }
    else
      {
	fprintf(stdout, "Model [%s], attempting to find model name [%s] widx (note, trace size is [%lu])\n", buildpath().c_str(), CAT(parsed, "/").c_str(), trace.size());
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

	fprintf(stdout, "Model [%s], going through submodel [%s] to find [%s]\n", buildpath().c_str(), models[mloc]->localname.c_str(), CAT(remainder, "/").c_str() );
	
	std::shared_ptr<symmodel> nextmodel = models[mloc];

	//Don't add to trace because if same model, parent will cause infinite loop in combin with trace.
	//However
	//Problem is if I go through a hole, and the hole is the same model, that is the main problem
	vector<elemptr> newtrace = trace;
	//size_t idx_in_submodel = idx; //no change, b/c submodel.
	vector<size_t> idx_in_submodel = idx; //no change, b/c submodel.
	//newtrace.push_back( elemptr( shared_from_this(), idx ) );
	
	return nextmodel->get_model_widx( remainder, idx_in_submodel, newtrace );
      }
    else if( hlocs.size() >= 1 )
      {
	if( hlocs.size() > 1)
	  {
	    fprintf(stderr, "WTF more than one HOLE found in getmodelwidx\n");
	    exit(1);
	  }
	

	size_t hloc = hlocs[0];
	fprintf(stdout, "Model [%s], going through hole [%s] to find [%s]\n", buildpath().c_str(), holes[hloc].name.c_str(), CAT(remainder, "/").c_str());
	if( holes[ hloc ].members.size() != 1 )
	  {
	    fprintf(stderr, "ERROR in get_model_widx, getting [%s] from HOLE, but hole [%s] has size [%lu], but it should be 1\n", submodel.c_str(), holes[hloc].name.c_str(), holes[hloc].members.size() );
	    exit(1);
	  }
	
	std::shared_ptr<symmodel> nextmodel = holes[hloc].members[0];
	
	if( check_same_toplevel_model( nextmodel ) )
	  {
	    //Dont add to trace because its same model so infinite loop with going to parent.x
	    vector<elemptr> newtrace = trace;
	    //size_t idx_in_submodel = idx; //no change, b/c submodel.
	    vector<size_t> idx_in_submodel = idx; //no change, b/c submodel.
	    //newtrace.push_back( elemptr( shared_from_this(), idx ) );
	    
	    return nextmodel->get_model_widx( remainder, idx_in_submodel, newtrace );
	  }
	else //not same toplevel model
	  {
	    //I NEED TO GO THROUGH A CORRESPONDENCE
	    
	    //std::shared_ptr<corresp> mycorresp;
	    auto mycorresp  = getcorresp( nextmodel );
	    if( !mycorresp )
	      {
		fprintf(stderr, "REV: getcorresp in get_model_widx, failed, no such corresp exists between [%s] and [%s]\n", buildpath().c_str(), nextmodel->buildpath().c_str());
		exit(1);
	      }
	    
	    
	    //REV; SANITY, if corresp not allocated yet, just return 0.
	    //size_t idx_in_submodel = 0;
	    vector<size_t> idx_in_submodel(1,0);
	    
	    //REV; Don't check this here, check this in the corresp struct? I.e. return dummy data if it is not existing yet (or exit?)
	    if(mycorresp->isinit())
	      {
		//REV: TODO HERE, just return it directly, with new IDX_IN_SUBMODEL ;0
		//REV: this is it!!! This is where I 
		vector<size_t> sanity = mycorresp->getall( idx );
		/*if( sanity.size() != 1 )
		  {
		    fprintf(stderr, "SANITY check for corresp during access failed! Expected corresp for idx [%lu] of model [%s] to have only 1 corresponding element in model [%s], but it had [%lu]\n", idx, buildpath().c_str(), nextmodel->buildpath().c_str(), sanity.size() );
		    exit(1);
		  }
		size_t idx_in_submodel = sanity[0]; //no change, b/c submodel.
		*/

		idx_in_submodel = sanity;
	      }

	    vector<elemptr> newtrace = trace;
	    newtrace.push_back( elemptr( shared_from_this(), idx ) );

	    return nextmodel->get_model_widx( remainder, idx_in_submodel, newtrace );
	  }
      } //end if not found in HOLES (or submodels)
    else
      {
	fprintf(stdout, "Model [%s], walking up to parent [%s] to find [%s]\n", buildpath().c_str(), parent->localname.c_str(), CAT(parsed, "/").c_str());
	//Else, try to bubble up to ROOT.
	if( parent && (parent->parent) )
	  {
	    std::shared_ptr<symmodel> nextmodel = parent;
	    
	    vector<elemptr> newtrace = trace;
	    //size_t idx_in_submodel = idx; //no change, b/c submodel.
	    vector<size_t> idx_in_submodel = idx; //no change, b/c submodel.
	    //newtrace.push_back( elemptr( shared_from_this(), idx ) );
	    
	    return nextmodel->get_model_widx( parsed, idx_in_submodel, newtrace );
	  }
	else if(  parent && !(parent->parent) )
	  {
	    //Couldn't find it! Return empty elemptr...bad.
	    elemptr ep;
	    return ep;
	  }
	else
	  {
	    fprintf(stderr, "REV; this should never happen weird, Neither parent nor parent->parent? In searching for model with idx. Exit\n");
	    if( parent )
	      {
		fprintf( stderr, "Parent of me [%s] exists and is [%s]\n", buildpath().c_str(), parent->buildpath().c_str() );
	      }
	    else
	      {
		fprintf( stderr, "Parent does not exist... (note current model is [%s])!\n", buildpath().c_str() );
	      }
	    exit(1);
	  }
      } //couldn't find in "else" (i.e. not in this model, so try bubbling up parents)
    
    if(trace.size() == 0)
      {
	fprintf(stderr, "Trace size zero. This should never happen (should have been caught above)\n");
	exit(1);
      }

    //REV: Did I mess something up? First it should check through all guys directly to see if it is same model? I.e. if target model matches b/c we can use that idx.
    fprintf(stdout, "Couldn't find model [%s] in previous model trace [%s], so moving to next! (trace size is [%lu])\n", CAT(parsed,"/").c_str(), buildpath().c_str(), trace.size() );
    
    //Move back model and try again?
    vector<elemptr> newtrace = trace;
    //size_t idx_in_submodel = newtrace[ newtrace.size() - 1].idx; //end of trace.
    vector<size_t> idx_in_submodel = newtrace[ newtrace.size() - 1].idx; //end of trace.
    std::shared_ptr<symmodel> nextmodel = newtrace[ newtrace.size() - 1].model;
    newtrace.pop_back();

    fprintf(stdout, "Will now try to run with new trace size [%lu]\n", newtrace.size() );
    return nextmodel->get_model_widx( parsed, idx_in_submodel, newtrace );
    
  } //end get_model_widx
  
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
  
  void update( global_store& globals )
  {
    global_store localglobals = globals;
    
    //For all members, execute? And for all submodels
    for( size_t x=0; x<get_modelsize(); ++x)
      {
	updatefunct.execute(x, localglobals);
      }
    
    for(size_t subm=0; subm<models.size(); ++subm )
      {
	models[subm]->update( localglobals );
      }

    //Note this is not updating a single "x" at a time?
    //E.g. first adex1[1], then gAMPA1[1], then gNMDA[1], then after all that is
    //done, it does adex1[2], etc.
    //All parts should be able to be done asynchronously with no ill effects.
    
  }
  
  
  //REV: meh these should be shared ptrs too?
  //std::shared_ptr<symvar> getvar_widx( const string& s, const size_t& idx, const vector<elemptr>& trace )
  std::shared_ptr<symvar> getvar_widx( const string& s, const vector<size_t>& idx, const vector<elemptr>& trace )
  {
    //std::vector<string> parsed = parse( s );
    string varname;
    elemptr containingmodel = get_containing_model_widx( s, idx, trace, varname );
    fprintf(stdout, "getvar widx: found containing model for var [%s] (model is [%s])\n", s.c_str(), containingmodel.model->buildpath().c_str());
    vector<size_t> loc = containingmodel.model->get_varloc( varname );
    //fprintf(stdout, "REV: doen getting varloc of requested var, size  is [%lu]\n", loc.size() );
    if(loc.size() == 0 )
      {
	if( parent )
	  {
	    return parent->getvar_widx( s, idx, trace );
	  }
	else
	  {
	    fprintf(stderr, "GETVAR, could not get variable through model hierarchy. Note I am now at root level so I do not know model name...[%s]\n", s.c_str());
	    exit(1);
	  }
      }
    if(loc.size() > 1 )
      {
	fprintf(stderr, "REV weird more than one var of same name [%s]\n", s.c_str());
	exit(1);
      }

    //actualcontaining = shared_from_this();
    return containingmodel.model->vars[ loc[0] ];
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
  void check_and_enumerate( size_t depth , bool checkupdate )
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

    prefixprint(depth);

    global_store tmpglobals;
    if( checkupdate )
      {
	fprintf(stdout, "++checking update function for variable reference resolution\n");
	check_update_funct(tmpglobals); //does for "this" model..
      }
    else
      {
	fprintf(stdout, "++SKIPPING checking update function for variable reference resolution\n");
      }
    prefixprint( depth );
    fprintf(stdout, "=SUBMODELS:\n");
    for(size_t subm=0; subm<models.size(); ++subm)
      {
	size_t jump=5;
	models[subm]->check_and_enumerate( depth+jump , checkupdate);
      }
    
    
  } //end check_and_enumerate
  
}; //end STRUCT SYMMODEL


//Make a "models list"

//get model "by name" for copying purposes? I.e. by type like thing.


//EOF symmodel.h
