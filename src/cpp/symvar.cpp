#include <symvar.h>


real_t symvar::getvalu( const size_t& _idx )
{
  
  if( !isinit() )
    {
      return 0;
    }

  if( isconst() )
    {
      return valu[0];
    }
  
  if( _idx >= valu.size() )
    {
      fprintf(stderr, "In symvar, getvalu, idx [%lu] > size of valu array [%lu], var name [%s] in containing model [%s]\n", _idx, valu.size(), name.c_str(), parent->buildpath().c_str());
      exit(1);
    }
  
  return valu[_idx];
  
}

vector<real_t> symvar::getvalus( const vector<size_t>& _idx )
{
  if( !isinit() )
    {
      return vector<real_t>( _idx.size(), 0);
    }

  vector<real_t> ret;
  for(size_t x=0; x<_idx.size(); ++x)
    {
      ret.push_back( getvalu( _idx[x] ) );
    }
  
  return ret;
  
}


varptr symvar::vgetvalus( const vector<size_t>& idx )
{
  varptr tmp;
  
  //Ugh, no type checking fuck this.
  if( !isinit() || generating() )
    {
      ++read;
      //REV; will this cause any problems?
      tmp.valu = vector<real_t>( idx.size(), 0);
      return tmp;
    }
  
  if( isint() )
    {
      tmp.idx = getivalus( idx );
    }
  else
    {
      tmp.valu = getvalus( idx );
    }

  return tmp;
  
  
}

void symvar::vsetvalus( const vector<size_t>& idx, const varptr& v )
{
  if( !isinit() || generating() )
    {
      ++written;
      return;
    }

  if( isint() )
    {
      setivalus( idx, v.idx );
    }
  else
    {
      setvalus(idx, v.valu );
    }
  return;
}

  
size_t symvar::getivalu( const size_t& _idx )
{
  if( !isinit() )
    {
      return 0;
    }

   if( isconst() )
    {
      return ivalu[0];
    }
  
  if( _idx >= ivalu.size() )
    {
      fprintf(stderr, "In symvar, getivalu, idx [%lu] > size of valu array [%lu], var name [%s] in containing model [%s]\n", _idx, ivalu.size(), name.c_str(), parent->buildpath().c_str());
      exit(1);
    }

  return ivalu[_idx];
  
  
}
vector<size_t> symvar::getivalus( const vector<size_t>& _idx )
{
  if( !isinit() )
    {
      return vector<size_t>( _idx.size(), 0);
    }

  vector<size_t> ret;
  for(size_t x=0; x<_idx.size(); ++x)
    {
      ret.push_back( getivalu( _idx[x] ) );
    }
  
  return ret;
}


void symvar::setivalu( const size_t& _idx, const size_t& val )
{
  if( !isinit() )
    {
      //do nothing
      //return;
      return;
    }
  else if( isconst() )
    {
      if( _idx != 0 )
	{
	  fprintf( stderr, "Whoa error in setval, isconst but idx zero element is not 0...\n");
	  exit(1);
	}
      
      ivalu[0] = val;
      
    }
  else
    {
      if( _idx >= ivalu.size() )
	{
	  fprintf(stderr, "In symvar, setvalu, idx [%lu] > size of valu array [%lu], var name [%s] in containing model [%s]\n", _idx, ivalu.size(), name.c_str(), parent->buildpath().c_str());
	  exit(1);
	}
      
      ivalu[_idx] = val;
    }
}

void symvar::setivalus( const vector<size_t>& _idx, const vector<size_t>& val )
{
  if( !init )
    {
      //do nothing
    }
  else
    {
  
      if(val.size() != _idx.size())
	{
	  fprintf(stderr, "setvalus, error idx size and val size not same\n");
	  exit(1);
	}
  
      for(size_t x=0; x<_idx.size(); ++x)
	{
	  setivalu( _idx[x], val[x] );
	}
    }
  
  return;
}
 

void symvar::setvalu( const size_t& _idx, const real_t& val )
{
  if( !init )
    {
      //do nothing
      //return;
    }
  else if( isconst() )
    {
      if( _idx != 0 )
	{
	  fprintf( stderr, "Whoa error in setval, isconst but idx zero element is not 0...\n");
	  exit(1);
	}
      
      valu[0] = val;
      
    }
  else
    {
      if( _idx >= valu.size() )
	{
	  fprintf(stderr, "In symvar, setvalu, idx [%lu] > size of valu array [%lu], var name [%s] in containing model [%s]\n", _idx, valu.size(), name.c_str(), parent->buildpath().c_str());
	  exit(1);
	}
      
      valu[_idx] = val;
    }
}

void symvar::setvalus( const vector<size_t>& _idx, const vector<real_t>& val )
{
  if( !init )
    {
      //do nothing
    }
  else
    {
  
      if(val.size() != _idx.size())
	{
	  fprintf(stderr, "setvalus, error idx size and val size not same\n");
	  exit(1);
	}
  
      for(size_t x=0; x<_idx.size(); ++x)
	{
	  setvalu( _idx[x], val[x] );
	}
    }
  
  return;
}



void symvar::addivalu( const size_t& i)
{
  ivalu.push_back( i );
  markinit();
  parent->notify_size_change( ivalu.size() );
}
  
void symvar::addfvalu( const real_t& f)
{
  valu.push_back( f );
  markinit();
  parent->notify_size_change( valu.size() );
    
}


void symvar::addivalus( const vector<size_t>& i)
{
  ivalu.insert( ivalu.end(), i.begin(), i.end() );
  parent->notify_size_change( ivalu.size() );
}
  
void symvar::addfvalus( const vector<real_t>& f)
{
  valu.insert( valu.end(), f.begin(), f.end() );
  parent->notify_size_change( valu.size() );
    
}



void symvar::addvalus( const varptr& vp )
{
  if( vp.idx.size() > 0 )
    {
      ivalu.insert( ivalu.end(), vp.idx.begin(), vp.idx.end() );
    }

  else
    {
      valu.insert( valu.end(), vp.valu.begin(), vp.valu.end() );
    }
  
  markinit();
}





void symvar::markinit()
{
  //REV: don't mark init if we are generating (we're just looking for var dependencies)
  if( generating() == false )
    {
      init = true;
  
      //REsize to this size.
      size_t mysize = valu.size();
      if( ivalu.size() > mysize )
	{
	  mysize = ivalu.size();
	}
      parent->notify_size_change( mysize );
    }
}


string symvar::buildpath()
{
  //Use local name?
  if( !parent )
    {
      fprintf(stderr, "REV: error var [%s] has no parent\n", name.c_str() );
    }

  string parentname = parent->buildpath();
  return ( parentname + "/" + name );
}
