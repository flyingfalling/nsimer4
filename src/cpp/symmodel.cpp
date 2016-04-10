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


//real_t DOCMD( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(DOCMD)
{
  //Cannot be comma separated (although we could do multiple in one line that way?)
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    {
      fprintf(stderr, "REV: cmd parsed is not size one in DOCMD\n");
      exit(1);
    }
  
  vector<string> functparse = cmds.fparse( arg );
  string fname = functparse[0]; //name of registered function in cmdstore that iI will be calling
  
  functparse.erase( functparse.begin() );
  vector<string> fargs = functparse; //array of string args I will pass as arg to fname
  
  string newarg = CAT( fargs, "," ); //array of string args as single string...
  
  cmd_functtype execfunct;
  bool found = cmds.findfunct( fname, execfunct );
  real_t retval;
  if( found )
    {
      //retval = execfunct( newarg, model, cmds, myidx, targidx );
      retval = execfunct( newarg, trace, cmds );
    }
  else
    {
      bool isnumer = checknumeric( fname, retval );

      if( isnumer == false )
	{
	  //This will call recursively if fname is not local to model.
	  retval = READ( fname, trace, cmds );
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
//real_t READ( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )

//In this case, everything will work because it is reading directly a value. However, the problem is that the "returned" value...is what?
//If I go inside and execute recursively, e.g. for all of a hole of that sub-model, then it will lose track of the local idx? No...no it won't.
//I'm iterating through the idx HERE, and passing those idx in the sub-function. So it is using those to search for all members in there I guess. Fine.
FUNCDECL(READ)
{
  //This parses into variable name!!!
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    {
      exit(1);
    }
  
  //May still be blah/blah/blah
  string varname = parsed[0];
  
  //"I" am the reading model. SO, I need to do model ->get
    
  
  //std::shared_ptr<symmodel> contmodel = get_containing_model_widx( varname, trace );
  elemptr ep = get_containing_model_widx( varname, trace );
  std::shared_ptr<symvar> var = ep.model->getvar( varname );
  std::shared_ptr<symmodel> contmodel = var->parent; //haha, true containing model... implies it stepped up  hierarchy some.
  
  std::shared_ptr<corresp> corr = getcorresp( contmodel, trace );
    
  real_t val;

  //REV: handles not being initialized
  val = var->getvalu( ep.idx );

  
  return val;
}


//REV ERROR is parsed[0] is not a variable!!
//real_t SET( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(SET)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    {
      exit(1);
    }

  string toexec = parsed[1];
  real_t val = DOCMD( toexec, trace, cmds );
  
  string varname = parsed[0];

  //std::shared_ptr<symmodel> contmodel = get_containing_model_widx( varname, trace );
  elemptr ep = get_containing_model_widx( varname, trace );
  std::shared_ptr<symvar> var = ep.model->getvar( varname );
  std::shared_ptr<symmodel> contmodel = var->parent; //haha, true containing model... implies it stepped up  hierarchy some.
  
  std::shared_ptr<corresp> corr = getcorresp( contmodel, trace );
    
  //Could have been read and set separately?
  var->setvalu( ep.idx, val );
  
  return -66666666666;
}


//Only other time I have to worry about idx, is when I do "forall"
FUNCDECL(SUM)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }
  
  real_t val=0;
  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val += DOCMD( toexec, trace, cmds );
    }

  return val;
}


//product
//real_t MULT( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(MULT)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  real_t val=1.0;

  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val *= DOCMD( toexec, trace, cmds );
    }

  return val;
}

//div
//real_t DIV( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(DIV)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];

  real_t val=DOCMD( toexec, trace, cmds );
  
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val /= DOCMD( toexec, trace, cmds );
    }

  return val;
}

//subtract
//real_t DIFF( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(DIFF)
{
  //subtract all from first one?
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val=DOCMD( toexec, trace, cmds );
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val -= DOCMD( toexec, trace, cmds );
    }
  
  return val;
}


//real_t NEGATE( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(NEGATE)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val= DOCMD( toexec, trace, cmds );
  return (-1.0 * val);
}

//real_t EXP( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(EXP)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val = DOCMD( toexec, trace, cmds );

  return exp( val );
}

FUNCDECL( SUMFORALL )
{
  vector<string> parsed = cmds.doparse( arg );
  
  if(parsed.size() == 2 )
    {
      return SUMFORALLHOLES( arg, trace, cmds );
    }
  else if(parsed.size() == 1)
    {
      return SUMFORALLCONNS( arg, trace, cmds );
    }
  else
    {
      fprintf(stderr, "REV: SUMFORALL is not appropriately spread...\n");
      exit(1);
    }
}


FUNCDECL( SUMFORALLHOLES )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if( parsed.size() != 2 )
    {
      fprintf(stderr, "REV: some big mistake SUMFORALLHOLES, parsed size != 2\n");
      exit(1);
    }
  
  string holename = parsed[0];
  string toexec = parsed[1];
  
  elemptr currmodel = get_curr_model( trace );
  hole myhole = currmodel.model->gethole( holename ); 
  
  real_t val=0;
  for( size_t h=0; h<myhole.members.size(); ++h)
    {
      std::shared_ptr<symmodel> holemod = myhole.members[h];
      
      val += exec_w_corresp( toexec, holemod, trace, cmds );
    }
  return val;
} //end SUMFORALLHOLES

