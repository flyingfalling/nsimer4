#include <dependency.h>


depstate::depstate( const std::shared_ptr<symmodel>& m, const size_t& line, global_store& globals )
  : model( m ), linenumber( line )
{
  std::shared_ptr<symmodel> root = model->get_root();
  
  //Execute it, and then see what state is...?
  //vector<string> readstate, writtenstate, pushedstate;

  size_t beforesize = globals.models.size();

  fprintf(stdout, "EXECUTING generator line from inside DEPSTATE constructor\n");
  root->reset_all();
  globals.reset_all();
  //Sanity check here.
  //fprintf(stdout, "Sanity check BEFORE executing DEPSTATE execute_gen_line\n");
  //model->sanity_check();

  model->execute_gen_line( linenumber, globals );

  //fprintf(stdout, "Sanity check AFTER executing DEPSTATE execute_gen_line\n");
  //model->sanity_check();

  size_t aftersize = globals.models.size();

  if( beforesize < aftersize )
    {
      creator = true;
    }

  //fprintf(stdout, "Sanity check BEFORE cleanup!\n");
  //model->sanity_check();
  //fprintf(stdout, "**END** Sanity check BEFORE cleanup!\n");

  //REV: just do it to basicall reset all ;)
    
  root->read_and_reset_all( read, written, pushed );
  globals.read_and_reset_all( read, written, pushed );
  
  //fprintf(stdout, "Sanity check AFTER cleanup!\n");
  //model->sanity_check();
  //fprintf(stdout, "**END** Sanity check AFTER cleanup!\n");

}

void depstate::execute( global_store& globals )
{
  model->execute_gen_line( linenumber, globals );
}


void generator_deps::fill_depstate(  const std::shared_ptr<symmodel>& m, global_store& globals )
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

void generator_deps::fill_all_depstates( const std::shared_ptr<symmodel>& m, global_store& globals )
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
  for(size_t x=0; x<evalorder.size(); ++x )
    {
      size_t toeval = evalorder[x];
	
      ret.push_back( nodes[toeval] );
      if( !nodes[toeval].model )
	{
	  fprintf(stderr, "REV: error in parse_dep, model pointer is NULL\n");
	  exit(1);
	}
      fprintf(stdout,  "Model [%s] line [%lu], [%s]\n", nodes[toeval].model->buildpath().c_str(), nodes[toeval].linenumber, nodes[toeval].model->gen->genfunct.lines[ nodes[toeval].linenumber ].c_str() );
    }

  return ret;
}


void depstate::enumerate()
{
  //model and line number
  fprintf(stdout,  "Model [%s] line [%lu], [%s]\n", model->buildpath().c_str(), linenumber, model->gen->genfunct.lines[ linenumber ].c_str() );

  fprintf(stdout, "  READ: ");
  for(size_t x=0; x<read.size(); ++x)
    {
      fprintf(stdout, "[%s]", read[x].c_str() );
    }
  fprintf(stdout, "\n");

  fprintf(stdout, "  WRITTEN: ");
  for(size_t x=0; x<written.size(); ++x)
    {
      fprintf(stdout, "[%s]", written[x].c_str() );
    }
  fprintf(stdout, "\n");

  fprintf(stdout, "  PUSHED: ");
  for(size_t x=0; x<pushed.size(); ++x)
    {
      fprintf(stdout, "[%s]", pushed[x].c_str() );
    }
  fprintf(stdout, "\n");
}
