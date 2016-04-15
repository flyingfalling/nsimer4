#include <symmodel.h>

FUNCDECL(DOCMD)
{
  //First, replace it with locals...
  string rearg = cmds.handlelocal( arg );
  
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
  varptr retval;
  if( found )
    {
      //retval = execfunct( newarg, model, cmds, myidx, targidx );
      retval = execfunct( newarg, trace, cmds, globals );
    }
  else
    {
      real_t res;
      bool isnumer = checknumeric( fname, res );
      

      if( isnumer == false )
	{
	  //This will call recursively if fname is not local to model.
	  //fprintf(stdout, "Trying to read non-numeric [%s]\n", fname.c_str());
	  retval = READ( fname, trace, cmds, globals );
	  //fprintf(stdout, "FINISHED Trying to read non-numeric [%s]. Got result of length [%lu]\n", fname.c_str(), retval.valu.size());
	}
      else
	{
	  retval.valu = vector<real_t>(1, res);
	}
    }
  //fprintf(stdout, "Returning from DOCMD\n");
  return retval;
}


elemptr findmodel( const string& s, const vector<elemptr>& trace, global_store& globals )
{
  auto mod =  get_model_widx( s, trace ); //This only iterates up to top level within me....I need to do a full search.
  if( mod.model )
    {
      //fprintf(stdout, "FOUND MODEL [%s] IN TRACE!\n", s.c_str());
      return mod;
    }
  else
    {
      mod = globals.findmodel( s );
      if( mod.model )
	{
	  //fprintf(stdout, "FOUND MODEL [%s] IN GLOBALS!\n", s.c_str());
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
	  if( mod.model )
	    {
	      //fprintf(stdout, "FOUND MODEL [%s] IN ROOT OF MAIN!\n", s.c_str());
	      return mod;
	    }
	  else
	    {
	      fprintf(stderr, "ERROR could not find model, [%s] even through globals, parents, and bubbling through trace (last trace is [%s])\n" , s.c_str(), ep.model->buildpath().c_str());
	      exit(1);
	    }
	}
    }
  //FINDS MODEL IN BOTH GLOBALS AND LOCALS?
}


//REV: trace will contain the most recent index? I guess? Why use trace????
//REV: trae contains last idx?
varptr get_proper_var_widx( const string& varname, const vector<elemptr>& trace, global_store& globals )
{
  fprintf(stdout, "Getting proper var, [%s], most recent model is [%s]\n" , varname.c_str(), get_curr_model(trace).model->buildpath().c_str());
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
      //std::shared_ptr<corresp> mycorr;
      auto mycorr = ep.model->getcorresp( m2.model );
      if( !mycorr )
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
	  vector<size_t> ret = mycorr->getall( myidx );
	  vp.idx = ret;
	  return vp;
	}
    } //end iscorr
  else //is VARIABLE (note, if it was from global, we need to get the varname differently? They all have only one fuck...)
    {
      //REV: if it was a global, I just get it there ;)
      //fprintf(stdout, "GET PROPER: getting as a variable [%s] (vartail is [%s])\n", varname.c_str(), vartail.c_str() );
      vector<size_t> loc = ep.model->get_varloc( vartail );
      auto gmodel = globals.findmodel( ep.model ).model;
      
      if( gmodel )
	{
	  if( gmodel->vars.size() != 1)
	    {
	      fprintf(stderr, "REV SUPER ERROR, HAX, gmodel doesnt have just 1 var!!\n");
	      exit(1);
	    }
	  loc = vector<size_t>(1, 0);
	}
      else
	{
	  loc = ep.model->get_varloc( vartail );
	}
            
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
	  auto realvar = ep.model->vars[ loc[0] ];
	  
	  //REV: SANITY CHECK, this will return SAME if it is SAME,
	  //Will return CONST if it is const ;)
	  auto corr = getcorresp_forvar( ep.model, trace, realvar );
	  //fprintf(stdout, "Finished finding corresp\n");
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
	  vector<size_t> transformed = corr->getall( ep.idx );

	  //fprintf(stdout, "Got transformed, now try to get valus...\n");
	  //REV" TODO HERE HERE HERE um, this should be rewritten with vget and vset
	  vector<real_t> vals = realvar->getvalus( transformed );
	  //fprintf(stdout, "Got valus...go into VP now ;)\n");
	  vector<size_t> idxs; //what are these? Literally idxs? Make sure idxs always points directly there? Nah... Just leave it empty?
	  vp.valu = vals;
	  //fprintf(stdout, "Finished VP, now return...\n");
	  return vp;
	}
      
    } //end ELSE (it is normal var)
} //end get proper var





