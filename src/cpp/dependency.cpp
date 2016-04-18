#include <dependency.h>


depstate::depstate( const std::shared_ptr<symmodel>& m, const size_t& line, global_state& globals )
  : model( m ), linenumber( line )
{
  std::shared_ptr<symmodel> root = model->get_root();
  
  //Execute it, and then see what state is...?
  //vector<string> readstate, writtenstate, pushedstate;

  size_t beforesize = globals.models.size();
  
  model->execute_gen_line( linenumber, globals );

  size_t aftersize = globals.models.size();

  if( beforesize < aftersize )
    {
      creator = true;
    }
  
  root->read_and_reset_all( read, written, pushed );
  globals.read_and_reset_all( read, written, pushed );
  
}

void depstate::execute( global_store& globals )
{
  model->execute_gen_line( linenumber, globals );
}


void generator_deps::fill_depstate(  const std::shared_ptr<symmodel>& m, global_state& globals )
{
  if( !m )
    {
      fprintf(stderr, "REV model m not resolvable\n");
      exit(1);
    }
  if( m->gen )
    {
      for( size_t x=0; x<m->gen->genfunct.lines.size(); ++x)
	{
	  depstate ds( m, x, globals);
	  nodes.push_back( ds );
	}
    }
}

void generator_deps::fill_all_depstates( const std::shared_ptr<symmodel>& m, global_state& globals )
{
  if( !m )
    {
      fprintf(stderr, "REV model m not resolvable\n");
      exit(1);
    }

  for(size_t x=0; x<m->models.size(); ++x)
    {
      fill_depstate( m->models[x], globals );

      //REV; recursively fill all depstates... if gen exists of course ;)
      fill_all_depstates( m->models[x], globals );
    }

  //Maybe there are gens for globals...? Nah...
  for(size_t g=0; g<globals.models.size(); ++g)
    {
      fill_depstate( globals.models[g], globals ); //wat...? Oh well ;0
    }
}


//Now that the graph is built, I can do the dependency parsing, i.e. get an "ordering" of nodes...
vector<depstate> generator_deps::parse_dependencies()
{
  vector<size_t> evalorder;
  vector<bool> visited( nodes.size(), false );
  while( findunmarked( visited ).size() > 0 )
    {
      size_t todo = findunmarked( visited )[0];
      vector<bool> tmpvisited( nodes.size(), false );
      visit( todo, tmpvisited, visited, evalorder );
    }

  std::reverse( evalorder.begin(), evalorder.end() );
    
  if( evalorder.size() != nodes.size() )
    {
      fprintf(stderr, "EVALORDER is not same size as nodes..\n");
      exit(1);
    }

  vector<depstate> ret;
  fprintf(stdout, "\n == EVALUATION ORDER\n");
  for(size_t x=0; x<evalorder.size() ++x )
    {
      size_t toeval = evalorder[x];
	
      ret.push_back( nodes[toeval] );
      if( !nodes[toeval].model )
	{
	  fprintf(stderr, "REV: error in parse_dep, model pointer is NULL\n");
	  exit(1);
	}
      fprintf( "Model [%s] line [%lu], [%s]\n", nodes[toeval].model->buildpath().c_str(), nodes[toeval].linenumber, nodes[toeval].model->gen->genfunct.lines[ nodes[toeval].linenumber ] );
    }

  return ret;
}
