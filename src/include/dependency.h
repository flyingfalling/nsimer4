#pragma once

#include <symmodel.h>



//Which model, which line number, generated which read/written/pushed.
//We will then go through, and find all "read". If it is "read" we must find any "written". If it is written, we must find any push.
//We must do that recursively, until all items are marked.
//I need to build a dependency graph. A dependency is defined as, all reads must be done after/with all writes, and all writes after all pushes.
//Each gen/line. Anyway, just go until there are no more? First do all note pushes. Order should not matter there.

struct symmodel;


//This is a "node" in the graph.
struct depstate
{
  bool creator=false;
  vector<string> read;
  vector<string> written;
  vector<string> pushed;

  //REV: I say model, but I mean gen I guess/
  std::shared_ptr<symmodel> model;
  size_t linenumber;
  
  bool depends_on( const depstate& d )
  {
    //Return FALSE if d is me.
    if( *this == d )
      {
	return false;
      }

    
    if( readdep( d ) || writedep( d ) )
      {
	return true;
      }

    //All guys depend on creators, unless I am a creator myself
    if( d.creator && !creator )
      {
	return true;
      }
    
    return false;
  }

  bool wroteme( const string& s ) const
  {
    for( size_t x=0; x<written.size(); ++x )
      {
	if( s.compare( written[x] ) == 0 )
	  {
	    return true;
	  }
      }
    return false;
  }

  bool pushedme( const string& s ) const
  {
    for( size_t x=0; x<pushed.size(); ++x )
      {
	if( s.compare( pushed[x] ) == 0 )
	  {
	    return true;
	  }
      }
    return false;
  }

  bool readme( const string& s ) const
  {
    for( size_t x=0; x<read.size(); ++x )
      {
	if( s.compare( read[x] ) == 0 )
	  {
	    return true;
	  }
      }
    return false;
  }

  
  
  bool readdep( const depstate& d ) const
  {
    for( size_t r=0; r<read.size(); ++r )
      {
	string tmp=read[r];
	//See if R occurs in WRITE or PUSH
	if( d.wroteme( tmp ) || d.pushedme( tmp ) )
	  {
	    return true;
	  }
      }
    return false;
  }

  bool writedep( const depstate& d ) const
  {
    for( size_t r=0; r<written.size(); ++r )
      {
	string tmp=written[r];
	//See if R occurs in WRITE or PUSH
	if( d.pushedme( tmp ) )
	  {
	    return true;
	  }
      }
    return false;
  }
  
  bool operator==( const depstate& d ) const
  {
    if( model == d.model && linenumber == d.linenumber )
      {
	return true;
      }
    return false;
  }

  bool operator!=( const depstate& d ) const
  {
    if( (*this) == d )
      {
	return false;
      }
    return true;
  }
};


struct generator_deps
{
  vector<depstate> nodes; //This is a line in the generation. Better to have a var list type thing...
  vector< vector<size_t> > pernode;
  vector<size_t> pre;
  vector<size_t> post;
  
  void build_graph( )
  {
    pernode.resize( nodes.size(), vector<size_t>() );
    
    for(size_t x=0; x<nodes.size(); ++x)
      {
	for(size_t y=0; y<nodes.size(); ++y)
	  {
	    bool isdep = nodes[x].depends_on( nodes[y] );
	    if( isdep )
	      {
		//This means, X needs Y to be evaluated first. So, those with no outgoings, are the winners.
		//Better to make a "per" guy?
		pernode[x].push_back( y );
		pre.push_back( x );
		post.push_back( y );
	      }
	  }
      }
  }

  vector<size_t> findunmarked( const vector<bool>& marked )
  {
    vector<size_t> notdone;
    for(size_t x=0; x<marked.size(); ++x)
      {
	if( !marked[x] )
	  {
	    notdone.push_back(x);
	  }
      }
    return notdone;
  }
  
  //Now that the graph is built, I can do the dependency parsing, i.e. get an "ordering" of nodes...
  vector<depstate> parse_dependencies()
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
  
  void visit( const size_t& startnode, vector<bool>& tmpvisited, vector<bool>& visited, vector<size_t>& evalorder )
  {
    if( startnode >= tmpvisited.size || startnode >= visited.size() )
      {
	fprintf(stderr, "REV: something is wrong, visited or tmpvisited is wrong size (%lu) and (%lu), should be nodes size (%lu)\n", tmpvisited.size(), visited.size(), nodes.size());
	exit(1);
      }
    if( tmpvisited[ startnode ] )
      {
	//REV: this may exit in middle of recursion
	fprintf(stderr, "REV: ERROR, graph is not directed/acyclic (DAG). Failed on node [%lu]\n", );
	exit(1);
      }
    else
      {
	if( !visited[ startnode ] )
	  {
	    tmpvisited[ startnode ] = true;
	    if( startnode >= pernode.size() )
	      {
		fprintf(stderr, "REV: error pernode size is too small compared to startnode\n");
		exit(1);
	      }
	    for(size_t postnode=0; postnode<pernode[startnode].size(); ++postnode)
	      {
		visit( pernode[startnode][postnode], tmpvisited, visited, evalorder );
	      }
	    visited[ startnode ] = true;
	    tmpvisited[ startnode ] = false;
	    evalorder.push_back( startnode ); //NOTE, evalorder is backwards!
	  }
      }
  } //end visit
  
};



struct dep_parser
{
  generator_deps
};