void set_proper_var_widx(const string& varname, const vector<elemptr>& trace, global_store& globals, const varptr& vp )
{

  //fprintf(stdout, "Setting proper var, [%s], most recent model is [%s]\n" , varname.c_str(), get_curr_model(trace).model->buildpath().c_str());
  
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
      //std::shared_ptr<corresp> mycorr;
      auto mycorr = ep.model->getcorresp( m2.model );
      //bool foundcorr = ep.model->getcorresp( m2, mycorr );
      if( !mycorr )
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
	      return set_proper_var_widx(varname, newtrace, globals, vp); //ep.model->parent->getvar_widx( varname, ep.idx, trace );
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
	  auto realvar = ep.model->vars[ loc[0] ];
	  auto corr = getcorresp_forvar( ep.model, trace, realvar );
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
	  vector<size_t> transformed = corr->getall( ep.idx );
	  
	  if(transformed.size() != 1)
	    {
	      fprintf(stderr, "Haha, trying to write VAR 1->may through a corresp\n");
	      exit(1);
	    }
	  	  
	  
	  //vector<real_t> vals = var->getvalus( ep.idx );
	  //vector<size_t> idxs; //what are these? Literally idxs? Make sure idxs always points directly there? Nah... Just leave it empty?
	  
	  //realvar->setvalus( transformed, vp.valu );
	  realvar->vsetvalus( transformed, vp );
	  return;
	}
      
    }
} //end set var proper






void push_proper_var_widx(const string& varname, const vector<elemptr>& trace, global_store& globals, const varptr& vp, const vector<size_t>& topushascorr )
{

  //fprintf(stdout, "Setting proper var, [%s], most recent model is [%s]\n" , varname.c_str(), get_curr_model(trace).model->buildpath().c_str());
  
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
  else if( iscorr ) //REV: note I don't have a way to set TEMPORARY correspondences rofl.
    {
      //get final model haha
      elemptr m2 = findmodel( postmod, trace, globals );
      //auto curr = get_current_model(trace);
      //std::shared_ptr<corresp> mycorr;
      auto mycorr = ep.model->getcorresp( m2.model );
      //bool foundcorr = ep.model->getcorresp( m2, mycorr );
      if( !mycorr )
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
	  //mycorr->set( myidx, newvals );
	  //mycorr.insert( mycorr.end(), vp.idx.begin(), vp.idx.end() );
	  mycorr->fill( newvals );
	  //REV: literally pushed it directly, didn't need the corresp ;)
	  
	  return;
	  
	}
    }
  else  //it's just a normal var
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
	      return set_proper_var_widx(varname, newtrace, globals, vp); //ep.model->parent->getvar_widx( varname, ep.idx, trace );
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
	  auto realvar = ep.model->vars[ loc[0] ];
	  
	  //REV: wat the fck, for pushing, I don't need corresp. I will make it...?
	  //REV: I may need to actually create a new correspondencde if it does not exist (i.e. in the case this or target is a global model); 
	  auto corr = getcorresp_forvar( ep.model, trace, realvar );

	  //Sanity check that one of them is global? Can I do that by checking if parent exists?
	  if( !corr )
	    {
	      fprintf(stdout, "REV: corr between source model [%s] and newly created model [%s] did not exist, (not only not filled), thus I will have to create it. Under normal symmodel conditions this would be an error, but I am PUSHING, so it is a generator, and one of those models must be global.\n", get_curr_model(trace).model->buildpath().c_str(), ep.model->buildpath().c_str() );
	      if( ep.model->parent )
		{
		  fprintf(stderr, "REV: target model HAS a parent, implying it is not global!\n");
		  exit(1);
		}
	      if( !ep.model->parent && ep.model->is_submodel( get_curr_model( trace ).model ) )
		{
		  fprintf(stderr, "REV: target model has no parent (i.e. is global or ROOT of main hierarchy), yet root is same as the source model. This implies that it is not in globals\n");
		  exit(1);
		}

	      ep.model->addcorresp( get_curr_model( trace ) );
	      corr = getcorresp_forvar( ep.model, trace, realvar );
	      if( !corr )
		{
		  fprintf(stderr, "ERROR in push_proper, etc.: corr doesn't exist even after adding corr...wtf?\n");
		  exit(1);
		}
	      
	    }
	  if( ep.idx.size() != 1 )
	    {
	      fprintf(stderr, "REV: we have a problem houston...\n");
	      exit(1);
	    }

	  if( corr->isinit() )
	    {
	      fprintf(stderr, "Trying to push back but already init fuck\n");
	      exit(1);
	    }

	  realvar->addvalus( vp );
	  
	  //Need to set corresp to that which was passed to me.
	  //Only do if size > 0 (i.e. there was a corr passed, shoudl even be 1 if it was just binding to a singel value?)
	  if( topushascorr.size() > 0 )
	    {
	      corr->fill( topushascorr );
	    }
	  
	  return;
	}
      
    }
} //end PUSH var proper
		   





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

  varptr vp = get_proper_var_widx( varname, trace, globals );

  return vp;
  
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
  //fprintf(stdout, "Returned from DOCMD in set\n");

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
  varptr vp;
  
  vector<vector<real_t>> val;
  
  for(size_t x=0; x<parsed.size(); ++x)
    {
      string toexec = parsed[x];
      varptr vp2 = DOCMD( toexec, trace, cmds, globals );
      //fprintf(stdout, "Returned from DOCMD in sum\n");
      val.push_back( vp2.valu );
    }

  vp.valu = vect_sum( val );

  return vp;
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

  varptr vp;
  
  vector<vector<real_t>> val;
  
  for(size_t x=0; x<parsed.size(); ++x)
    {
      string toexec = parsed[x];
      varptr vp2 = DOCMD( toexec, trace, cmds, globals );
      //fprintf(stdout, "Returned from DOCMD in mult\n");
      val.push_back( vp2.valu );
    }

  vp.valu = vect_mult( val );
  
  return vp;
}

