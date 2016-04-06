
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

typedef std::vector vec;
typedef std::string string;


vec<string> tokenize_string( const string& src, const str& delim, const bool& include_empty_repeats=false )
{
  vec<str> retval;
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

vec<string> parse( const str& name)
{
  bool emptyrepeats=false;
  return tokenize_string( name, "/", emptyrepeats );
}


struct SYMB_MODEL
{
  vec< str > vars;
  
  vec< str> holes;
  
  vec< SYMB_MODEL > models;
  vec< str > localnames;
  vec< str > localtypes;
  
  //std::function update;
  
  void var( const str& v ) { vars.push_back( v ); }
  void hole( const str& h ) { holes.push_back( h ); }
  
  
  //Only finds the first one...
  vec< size_t > find_model( const str& name )
  {
    vec<size_t> ret;
    vec<str>::iterator it = std::find( localnames.begin(), localnames.end(), name );
    if( it != localnames.end() )
      {
	ret.push_back( *it );
      }
    return ret;
  }
  
  std::shared_ptr<SYMB_MODEL> get_model( const string& n )
  {
    vec<string> parsed = parse( n );
    return get_model( parsed );
  }
  
  std::shared_ptr<SYMB_MODEL> get_model( const vec<string>& parsed )
  {
    
    if( parsed.size() < 1 )
      {
	fprintf(stderr, "REV: Error in get model, we should never have a path that has less than 1 parts: VAR. But we got [%u]\n", parsed.size() );
	exit(1);
      }
    else //if ( parsed.size() >= 1 )
      {
	vec<size_t> locs = find_model( parsed[0] );
	if( locs.size() == 1 )
	  {
	    size_t mloc = locs[0];

	    if( parsed.size() > 1 )
	      {
		//Strip off the first part.
		parsed.erase( parsed.begin() ); 
		return ( models[ mloc ].get_model( parsed ) );
	      }
	    else //This is the model that contains the var...
	      {
		return std::shared_ptr< SYMB_MODEL > ( this );
	      }
	    
	  }
	else
	  {
	    fprintf(stderr, "REV: get_model, find model, model doesn't exist...[%s]\n", parsed[0].c_str());
	    exit(1);
	  }
      }
  } //end get_model( vect<str> )

  
  void add_model_part( const SYMB_MODEL& s, const str& localname, const str& localtype )
  {
    parts.push_back( s );
    localnames.push_back(localname);
    localtypes.push_back(localtype);
  }

  void set_connection( const string& m1, const string& m2 )
  {
    //Finds m1 and m2, sets the correct HOLE in m1 to m2?
    //A HOLE actually has the pointer to the model? It's just the GLOBAL string reference (for now). I like that...
    //I need to be able to "climb back up" models to get to the top level.
    //What do I want to CREATE? Just the logical connections, so that when I build it, it knows.
    //And so that the update functions can appropriately access everything.

    std::shared_ptr< SYMB_MODEL > mymodel = get_model( m1 );
    
  }

  //SYMB_MODEL( const str& n, const str& t )
  //: name(n), type(t)
  SYMB_MODEL()
  {
  }
  
};


//REV: basically, everything is a list of strings.
//Based on the strings, I can know hierarchy etc.
//I need to ensure that connections are properly filled?

//REV: There is no such thing as a circuit model. There is only a "main" symb model haha.




struct CIRCUIT_MODEL
{
  vec< str > modelnames;
  vec< SYMB_MODEL > models;

 
  
  //Ugh, using pointer? Not nice...retur
  SYMB_MODEL& get_model( const str& name )
  {
    
  }
  
  void add_model_part( const str& existingmodelname, const SYMB_MODEL& s, const str& localname, const str& localtype )
  {
    vec< size_t > locs = find_model( existingmodelname );
    if( locs.size() == 1 )
      {
	models[locs[0]].add_model_part( s, localname, localtype );
      }
    else
      {
	fprintf(stderr, "REV: error couldn't find existing model [%s]\n" );
	exit(1);
      }
  }

  //REV: This is filling a "hole". does it matter external or internal?
  //REV: note this may be super nested somewhere...
  void set_connection( const str& m1, const str& m2 )
  {
    //Parses var into model name, finds the model, then finds internal var...
    model1 = getmodel(m1);
    hole = model1.gethole(m1);
    hole.fillwith( m2 ); //could parse m2 down to a model/var pointer? Leave it all as strings seems much easier for now, problm is I need to keep "types" around too.
    //They are not types, they are instances (of a SYMB model). So all I need is type name and lookup. ;)
  }
};


//Symb models are just shortcuts...

SYMB_MODEL pos3d( "pos3d", "position3d" );
pos3d.var("x");
pos3d.var("y");
pos3d.var("z");

SYMB_MODEL gAMPA( "gAMPA", "conductance" );
gAMPA.var("g");
gAMPA.var("E");

SYMB_MODEL gLeak( "gLeak", "conductance" );
gLeak.var("g");
gLeak.var("E");

SYMB_MODEL adex( "adex", "spiking_neuron" );
adex.var("V");
adex.var("W");
adex.var("tspk");

adex.hole("conductance");
adex.hole("current");

adex.add_model_part( gLeak, "gL", "conductance" ); //REV: huh it doesnt use name in model... so model has internal list of "other models" inside of it.
//Those are different than "holes" which simply point to things that can be iterated through...

adex.add_model_part( pos3d, "pos", "position3d" );


SYMB_MODEL spiking_syn( "spiking_syn", "spiking_syn");
spiking_syn.var( "delay" );
spiking_syn.var( "weight" );
spiking_syn.hole( "postsyn_receptors" );
spiking_syn.hole( "presyn_firetime" );

SYMB_MODEL Iinj( "Iinj", "current" );
Iinj.var( "I" );  //set to zero every time step? etc? Draw from random? User can set update function ;)





CIRCUIT_MODEL sc;

sc.add_model( adex, "adex1" );
sc.add_model( adex, "adex2" );
sc.add_model( spiking_syn, "syn2-1");
sc.add_model( spiking_syn, "syn1-1");

sc.add_model_part( "adex1", Iinj, "Iinj1", "current" ); //Whoa, so it fills a hole here?!?!!

sc.add_model_part( "syn2-1", gAMPA, "gAMPA", "postsyn_conductance");
sc.add_model_part( "syn2-1", gNMDA, "gNMDA", "postsyn_conductance");

sc.add_model_part( "syn1-1", gAMPA, "gAMPA", "postsyn_conductance");
sc.add_model_part( "syn1-1", gNMDA, "gNMDA", "postsyn_conductance"); 

//This is telling it that the HOLES are the SAME THING (wow). I.e. any references in update function will actually reference the same memory vector.
//I need to know where the "owner" of the memory is (?) In this case it is telling it that the owner is the adex1/conductance...so it should look for any guys there
//e.g. gGABA, gAMPA. They must be named the same thing? conductance IS the postsyn receptors there. I.e. I add to conductance for each postsyn receptor that is made.
//So this is "filling" holes.

//The spike syn guy has some number of postsyn_receptors. And I need to add those as conductance to adex1 I guess. That is what it is saying.
//Furthermore, those are the connections specifically referenced ;)

//In this case, adex has tspk, and I need to "add" that to the "presyn firetime hole" of me. OK. They are named something, but that is irrelevant.

//Or is it ever relevant? for how we handle gGABA etc.?

//So, for each "variable" in "conductance" or etc., I go through and add a mention/reference of it in adex conductance or some shit..
//There's only a single variable, and it can be filled with new guys added, or it can be filled with references to other guys.

//What *are* these though? Each variable or hole, holes are just vectors of variables basically. And, each variable or hole may either be a varaible (name) itself, and it
//may point to "content", which is the actual vector, which is a variable in global reference space. OK.

//So, everything has a "name" (local name?) which may be just a hole (?). And it also has "content" i.e. what variable it ACTUALLY points to. So, what about the case with
//holes? In the case of holes, e.g. I may have some number of postsyn conductances (?), so I simply add those one by one? I have presyn GLU, so I need to attach to all guys
//of GLU that are postsyn? Even easier to manually connect it... For example, I'm a GLU synapse, so I need to connect to all postsyn of type GLU_RECEPTOR. Just manually
//hook it up I guess...I set presyn first. Or do I add GLU to presyn based on existence of postsyn receptors? I don't know the update equations/parameters for the postsyn
//guys...so they must reference that I guess? Each neuron must be different! So postsyn must reference PARAMETERS of the neuron conductances. Tell it they are the same thing.
//I need to tell it TAU etc. are the same...? Just do it variable by variable!!! No, not var by var, because a "hole" will be filled with MODEL (not a var).
//So, I literally fill the conductances of the neuron with the postsyn conductances of the synapse guys. But those guys all reference a single parameter set, which is the
//gGABA model parameter of the postsyn neuron. So, it's not a hole, it's an external variable. Is that a "connection"? I guess so, because each actual member could
//reference many possible targets.


//So, I make e.g. adex1. Adex has a "conductance" hole, which it will iterate through. Arbitrary numbers of models can "connect" into there (or be added), and be handled
//the same way. However what if we e.g. attach a synapse to the neuron. Then either the neuron needs to "add" into an existing conductance (in which case might be a race
//condition if it's single size?). Or it might have a single one for every pre synapse? Or it needs to add a new conductance. However, e.g. if the pre synapse is a
//GLU synapse, and wants to use the TAU of gAMPA, it will read that value from the postsyn neuron.
//The question of whether to add to AMPA/NMDA, the relative weight of each, etc. will be based on a decision at the post synapse though...? What if they don't linearly
//combine? That would be up to a model.

//OK, so we "add" conducntaces to the postsyn neuron to account for my "model". We have to tell that it will use the tauAMPA from the postsyn neuron's gAMPA (which one?).
//It will just be an unnamed conductance? No, it is actually a (sub) model added...? Which will happen to be contained in conductances...with a specific name. Are sub-models
//added in the normal sub-model list? Only if they are referenced in the update function?
//So, allow any params/variables be "external"????? It might be a parameter of the specific postsynaptic neuron. I need to tell it what it is. Ah, it's a connection.
//It's not contained internally. I just need to tell the "parts". All internal vars are contained internally. If I say "var", it is my freaking var. No, I do that in circuit
//connection, where I tell those that are "external", and those that are not are automatically assumed to be internal.

//All "references" are then made via "connections" by default OK.


//So what does it mean when I set a "model" in mine to a "model" in theirs? E.g. postsynaptic conductance in SYN2-1, is same as postsynaptic conductance in ADEX1. For all them.
//They are models. Since I set the ADEX1 one to reference it, the "owner" is the SYN2-1. However, we need to know to "add" to the conductance list (or equilvanently, add both
//GNMDA and GAMPA to the "postsyn receptor" list of the SYN2-1 grp.) Um, because postsyn has "holes", and we need to add a "target" for each of those? Like it might add to
//each of those some amount based on their "type". OOh, selectively add to conductances based on "type". Gotcha. E.g. fill hole with adex1 conductances off type "GLU_RECEPT".
//And, things can have multiple types, so we look for any of them (must match all that I want it to match, or any to match). Allow options of which to do.

//So like, "fill holes by type", etc.

//So have "name" and "actual target" or whatever?
//Is the actual target a "model" or is it a var? Always VAR (holes are). But we can set individual external vars to point to "contents" of another guy. 

struct symvar
{
  string name;
  string type;
  string valu; //Valu is (global reference) location of what it is.

  //Default is "my location"
  symvar( const string& n )
  : name(n)
  {
  }
};

struct hole
{
  string name;
  //string type;
  vec<string> members; //E.g. names (global) of models that have filled this hole?

  hole( const string& n )
  : name(n)
  {
  }
};

struct symmodel
{
  string name;
  vec<string> type;
  
  vec<symvar> vars;
    
  vec<symmodel> models;
  vec<string> modelnames;
  vec<string> modeltypes;

  vec<hole> holes;
  //vec<string> holenames;
  //vec<string> holetypes;


  void addtype( const string& t )
  {
    //adds to type. Parses first.
  }
  
  
  
  symmodel( const string& s, const string& t )
  : name( s), type( t )
  {
    //parse t into types, and push back to type.
  }
  
  void addvar( const string& s )
  {
    vars.push_back( symvar(s) );
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
    //Literally add a (new) submodel to me. This may also be used to fill a hole, but this model "owns" the data.
  }

  void fillhole( const string& hole, const string& modeltofillwith )
  {
    //Fill the hole with the model at modeltofillwith
  }

  //Will this search all "holes" too?? Or only sub-models...? And only one layer down? This will look at "type" of model
  void fillhole_fromtype( const string& hole, const string& modeltofillfrom, const string& type )
  {
    //Search models of target model, for those of TYPE, and fill hole.
  }
};


//Needs to know what to "read from" to see if I increase?
symmodel gAMPA("gAMPA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
gAMPA.addvar( "E" );
gAMPA.addvar( "g" );
gAMPA.addhole( "membrane" );

symmodel gLeak("gLeak", "conductance");
//I need to tell it that the V used in the update equation of gLeak is the V of adex!!!
gLeak.addvar( "E" );
gLeak.addvar( "g" );
gLeak.addhole( "membrane" ); //do I always need to tell it this? Do I need to explicitly connect all of these? Do I automatically view all guys "up"?

symmodel adex("adex", "spiking|neuron");
adex.addvar( "V" ); //membrane potential
adex.addvar( "W" ); //recovery potential
adex.addvar( "tspk" ); //spiketime, time,

adex.addhole( "currents" );
adex.addhole( "conductances" ); //Could separate these into synapses etc? E.g. presyn and postsyn? Inhib/excit. etc.

adex.addmodel( gLeak, "gL", "" );

synmodel Iinj( "Iinj", "current" );
Iinj.addvar( "I" );

synmodel spksyn( "spksyn", "synapse");
spksyn.addvar( "delay" );
spksyn.addvar( "weight" );
spksyn.addhole( "presyn_neuron" );
spksyn.addhole( "postsyn_neuron" ); //conductance?



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






sc.set_connection( "syn2-1/postsyn_receptors", "adex1/conductance" ); //REV: a connection to a HOLE (!?!?!?!)
sc.set_connection( "syn2-1/presyn_firetime", "adex2/tspk" );

sc.set_connection( "syn1-1/postsyn_receptors", "adex1/conductance" ); //REV: a connection to a HOLE (!?!?!?!)
sc.set_connection( "syn1-1/presyn_firetime", "adex1/tspk" );

//REV: I literally need to make this into a graph...
