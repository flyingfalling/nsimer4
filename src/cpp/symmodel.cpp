//REV: 4 Apr 2016
//Contains symmodel, for constructing symbolic models

//TODO fix types of guys...


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

vector<string> parsecorr( const string& name)
{
  //bool emptyrepeats=false;
  bool emptyrepeats=true;
  return tokenize_string( name, "->", emptyrepeats );
}


//REV: TYPE
// varptr funct( string, trace, cmds, globalstore )
// All accesses are done via reference to globalstore and trace, we don't do raw TRACE gets usually right?
// Hm, make TRACE a trace struct, which has a global_store, and a thing?
// Um, shit, fuck, shit. Where are global stores created? They are created possibly in every update function? It may use a "new" and "local" global store.
// But, all have access to a (copy of?) the GLOBAL global store? Which may have some things like dt etc.?
// Does it "set" dt? I can never modify dt I guess... A local copy of DT?
// The local globalstore is used for tmp variables, and also for accessing temp variables. So...we have two global stores. One for tmp, and one that has "global"
// variables like DT, etc.? We "choose" which ones to go into? variables like dt are automatically written there I guess... It's in global store, i.e. top level model
// Oh well. EZ. Same one, just don't overlap rofl. Global store models are DIRECTLY variables. Good.
// When I call "GET CORRESP", what does it do? It looks for the one that has the match between them. First it checks if they are part of same "parent" model?
// in this case, !parent, so nothing will happen (fuck?). They will never be part of same parent, so that is good.
// How do I handle schedulers? :)

//I  need to manually set the correspondence? For some cases it is start/size/corresp. I only set corresp, and compute start/size from there. For guys I manaully create
//I assume it needs blah. But what if I pass a size/start cmd as well? Then I can construct that way? rofl fuck me.

// I always specify "big-small". And it generates everything else ;)


//REV: first, fix this.
//Make sure it searches through globals as well (to find variables, not functs)
//Make sure corresp are appropriately checked?
//Vartypes contain their own thing, like they might be "PARAM", in which case, I don't look for a corresp or some shit.
//Or I return a special "const" guy that always returns 0 heh.
FUNCDECL(DOCMD)
{
  //First, replace it with locals...
  string rearg = cmds.handlelocal( rearg );
  
  //Cannot be comma separated (although we could do multiple in one line that way?)
  vector<string> parsed = cmds.doparse( rearg );
  if( parsed.size() != 1 )
    {
      fprintf(stderr, "REV: cmd parsed is not size one in DOCMD [%s]\n", rearg.c_str());
      exit(1);
    }
    
  vector<string> functparse = cmds.fparse( rearg );
  string fname = functparse[0]; //name of registered function in cmdstore that iI will be calling
  
  functparse.erase( functparse.begin() );
  vector<string> fargs = functparse; //array of string args I will pass as arg to fname
  
  string newarg = CAT( fargs, "," ); //array of string args as single string...

  fprintf(stdout, "Model [%s]: DOCMD [%s] (arg [%s])\n", get_curr_model(trace).model->buildpath().c_str(), fname.c_str(), newarg.c_str());
  
  cmd_functtype execfunct;
  bool found = cmds.findfunct( fname, execfunct );
  real_t retval;
  if( found )
    {
      //retval = execfunct( newarg, model, cmds, myidx, targidx );
      retval = execfunct( newarg, trace, cmds, globals );
    }
  else
    {
      bool isnumer = checknumeric( fname, retval );

      if( isnumer == false )
	{
	  //This will call recursively if fname is not local to model.
	  retval = READ( fname, trace, cmds, globals );
	}
    }
  
  return retval;
}

//Problem is it has a pointer based on what "type" is, and I have to selectively change based on that heh...
//Just do in READ and SET, much easier (problem is when I go to try and find the one to push to, etc.)

//Anyway, fine...

//Issue now is to sort out my method of referencing them ! :)
//After parsing "varname", I do:
//If IDX, I return IDX,
//If SIZE, I return that model's SIZE (note, model could be a corresp, or a var!)
//If X->Y, it is a corrsp. So, I'm going to push back to CORRESP, or something...
//That's fine. I want to get..?

