#include <global_store.h>



void global_store::addempty( const string& localname )
{
  //Try to find one of the same...?
  auto ep = findmodel( localname );
  if( ep.model )
    {
      fprintf(stdout, "During generation (??): global_store ADDEMPTY [%s], model is already found, so just leaving things as they are.\n", localname.c_str());
      return;
    }
  else
    {
      //My type is "TEMP"
      auto nguy = symmodel::Create( "__TEMPVAR", "__TEMPVAR", localname );
      //A single, unnamed variable haha.
      nguy->addvar( "__TMPVARNAME", "__TMPVARTYPE");
      models.push_back( nguy );
    }
}


vector<size_t> global_store::modellocs( const string& s )
  {
    vector<size_t> loc;
    for( size_t m=0; m<models.size(); ++m )
      {
	if( models[m]->localname.compare( s ) == 0 )
	  {
	    loc.push_back( m );
	  }
      }
    return loc;
  }

vector<size_t> global_store::modellocs(const std::shared_ptr<symmodel>& m  )
{
  vector<size_t> loc;
  for( size_t a=0; a<models.size(); ++a )
    {
      if( models[a] == m )
	{
	  loc.push_back( a );
	}
    }
  return loc;
}

elemptr global_store::findmodel( const std::shared_ptr<symmodel>& m )
{
  vector<size_t> found = modellocs( m );
  if( found.size() > 1 )
    {
      fprintf(stderr, "REV error in find model in GLOBAL STORE, found more than one examples of model [%s]\n", m->buildpath().c_str() );
      exit(1);
    }
  
  //std::shared_ptr<symmodel> foundguy;
  elemptr res; //( foundguy, vector<size_t>(0) );
  //res.model = foundguy;
  //just get this model, right now ;) It will be a var in a model I assume.
  if( found.size() == 1)
    {
      //foundguy = models[ found[0] ];
      res.model = models[ found[0] ];
    }
  
  return res;
}

elemptr global_store::findmodel( const string& s ) //, const vector<size_t>& idx )
{
  vector<size_t> found = modellocs( s );
  if( found.size() > 1 )
    {
      fprintf(stderr, "REV error in find model in GLOBAL STORE, found more than one examples of model [%s]\n", s.c_str() );
      exit(1);
    }

  vector<size_t> idx;
  std::shared_ptr<symmodel> foundguy;
  elemptr res( foundguy, idx );
  //just get this model, right now ;) It will be a var in a model I assume.
  if( found.size() == 1)
    {
      //foundguy = models[ found[0] ];
      res.model = models[ found[0] ];
    }
  
  return res;
} //end findmodel



void global_store::addiparam( const string& lname, const vector<size_t>& val )
  {
    //auto m = symmodel::Create( "", "", lname );
    addempty( lname );
    models[ models.size() - 1 ]->addivars( "", "", val );
  }

void global_store::addfparam( const string& lname, const vector<real_t>& val )
  {
    addempty( lname );
    models[ models.size() - 1 ]->addfvars( "", "", val );
  }
  


void global_store::read_and_reset_all( vector<string>& readstate, vector<string>& writtenstate, vector<string>& pushedstate )
{
  for(size_t x=0; x<models.size(); ++x)
    {
      models[x]->read_and_reset_all( readstate, writtenstate, pushedstate );
    }
  
}

void global_store::reset_all( )
{
  for(size_t x=0; x<models.size(); ++x)
    {
      models[x]->reset_all( );
    }
  
}


void global_store::set_non_generating( )
{
  for(size_t x=0; x<models.size(); ++x)
    {
      models[x]->set_non_generating();
    }
}
