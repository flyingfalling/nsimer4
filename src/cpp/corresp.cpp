
#include <corresp.h>

void corresp::fill( const vector<size_t>& arg )
{
  if( generating() || !isinit() )
    {
      ++pushed;
      ++written;
    }
  else
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
      init=true;
    }
}

vector<vector<size_t>> corresp::make_mirror( const size_t& src_modelsize )
  {
    vector<vector<size_t>> mirrored( src_modelsize );
    for(size_t n=0; n<correspondence.size(); ++n)
      {
	size_t c=correspondence[n];
	
	if( c >= src_modelsize )
	  {
	    fprintf(stderr, "REV: error in mirroring correspondence...\n");
	    exit(1);
	  }
	
	mirrored[c].push_back( n );
      }

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
