
#include <corresp.h>

void corresp::fill( const vector<size_t>& arg )
{
  if( generating() || !isinit() )
    {
      ++pushed;
      ++written;
    }
  if( !isinit() && !generating() )
    {
      if(arg.size() == 0)
	{
	  fprintf(stderr, "REV: corresp::fill, trying to fill with 0 length, does this ever make sense?\n");
	  exit(1);
	}
      
      correspondence.insert( correspondence.end(), arg.begin(), arg.end() );
      numidx = vector<size_t>( correspondence.size(), 1 );
      
      startidx.resize( correspondence.size() );
      std::iota(std::begin(startidx), std::end(startidx), 0);
    }

  fprintf(stdout, "Finished setting up FILL, will now notify other side...\n");
  
  //This will mark init, and also fill the other side ;)
  //It will theoretically mark the other side Filled...
  parent->notify_filled_corresp( targmodel );
  return;
}




void corresp::markinit()
{
  //REV: don't mark init if we are generating( just looking for var deps )
  if( generating() == false )
    {
      parent->notify_size_change( startidx.size() );
      fprintf(stdout, "In corresp MARKINIT, in corresp [%s] notifying parent to size change to size [%lu]\n", buildpath().c_str(), startidx.size() );
      init=true;
    }
}

vector<vector<size_t>> corresp::make_mirror( const size_t& src_modelsize )
  {
    fprintf( stdout, "Started out...\n");
    vector<vector<size_t>> mirrored( src_modelsize );
    fprintf( stdout, "Will iter thru corresp......\n");
    for(size_t n=0; n<correspondence.size(); ++n)
      {
	fprintf( stdout, "Crressp [%lu] \n", n);
	size_t c=correspondence[n];
	fprintf( stdout, "Crressp [%lu] is [%lu]\n", n, c);
	if( c >= mirrored.size() )
	  {
	    fprintf(stdout, "C >= mirror size\n");
	    mirrored.resize( c+1 );
	  }
	/*if( c >= src_modelsize )
	  {
	    fprintf(stderr, "REV: error in mirroring correspondence...I am trying to access [%lu] in current, but it's larger than model size [%lu]\n", c, src_modelsize);
	    exit(1);
	    }*/
	

	fprintf(stdout, "Will try to push back to c now...\n");
	mirrored[c].push_back( n );
	fprintf(stdout, "FINISHED Will try to push back to c now...\n");

      }

    fprintf(stdout, "Finished, will ret\n");
    return mirrored;
    
  }



string corresp::buildpath()
  {
    if( !parent )
      {
	fprintf(stderr, "REV: error, correspondence has no parent\n");
	exit(1);
      }
    if( !targmodel )
      {
	fprintf(stderr, "REV: error, correspondence has no targmodel\n");
	exit(1);
      }
    string premodel = parent->get_toplevel_model()->buildpath();
    string postmodel = targmodel->get_toplevel_model()->buildpath();
    string result = premodel + "->" + postmodel;
    return result;
  }
