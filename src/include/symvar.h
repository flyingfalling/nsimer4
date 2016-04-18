#pragma once

#include <symmodel.h>
#include <commontypes.h>
#include <cmdstore.h>
#include <corresp.h>

struct symmodel;
struct varptr;
struct elemptr;
struct corresp;

struct symvar
{
public:
  std::shared_ptr<symmodel> parent;
  string name;
  string type;

private:
  std::vector<real_t> valu;
  std::vector<size_t> ivalu;

  bool init=false;
  bool genmode=true;

  size_t read=0;
  size_t written=0;
  size_t pushed=0;




private:
  bool generating()
  {
    return genmode;
  }

    

  bool isint()
  {
    return (ivalu.size() > 0);
  }

  
  real_t getvalu( const size_t& idx );
  

  size_t getivalu( const size_t& idx );
  
  
  vector<real_t> getvalus( const vector<size_t>& idx );
  vector<size_t> getivalus( const vector<size_t>& idx );
  
  void setvalu( const size_t& idx, const real_t& val );
  void setvalus( const vector<size_t>& idx, const vector<real_t>& val );
  
  void setivalu( const size_t& idx, const size_t& val );
  void setivalus( const vector<size_t>& idx, const vector<size_t>& val );

  void addivalu( const size_t& i);
  void addfvalu( const real_t& f);
    
public:


  void donegenerating()
  {
    genmode = false;
  }
  

  bool isconst()
  {
    //only if size is 1????
    if( ivalu.size() == 1 || valu.size() == 1 )
      {
	return true;
      }
    return false;
  }

  string buildpath();

  
  bool isinit()
  {
    return init;
  }
  
  void markinit();
  
  varptr vgetvalus( const vector<size_t>& idx );
  void vsetvalus( const vector<size_t>& idx, const varptr& v );
  
  void addivalus( const vector<size_t>& i);
  void addfvalus( const vector<real_t>& f);

  void addvalus( const varptr& vp );
    
  
  void reset()
  {
    read=0;
    written=0;
    pushed=0;
  }
  bool wasread()
  {
    if(read>0) { return true; }
    return false;
  }

  bool waswritten()
  {
    if(written>0) { return true; }
    return false;
  }

  bool waspushed()
  {
    if(pushed>0) { return true; }
    return false;
  }
  
  //Default is "my location"
symvar( const string& n, const std::shared_ptr<symmodel>& p )
: name(n), parent(p)
  {
  }
  
symvar( const string& n, const string& t, const std::shared_ptr<symmodel>& p  )
: name(n), type(t), parent(p)
  {
  }

}; //end struct symvar
