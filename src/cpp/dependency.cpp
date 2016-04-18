#include <dependency.h>

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
