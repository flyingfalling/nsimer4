
//NSIMer 4
//Simulation of NSIM model on CPU/GPU.
//Generation of GPU code is automatic based on update function defn, but
//initially just do on GPU?



//REV: redesign!!

//everything is a SYMB model. And we "search" for symb-model references recursively to find the one referred to at the lowest level. Is the last element always a variable?
//Inelegant to have it search circuit model first, then search the other guy. Just have everything always as a SYMB_MODEL, and treat circuit_model as symb_model?
//Is there any differences? A circuit model wont have holes...?

//Whatever, just make everything go together as symb_model, and when we search something it just searches it from "this" level? A kind of um, "visitor" pattern,
//which gets pointer to last guy and could build up a list along the way...?

//REV: Whole idea is user program will only compile HIS stuff, i.e. it will be created in a user-program centric way. It will NOT affect anything in the library.
//In other words, everything could be deleted with no issue (if error).
//Some like, trash header files etc. However, there needs to be special rules to compile user-programs that are separate than library compilation. Nothing in library will
//change, but user will need to include "library generated headers and source".


#pragma once

//"natural" time is in milliseconds?

#include <sys/types.h>
#include <vector>
#include <cstdlib>
#include <cstdio>

#include <algorithm>

#include <boost/tokenizer.hpp>

typedef double float64_t;

typedef float64_t real_t;

//typedef std::vector vec;

using std::vector;

typedef std::string string;


vector<string> tokenize_string( const string& src, const string& delim, const bool& include_empty_repeats=false )
{
  vector<string> retval;
  boost::char_separator<char> sep( delim.c_str() );
  boost::tokenizer<boost::char_separator<char>> tokens(src, sep);
  for(const auto& t : tokens)
    {
      retval.push_back( t );
    }
  return retval;
}

bool string_prefix_match( const std::string& orig, const std::string& prefix )
{
  if( orig.compare(0, prefix.size(), prefix) == 0 )
    {
      return true;
    }
  return false;
}

std::string remove_prefix( const std::string& str, const std::string& prefix )
{
  if( string_prefix_match( str, prefix ) == true ) 
    {
      size_t last = str.find_last_of( prefix );
      std::string noprefix = str.substr( last+1, str.size() );
      return noprefix;
    }
  else
    {
      fprintf(stderr, "ERROR in remove_prefix: requested prefix to remove [%s] is not a prefix of me [%s]!!\n", prefix.c_str(), str.c_str());
      exit(1);
    }
}

vector<string> parse( const str& name)
{
  bool emptyrepeats=false;
  return tokenize_string( name, "/", emptyrepeats );
}

struct symvar
{
  string name;
  string type;
  //string valu; //Valu is (global reference) location of what it is.
  //Wait, can't vars be size_t also?

  real_t valu;
  
  size_t read=false;
  size_t written=false;

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
symvar( const string& n )
: name(n)
  {
  }

symvar( const string& n, const string& t )
: name(n), type(t)
  {
  }
};

struct hole
{
  string name;
  //string type;
  vector<string> members; //E.g. names (global) of models that have filled this hole?

  hole( const string& n )
  : name(n)
  {
  }
};


typedef std::function< real_t( string&, symmodel& ) > cmd_functtype;


bool checknumeric( const string& s, real_t& ret )
{
  bool ok=false;
  std::istringstream iss( s );

  iss >> ret;
  //check that iss is all done! If it successfully parsed it, we good?
  if( iss.fail() )
    {
      iss.clear();
      string failedstring;
      iss >> failedstring;

      ok = false;
    }
  else
    {
      ok = true;
    }

  //Do I need to try to read "next" to read end of string? Will it return fail????
  string wat;
  iss >> wat;
  if( !iss.eof() )
    {
      fprintf(stderr, "REV: error, checknumeric, didn't finish parse of input string to numeric [%s], read in unexpected [%s]?!?!?!?!\n", s.c_str(), wat.c_str());
      exit(1);
    }
  

  return ok;
}

