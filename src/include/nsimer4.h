
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


struct SYMB_MODEL
{
  vector< string > vars;
  
  vector< string > holes;
  
  vector< SYMB_MODEL > models;
  vector< string > localnames;
  vector< string > localtypes;
  
  //std::function update;
  
  void var( const string& v ) { vars.push_back( v ); }
  void hole( const string& h ) { holes.push_back( h ); }
  
  
  //Only finds the first one...
  vector< size_t > find_model( const string& name )
  {
    vector<size_t> ret;
    vector<string>::iterator it = std::find( localnames.begin(), localnames.end(), name );
    if( it != localnames.end() )
      {
	ret.push_back( *it );
      }
    return ret;
  }
  
  std::shared_ptr<SYMB_MODEL> get_model( const string& n )
  {
    vector<string> parsed = parse( n );
    return get_model( parsed );
  }
  
  std::shared_ptr<SYMB_MODEL> get_model( const vector<string>& parsed )
  {
    
    if( parsed.size() < 1 )
      {
	fprintf(stderr, "REV: Error in get model, we should never have a path that has less than 1 parts: VAR. But we got [%u]\n", parsed.size() );
	exit(1);
      }
    else //if ( parsed.size() >= 1 )
      {
	vector<size_t> locs = find_model( parsed[0] );
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

  
  void add_model_part( const SYMB_MODEL& s, const string& localname, const string& localtype )
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
  vector< str > modelnames;
  vector< SYMB_MODEL > models;

 
  
  //Ugh, using pointer? Not nice...retur
  SYMB_MODEL& get_model( const string& name )
  {
    
  }
  
  void add_model_part( const string& existingmodelname, const SYMB_MODEL& s, const string& localname, const string& localtype )
  {
    vector< size_t > locs = find_model( existingmodelname );
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
  void set_connection( const string& m1, const string& m2 )
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


//Var returns a reference (whoa?) to it?
//REV: This won't work, because I need to "run" it, and it will register which variables are "set" and which are "used" (and their order?)
//So, I need to write my own parser...
//What do I need? I need to tell when variables are referenced or set
//I need to be able to compile it normally/run it as a function.
//So...I can just run it once, and have a "side effect" counter...? For number of times referenced/set? Ohhhh ghetto.
//Problem is, I want it to compile down directly to be replaced with the target array. To do that, it needs to "search" for it, etc.
//Which is way too slow... so, I can't do it "inline" like this, I need to literally run the compiler twice? Or, run a parser, and parse it into
//base calls, which directly have pointers to memory locations (arrays). Fine. So literally, it parses it into some struct, which is just a functional
//And the functional (function pointer?) each does some work. BIG SHIT OH SHIT, it won't optimize those, since they're dynamically generated...ugh.

//So, in the end, yea, I will build the model, and generate the update functions. Based on that, it will make the "structs" for me (which may be moved to GPU?)
//and print out the "raw C" update functions. That file will be included now in my (new) model file (uhhhh, automatically?). I will now call only names from those
//structs. Do I always include that file? Is it header-only? Fuck my life. No, some of it must be NVCC, for cuda stuff...but any rate, it will just be
//like a struct of arbitrarily named arrays...and it will just be referencing those, with an offset (possibly via an offset array?). Hm, in that case, I should just
//literally make pointers to every variable (based on offset). Will that speed things up? It will just be one pointer dereference, versus offset dereference, followed by
//offset, followed by X... I.e. keep it around but don't use it...

//Slow version, with just manual function generation
//Or C++ generated version, which is recompiled.

//Best is to have it compile into a "new" class that user can call, and generate input to/from. That has like, reset etc. functions. To set to different values...difficult.
//Need to "remember" how to update each value. With external accessors. E.g. it is just a struct that will automatically run/call kernel function, and get output?
//User can say like, go for X seconds with Y input (specified beforehand, over that period of time).

//Make every "line" (or function specified by user?) compile into a specific shape in C++. I can specify that in back-end, after the fact.
//In this way I can determine if I need to make extra. I literally call each one with the "dummy" access variables?
//That does the reference counting for each line (as "set" or "read").
//Within an update equation, if it is both read and written, that's fine..?  I need to do it for each model though...?
//This will determine dependency structure.
//Make it "runnable"





//OK, so these are compiled into one-by-one executable things, which access vars etc.
//Eventually those will compile into "true" code which reference elements in a "struct" that is just the real arrays/models
//It will generate the code, and user will now have a struct/class that executes that "model". Will any changes in size require me to re-run it?
//I guess it will generate it in a new class file? And it will have "impl" in other side of things, which is compiled with NVCC etc.
//Ugh, it needs to "find" libraries etc.? So, this is a utility to "build" models.

//I have a "class" which is what it builds ;)
//The class has certain access methods. What it does, is it builds a certain "implementation" of it, which includes a function?
//How about if I want to selectively turn on/off certain model updates? For example STDP? Figure out how to do that later (i.e. give tags in class to turn those parts off).
//Order of updates should not matter...if I start a bunch of kernels in here, that's fine. So I need to separate into 2 kernels, "before" and "after", so that way we know one
//is finished before next starts ;)

//KERNEL
//{
// synupdate //these can all be simult
// neurupdate // these can all be simult
//}

//dev synupdate { update each model related to syns }
//Everything is based on whether something else "reads" from it before end of turn.

//Anyway, figure all that later.
//We have a "runcircuit" class, which we make an INSTANCE of via compiling. e.g. A "buildcircuit", which makes the vectors, runs the generators, etc.
//Then, we have a "list" of update function (each model has its own update function thing). So, we have a "list" of compiled models? Model update functions specifically.
//Which are local functions? Which reference their local variables? So we literally build a struct for each, with a list of guys, etc. Problem is sometimes they have
//external references.

//So, easiest to build a single large struct, with a global list of variables etc.
//And it just has some number of functions, which are the member update functions. They are named/referenced by the updating group name in the symb model?
//OK.


//At any rate we alredy have the struct/class, which we know some functions of. And we make an instance of it, named like, mycircuit and shit. And the thing about it is that
//we have the variables pushed back, and the symbolics pushed back, and somehow we have update functions, which get the pointer to the variable list and offsets in it...
//So, they must be virtual functions...otherwise how do we "add" them? Or I overload it and "implement"? OK, fine, so we have many classes, and each one "implements" the
//update function...which may be a bunch of device code ;)
//Yea, so I guess it is just a single large update function for the whole circuit.
//That's fine. update for N time points. With some "input" specified (by user).
//Need to specify "input" or "events" during that period, and what to "read out". Then, user basically can use "that class", and use the "write" and "read" functions to get
//stuff out of it ;)

//So, this only builds the circuit, and generates it. But then I basically include in my model. Note, for a given symbolic model, the code is the same (?). Although it
//may change the number of schedulers etc.? Nah...

//So, I compile it once, it generates the GPU code...and then what? I can now use that as the "optimized" thing to run?
//Yea, I basically generate an "optimized" implementation of "update" (for my specific OBJECT?), which is a function pointer. Can I only do one at a time? How do
//I include it? Do I open it as a shared library? I basically say "generate the code to mymodel5.h" (note it will also generate CPP/CU files?)
//No, it simply generates a "new" .cu file, which is then compiled INSTEAD of the old one. At "build" time, I now specify as command-line argument, the name of the "generated"
//impl files to use. Which contain different implementations for "update" (of my specific class?). No, remember, it is a function POINTER. The function pointer takes the
//class guy as arguments, and the ordering of the arrays to know what to operate on. Not only the update function is changed, it needs the list of pointers to its appropriate
//data. So, now, I simply say std::function update = mycompiledfunct5;, rather than the default.
//Yea, main problem is how to do that? Have all functions automatically registered (with arbitrary names) as some string. And user now basically just specifies at runtime,
//the name of the one to use. They are all compiled and registered in a "compiled_functs" file? A .h file? Which is just arbitrarily generated names. Problem is they
//must automatically register, which is impossible? No, I generate the header each time and generate the names? Ah, literally have a txt config file that is the "list" of
//legal names, and I read it from there. So when I add one, I append to that list. If user deleted one, what happened? It won't compile because the implementation will have
//been deleted. Note, I need to keep around a corresponding "model" name for each guy. I.e. generate a *new* cu/cpp file for each one? But keep them all available/registered.
//So the actual runner only selects the blah, it doesn't generate the running. So I select model name, and select -optimized or -optimized-gpu. Then it will check to see if
//it exists. If it does, it will get from that list.

//That is just literally a list of functions to run (each one has optim, optim-gpu, and safety-model). The list is a list, of "model" name (like generated by user?),
//and the actual code that implements the update function etc. Those are separately generated/compiled into separate header/cu files, which are stored in a different
//special directory/directories to hold that.

//The problem is that the makefile needs to automatically include/compile it all. It needs to know to now include those HEADER files (i.e. add them in a new "header-header"
//list, fine). And it also needs to know to COMPILE the implementation files. It needs to know the rule for each of those. I guess my makefile can automatically handle that
//it will compile them all, always? Assuming it is in the correct location. Fine.
//If I delete the symbolic model (?), it will also delete the corresponding code? Do I always have a list of files to run?


//Remember, I want to e.g. make a user program, with a MAIN, to build the thing.
//So, my user program compiles, it already includes existing guys. I build the guy, I have the update function.
//Then, I specify, run with optim. And it will go and exit temporarily (if it doesn't find an existing one for my model?), and say, generated code to XX.cu/XX.h, added
//to <blah.headers>. So, re-compile, and re-run.

//And now, it will exist in that store to search for. Fine, DO IT.


//REV: No...it returns a DOUBLE? Always? And takes a list of doubles (no...strings?). Right...
//Each inner guy, returns a double.
//DOCMD( "SETVAR( GSYN, SUMFORALL( conductances, DIV( g, E ) ) )" );
//g, E are variable references. They return "double" I guess (but call a funct)
//So, problem is that SUMFORALL, it literally takes string and string as arguments.
//So, all functions take a string of args, and parses it themselves. And all functions return doubles (?)


//All functions do their own thing. Some functions take "string" and "blah" as arguments. That one happens to take 2 strings.
//How about, all functions just take strings (the list of args)
//I parse everything first, and determine where the arguments start/end.
//Then I call from outside-in, left to right?
//What exactly is a function? Literally a string. And I do the string, and it looks up the function name.
//And that function, that function also will take some arguments in some way. So, it takes the string and parses it.
//So, all functions return a double. All functions take a string.

//REV: Parse a string into appropriate function form...
//Include equals sign for "SET", and +, -, /, * with their normal meanings.


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
  real_t val = model.getvar( parsed[0] );
  
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
  
  //just a list of std::string NAME and std::function FUNCT, of same type?
  //The type just takes list of guys. Eventually it takes raw (strings?) when they're not recognized. And does things with it. E.g. for conductances, it knows to
  //look through "holes" for it. All functions take the model.
};






void adexupdate()
{
  //Can I make a temporary variable? E.g. "forall" type thing?
  //holeiter conditer( "conductance" );
  
  //Forall just does a for loop, returning each time the next hole iter thing. Only sum? Ah, need a "loop roller" type thing, I.e. it takes a function as an argument
  //REV: literally need to pass it the "name" of the function...? Otherwise, it doesn't know how to handle this complex statement, it will evaluate before passing through...
  //Super-ghetto...to pass functions as strings rofl.
  //OK, so do it the ghetto way. Make arbitrary length lines of prefix-notation function (arg arg arg) type thing. Which can then be executed on the other side.
  //Note the prefix notation may also contain further prefix notations to do, e.g. for each synapse of a conductance type or some shit? heh...
  //And when I execute it, it literally does the lookup. And for each guy it finds, it executes it. Fine... So, this is top-level. But I say just "execute", and pass
  //either a string, or I pass it a vector of strings (to execute). If each has a set number of arguments, that is fine, it will only consume those?
  //I need to go through the whole thing...to determine where arguments/start end? If we encounter a FUNCTION, it has an ARITY, and so basically we do lambda shit hahahaha
  //and we keep "pushing" on it. For some, it might do some iteration over some shit. Note, when it encounters a "variable" (i.e. non-function), it just uses it (as a lookup)
  //OK...that will "work" but only in the sense that we want to bother writing another freaking parser for a language that looks up things to actually do. I can use it
  //to parse scripts though which is nice I guess?


  //So, I need to handle that. For now just use the prefix, do the parsing later.
  //For the generation side of things...?
  //At any rate, at every "final reference" (stuff gets passed along with me? Fuck???), I need to reference that it was read/written...and that way I know ordering.
  //Finally, I generate C++ to handle everything.
  //Just use parens...and do it that way heh. In other words, list of strings. EZ.
  DOCMD( "SETVAR( GSYN, SUMFORALL( conductances, DIV( g, E ) ) )" );
  
  //Major (final) question is fine, we generate the C++ as update function for this "group", 
  setvar( "gSyn", SUMFORALL( "conductances", DIV( var("g"), var("E") ) ) );
  setvar( "V", ADD( EXP( TIMES( var("W"), var("TAU1") ) ), var("gt"), var("V") ) ); //arbitrary number to add...?
  
  
}

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

  

  
};


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