//Note, I'm going to generate for syn2-1. Specifically, I reference presyn and postsyn. So, I even more specifically. So, I specifically want to do ->presyn and ->postsyn.
//-> indicates "source" Ah, got it, empty means nothing. -> means conn to X. So, I divide by -> and it gives left is empty, right is blah. Fine. Right means "me" :)

//Shit, problem, what does X->Y mean where there is slashes? I assume in both cases, it refers to a CORRESPONDENCE. I.e. the correspondence itself.
//Yea, so literally, for presyn model idxs (X) that are passed, what are corresponding postsyn model idxs. Note, I "getall" for each.

//Easiest to just have only real types in there...problem is when I return "idxs" it does something...
//Note, in SET I have to do something to...?



elemptr findmodel( const string& s, const vector<elemptr>& trace, const global_store& globals )
{
  auto mod =  get_model_widx( s, trace ); //This only iterates up to top level within me....I need to do a full search.
  if( mod )
    {
      return mod;
    }
  else
    {
      mod = globals.findmodel( s );
      if( mod )
	{
	  return mod;
	}
      else
	{
	  //try searching root?
	  elemptr ep = get_curr_model(trace);
	  auto root = ep.model->get_root();
	  elemptr tmp( root, ep.idx );
	  vector<elemptr> newtrace;
	  newtrace.push_back( tmp );
	  mod = get_model_widx( s, newtrace ); //yolo.
	  if( mod )
	    {
	      return mod;
	    }
	  else
	    {
	      fprintf(stderr, "ERRO could not find model, even through globals, parents, and bubbling through trace (last trace is [%s])\n" , s.c_str(), ep.model->buildpath().c_str());
	      exit(1);
	    }
	}
    }
  //FINDS MODEL IN BOTH GLOBALS AND LOCALS?
}

bool check_idx( const string& varname )
{
  if( varname.compare( "IDX" ) == 0 )
    {
      return true;
    }
  return false;
}

bool check_issize( const string& varname )
8{
  if( varname.compare( "SIZE" ) == 0 )
    {
      return true;
    }
  return false;
}

//REV: shit, if I parse "/" will it give me "nothing" or will it give me ""/""?
//I'm leaning towards nothing? Heh...
bool check_iscorr( const string& varname, string& presmodelname, string& postmodelname )
{
  vector<string> res = parsecorr( varname );
  
  if( res.size() == 2 )
    {
      premodelname = res[0];
      postmodelname = res[1];
      return true;
    }
  return false;
}