real_t DOCMD( const string& arg, symmodel& model, const cmdstore& cmds )
{
  //most basic. Just does the search for it and executes. Returns a real_t, but really we need a "set"
  vector<string>  parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    {
      exit(1);
    }

  vector<string> functparse = cmds.fparse( arg );
  string fname = functparse[0];
  functparse.erase( functparse.begin() );
  vector<string> fargs = functparse;

  //Lol, just re-parse them?
  string newarg = CAT( fargs, "," );
  
  cmd_functtype execfunct;
  bool found = cmds.findfunct( fname, execfunct );
  real_t retval;
  if( found )
    {
      retval = execfunct( newarg, model, cmds );
    }
  else
    {
      bool isnumer = checknumeric( fname, retval );

      if( isnumer == false )
	{
	  retval = READVAR( fname, model, cmds);
	}
    }
  
  return retval;
  
}

real_t READVAR( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = doparse( arg );
  //Set some "var_counter" in model to be read.
  if( parsed.size() != 1 )
    {
      exit(1);
    }
  real_t val = model.getvar( parsed[0] ).valu;
  
  return val;
}


//REV ERROR is parsed[0] is not a variable!!
real_t SETVAR( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    {
      exit(1);
    }

  string toexec = parsed[1];
  real_t val = DOCMD( toexec, model, cmds );
  //Could have been read and set separately?
  model.setvar( parsed[0], val );
  
  return 0;
}


//Easier to just make sure it's 2...
real_t SUM( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }
  
  real_t val=0;
  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val += DOCMD( toexec, model, cmds );
    }

  return val;
}


//product
real_t MULT( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  real_t val=1.0;

  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val *= DOCMD( toexec, model, cmds );
    }

  return val;
}

//div
real_t DIV( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];

  real_t val=DOCMD( toexec, models, cmds );
  
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val /= DOCMD( toexec, model, cmds );
    }

  return val;
}

//subtract
real_t DIFF( const string& arg, symmodel& model, const cmdstore& cmds )
{
  //subtract all from first one?
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val=DOCMD( toexec, model, cmds );
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val -= DOCMD( toexec, model, cmds );
    }
  
  return val;
}


real_t NEGATE( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val= DOCMD( toexec, model, cmds );
  return (-1.0 * val);
}

real_t EXP( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val = DOCMD( toexec, model, cmds );

  return exp( val );
}

//How to deal with numerals? Just parse them as base vectors...
real_t SUMFORALL( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if(parsed.size() != 2 )
    {
      exit(1);
    }
  
  vector<hole> myholes = model.gethole( parsed[0] );
  
  string toexec = parsed[1];
  real_t val=0;
  for( size_t h=0; h<myholes.size(); ++h)
    {
      symmodel m = myholes[h];
      val += DOCMD( toexec, m, cmds ); //does DOCMD return a real_t val? Fuck...
    }
  return val;
}

//How to deal with numerals? Just parse them as base vectors...
real_t MULTFORALL( const string& arg, symmodel& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if(parsed.size() != 2 )
    {
      exit(1);
    }
  
  vector<hole> myholes = model.gethole( parsed[0] );
  
  string toexec = parsed[1];
  real_t val=1.0;
  for( size_t h=0; h<myholes.size(); ++h)
    {
      symmodel m = myholes[h];
      val *= DOCMD( toexec, m, cmds ); //does DOCMD return a real_t val? Fuck...
    }
  return val;
}

struct cmdstore
{
  vector< string > functnames;
  vector< cmd_functtype > functs;

  //REV: if it doesnt find it, it is a variable or a number
  bool  findfunct( const string& s, cmd_functtype& f )
  {

    vector<string>::iterator it = std::find( functnames.begin(), functnames.end(), s );
    if( it != functnames.end() )
      {
	f = functs[ *it ];
	return true;
      }
    else
      {
	return false;
      }
    
  }


  //Parses just by commas, but leaves matching parens (i.e. functions) intact.
  //This is just a literal parse of inside of funct? Is there any point in this? Just use the remnants from fparse...
  vector<string> doparse( const string& s )
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

  
  //If it has a legal function parse, I leave it alone?
  
