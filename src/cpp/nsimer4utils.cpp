
#include <nsimer4utils.h>

vector<string> parse( const string& name)
{
  bool emptyrepeats=false;
  return tokenize_string( name, "/", emptyrepeats );
}

vector<string> parsetypes( const string& name)
{
  bool emptyrepeats=false;
  return tokenize_string( name, "|", emptyrepeats );
}

vector<string> parsecorr( const string& name)
{
  bool emptyrepeats=true;
  
  return tokenize_string( name, "->", emptyrepeats );
}




bool check_idx( const string& varname )
{
  if( varname.compare( "IDX" ) == 0 )
    {
      return true;
    }
  return false;
}

bool check_issize( const string& varname )
{
  if( varname.compare( "SIZE" ) == 0 )
    {
      return true;
    }
  return false;
}

//REV: shit, if I parse "/" will it give me "nothing" or will it give me ""/""?
//I'm leaning towards nothing? Heh...
bool check_iscorr( const string& varname, string& premodelname, string& postmodelname )
{
  vector<string> res = parsecorr( varname );

  fprintf(stdout, "CHECK IF CORRESP: [%s] became [%lu]\n", varname.c_str(), res.size());
  if( res.size() == 2 )
    {
      
      premodelname = res[0];
      postmodelname = res[1];
      fprintf(stdout, "PRE [%s], POST [%s]\n", premodelname.c_str(), postmodelname.c_str() );
      return true;
    }
  return false;
}

bool check_cmd_is_multi( const string& s )
{
  const string mult = "MULTFORALL";
  const string sum = "SUMFORALL";
  if( s.compare( mult ) == 0 || s.compare( sum ) == 0 )
    {
      return true;
    }
  return false;
}



vector<real_t> vect_mult( const vector< vector<real_t> >& v )
{
  if( v.size() == 0 )
    {
      fprintf(stderr, "vect mult size 0\n");
      exit(1);
    }
  
  vector<real_t> r = v[0];
  if( r.size() == 0 )
    {
      fprintf(stderr, "multiplying zero size vector...\n");
      exit(1);
    }
  
  else if( v.size() == 1 )
    {
      real_t res=1;
      for(size_t x=0; x<v[0].size(); ++x)
	{
	  res *= r[x];
	}
      r.resize(1);
      r[0] = res;
      //r.push_back( res ); //size 1 lol
    }
  else
    {
      for(size_t a=1; a<v.size(); ++a)
	{
	  if( v[a].size() != r.size() )
	    {
	      fprintf(stderr, "REV error vect mult, v1 != v2\n");
	      exit(1);
	    }

	  for(size_t x=0; x<r.size(); ++x)
	    {
	      r[x]*=v[a][x];
	    }
	}
    }
  
  return r;
}


vector<real_t> vect_sum( const vector< vector<real_t> >& v )
{
  if( v.size() == 0 )
    {
      fprintf(stderr, "vect sum size 0\n");
      exit(1);
    }
  
  vector<real_t> r = v[0];
  if( r.size() == 0 )
    {
      fprintf(stderr, "sum zero size vector...\n");
      exit(1);
    }
  else if( v.size() == 1 )
    {
      real_t res=0;
      for(size_t x=0; x<v[0].size(); ++x)
	{
	  res += r[x];
	}
      r.resize(1);
      r[0] = res;
      //r.push_back( res ); //size 1 lol
    }
  else
    {
      for(size_t a=1; a<v.size(); ++a)
	{
	  if( v[a].size() != r.size() )
	    {
	      fprintf(stderr, "REV error vect mult, v1 != v2\n");
	      exit(1);
	    }

	  for(size_t x=0; x<r.size(); ++x)
	    {
	      r[x] += v[a][x];
	    }
	}
    }
  
  return r;
} //end vect sum.


vector<real_t> vect_div(  const vector< vector<real_t> >& v )
{
  if( v.size() == 0 )
    {
      fprintf(stderr, "vect div size 0\n");
      exit(1);
    }

  vector<real_t> r = v[0];
  for(size_t x=1; x<v.size(); ++x)
    {
      if( v[x].size() != r.size() )
	{
	  fprintf(stderr, "REV WTF vect div v1 != v2\n");
	  exit(1);
	}
      for(size_t y=0; y<v[x].size(); ++y)
	{
	  r[x] /= v[x][y];
	}
    }

  return r;
  
}

vector<real_t> vect_diff(  const vector< vector<real_t> >& v )
{
   if( v.size() == 0 )
    {
      fprintf(stderr, "vect diff size 0\n");
      exit(1);
    }

  vector<real_t> r = v[0];
  for(size_t x=1; x<v.size(); ++x)
    {
      if( v[x].size() != r.size() )
	{
	  fprintf(stderr, "REV WTF vect diff v1 != v2\n");
	  exit(1);
	}
      for(size_t y=0; y<v[x].size(); ++y)
	{
	  r[x] -= v[x][y];
	}
    }

  return r;
  
}

vector<real_t> vect_negate( const vector<real_t>& val )
{
  vector<real_t> r = val;
  for(size_t x=0; x<r.size(); ++x)
    {
      r[x] = -r[x];
    }
  return r;
}

vector<real_t> vect_exp( const vector<real_t>& val )
{
  vector<real_t> r = val;
  for(size_t x=0; x<r.size(); ++x)
    {
      r[x] = exp(r[x]);
    }
  return r;
}
vector<real_t> vect_sqrt( const vector<real_t>& val )
{
  vector<real_t> r = val;
  for(size_t x=0; x<r.size(); ++x)
    {
      r[x] = sqrt(r[x]);
    }
  return r;
}
vector<real_t> vect_sqr( const vector<real_t>& val )
{
  vector<real_t> r = val;
  for(size_t x=0; x<r.size(); ++x)
    {
      r[x] = r[x]*r[x];
    }
  return r;
}

vector<real_t> vect_normal( const vector<real_t>& meanval, const vector<real_t>& stdval, std::default_random_engine& RANDGEN )
{
  vector<real_t> r = meanval;
  if(meanval.size() != stdval.size())
    {
      fprintf(stderr, "vect_normal error mean != std size\n"); exit(1);
    }
  for(size_t x=0; x<r.size(); ++x)
    {
      std::normal_distribution<real_t> mydist( meanval[x], stdval[x] );
      r[x] = mydist( RANDGEN );
    }

  return r;
     
}

vector<real_t> vect_uniform( const vector<real_t>& minval, const vector<real_t>& maxval, std::default_random_engine& RANDGEN )
{
  vector<real_t> r = minval;
  if(minval.size() != maxval.size())
    {
      fprintf(stderr, "vect_unif error min != max size\n"); exit(1);
    }
  for(size_t x=0; x<r.size(); ++x)
    {
      std::uniform_real_distribution<real_t> mydist( minval[x], maxval[x] );
      r[x] = mydist( RANDGEN );
    }
  
  return r;
     
}
