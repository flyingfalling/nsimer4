#include <generator.h>

void genfunct_t::execute( std::shared_ptr<symmodel>& model, global_store& globals )
{
  for(size_t l=0; l<lines.size(); ++l)
    {
      vector<elemptr> trace;
      vector<size_t> myidx( 1, 0 );
      
      elemptr tmp( model, myidx );
      
      trace.push_back(tmp);
      
      DOCMD( lines[l], trace, cmds, globals );
    }
}

void generator::generate( std::shared_ptr<symmodel>& model, global_store& globals )
{
  global_store localglobals = globals;
  genfunct.execute( model, localglobals );
}