//REV: trace will contain the most recent index? I guess? Why use trace????
//REV: trae contains last idx?
varptr get_proper_var_widx( const string& varname, const vector<elemptr>& trace, const global_store& globals )
{
  varptr vp;
  
  string vartail;
  string premod, postmod;
  bool iscorr = check_iscorr( varname, premod, postmod );

  //With iscorr, the model itself is the ->. In other words, I will parse it first into pre-> and post->
  //I will *directly* get the pre model. In pre model, I will directly access the variable ->POSTMODEL

  string containingpath = get_containing_model_path( varname, vartail );

  elemptr ep;
  if( !iscorr )
    {
      ep = findmodel( containingpath, trace, globals );
    }
  else
    {
      //Problem is, if it is "", we are fucked? It *always* must be called from inside a model. If the model is syn2-1 fine. That will be on trace?
      ep = findmodel( premod, trace, globals );
    }

  bool isidx = check_idx( vartail );
  bool issize = check_issize( vartail );

  if( isidx )
    {
      //Literally get my "idxs" in that model (whoa...?)
      //So, get correspondence, this is same as corresp? Fuck...
      vector<size_t> idxs = ep.idx;
      vp.idx = idxs;
      return vp;
    }
  else if( issize )
    {
      size_t size = ep.model->get_toplevel_model()->modelsize;
      vector<size_t> v( 1, size );
      vp.idx = v;
      return vp;
      //get size of "that" model
    }
  else if( iscorr )
    {
      //get final model haha
      elemptr m2 = findmodel( postmod, trace, globals );
      //auto curr = get_current_model(trace);
      std::shared_ptr<corresp> mycorr;
      bool foundcorr = ep.model->getcorresp( m2, mycorr );
      if( !foundcorr )
	{
	  fprintf(stderr, "REV: rofl stupid richard, trying to read a corresp that doesn't exist\n");
	  exit(1);
	}
      else
	{
	  //build and return from corresp. Based on idx.
	  //Do I automatically append?
	  vector<size_t> myidx = ep.idx;
	  if(myidx.size() != 1 )
	    {
	      fprintf(stderr, "REV: in getvar for real, something fxxup, myidx in IS CORR is not 1 [%s]->[%s]\n", premod.c_str(), postmod.c_str());
	      exit(1);
	    }
	  vector<size_t> ret = mycorr->getall( myidx[0] );
	  vp.idx = ret;
	  return vp;
	}
    }
  else
    {
      vector<size_t> loc = ep.model->get_varloc( vartail );
      //fprintf(stdout, "REV: doen getting varloc of requested var, size  is [%lu]\n", loc.size() );
      if(loc.size() == 0 )
	{
	  if( ep.model->parent )
	    {
	      //REV: what is IDX here? haha...

	      //Replace last one with parent and go?
	      vector<elemptr> newtrace = trace;
	      elemptr tmp( ep.model->parent, ep.idx );
	      newtrace.pop_back();
	      newtrace.push_back( tmp );
	      return get_proper_var_widx(varname, newtrace, globals); //ep.model->parent->getvar_widx( varname, ep.idx, trace );
	    }
	  else
	    {
	      fprintf(stderr, "GETVAR, could not get variable through model hierarchy. Note I am now at root level so I do not know model name...[%s]\n", varname.c_str());
	      exit(1);
	    }
	}
      else if(loc.size() > 1 )
	{
	  fprintf(stderr, "REV weird more than one var of same name [%s]\n", varname.c_str());
	  exit(1);
	}
      else
	{
	  //REV: SANITY CHECK, this will return SAME if it is SAME,
	  //Will return CONST if it is const ;)
	  auto corr = getcorresp( ep.model, trace );
	  if( !corr )
	    {
	      fprintf(stderr, "REV: error no correspondence exists between models but I'm trying to link them? SHIT\n");
	      exit(1);
	    }
	  if( ep.idx.size() != 1 )
	    {
	      fprintf(stderr, "REV: we have a problem houston...\n");
	      exit(1);
	    }
	  vector<size_t> transformed = corr->getall( ep.idx[0] );
	  
	  auto realvar = ep.model->vars[ loc[0] ];
	  vector<real_t> vals = var->getvalus( transformed );
	  vector<size_t> idxs; //what are these? Literally idxs? Make sure idxs always points directly there? Nah... Just leave it empty?
	  vp.valu = vals;
	  return vp;
	}
      
    } //end ELSE (it is normal var)
} //end get proper var





