#pragma once

#include <commontypes.h>
#include <symmodel.h>


struct symmodel;
struct symvar;
struct varptr;
struct elemptr;



struct corresp
{
public:
  std::shared_ptr<symmodel> targmodel;
  std::shared_ptr<symmodel> parent;

private:
  bool init=false;
  bool genmode=true;
  
  size_t read=0;
  size_t written=0;
  size_t pushed=0;
  
  std::vector<size_t> startidx;
  std::vector<size_t> numidx;
  std::vector<size_t> correspondence;


public:
  void reset()
  {
    read=0;
    written=0;
    pushed=0;
  }
  
  vector<size_t> getcorresprange( const size_t& start, const size_t& end )
  {
    if( correspondence.size() <= end )
      {
	fprintf(stderr, "ERROR get corresprange, end > size of corresp\n");
	exit(1);
      }

    if( start > end )
      {
	fprintf(stderr, "REV: error in getcorresprange, start > end [%lu] and [%lu]\n", start, end );
	exit(1);
      }
    vector<size_t> tmpvect( correspondence.begin()+start, correspondence.begin()+end );
    return tmpvect;
  }

  size_t getcorresp( const size_t& s )
  {
     if( s > numidx.size() )
      {
	fprintf(stderr, "REV: error in getstart idx, requested idx [%lu] > size of startidx [%lu]\n", s, startidx.size() );
      }
    return correspondence[s];
  }

  size_t getstartidx( const size_t& s)
  {
    if( s > numidx.size() )
      {
	fprintf(stderr, "REV: error in getstart idx, requested idx [%lu] > size of startidx [%lu]\n", s, startidx.size() );
      }
    return startidx[s];
  }

  size_t getnumidx( const size_t& s )
  {
    if( s > numidx.size() )
      {
	fprintf(stderr, "REV: error in getnum idx, requested idx [%lu] > size of numidx [%lu]\n", s, numidx.size() );
      }
    return numidx[s];
  }

  void setcorresp( const size_t& s, const size_t v)
  {
    if( s > correspondence.size() )
      {
	fprintf(stderr, "REV: error in setcorresp requested idx [%lu] > size of corresp [%lu]\n", s, correspondence.size() );
      }
    correspondence[s] = v;
  }


  vector<vector<size_t>> make_mirror( const size_t& src_modelsize )
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
  
  bool generating()
  {
    return genmode;
  }
  
  void donegenerating()
  {
    genmode = false;
  }

  void clearcorresp()
  {
    correspondence.clear();
    startidx.clear();
    numidx.clear();
  }
  
  void push( const size_t& size, const vector<size_t>& topush )
  {
    ++pushed;
    
    startidx.push_back( correspondence.size() );
    numidx.push_back( topush.size() );
    correspondence.insert( correspondence.end(), topush.begin(), topush.end () );
  }

  void markinit();
    
  bool isinit()
  {
    return init;
  }


  virtual void set( const vector<size_t>& idxs, const vector<size_t> newvals )
  {
    ++written;
    return;
  }

  vector<size_t> getall( const vector<size_t>& s )
  {
    //Only if not init? only if generating?
    if( !isinit() || generating() )
      {
	++read;

	//vector<size_t> tmp( 1, 0 );
	//return tmp;
      }
    
    vector<size_t> ret;
    for(size_t a=0; a<s.size(); ++a)
      {
	size_t idx = s[a];
	vector<size_t> toadd = getall(idx);
	for(size_t t=0; t<toadd.size(); ++t)
	  {
	    ret.push_back( toadd[t] );
	  }
      }
    return ret;
  }

  //Whenver I "would" fill it, I set "pushed" to true...?
  void fill( const vector<size_t>& arg );
  
  corresp()
  {
  }

corresp( const std::shared_ptr<symmodel>& t, const std::shared_ptr<symmodel>& p)
: targmodel( t ), parent(p )
  {
  }
  
    
private:
  virtual vector<size_t> getall( const size_t& s ) = 0;

}; //end struct corresp






//Are they re-ordered? Possibly...same size?
//ONLY LOCAL MODELS SET, SO EVERYTHING ELSE IS A READ!! :)
struct identity_corresp : public corresp
{
  vector<size_t> getall( const size_t& s )
  {
    return vector<size_t>(1, s);
  }
  
 identity_corresp( const std::shared_ptr<symmodel>& targ,  const std::shared_ptr<symmodel>& p )
   :
  corresp( targ, p )
    {
      
    }
  
 
};


struct const_corresp : public corresp
{
  vector<size_t> getall( const size_t& s )
  {
    return vector<size_t>(1, 0);
  }
  
 const_corresp( const std::shared_ptr<symmodel>& targ,  const std::shared_ptr<symmodel>& p )
    :
  corresp( targ, p )
  {
    
  }
};


struct conn_corresp : public corresp
{
  std::vector<size_t> getall( const size_t& s )
  {
    //DUMMY. Just return zero...because we have no way of knowing what it will be until it's actually filled
    if( isinit()  == false || generating() )
      {
	return vector<size_t>( 1, 0 );
      }
    else
      {
	
	size_t start = getstartidx( s ); //startidx[s];
	size_t size = getnumidx( s ); //numidx[s];
	return getcorresprange( start, start+size ); 
      }
  }

  void set( const vector<size_t>& idxs, const vector<size_t> newvals )
  {
    if(idxs.size() != newvals.size() )
      {
	fprintf(stderr, "corresp set, size idx neq newvals\n");
	exit(1);
      }
    
    for(size_t x=0; x<idxs.size(); ++x)
      {
	size_t myn = idxs[x];
	size_t myv = newvals[x];
	
	setcorresp( myn, myv );

	if( getstartidx( myn ) != myn )
	  {
	    fprintf(stderr, "REV trying to SET in corresp, but it is not a LARGE SIDE\n");
	    exit(1);
	  }

	if( getnumidx( myn ) != 1 )
	  {
	    fprintf(stderr, "REV trying to SET in corresp, but it is not SIZE=1 (i.e. it's not LARGE SIDE )\n");
	    exit(1);
	  }
      }
  }
  
  
 conn_corresp( const std::shared_ptr<symmodel>& targ ,const std::shared_ptr<symmodel>& p )
   : corresp( targ, p )
  {
  }
  
  
};