  //parses function, i.e. expects only single FNAME( COMMA, ARGS )
  vector<string> fparse( const string& s )
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


struct updatefunct_t
{
  vector<string> lines;

  void add( const string& s )
  {
    lines.push_back( s );
  }
};

struct symmodel
{
  updatefunct_t updatefunct;

  std::shared_ptr<symmodel> parent;
  
  string name;
  vector<string> type;
  
  vector<symvar> vars;
    
  vector<symmodel> models;
  vector<string> modelnames;
  vector<string> modeltypes;

  vector<hole> holes;
  //vector<string> holenames;
  //vector<string> holetypes;


  void addtype( const string& t )
  {
    //adds to type. Parses first.
  }
  
  
  
symmodel( const string& s, const string& t )
: name( s), type( t )
  {
    //parse t into types, and push back to type.
  }
  
  void addvar( const string& s, const string& t )
  {
    vars.push_back( symvar(s, t) );
  }

  void addhole( const string& s )
  {
    holes.push_back( hole(s) );
  }

  //Is this filling a hole? E.g. with an external model?
  //At what point do I actually "resolve" all variables/models?
  //Can adding models actually be external? No, they can't. Those must be holes? No, they can be external? If they're connected?
  //What's the point of a hole? Something that MIGHT be external or interanl? OK... And might be of size N.
  //Is pos a hole? It is filled by a local model pos
  //When I specify var...yea it's just single thing.
  //Can holes be for variables too? Or are they model holes? Separate holes? Make all holes just variables? No, models...
  //We need to know when to update. If we only reference variables, we don't know when they've been updated. They must only be parameters?
  void addmodel( const symmodel& m, const string& localname, const string& localtype )
  {
    //not a pointer, I assume? Pushes a COPY of it?
    models.push_back( m );
    modelnames.push_back( localname );
    modeltypes.push_back( localtype );
    models[ models.size() -1 ].parent = std::shared_ptr<symmodel>( this );
    
    //Literally add a (new) submodel to me. This may also be used to fill a hole, but this model "owns" the data.
  }

  void fillhole( const string& hole, const string& modeltofillwith )
  {
    //Fill the hole with the model at modeltofillwith. Note that both hole and modeltofillwith may be HIERARCHY references *from that model*
    //Another problem is how to do "containing" models, i.e. I want to know who "contains" this gAMPA. I don't want to search all guys until I find this one.
    //I want to have a ".." pointer or smthing...
    //All guys should have a "global" name, but then if it is added to another model, it may be still nested.
  }

  //Will this search all "holes" too?? Or only sub-models...? And only one layer down? This will look at "type" of model
  void fillhole_fromtype( const string& hole, const string& modeltofillfrom, const string& type )
  {
    //Search models of target model, for those of TYPE, and fill hole.
  }

  //From any point, can I always get a global reference? When I "add" to a model,
  //I get the "parent" reference...always? Do I "always" need to be able to go up?
  //Literally give a symptr to it?
  
  //music
  //kyuss
  //kylesa
  //the sword
  //sleep (dopesmoker)
  //

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

