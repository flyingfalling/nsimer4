#include <generator.h>

void genfunct_t::execute( std::shared_ptr<symmodel>& model, global_store& globals )
{
  for(size_t l=0; l<lines.size(); ++l)
    {
      execute_line( model, globals, l );
    }
}


void genfunct_t::execute_line( std::shared_ptr<symmodel>& model, global_store& globals, const size_t& line )
{
  if( line >= lines.size() )
    {
      fprintf(stderr, "REV: trying to execute a line outside of number of lines in generator!\n");
      exit(1);
    }
  
  vector<elemptr> trace;
  vector<size_t> myidx( 1, 0 );
  
  elemptr tmp( model, myidx );
  
  trace.push_back(tmp);

  if( !cmds )
    {
      fprintf(stderr, "REV in execute, CMDS not defined yet!\n" );
      exit(1);
    }
  
  DOCMD( lines[line], trace, *cmds, globals );
}

void generator::generate( std::shared_ptr<symmodel>& model, global_store& globals )
{
  global_store localglobals = globals;
  genfunct.execute( model, localglobals );
}

void genfunct_t::addlocal(const string& fname, const string& f )
  {
    cmds->addlocal( fname, f );
  }