void set_proper_var_widx(const string& varname, const vector<elemptr>& trace, const global_store& globals, const varptr& vp )
{
  string vartail;
  string premod, postmod;
  bool iscorr = check_iscorr( varname, premod, postmod );

  //With iscorr, the model itself is the ->. In other words, I will parse it first into pre-> and post->
  //I will *directly* get the pre model. In pre model, I will directly access the variable ->POSTMODEL

  string containingpath = get_containing_model_path( varname, vartail );
  
  elemptr ep;
  if( !iscorr )
    {
      ep = findmodel( containingpath, trace, globals );
    }
  else
    {
      //Problem is, if it is "", we are fucked? It *always* must be called from inside a model. If the model is syn2-1 fine. That will be on trace?
      ep = findmodel( premod, trace, globals );
    }

  bool isidx = check_idx( vartail );
  bool issize = check_issize( vartail );

  if( isidx )
    {
      //Literally get my "idxs" in that model (whoa...?)
      //So, get correspondence, this is same as corresp? Fuck...
      //vector<size_t> idxs = ep.idx;
      //REV: I never "set" indices, so OK.
      //vp.idx = idxs;
      //return vp;
      fprintf(stderr, "REV: error, trying to SET indices...lol\n");
      exit(1);
    }
  else if( issize )
    {
      //size_t size = ep.model->get_toplevel_model()->modelsize;
      //vector<size_t> v( 1, size );
      //vp.idx = v;
      //return vp;
      fprintf(stderr, "REV: error trying to directly SET size!!\n");
      exit(1);
    }
  else if( iscorr )
    {
      //get final model haha
      elemptr m2 = findmodel( postmod, trace, globals );
      //auto curr = get_current_model(trace);
      std::shared_ptr<corresp> mycorr;
      bool foundcorr = ep.model->getcorresp( m2, mycorr );
      if( !foundcorr )
	{
	  fprintf(stderr, "REV: rofl stupid richard, trying to read a corresp that doesn't exist\n");
	  exit(1);
	}
      else
	{
	  //build and return from corresp. Based on idx.
	  //Do I automatically append?
	  //How to set it? Lol, I'm literally setting only "major" size. Thus, start/etc. are all the same (identity)
	  
	  vector<size_t> myidx = ep.idx;
	  /*if(myidx.size() != 1 )
	    {
	      fprintf(stderr, "REV: in getvar for real, something fxxup, myidx in IS CORR is not 1 [%s]->[%s]\n", premod.c_str(), postmod.c_str());
	      exit(1);
	      }*/

	  vector<size_t> newvals = vp.idx;
	  mycorr->set( myidx, newvals );
	  return;
	  
	}
    }
  else
    {
      vector<size_t> loc = ep.model->get_varloc( vartail );
      //fprintf(stdout, "REV: doen getting varloc of requested var, size  is [%lu]\n", loc.size() );
      if(loc.size() == 0 )
	{
	  if( ep.model->parent )
	    {
	      //REV: what is IDX here? haha...
	      
	      //Replace last one with parent and go?
	      vector<elemptr> newtrace = trace;
	      elemptr tmp( ep.model->parent, ep.idx );
	      newtrace.pop_back();
	      newtrace.push_back( tmp );
	      return get_proper_var_widx(varname, newtrace, globals); //ep.model->parent->getvar_widx( varname, ep.idx, trace );
	    }
	  else
	    {
	      fprintf(stderr, "GETVAR, could not get variable through model hierarchy. Note I am now at root level so I do not know model name...[%s]\n", varname.c_str());
	      exit(1);
	    }
	}
      else if(loc.size() > 1 )
	{
	  fprintf(stderr, "REV weird more than one var of same name [%s]\n", varname.c_str());
	  exit(1);
	}
      else
	{
	  //REV: SANITY CHECK, this will return SAME if it is SAME,
	  //Will return CONST if it is const ;)
	  auto corr = getcorresp( ep.model, trace );
	  if( !corr )
	    {
	      fprintf(stderr, "REV: error no correspondence exists between models but I'm trying to link them? SHIT\n");
	      exit(1);
	    }
	  if( ep.idx.size() != 1 )
	    {
	      fprintf(stderr, "REV: we have a problem houston...\n");
	      exit(1);
	    }
	  //This should be roughly 1-1 right? Will I ever be trying to SET
	  //values in a "different" array?
	  //In the other case, I was trying to "get" LARGE-SMALL.?
	  //So, 2, got a bunch of guys. And then I returned those.
	  //In this case, it BETTER be the other way around...
	  //Like, this kind of actually will work maybe though?
	  //If I want to set all POSTSYN guys or some shit, to some value?
	  vector<size_t> transformed = corr->getall( ep.idx[0] );
	  
	  if(transformed.size() != 1)
	    {
	      fprintf(stderr, "Haha, trying to write VAR 1->may through a corresp\n");
	      exit(1);
	    }
	  	  
	  auto realvar = ep.model->vars[ loc[0] ];
	  //vector<real_t> vals = var->getvalus( ep.idx );
	  //vector<size_t> idxs; //what are these? Literally idxs? Make sure idxs always points directly there? Nah... Just leave it empty?
	  realvar->setvalus( transformed, vp.valu );
	  return;
	}
      
    }
} //end set var proper
		     