  //REV: This finds "variable" inside a model? Or it finds model?
  std::shared_ptr<symmodel> get_model( const vector<string>& parsed )
  {
    if( parsed.size() == 0 ) //< 1 )
      {
	return std::shared_ptr< symmodel > ( this );
      }
    else //if ( parsed.size() >= 1 )
      {
	vector<size_t> locs = find_model( parsed[0] );
	if( locs.size() == 1 )
	  {
	    size_t mloc = locs[0];
	    
	    //Strip off the first part.
	    //parsed.erase( parsed.begin() );
	    vector<string> nparsed( parsed.begin()+1, parsed.end() );
	    return ( models[ mloc ].get_model( nparsed ) );
	  }
	else
	  {
	    fprintf(stderr, "REV: get_model, find model, model doesn't exist...[%s]\n", parsed[0].c_str());
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
  
  //Enumerates all (at a global scope) variables, and recursively does it for all my sub-models
  //All holes which are filled by models, have their global scope printed too.
  void enumerate( size_t depth = 0 )
  {
    prefixprint( depth );
    fprintf( stdout, "MODEL [%s]\n", name.c_str() );

    prefixprint( depth );
    fprintf( stdout, "TYPES: ");
    for(size_t t=0; t<types.size(); ++t)
      {
	fprintf(stdout, "[%s] ", types[t] );
      }
    fprintf(stdout, "\n");

    prefixprint( depth );
    //Need to know which variables are referenced in my update function!!!!! So I know that I can actually find them and that they exist in the target models!!!!
    //Variables "read" in update function
    //Variables "written" in update function
    //Verified location. Since I only reference "foreign models", I need to know which ones I will actually read/write and ensure they exist ;)
  }
  
}; //end STRUCT SYMMODEL


symmodel pos3d("pos3d", "3dposition|location|um" );
pos3d.addvar( "x", "xdimension|um" );
pos3d.addvar( "y", "ydimension|um" );
pos3d.addvar( "z", "zdimension|um" );

//Needs to know what to "read from" to see if I increase?
symmodel gAMPA("gAMPA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
gAMPA.addvar( "E", "reversal-potential|mV" );
gAMPA.addvar( "g", "conductance|nS" );
gAMPA.addvar( "tau1", "exp-rise-time-constant|ms" );
gAMPA.addvar( "tau2", "exp-decay-time-constant|ms" );
gAMPA.addvar( "affinity", "Glu-affinity|transmitter-affinity" );
gAMPA.addhole( "membrane" );

symmodel gNMDA("gNMDA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
gAMPA.addvar( "E", "reversal-potential|mV" );
gAMPA.addvar( "g", "conductance|nS" );
gAMPA.addvar( "g2", "ungated-conductance|nS" );
gAMPA.addvar( "tau1", "exp-rise-time-constant|ms" );
gAMPA.addvar( "tau2", "exp-decay-time-constant|ms" );
gAMPA.addvar( "affinity", "Glu-affinity|transmitter-affinity" );
gAMPA.addhole( "membrane" );

symmodel gLeak("gLeak", "conductance");
//I need to tell it that the V used in the update equation of gLeak is the V of adex!!!
gLeak.addvar( "E", "reversal-potential|mV" );
gLeak.addvar( "g", "conductance|nS" );
gLeak.addhole( "membrane" ); //do I always need to tell it this? Do I need to explicitly connect all of these? Do I automatically view all guys "up"?

symmodel adex("adex", "spiking|neuron");
adex.addvar( "V", "membrane-potential|mV" ); //membrane potential
adex.addvar( "W", "recovery-potential|mV" ); //recovery potential
adex.addvar( "tspk", "spike-time|time|ms" ); //spiketime, time,

adex.addhole( "currents" );
adex.addhole( "conductances" ); //Could separate these into synapses etc? E.g. presyn and postsyn? Inhib/excit. etc.
adex.addhole( "postsyn" );
adex.addhole( "presyn" );

adex.addmodel( gLeak, "gL", "" );

adex.addmodel( pos3d, "position", "" );

synmodel Iinj( "Iinj", "current" );
Iinj.addvar( "I", "current|uA" );

synmodel spksyn( "spksyn", "synapse");
spksyn.addvar( "delay", "delay|ms" );
spksyn.addvar( "weight", "synaptic-efficacy|nS" );
spksyn.addvar( "hitweight", "spike-efficacy|nS" );
spksyn.addhole( "presyn-neuron" );
spksyn.addhole( "postsyn-neuron" ); //conductance?
spksyn.addhole( "presyn-spiketimer" ); //must be of type "spiker" or something?
spksyn.addhole( "postsyn-conductances" ); //How do I know which one corresponds to gAMPA, and GNMDA. Oh, I make them with same name so fine. Problem is, how much do I add to
//each? E.g. does each differnet synapse model have differnet weightings for the different receptor types. Does it normalize automatically between them?
//Uh, add it at the postsyn level? I.e. how strong gNMDA/gAMPA is for each...? Like some may be clustered? Do it per synapse? In which case we'd add multiple weightings if
//we want to do it here? How about every single one has its own weight? Is there a way to e.g. add a thing for every model, but outside it...

//Fill-hole-with-holes (by type) seems like definitely something I want
//Or, fill-by-type, but where it searchers holes? fuck...


//So, I connect this guy to other guy as a postsyn guy. And tell it, to only add to GLU type guys. OK, and then it goes and adds. Or I manually specify which ones to add to.
//If I only generate e.g. a hole, and tell it to "point to" those guys, it will get in trouble because 

//It will "generate" hitweights in here based on which guys I tell it is connected to!



//This is an empty model. Need to know "root" of my reference in order to get it. There must always be a path...so that's fine...
//Updates all of neuron type, then updates all of synapse type, etc.? No, there is no "explicitly update model X", it is all implicit... So this has no update function.
symmodel sc("sc", "circuit");

//Neurons and synapses are "holes"? They're just types haha.
//Only vars and holes can be referenced directly...? Nah, models can too, they are just "variables" of this model...? Shit. How to update?
//All models are updated...
sc.addmodel( adex, "adex1", "" ); //Has no specific "local functions"
sc.addmodel( adex, "adex2", "" );
sc.addmodel( spksyn, "syn2-1", "" );
sc.addmodel( spksyn, "syn1-1", "" ); //Specify type of synapse? Or might have different receptor to each postsyn target?

//sc.connect( "syn2-1", "adex1" );
//sc.connect( "adex2", "syn2-1" );



//Where do I want to put the conductances? They should be where they are "size"
//Best way is to add as postsyn, and "force" all presyn guys to add the right thing for me (how do I know when to do this?). In other words, if I add gNMDA, all presyn guys
//know to automatically add variable for it.
sc.addmodel( gAMPA, "adex1/gAMPA1", "" );
sc.addmodel( gNMDA, "adex1/gNMDA1", "" );


sc.addmodel( Iinj, "adex1/Iinj1", "" );
sc.fillhole( "adex1/currents", "adex1/Iinj1" );

sc.fillhole( "adex1/conductances", "adex1/gL" );
sc.fillhole( "adex1/conductances", "adex1/gAMPA1" );
sc.fillhole( "adex1/conductances", "adex1/gNMDA1" );

//Fill hole holes? I.e. for all conductances, fill V with me?
//Do "names" appear in models? What becomes the local name??? Is it in "hole" after all?
//I.e. holes are first-order, just as models are...no I.e. blah/conductances/blah
//It might have MANY postsynaptic connections...shit.
sc.fillhole( "adex1/gAMPA1/membrane", "adex1" );
sc.fillhole( "adex1/gNMDA1/membrane", "adex1" );
sc.fillhole( "adex1/gL/membrane", "adex1" );


//REV: I can literally artificially make a list of all postsyn grps (by making a list of postsyn_synapses or something?)
//Will it automatically fill everything from those conductances?
sc.fillhole( "syn2-1/postsyn-neuron", "adex1" );
sc.fillhole( "syn2-1/presyn-neuron", "adex2" );
sc.fillhole( "syn2-1/presyn-spiketimer", "adex2" );

//Alternatively, use sc.fillhole_bytype( "syn2-1/postsyn-conductance", "GluR-mediated-conductance" );
sc.fillhole( "syn2-1/postsyn-conductance", "adex1/gAMPA1" );
sc.fillhole( "syn2-1/postsyn-conductance", "adex1/gNMDA1" );




sc.fillhole( "adex1/conductances", "adex1/gL" );
sc.fillhole( "adex2/gL/membrane", "adex2" );


sc.fillhole( "syn1-1/postsyn-neuron", "adex1" );
sc.fillhole( "syn1-1/presyn-neuron", "adex1" );

sc.fillhole( "syn1-1/presyn-spiketimer", "adex1" );

//Alternatively, use sc.fillhole_bytype( "syn2-1/postsyn-conductance", "GluR-mediated-conductance" );
sc.fillhole( "syn1-1/postsyn-conductance", "adex1/gAMPA1" );
sc.fillhole( "syn1-1/postsyn-conductance", "adex1/gNMDA1" );



//Note, some groups may NOT have presyn (!!!) or postsyn or something ;) E.g. "non-source axons"
sc.fillhole( "adex2/postsyn", "syn2-1" );
sc.fillhole( "adex1/postsyn", "syn1-1" );
sc.fillhole( "adex1/presyn", "syn1-1" );
sc.fillhole( "adex1/presyn", "syn2-1" );





//REV: OK, I think all boilerplate is in place.
//Now, I need to, for every update function, check whether it is read/written in the same update function?
//So, this way I know "which" model update functions update/read which variables?

//The purpose of this, in the end, is so that I can appropriately generate "dummy" variables, and know where/when to reference guys.

//Each "update" function will have a list of variables

//What do I want to do? I need to make temp variables now.
//Right now, it goes through and does the stuff. Whatever, I can figure that out later. For now, just run it in "slow" mode.

//In some cases, E.g. if V references V this turn, we need two V. We can refer to them individually? No. But we may need to reference new V and old V.
//Obviously, user explicitly references it? Only case I can think it matters is if e.g. W reads from new or old. Does it ever matter?
//If we want things to update in parallel, e.g. we want V to update based on V(t-1), and then new V becomes V(t-1) at end of turn...
//From the point of view of all models, it is V(t-1) until the turn ends.
//This is important, because in case of e.g. re-computing spike time, we need to reference the "new" V. If it is referenced in the SAME function, then it always references the
//new one? Or, give user a syntax to access the old one (e.g. V(t-1) or something?). How about special syntax like V[t-1] versus V[t], etc. In which case multiple copies are made.
//By default "read" references are made to V[t-1], and "write" are made to V[t]. However, user can sometimes explicitly reference to read from a write (V[t]) value.
//If I read from the same variable after writing to it, that is an indication that something has happened. But a lot of things do that...? If any other models read from
//me, that is a problem? That means that e.g. they want to read from my old value? Or, it "doesn't matter"? If I specifically state to read from the old value,
//that means that MY MODEL needs to update first. However, if it references my V[t-1], and I reference its X[t-1], there is a conflict. In that case, one of them must
//create a dummy "temp var" which is referenced, and it updates into the new V[t] variable (i.e. writes into it), and then at end of turn, V[t] is copied to V[t-1].

//For example, within myself, if I reference V[t-1], for setting V[t] (of a different neuron!!??!???). How can I check whether it is of a different neuron?
//Anything that goes through a CONNECTION (that is non-unity) does this.
//If it is self...then what? If it is a V, it goes through a connection? Anything through a connection will use V[t-1] to ensure all update is synchronous.
//Just, everything has previous values...but we only explicilty reference updated/non-updated. Fine.
//SO, then, why do we care about being read/written to?

//Just check that I can read/write all variables.

//What is diff btween param and variable? None?

//If there is a conflict, it will keep two around.

//Oh shit, I need to implement spike schedulers... and DT type things...

//At any rate, do all updates now ;)



//What can I build/check? Make all the update functions? How about "connections"? Assume they are constructed automatically (based on size?) Yea.

//What the fuck do I compile into? Some structure which has explicit connections between all variable references? Between all models?
//In some cases I explicitly generate connections.


//So, right now I theoretically have everything connected (symbolically). So, I can theoretically draw it out from a global point of view, the whole
//circuit.




//REV: Everything should be connected now. Now, everything referenced in update functions should be referenceable.

//Now, I will "enumerate" sc, and it will print all sub-models, with all their holes (global references of it?)

//This will ensure that all holes are filled. Fine.

//Then, the next step is to compile and generate "connections", and finally to generate dependencies on generators?

//"Compiling" just means make sure that all references in update function are valid. All references are only directly to "variables", so those must be determined the source
//i.e. full (actual) source.

//Then, based on that, and dependencies, I can determine if I need to make temporary variables (?)

//Ugh, thats not nice. That means in update equation, when I go through, I need to reference hole/model/name or something? Ugh... They must all have variables of correct type?
//By name? By type? Dont like these question marks...
//At any rate, I need to make the variables...
//I can determine if I need state based only on symbolic model ;)