FUNCDECL( SUMFORALLCONNS )
{
  vector<string> parsed = cmds.doparse( arg ); //Make sure it is a legal statement (i.e. not args comma sep)
  if( parsed.size() != 1 )
    {
      fprintf(stderr, "REV: some big mistake SUMFORALLCONNS, parsed size != 1\n");
      exit(1);
    }
  
  if(trace.size() < 2)
    {
      fprintf(stderr, "ERROR, In SUMFORALLCONNS, trace is < 2 (i.e. not prev model and corrent model)\n");
      exit(1);
    }
  
  elemptr currmodel = trace[ trace.size()-2 ];
  elemptr holemodel = trace[ trace.size()-1 ];
  vector<elemptr> newtrace = trace;
  
  //elemptr currmodel = get_curr_model(trace);
  std::shared_ptr<corresp> corr = getcorresp( currmodel.model,  holemodel.model ); //REV: this may have returned the identity pointer if they are the same model?
  
  real_t val=0;
  
  vector<size_t> mypost = corr->getall( currmodel.idx );

  for( size_t i = 0; i<mypost.size(); ++i )
    {
      size_t idx = mypost[ i ];
      newtrace[ newtrace.size()-1 ].idx = idx; //set idx to correct idx for execution.
      val += DOCMD( arg, newtrace, cmds );
    }

  return val;
  
} //end SUMFORALLCONNS

FUNCDECL( MULTFORALL )
{
  vector<string> parsed = cmds.doparse( arg );
  
  if(parsed.size() == 2 )
    {
      return MULTFORALLHOLES( arg, trace, cmds );
    }
  else if(parsed.size() == 1)
    {
      return MULTFORALLCONNS( arg, trace, cmds );
    }
  else
    {
      fprintf(stderr, "REV: MULTFORALL is not appropriately spread...\n");
      exit(1);
    }
} //end MULTFORALL


FUNCDECL( MULTFORALLHOLES )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if( parsed.size() != 2 )
    {
      fprintf(stderr, "REV: some big mistake SUMFORALLHOLES, parsed size != 2\n");
      exit(1);
    }
  
  string holename = parsed[0];
  string toexec = parsed[1];

  elemptr currmodel = get_curr_model( trace );
  hole myhole = currmodel.model->gethole( holename ); 
    
  real_t val=1;
  for( size_t h=0; h<myhole.members.size(); ++h)
    {
      std::shared_ptr<symmodel> holemod = myhole.members[h];
      
      val *= exec_w_corresp( toexec, holemod, trace, cmds );
    }
  return val;
} //end MULTFORALLHOLES

FUNCDECL( MULTFORALLCONNS )
{
  vector<string> parsed = cmds.doparse( arg ); //Make sure it is a legal statement (i.e. not args comma sep)
  if( parsed.size() != 1 )
    {
      fprintf(stderr, "REV: some big mistake SUMFORALLCONNS, parsed size != 1\n");
      exit(1);
    }
  
  if(trace.size() < 2)
    {
      fprintf(stderr, "ERROR, In SUMFORALLCONNS, trace is < 2 (i.e. not prev model and corrent model)\n");
      exit(1);
    }
  
  elemptr currmodel = trace[ trace.size()-2 ];
  elemptr holemodel = trace[ trace.size()-1 ];
  vector<elemptr> newtrace = trace;
  
  //elemptr currmodel = get_curr_model(trace);
  std::shared_ptr<corresp> corr = getcorresp( currmodel.model,  holemodel.model ); //REV: this may have returned the identity pointer if they are the same model?
  
  real_t val=1;
  
  vector<size_t> mypost = corr->getall( currmodel.idx );

  for( size_t i = 0; i<mypost.size(); ++i )
    {
      size_t idx = mypost[ i ];
      newtrace[ newtrace.size()-1 ].idx = idx; //set idx to correct idx for execution.
      val *= DOCMD( arg, newtrace, cmds );
    }

  return val;
  
} //end MULTFORALLCONNS



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

    ADDFUNCT( SUMFORALLHOLES );
    ADDFUNCT( MULTFORALLHOLES );
    ADDFUNCT( SUMFORALLCONNS );
    ADDFUNCT( MULTFORALLCONNS );
  }



real_t symvar::getvalu( const size_t& idx )
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

void symvar::setvalu( const size_t& idx, const real_t& val )
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
  elemptr lastguy = get_curr_model(trace); //trace[trace.size()-1];
  
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
  if( s.compare( mult ) == 0 || s.compare( sum ) == 0 )
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

  
  if( check_cmd_is_multi( toexec ) )
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
      elemptr currmodel = get_curr_model(trace);
      std::shared_ptr<corresp> corr = getcorresp( currmodel.model, m );
      size_t curridx = currmodel.idx;
      vector<size_t> c = corr->getall( curridx );
      if( c.size() != 1 )
	{
	  fprintf(stderr, "REV: SUPER ERROR in exec_w_corresp, trying to execute [%s] but model [%s]->[%s] is one-to-many (or zero) ([%lu])\n", toexec.c_str(), currmodel.model->buildpath().c_str(), m->buildpath().c_str(), c.size() );
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
  
  elemptr lastguy = get_curr_model(trace); //trace[trace.size()-1];
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