//REV: Handle globals here
//REV: handle checking if name is a READ_CORRESP or READ_IDX or READ_SIZE or READ_NORMAL, and return appropriately.
//For all other guys, if type is incorrect, they error out (e.g. trying to pass idx to a mult funct?)
//Fuck it, if user fucks up, that's his problem? lol...

//REV: the models return the variable things (?) to write to? Nah, do it here...?
//They can return a corresp or a var wrapped? I'll have to code all this logic anyways...for CUDA
//Main problem is that I get "containing model" which is fine. It returns the containing model. And then, after that,
FUNCDECL(READ)
{
  fprintf( stdout, "Executing READ arg: [%s]\n", arg.c_str() );
  //This parses into variable name!!!

  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    {
      exit(1);
    }
  //May still be blah/blah/blah
  string varname = parsed[0];
  
  return get_proper_var_widx( varname, trace, globals );
  
}




FUNCDECL(SET)
{
  fprintf( stdout, "Executing SET arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    {
      exit(1);
    }
  
  string toexec = parsed[1];
  varptr res = DOCMD( toexec, trace, cmds, globals );

  string varname = parsed[0];

  set_proper_var_widx( varname, trace, globals, res );

  varptr emptyv;
  
  return emptyv;
}



//Only other time I have to worry about idx, is when I do "forall"
FUNCDECL(SUM)
{
  fprintf( stdout, "Executing SUM arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }
  
  real_t val=0;
  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val += DOCMD( toexec, trace, cmds, globals );
    }

  return val;
}


//Better way:
// get "vector" of values by coalescing multiple args.
//Then call MULT(<vect>) on it! OK.


//Whoa problem. How to deal with IDXS.
//Mult-idxs can only be generated by FORALL (not true?)

//product
//If only one arg, it automatically mults them together? Etc.
FUNCDECL(MULT)
{
  fprintf( stdout, "Executing MULT arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  real_t val=1.0;
  
  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val *= DOCMD( toexec, trace, cmds, globals );
    }

  return val;
}

//div
FUNCDECL(DIV)
{
  fprintf( stdout, "Executing DIV arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];

  real_t val=DOCMD( toexec, trace, cmds );
  
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val /= DOCMD( toexec, trace, cmds, globals );
    }

  return val;
}

//subtract
FUNCDECL(DIFF)
{
  fprintf( stdout, "Executing DIFF arg: [%s]\n", arg.c_str() );
  //subtract all from first one?
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val=DOCMD( toexec, trace, cmds );
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val -= DOCMD( toexec, trace, cmds, globals );
    }
  
  return val;
}


