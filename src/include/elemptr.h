#pragma once

#include <symmodel.h>

struct symmodel;

struct elemptr
{
  std::shared_ptr<symmodel> model;
  vector<size_t> idx;
  
  //model is null...
  elemptr()
  {
  }
  
elemptr( const std::shared_ptr<symmodel>& p, const size_t& i )
: model( p ), idx( vector<size_t>(1,i) )
  {
  }

elemptr( const std::shared_ptr<symmodel>& p, const vector<size_t>& i )
: model( p ), idx( i )
  {
  }
};
