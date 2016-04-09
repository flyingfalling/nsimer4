//REV: 4 Apr 2016
//Contains symmodel, for constructing symbolic models

#include <symmodel.h>

vector<string> parse( const string& name)
{
  bool emptyrepeats=false;
  return tokenize_string( name, "/", emptyrepeats );
}

vector<string> parsetypes( const string& name)
{
  bool emptyrepeats=false;
  return tokenize_string( name, "|", emptyrepeats );
}


real_t DOCMD( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
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
	  //This will call recursively if fname is not local to model.
	  retval = READ( fname, model, cmds);
	}
    }
  
  return retval;
}


//REV: this is "fake" read...I want to compile it into a "real" read, which requires actually looking at correspondence! :)
//If correspondences are not made yet, then whatever?
//So, yea, they should take "symvar" after all ;)
//Which variable I "read" will depend on my "local idx" in the model (i.e. what is my thread?)
//Can I pass that? I.e. in reality it will iterate each one in turn, calling "model" with "index in model".
//Literally a for loop through the model indexes? :) Calling update function.
//Need to check variable update dependencies to know if we need to automatically generate new variables?

//REV: I am making this over-complex. But, I don't want to "reprogram" these, so better to have it actually access
//the variable with offset, based on calling model?
//But, I'm making it do too many things. Best idea is to have differnet structs to do the actual lookup etc.
//Problem is that, it always must know member index in it?
//Which means all variables must pass the index through ;)
//"MODEL" is the model I am "calling" from (?)
//In which case it might go inside a "sub" (hole) different model at some point. In which case, index basis will change.
real_t READ( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  //This parses into variable name!!!
  vector<string> parsed = cmds.doparse( arg );
  //Set some "var_counter" in model to be read.
  if( parsed.size() != 1 )
    {
      exit(1);
    }

  //May still be blah/blah/blah
  string varname = parsed[0];

  //"I" am the reading model. SO, I need to do model ->get
  std::shared_ptr<corresp> c;
  bool gotcorresp = model->getcorresp( model->readvar( varname ), c );
  if( !gotcorresp )
    {
      fprintf(stderr, "REV: in READ, error, could not get correspondence of varname [%s] from model [%s]\n", varname.c_str(), model->buildpath().c_str());
      exit(1);
    }
  //symvar& v = model->readvar( varname );
  
  //REV: HERE -- at model.getvar, we need to recursively search
  //This will do the recursive search
  //This is a dummy call!
  real_t val;
  if( model->readvar( varname ).valu.size() == 0 )
    {
      val = 0;
    }
  else //this is a real call!
    {
      size_t idx = 0; //is idx of calling guy?
      
      //Need to like "get nth guy", or need to "for all guys, do x"? Where it will use an iterator variable?
      //Whenever you read a variable, you need to make sure that you are going through the correct correspondence? At each point, it will go through a "correspondence"
      //REV; FUCK FUCK FUCK problem is if we go "through" hole, then "back" it will go many-to-one one way (which is fine), but then when I go "back", it won't know
      //which one of the many was the original one I accessed? So I need to keep an index variable to remember all models what my index in that model was?
      //Like, a model "trace"
      //This is final problem I need to solve...which is how/what index to pass with?
      //val = get_via_corresp( model->readvar( parsed[0] ), idx) );
    }
  
  return val;
}


//REV ERROR is parsed[0] is not a variable!!
real_t SET( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    {
      exit(1);
    }

  
  
  string toexec = parsed[1];
  real_t val = DOCMD( toexec, model, cmds );

  string varname = parsed[0];

  //"I" am the reading model. SO, I need to do model ->get
  std::shared_ptr<corresp> c;
  bool gotcorresp = model->getcorresp( model->getvar( varname ), c );
  if( !gotcorresp )
    {
      fprintf(stderr, "REV: in SET, error, could not get correspondence of varname [%s] from model [%s]\n", varname.c_str(), model->buildpath().c_str());
      exit(1);
    }
  
  //Could have been read and set separately?
  model->setvar( varname, val );
  
  return 0;
}


//Easier to just make sure it's 2...
real_t SUM( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
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
real_t MULT( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
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
real_t DIV( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];

  real_t val=DOCMD( toexec, model, cmds );
  
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val /= DOCMD( toexec, model, cmds );
    }

  return val;
}

//subtract
real_t DIFF( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
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


real_t NEGATE( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val= DOCMD( toexec, model, cmds );
  return (-1.0 * val);
}

real_t EXP( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val = DOCMD( toexec, model, cmds );

  return exp( val );
}

//PROBLEM: How to know what it is talking about "inside" the hole? I assume in this case, it will be referring to the base variables (as local to the returned model)
//Which means...how do I know about correspondence etc.?
//Might be better to do like, sumall( current/V ), which will iterate through all current/V for each member.
//Or, pass it all the way through, like SUMALL( current, SUM(current/V, current/E) ). So, in other words, there is no need to explicitly know it's a hole?
//Although holes contain multiple, which we know. We could do like a "for all by type", which is basically what currents are?
//Note, some current/cond models might be LOCAL, others foreign?
//I guess, in this case, all local...? Shit.
real_t SUMFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if(parsed.size() != 2 )
    {
      exit(1);
    }
  
  hole myhole = model->gethole( parsed[0] );
  
  string toexec = parsed[1];
  real_t val=0;
  for( size_t h=0; h<myhole.members.size(); ++h)
    {
      std::shared_ptr<symmodel> holemod = myhole.members[h];
      val += DOCMD( toexec, holemod, cmds ); //does DOCMD return a real_t val? Fuck...
    }
  return val;
}

//How to deal with numerals? Just parse them as base vectors...
real_t MULTFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if(parsed.size() != 2 )
    {
      exit(1);
    }

  hole myhole = model->gethole( parsed[0] );
  
  string toexec = parsed[1];
  real_t val=1.0;
  for( size_t h=0; h<myhole.members.size(); ++h)
    {
      std::shared_ptr<symmodel> holemod = myhole.members[h];
      val *= DOCMD( toexec, holemod, cmds ); //does DOCMD return a real_t val? Fuck...
    }
  
  return val;
}



void hole::add( const std::shared_ptr<symmodel>& h )
  {
    members.push_back(h);
    
    if( parent->is_submodel( h ) ) // is *NOT* external.
      {
	external.push_back( false );
      }
    else
      {
	external.push_back( true );
      }
    //set external or not?
    //by checking whether h is a submodel of this? E.g. iterate h until it hits null or this.
  }



#define ADDFUNCT(fname)							\
  {									\
    cmd_functtype fa = fname;						\
    add( #fname, fa );							\
  }

cmdstore::cmdstore()
  {
    ADDFUNCT( DOCMD );
    ADDFUNCT( READ );
    ADDFUNCT( SET );
    ADDFUNCT( SUM );
    ADDFUNCT( MULT );
    ADDFUNCT( DIV );
    ADDFUNCT( DIFF );
    ADDFUNCT( EXP );
    ADDFUNCT( NEGATE );
    ADDFUNCT( SUMFORALL );
    ADDFUNCT( MULTFORALL );
  }




