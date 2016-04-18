#include <symmodel.h>





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

std::shared_ptr<corresp> symmodel::getcorresp( const std::shared_ptr<symvar>& s )
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


  //This asks, is this S a submodel of me?
bool symmodel::is_submodel( const std::shared_ptr<symmodel>& s )
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


std::shared_ptr<symmodel> symmodel::clone()
    {
      //Whoa, ghetto copy of object DATA. Will do deep copy to get contained pointed to data.
      auto newmodel = std::make_shared<symmodel>( *( shared_from_this() ) );

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


void updatefunct_t::execute( const size_t& myidx, global_store& globals )
  {
    for(size_t c=0; c<lines.size(); ++c)
      {
	vector<elemptr> trace;
	elemptr elem( model, myidx );
	trace.push_back( elem );
	DOCMD( lines[c], trace, *cmds, globals );
      }
  }

std::shared_ptr<corresp> symmodel::getcorresp( const std::shared_ptr<symmodel>& targ )
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

void symmodel::update( global_store& globals )
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

void symmodel::check_and_enumerate( size_t depth , bool checkupdate )
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
  

void symmodel::fill_corresp( const std::shared_ptr<corresp>& hiscorr )
  {
    vector< vector<size_t> > mycorresps = hiscorr->make_mirror( get_modelsize() );
        
    //this is MY parst.
    auto corr = getcorresp( hiscorr->parent );

    corr->clearcorresp();
    for(size_t x=0; x<get_modelsize(); ++x)
      {
	corr->push( mycorresps[x].size(), mycorresps[x] );
      }
    
    return;
  }


elemptr symmodel::get_model_widx( const vector<string>& parsed, const vector<size_t>& idx, const vector<elemptr>& trace )
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


void symmodel::notify_filled_corresp( const std::shared_ptr<symmodel>& targ )
  {
    //Call this to "mirror" from large-small on othe side.
    auto tmp = getcorresp( targ );
    tmp->markinit(); //I better be init haha;
    targ->fill_corresp( tmp );
    return;
  }

void symmodel::addcorresp( const std::shared_ptr<symmodel>& targ )
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
  


void symmodel::read_and_reset_all( vector<string>& readstate, vector<string>& writtenstate, vector<string>& pushedstate )
  {
    for(size_t x=0; x<correspondences.size(); ++x)
      {
	if( correspondences[x]->wasread() )
	  {
	    readstate.push_back( correspondences[x]->buildpath() );
	  }
	if( correspondences[x]->waswritten() )
	  {
	    writtenstate.push_back( correspondences[x]->buildpath() );
	  }
	if( correspondences[x]->waspushed() )
	  {
	    pushedstate.push_back( correspondences[x]->buildpath() );
	  }
	correspondences[x].reset();
      }

    for(size_t x=0; x<vars.size(); ++x)
      {
	if( vars[x]->wasread() )
	  {
	    readstate.push_back( vars[x]->buildpath() );
	  }
	if( vars[x]->waswritten() )
	  {
	    writtenstate.push_back( vars[x]->buildpath() );
	  }
	if( vars[x]->waspushed() )
	  {
	    pushedstate.push_back( vars[x]->buildpath() );
	  }
	vars[x].reset();
      }
    
    for(size_t x=0; x<models.size(); ++x)
      {
	models[x]->read_and_reset_all( readstate, writtenstate, pushedstate );
      }

  }

void symmodel::execute_gen_line( const size_t& line, global_store& globals )
{
  if( !gen )
    {
      fprintf(stderr, "REV: error, trying to execute gen line [%lu] in model [%s], but it does not have a generator!\n", line, buildpath().c_str());
      exit(1);
    }
  else
    {
      gen->execute_line( this, globals, line );
    }
}


void symmodel::make_dependencies_and_generate( global_state& globals )
{
  generator_deps gd;
  gd.fill_all_depstates( std::shared_from_this(), globals );
  vector<depstate> execorder = parse_dependencies();
  
  set_non_generating(); //Now I can actually execute everything if I want.
  globals.set_non_generating();

  for( size_t l=0; l<execorder.size(); ++l )
    {
      execorder[l].execute( globals );
    }
}


void symmodel::set_non_generating( )
{
  for(size_t x=0; x<vars.size(); ++x)
    {
      vars[x]->donegenerating();
    }

  for(size_t x=0; x<correspondences.size(); ++x)
    {
      correspondences[x]->donegenerating();
    }
  
  for(size_t x=0; x<models.size(); ++x)
    {
      models[x]->set_non_generating();
    }
}
