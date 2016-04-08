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


real_t READ( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds )
{
  vector<string> parsed = cmds.doparse( arg );
  //Set some "var_counter" in model to be read.
  if( parsed.size() != 1 )
    {
      exit(1);
    }

  //REV: HERE -- at model.getvar, we need to recursively search
  //This will do the recursive search
  real_t val = model->readvar( parsed[0] ).valu;
  
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

  //Could have been read and set separately?
  model->setvar( parsed[0], val );
  
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