FUNCDECL(NEGATE)
{
  fprintf( stdout, "Executing NEGATE arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val= DOCMD( toexec, trace, cmds, globals );
  return (-1.0 * val);
}

FUNCDECL(EXP)
{
  fprintf( stdout, "Executing EXP arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val = DOCMD( toexec, trace, cmds, globals );

  return exp( val );
}

FUNCDECL(GAUSSRAND)
{
  fprintf( stdout, "Executing GAUSSRAND arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    { exit(1); }

  string meanstr = parsed[0];
  string stdstr = parsed[1];
  real_t meanval = DOCMD( meanstr, trace, cmds, globals );
  real_t stdval = DOCMD( stdstr, trace, cmds, globals );

  std::normal_distribution<real_t> mydist( meanval, stdval );

  //CMDS contains the randgen I guess?
  real_t val = mydist( cmds.RANDGEN );
  return val;
}

FUNCDECL(UNIFORMRAND)
{
  fprintf( stdout, "Executing UNIFORMRAND arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    { exit(1); }

  string minstr = parsed[0];
  string maxstr = parsed[1];
  real_t minval = DOCMD( minstr, trace, cmds, globals );
  real_t maxval = DOCMD( maxstr, trace, cmds, globals );

  std::uniform_distribution<real_t> mydist( minval, maxval );

  //CMDS contains the randgen I guess?
  real_t val = mydist( cmds.RANDGEN );
  return val;
}

FUNCDECL( SUMFORALL )
{
  fprintf( stdout, "Executing SUMFORALL arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  
  if(parsed.size() == 2 )
    {
      return SUMFORALLHOLES( arg, trace, cmds, globals );
    }
  else if(parsed.size() == 1)
    {
      return SUMFORALLCONNS( arg, trace, cmds, globals );
    }
  else
    {
      fprintf(stderr, "REV: SUMFORALL is not appropriately spread...\n");
      exit(1);
    }
}


FUNCDECL( SUMFORALLHOLES )
{
  fprintf( stdout, "Executing SUMFORALLHOLES arg: [%s]\n", arg.c_str() );
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

      //REV: this is the hack to get a single value out of something that would return multiple.
      //Easier to make everything instead do vector-stuff, i.e. match it. Problem is if it encounters different sizes at different points "inside" one of those elements.
      //By default, make everything work in lockstep. Only error out if I would return a "naked" array. No, only error out if I would
      //Combine elements that are not of same size.
      //Note, parallelization only happens at the "first level" stage anyway ;0. So, what happens if I SUM( DOCMD( blah1, blah2) ). blah1/blah2, return Ndim array,
      //I sum, so I get Ndim array back. I.e. it does element-wise. However, at some point (lowest level) it must be only a single index. That is the difference.
      //Since execution does not spread per-member...fine. I don't need to worry about "index" being overwritten

      //REV: all update funct have implicit (FORALL(items, )) appended at beginning. I.e. that is what determines thread size.
      
      //Fuck all that for now? If I am just reading, that is fine. Problem is if I try to write or some shit? It causes problems, because it returns a READ type array
      //that is the issue. At the most basic level, when I want to SET, I directly get the array anyway. How do I know what SET size is.
      varptr v = exec_w_corresp( toexec, holemod, trace, cmds );
      if( v.idx.size() != 1 || v.valu.size() != 1 )
	{
	  fprintf(stderr, "ERROR in SUMFORALLHOLES due to exec_w_corresp reutrning non-single value/idx...");
	  exit(1);
	}
      if( v.idx[0] != 0 )
	{
	  fprintf(stderr, "Huh, SUM FOR ALL HOLES still error from DOCMD returning unrealistic idx??!!\n");
	  exit(1);
	}
      val += v.valu[0]; //same as v.valu[v.idx[0] ]?
    }
  return val;
} //end SUMFORALLHOLES

FUNCDECL( SUMFORALLCONNS )
{
  fprintf( stdout, "Executing SUMFORALLCONNS arg: [%s]\n", arg.c_str() );
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
  fprintf( stdout, "Executing MULTFORALL arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  
  if(parsed.size() == 2 )
    {
      return MULTFORALLHOLES( arg, trace, cmds, globals );
    }
  else if(parsed.size() == 1)
    {
      return MULTFORALLCONNS( arg, trace, cmds, globals );
    }
  else
    {
      fprintf(stderr, "REV: MULTFORALL is not appropriately spread...\n");
      exit(1);
    }
} //end MULTFORALL


FUNCDECL( MULTFORALLHOLES )
{
  fprintf( stdout, "Executing MULTFORALLHOLES arg: [%s]\n", arg.c_str() );
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

      varptr v = exec_w_corresp( toexec, holemod, trace, cmds );
      if( v.idx.size() != 1 || v.valu.size() != 1 )
	{
	  fprintf(stderr, "ERROR in MULTFORALLHOLES due to exec_w_corresp reutrning non-single value/idx...");
	  exit(1);
	}
      if( v.idx[0] != 0 )
	{
	  fprintf(stderr, "Huh, MULT FOR ALL HOLES still error from DOCMD returning unrealistic idx??!!\n");
	  exit(1);
	}
      val *= v.valu[0]; //same as v.valu[v.idx[0] ]?
      
    }
  return val;
} //end MULTFORALLHOLES

FUNCDECL( MULTFORALLCONNS )
{
  fprintf( stdout, "Executing MULTFORALLCONNS arg: [%s]\n", arg.c_str() );
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

vector<real_t> symvar::getvalus( const vector<size_t>& idx )
{
  
    if( !init )
      {
	return vector<real_t>(0);
      }

    vector<real_t> ret;
    for(size_t x=0; x<idx.size(); ++x)
      {
	ret.push_back( getvalu( idx[x] ) );
      }

    return ret;
  
}

void symvar::setvalu( const size_t& idx, const real_t& val )
{
  if( !init )
    {
      //do nothing
      return;
    }
    
  if( idx >= valu.size() )
    {
      fprintf(stderr, "In symvar, setvalu, idx [%lu] > size of valu array [%lu], var name [%s] in containing model [%s]\n", idx, valu.size(), name.c_str(), parent->buildpath().c_str());
      exit(1);
    }

  valu[idx] = val;
  return;
}

void symvar::setvalus( const vector<size_t>& idx, const vector<real_t>& val )
{
  if( !init )
    {
      //do nothing
      return;
    }

  if(val.size() != idx.size())
    {
      fprintf(stderr, "setvalus, error idx size and val size not same\n");
      exit(1);
    }
  
  for(size_t x=0; x<idx.size(); ++x)
    {
      setvalu( idx[x], val[x] );
    }
  
  return;
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


//REV: TODO NEED TO REWRITE THIS TO HANDLE idx being vector<size_t>
//What does this do? This takes a string toexec, target model, and trace, and executes it.
//This is *ONLY* calle when going through a hole I think (i.e. MULTFORALLHOLES, or SUMFORALLHOLES). In other words, nothing has changed, we now
//This will always only compute a SINGLE value, regardless of multi-idxs etc.

//real_t exec_w_corresp( const std::string& toexec, const std::shared_ptr<symmodel>& m, const vector<elemptr>& trace, const cmdstore& cmds )
varptr exec_w_corresp( const std::string& toexec, const std::shared_ptr<symmodel>& m, const vector<elemptr>& trace, const cmdstore& cmds, global_store& globals )
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
      //curr model is already pushed back. We need target model
      //Note, this arbitrary idx MUST BE OVERWRITTEN by the caller, set to appropriate value.
      size_t arbitrary_idxval = 666;
      vector<size_t> arbitrary_idx( 1, arbitrary_idx );
      elemptr tmpelem( m, arbitrary_idx ); //idx is arbitrary
      vector<elemptr> newtrace = trace;
      newtrace.push_back( tmpelem );

      //Note toexec is the whole thing passed, we didn't strip it.
      //we are just handling it in a special way based on #args and pushing back to trace.
      //DOCMD will strip the first part and execute it.
      return DOCMD( toexec, newtrace, cmds, globals );
    }
  else //Otherwise, I just execute it. But, in that, I make sure that corresp is not fucked up.
    {
      elemptr currmodel = get_curr_model(trace);

      //REV: Do I need to be careful here with GLOBALS vs etc.? Nah...
      
      std::shared_ptr<corresp> corr = getcorresp( currmodel.model, m );
      //size_t curridx = currmodel.idx;
      vector<size_t> vcurridx = currmodel.idx;
      
      if( vcurridx.size() != 1 )
	{
	  fprintf(stderr, "REV: whoa (ERROR), doing nested exec_w_corresp\n");
	  exit(1);
	}
      
      size_t curridx = vcurridx[0];
      
      vector<size_t> c = corr->getall( curridx );
      
      if( c.size() != 1 )
	{
	  fprintf(stderr, "REV: SUPER ERROR in exec_w_corresp, trying to execute [%s] but model [%s]->[%s] is one-to-many (or zero) ([%lu])\n", toexec.c_str(), currmodel.model->buildpath().c_str(), m->buildpath().c_str(), c.size() );
	  exit(1);
	}

      size_t newidxval = c[0];
      vector<size_t> newidx(1, newidxval);
      vector<elemptr> newtrace = trace;
      elemptr tmpelem( m, newidx );
      newtrace.push_back( tmpelem );

      return DOCMD( toexec, newtrace, cmds, globals);
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

elemptr get_containing_model_widx( const string& parsearg, const vector<elemptr>& trace, string& varname )
{
  
  elemptr lastguy = get_curr_model( trace );
  vector<elemptr> newtrace = trace;
  newtrace.pop_back();
  return lastguy.model->get_containing_model_widx( parsearg, lastguy.idx, newtrace, varname );
}