//div
FUNCDECL(DIV)
{
  fprintf( stdout, "Executing DIV arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  varptr vp;
  
  vector<vector<real_t>> val;
  
  for(size_t x=0; x<parsed.size(); ++x)
    {
      string toexec = parsed[x];
      varptr vp2 = DOCMD( toexec, trace, cmds, globals );
      //fprintf(stdout, "Returned from DOCMD in div\n");
      val.push_back( vp2.valu );
    }

  vp.valu = vect_div( val );
  
  return vp;
}



//subtract
FUNCDECL(DIFF)
{
  fprintf( stdout, "Executing DIFF arg: [%s]\n", arg.c_str() );
  //subtract all from first one?
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    {
      exit(1);
    }

  varptr vp;
  
  vector<vector<real_t>> val;
  
  for(size_t x=0; x<parsed.size(); ++x)
    {
      string toexec = parsed[x];
      varptr vp2 = DOCMD( toexec, trace, cmds, globals );
      //fprintf(stdout, "Returned from DOCMD in diff\n");
      val.push_back( vp2.valu );
    }

  vp.valu = vect_diff( val );
  
  return vp;
}


FUNCDECL(NEGATE)
{
  fprintf( stdout, "Executing NEGATE arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    {
      exit(1);
    }

  varptr vp;
  
  vector<real_t> val;
  
  
  string toexec = parsed[0];
  varptr vp2 = DOCMD( toexec, trace, cmds, globals );
  //fprintf(stdout, "Returned from DOCMD in negate\n");
  val = vp2.valu; //.push_back( vp2.valu );
  
  vp.valu = vect_negate( val );
  
  return vp;
}

FUNCDECL(EXP)
{
  fprintf( stdout, "Executing EXP arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  varptr vp;
  
  vector<real_t> val;
  
  string toexec = parsed[0];
  varptr vp2 = DOCMD( toexec, trace, cmds, globals );
  //fprintf(stdout, "Returned from DOCMD in exp\n");
  val = vp2.valu ;
  
  vp.valu = vect_exp( val );
  
  return vp;
}

FUNCDECL(GAUSSRAND)
{
  fprintf( stdout, "Executing GAUSSRAND arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    { exit(1); }


  varptr vp;
  
  string meanstr = parsed[0];
  string stdstr = parsed[1];

  
  varptr mvp = DOCMD( meanstr, trace, cmds, globals );
  //fprintf(stdout, "Returned from DOCMD in gaussrand1\n");
  varptr svp = DOCMD( stdstr, trace, cmds, globals );
  //fprintf(stdout, "Returned from DOCMD in gaussrand2\n");
  
  vp.valu = vect_normal( mvp.valu, svp.valu, cmds.RANDGEN );
  //std::normal_distribution<real_t> mydist( meanval, stdval );

  return vp;
  
  //CMDS contains the randgen I guess?
  //real_t val = mydist( cmds.RANDGEN );
  //return val;
}

FUNCDECL(UNIFORMRAND)
{
  fprintf( stdout, "Executing UNIFORMRAND arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    { exit(1); }


  varptr vp;
  
  string minstr = parsed[0];
  string maxstr = parsed[1];
  
  varptr minval = DOCMD( minstr, trace, cmds, globals );
  //fprintf(stdout, "Returned from DOCMD in unirand1\n");
  varptr maxval = DOCMD( maxstr, trace, cmds, globals );

  //fprintf(stdout, "Returned from DOCMD in unirand2\n");
  
  //std::uniform_distribution<real_t> mydist( minval, maxval );
  vp.valu = vect_uniform( minval.valu, maxval.valu, cmds.RANDGEN );

  return vp;
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
  
  
  varptr vp;
  vp.valu = vector<real_t>(1, 0);
  
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
      varptr v = exec_w_corresp( toexec, holemod, trace, cmds, globals );
      //if( v.idx.size() != 1 || v.valu.size() != 1 )
      if( v.valu.size() != 1 )
	{
	  fprintf(stderr, "ERROR in SUMFORALLHOLES due to exec_w_corresp reutrning non-single value/idx...\n");
	  exit(1);
	}
      if( v.idx.size() != 0 )
	{
	  fprintf(stderr, "Huh, SUM FOR ALL HOLES still error from DOCMD returning unrealistic idx??!!\n");
	  exit(1);
	}

      vp.valu[0] += v.valu[0];
      
    }
  
  return vp;
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

  
  varptr vp;
  vp.valu = vector<real_t>(1, 0);

  if(currmodel.idx.size() != 1)
    {
      fprintf(stderr, "REV error in SUMFORALLCONNS, currmodel idx size is != 1 (is [%lu])\n", currmodel.idx.size() );
      exit(1);
    }
  
  vector<size_t> mypost = corr->getall( currmodel.idx );
  
  for( size_t i = 0; i<mypost.size(); ++i )
    {
      size_t localidx = mypost[ i ];
      newtrace[ newtrace.size()-1 ].idx = vector<size_t>(1, localidx); //set idx to correct idx for execution.
      varptr v2 = DOCMD( arg, newtrace, cmds, globals );
      if(v2.valu.size() != 1)
	{
	  fprintf(stderr, "REV: multi call sumfor all conns in multi-size v2 valu\n");
	  exit(1);
	}
      vp.valu[0] += v2.valu[0];
    }

  return vp;
  
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


  varptr vp;
  vp.valu = vector<real_t>(1 , 1.0);
  
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
      varptr v = exec_w_corresp( toexec, holemod, trace, cmds, globals );
      if( v.valu.size() != 1 )
	{
	  fprintf(stderr, "ERROR in SUMFORALLHOLES due to exec_w_corresp reutrning non-single value/idx...");
	  exit(1);
	}
      if( v.idx.size() != 0 )
	{
	  fprintf(stderr, "Huh, SUM FOR ALL HOLES still error from DOCMD returning unrealistic idx??!!\n");
	  exit(1);
	}
      vp.valu[0] *= v.valu[0];
      
    }
  return vp;
  
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


  varptr vp;
  vp.valu = vector<real_t>(1, 1.0);

  if(currmodel.idx.size() != 1)
    {
      fprintf(stderr, "REV error in MULTFORALLCONNS, currmodel idx size is != 1 (is [%lu])\n", currmodel.idx.size() );
      exit(1);
    }
  
  vector<size_t> mypost = corr->getall( currmodel.idx );
  
  for( size_t i = 0; i<mypost.size(); ++i )
    {
      size_t localidx = mypost[ i ];
      newtrace[ newtrace.size()-1 ].idx = vector<size_t>(1, localidx); //set idx to correct idx for execution.
      varptr v2 = DOCMD( arg, newtrace, cmds, globals );
      if(v2.valu.size() != 1)
	{
	  fprintf(stderr, "REV: multi call sumfor all conns in multi-size v2 valu\n");
	  exit(1);
	}
      vp.valu[0] *= v2.valu[0];
    }

  return vp;
  
} //end MULTFORALLCONNS


FUNCDECL( SQRT )
{
  fprintf( stdout, "Executing SQRT arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  varptr vp;
  
  vector<real_t> val;
  
  string toexec = parsed[0];
  varptr vp2 = DOCMD( toexec, trace, cmds, globals );
  //fprintf(stdout, "Returned from DOCMD in exp\n");
  val = vp2.valu ;
  
  vp.valu = vect_sqrt( val );
  
  return vp;
}

FUNCDECL( SQR )
{
  fprintf( stdout, "Executing SQR arg: [%s]\n", arg.c_str() );
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  varptr vp;
  
  vector<real_t> val;
  
  string toexec = parsed[0];
  varptr vp2 = DOCMD( toexec, trace, cmds, globals );
  //fprintf(stdout, "Returned from DOCMD in exp\n");
  val = vp2.valu ;
  
  vp.valu = vect_sqr( val );
  
  return vp;
}



FUNCDECL( NEWLOCAL )
{
  //Ret doesnt matter.
  //Add a local variable to GLOBAL
  //The name needs to be resolved?
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }
  
  vector<string> functparse = cmds.fparse( arg );
  string fname = functparse[0];
  if( functparse.size() != 1 )
    { exit(1); }


  //He better not have named it something like /blah/yolo/faggit
  globals.addempty( fname );

  varptr emptyv;
  
  return emptyv;
}


//If this is a single one, do I specify the "current" model and its index?
//If so, it will add the correspondence one-by-one.
//I might actually want to do that (in the future).
//Note, in this case targmodel is "", i.e. the current model?
//
FUNCDECL( PUSH )
{
  
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 2 )
    {
      fprintf(stderr, "PUSH [%s], not 2 args \n", arg.c_str());
      exit(1);
    }

  string targmodel, pushto, toexec;
  
  targmodel = "";
  pushto = parsed[0]; //this is var name.
  toexec = parsed[1];
  
  
  //fprintf(stdout, "Will FORALL execute [%s] for every member of [%s] (seen from model [%s]), PUSHING TO [%s]\n", toexec.c_str(), targmodel.c_str(), get_curr_model(trace).model->buildpath().c_str(), pushto.c_str());


  //Get model refererenced from here... must be initialized/size. OK. Can it be a var?
  //Note the model must be found inside trace? Yea...
  //auto ep = findmodel( targmodel, trace, globals);

  //REV: this must be a variable? However, it might be a model in GLOBALS. Findmodel will handle finding model QUA varname in globals.
  //That is a hack. I should make globals straight vars, and thus make corresp be held inside vars....?
  //Or, force it find "naked" variables in globals. I guess that makes sense...
  auto pushep = findmodel( pushto, trace, globals); 
  
  if( !pushep.model )
    {
      fprintf(stderr, "Couldnt find push EP model in PUSHFORALL\n");
      exit(1);
    }
  
  //vector<elemptr> newtrace = trace;
  //ep.idx = vector<size_t>(1,0);
  //newtrace.push_back( ep );


  //New trace is the "inside" model. But, e.g., If I am adding 
  //newtrace[ newtrace.size() -1].idx[ 0 ] = 0;
  //Is...is this right? If I execute something, it will try to access a bunch of variables corresponding. But...who cares?
  //I am just trying to push back a single value I assume.
  //The value should be numeric...?
  //Or combinations of numeric data with "set" values?
  //Do I want user to be able to "PUSH" at non-outside locations? Ooooh that might actually work rofl.
  //I.e. push inside a "forall". Interesting.
  auto curr = get_curr_model( trace );
  if( curr.idx.size() != 1 && curr.idx[0] != 0 )
    {
      fprintf(stderr, "REV: major error in PUSH, the PARENT trace has idx size != 1 or it is not index 0...\n");
      exit(1);
    }
  
  varptr vp = DOCMD( toexec, trace, cmds, globals );
  //fullvp.valu.insert( fullvp.valu.end(), vp.valu.begin(), vp.valu.end() );
  //fullvp.idx.insert( fullvp.idx.end(), vp.idx.begin(), vp.idx.end() );

  //REV: shouldn't be "x", it should be whatever the idx is that i pushed to.
  //Fuck.... same for when I push multiple guys...???
  //E.g. if now I pushed #3 of last guy...

  vector<size_t> corr;
  push_proper_var_widx( pushto, trace, globals, vp, corr );
  
  return vp;
} //end PUSH

FUNCDECL( PUSHFORALL )
{
  vector<string> parsed = cmds.doparse( arg );
  //Basically, I do a "FORALL", but the results I get back, I push :)
  //Are FORALL coalesced anyway?
   if( parsed.size() < 2 || parsed.size() > 3 )
    {
      fprintf(stderr, "PUSHFORALL [%s], not 2 or 3 args \n", arg.c_str());
      exit(1);
    }
  string targmodel, pushto, toexec;
  
  if( parsed.size() == 2 )
    {
      targmodel = "";
      pushto = parsed[0];
      toexec = parsed[1];
    }
  else
    {
      targmodel = parsed[0];
      pushto = parsed[1];
      toexec = parsed[2];
    }

  //Ghetto hack, check isnumeric lol
  size_t reptimes;
  bool isint = checknumericint( targmodel, reptimes );

  if( !isint )
    {
      fprintf(stdout, "Will FORALL execute [%s] for every member of [%s] (seen from model [%s]), PUSHING TO [%s]\n", toexec.c_str(), targmodel.c_str(), get_curr_model(trace).model->buildpath().c_str(), pushto.c_str());
  

      //Get model refererenced from here... must be initialized/size. OK. Can it be a var?
      //Note the model must be found inside trace? Yea...
      auto ep = findmodel( targmodel, trace, globals);
      auto pushep = findmodel( pushto, trace, globals);
  
      if( !ep.model )
	{
	  fprintf(stderr, "Couldnt find EP model in PUSHFORALL\n");
	  exit(1);
	}
      if( !pushep.model )
	{
	  fprintf(stderr, "Couldnt find push EP model in PUSHFORALL\n");
	  exit(1);
	}

      //REV: this automatically will execute for every one, getting it back, and
      //appending...
      size_t modelsize = ep.model->get_modelsize();
      if(modelsize == 0 )
	{
	  //Fuck...
	  fprintf(stderr, "ERROR, model size is zero!\n");
	  exit(1);
	}
  
      //vector<size_t> idxs( modelsize );
      //std::iota(std::begin(idxs), std::end(idxs), 0);
      //std::generate( idxs.begin(), idxs.end(), UniqueNumber );
  
      //ep.idx = idxs;
  
      //IDX overwrites it, I use IDX directly ;)
      vector<elemptr> newtrace = trace;
      ep.idx = vector<size_t>(1,0);
      newtrace.push_back( ep );
      //elemptr.idx = vector<size_t>(1, x); //1 through N? Does it make more sense to
      //do one at a time and append them, or to do them all together here?
      //What if each model is doing some sneaky shit like um, nested forall? That's fine.
      //In that case, our IDX would be multi as well. I wouldn't know which is "pre",
      //so lol. Inside of me, ref to PRE would look up old idx values through TRACE
      //What would happen if IDX at that time point is like idx[1][2][3][4] etc.?
      //And now it is like a gazillion. So which go with which? Each IDX generated some
      //guys... I kind of like this way better...but...let's go with all together.
  
      //Assuming no mumbo fuck-ups this will work.
      //varptr vp = DOCMD( toexec, trace, cmds, globals );
  
      varptr fullvp;

      vector<size_t> corr;
  
      for(size_t x=0; x<modelsize; ++x)
	{
	  newtrace[ newtrace.size() - 1].idx[ 0 ] = x;
	  varptr vp = DOCMD( toexec, newtrace, cmds, globals );
	  fullvp.valu.insert( fullvp.valu.end(), vp.valu.begin(), vp.valu.end() );
	  fullvp.idx.insert( fullvp.idx.end(), vp.idx.begin(), vp.idx.end() );
	  //REV: FUCK FUCK FUCK, do I need to insert idxs as corresp?

      
	  //I either got SIZE_T or VAL_T.
	  if( vp.idx.size() > vp.valu.size() )
	    {
	      size_t nret = vp.idx.size();
	      vector<size_t> toaddcorr( nret, x );
	      corr.insert( corr.end(), toaddcorr.begin(), toaddcorr.end() );
	      //idxs
	    }
	  else
	    {
	      size_t nret = vp.valu.size();
	      vector<size_t> toaddcorr( nret, x );
	      corr.insert( corr.end(), toaddcorr.begin(), toaddcorr.end() );
	      //valus
	    }

	  //Fine, problem is, what to add for the POSTSYN corresps?
	  //In other words, for each one generated, it must know what that number was
	  //I need to generate for THAT as well? Fuck... That's fine though.
	}

      //REV: HERE HERE HERE. Note, I need to fucking PUSH to proper var, not just
      //SET IT!@!!
      //Gotta know which corr to set too!
      
      push_proper_var_widx( pushto, trace, globals, fullvp, corr );
  
      //YESSSSSSSSSSSSSSSSSSSSS  
      return fullvp;
    }

  else //is int

    {
      fprintf(stdout, "Will FORALL execute [%s] for N times [%lu] (seen from model [%s]), PUSHING TO [%s]\n", toexec.c_str(), reptimes, get_curr_model(trace).model->buildpath().c_str(), pushto.c_str());
  

      //Get model refererenced from here... must be initialized/size. OK. Can it be a var?
      //Note the model must be found inside trace? Yea...
      auto pushep = findmodel( pushto, trace, globals);
      
      if( !pushep.model )
	{
	  fprintf(stderr, "Couldnt find push EP model in PUSHFORALL\n");
	  exit(1);
	}
      
      //REV: this automatically will execute for every one, getting it back, and
      //appending...
      if( reptimes == 0)
	{
	  //Fuck...
	  fprintf(stderr, "ERROR, requested rep times is zero!\n");
	  exit(1);
	}

      auto curr = get_curr_model(trace);
      if( curr.idx.size() != 1 && curr.idx[0] != 0 )
	{
	  fprintf(stderr, "REV: major error in PUSH, the PARENT trace has idx size != 1 or it is not index 0...\n");
	  exit(1);
	}
      
      varptr fullvp;
      vector<size_t> corr; //EMPTY BECAUSE THIS IS NOT A CORR!!!???!

      //REV: think this through...what should corr contain? All new values I create have what relationship to host model? None...I assume?
      //Normally I would push/pass x, which is the index of the "iterated through" model. And thus, e.g., iterated through model 1 would correspond to
      //whatever X guys I generated.
      //But, there is no iterated model. It is not even currmodel. Thus, corr is irrelevant. I.e. there are no correspondences.
      for(size_t x=0; x<reptimes; ++x)
	{
	  varptr vp = DOCMD( toexec, trace, cmds, globals );
	  fullvp.valu.insert( fullvp.valu.end(), vp.valu.begin(), vp.valu.end() );
	  fullvp.idx.insert( fullvp.idx.end(), vp.idx.begin(), vp.idx.end() );
	}
      
      push_proper_var_widx( pushto, trace, globals, fullvp, corr );
      
      return fullvp;
    }
}


FUNCDECL( FORALL )
{
  //May return IDXs or REALS. Will always return large of iterated size.
  //Most important thing, this does NOT carry through CORRESP. In other words,
  //all local variables use their local variable that I create!

  //Executes CMD for each guy, and returns. Of course, nothing special about that
  //There is really no need to specify, except it explicitly scaffolds off size
  //of a given model/target, withoutu going through corresp.

  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    {
      fprintf(stderr, "FORALL [%s], not 2args \n", arg.c_str());
      exit(1);
    }

  string targmodel = parsed[0];
  string toexec = parsed[1];
  fprintf(stdout, "Will FORALL execute [%s] for every member of [%s] (seen from model [%s])\n", toexec.c_str(), targmodel.c_str(), get_curr_model(trace).model->buildpath().c_str());

  //Get model refererenced from here... must be initialized/size. OK. Can it be a var?
  //Note the model must be found inside trace? Yea...
  auto ep = findmodel( targmodel, trace, globals);

  if( !ep.model )
    {
      fprintf(stderr, "Couldnt find model in FORALL\n");
      exit(1);
    }
  size_t modelsize = ep.model->get_modelsize();
  if(modelsize == 0 )
    {
      //Fuck...
      fprintf(stderr, "ERROR, model size is zero!\n");
      exit(1);
    }

  //It's doing this for each postsyn, e.g. blah...OK. It will return the list directly.
  //REV: be careful with how I handle
  //Note, X is the idx?!!!!. I will compress them here!!!
  //Or, do I just directly access all variables?
  //There is no corresp yet, I am constructing and returning.
  vector<size_t> idxs( modelsize );
  std::iota(std::begin(idxs), std::end(idxs), 0);
  //std::generate( idxs.begin(), idxs.end(), UniqueNumber );

  ep.idx = idxs;
  
  //IDX overwrites it, I use IDX directly ;)
  vector<elemptr> newtrace = trace;
  newtrace.push_back( ep );
  //elemptr.idx = vector<size_t>(1, x); //1 through N? Does it make more sense to
  //do one at a time and append them, or to do them all together here?
  //What if each model is doing some sneaky shit like um, nested forall? That's fine.
  //In that case, our IDX would be multi as well. I wouldn't know which is "pre",
  //so lol. Inside of me, ref to PRE would look up old idx values through TRACE
  //What would happen if IDX at that time point is like idx[1][2][3][4] etc.?
  //And now it is like a gazillion. So which go with which? Each IDX generated some
  //guys... I kind of like this way better...but...let's go with all together.

  //Assuming no mumbo fuck-ups this will work.
  varptr vp = DOCMD( toexec, newtrace, cmds, globals );
  
  //for(size_t x=0; x<modelsize; ++x)
  //  {
      //newtrace[ newtrace.size() -1].idx[ 0 ] = x;
      //varptr vp = DOCMD( toexec, trace, cmds, globals );
  //  }

  return vp;
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


std::shared_ptr<corresp> getcorresp_forvar( const std::shared_ptr<symmodel>& curr, const std::shared_ptr<symmodel>& targ, const std::shared_ptr<symvar>& var )
{
  if( !var->parent)
    {
      fprintf(stderr, "getcorresp forvar , var has no parent exiting\n");
      exit(1);
    }
  if( var->parent != targ )
    {
      fprintf(stderr, "REV: wtf in getcorresp_forvar, var parent is not target model we are looking for corresp in????\n");
      exit(1);
    }
  
  auto tmp = curr->getcorresp( var );
  if(!tmp)
    {
      fprintf(stderr, "ERROR in some function execution, could not find required corresp between models [%s] and [%s]\n", curr->buildpath().c_str(), targ->buildpath().c_str() );
      exit(1);
    }
  
  return tmp;
}

std::shared_ptr<corresp> getcorresp_forvar( const std::shared_ptr<symmodel>& targ, const vector<elemptr>& trace, const std::shared_ptr<symvar>& var )
{
  elemptr lastguy = get_curr_model(trace); //trace[trace.size()-1];
  
  return getcorresp_forvar( lastguy.model, targ, var );
}

std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& curr, const std::shared_ptr<symmodel>& targ)
{
  auto tmp = curr->getcorresp( targ );
  if(!tmp)
    {
      fprintf(stderr, "ERROR in some function execution, could not find required corresp between models [%s] and [%s]\n", curr->buildpath().c_str(), targ->buildpath().c_str() );
      exit(1);
    }
  
  return tmp;
}

std::shared_ptr<corresp> getcorresp( const std::shared_ptr<symmodel>& targ, const vector<elemptr>& trace )
{
  elemptr lastguy = get_curr_model(trace); //trace[trace.size()-1];
  
  return getcorresp( lastguy.model, targ);
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

varptr exec_w_corresp( const std::string& toexec, const std::shared_ptr<symmodel>& m, const vector<elemptr>& trace, cmdstore& cmds, global_store& globals )
{
  //RE-parse toexec, to check what function it is.
  //If it is one of the given functions, then we go.

  fprintf(stdout, "Executing with corresp [%s]\n", toexec.c_str() );
  
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
      vector<size_t> arbitrary_idx( 1, arbitrary_idxval );
      elemptr tmpelem( m, arbitrary_idx ); //idx is arbitrary
      vector<elemptr> newtrace = trace;
      newtrace.push_back( tmpelem );

      //Note toexec is the whole thing passed, we didn't strip it.
      //we are just handling it in a special way based on #args and pushing back to trace.
      //DOCMD will strip the first part and execute it.
      varptr vp = DOCMD( toexec, newtrace, cmds, globals );

      fprintf(stdout, "RETURNED FROM DOCMD, COMMAND IS MULTI\n");
      
      return vp;
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
	  fprintf(stderr, "REV: whoa (ERROR), doing nested exec_w_corresp (curridx.size != 1)\n");
	  exit(1);
	}
      
      //size_t curridx = vcurridx[0];
      
      vector<size_t> c = corr->getall( vcurridx );
      
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
      varptr vp = DOCMD( toexec, newtrace, cmds, globals);
      fprintf(stdout, "RETURNED FROM DOCMD, COMMAND IS **NOT** MULTI\n");
      return vp;
      
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







string get_containing_model_path( const string& parsearg, string& vartail )
{
  vector<string> parsed = parse(parsearg);
  if( parsed.size() < 1 )
    {
      fprintf(stderr, "No var specified???\n");
      exit(1);
    }
  else
    {
      vartail = parsed[ parsed.size() -1 ];
      parsed.pop_back();
      return CAT( parsed, "/" );
    }
}


void symmodel::setgenformodel( const string& modelname, const generator& g )
{
  //copy generator?
  auto mod = get_model( modelname );
  mod->gen = std::make_shared<generator>(g);
  fprintf(stdout, "Generator set for model [%s]\n", modelname.c_str() );
  mod->gen->enumerate();
}





bool symmodel::checkcorrready()
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

bool symmodel::checkvarsready()
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


void symmodel::addfvars( const string& s, const string& t, const vector<real_t>& f )
  {
    addvar( s, t );
    vars[ vars.size() - 1 ]->addfvalus( f );
  }

void symmodel::addivars( const string& s, const string& t, const vector<size_t>& i )
  {
    addvar( s, t );
    vars[ vars.size() - 1 ]->addivalus( i );
  }


void symmodel::fillemptymodels( )
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
	models[m]->fillemptymodels();
      }
  }
	


vector<size_t> symmodel::get_varloc( const string& s )
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



std::shared_ptr<symvar> symmodel::getvar_widx( const string& s, const vector<size_t>& idx, const vector<elemptr>& trace )
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
