#include <fparser.h>


int main()
{
  vector<string> fstrs = {
    "FUNC1( VAR1, NFUNC1( NVAR1, NVAR2 ), NFUNC2( NVAR3 ), VAR2 )",
    "FUNC1( VAR1, NFUNC1( NFUNC11(NVAR11), NVAR1, NVAR2 ), VAR2 )",
    "FUNC1( VAR1, NFUNC1(), VAR2 )",
    "VAR1"
  };

  vector<string> lstrs = {
    "C1, C2, C3( NC3(), NC4(VAR1, VAR2), NCX )",
    "C1",
    "C1( X1, NC3( NVAR1 ) )"
  };

  /*
  string str1 = "FUNC1( VAR1, NFUNC1( NVAR1, NVAR2 ), NFUNC2( NVAR3 ), VAR2 )";
  
  string str2 = "FUNC1( VAR1, NFUNC1( NFUNC11(NVAR11), NVAR1, NVAR2 ), VAR2 )";
  
  string str3 = "FUNC1( VAR1, NFUNC1(), VAR2 )";
  
  string str4 = "VAR1";
  
  string str5 = "C1, C2, C3( NC3(), NC4(VAR1, VAR2), NCX )";
  
  string str6 = "C1";
  
  string str7 = "C1( X1, NC3( NVAR1 ) )";
  */
  
  auto tmpf(std::begin(fstrs[0]));
  
  vector<string> result;
  
  const static fparser::parser<decltype(tmpf)> myparser;
  const static fparser::doparser<decltype(tmpf)> mydoparser;

  for(size_t s=0; s<fstrs.size(); ++s)
    {
      result.clear();
      
      auto f(std::begin(fstrs[s]));
      auto l(std::end(fstrs[s]));
      
      fprintf(stdout, "Parsing (as function) [%s]\n", fstrs[s].c_str());
      
      bool ok = fparser::qi::phrase_parse( f, l, myparser, fparser::qi::space, result );
      
      
      fprintf(stdout, "Finished parse (into [%lu] elements)! Will print...\n", result.size());
      for(size_t x=0; x<result.size(); ++x)
	{
	  fprintf(stdout, "[%s]\n", result[x].c_str() );
	}
      fprintf(stdout, "\n" );
    }

  fprintf(stdout, "\nAS LIST\n\n");
  for(size_t s=0; s<lstrs.size(); ++s)
    {
      result.clear();
      
      auto f(std::begin(lstrs[s]));
      auto l(std::end(lstrs[s]));
      
      fprintf(stdout, "Parsing (as LIST) [%s]\n", lstrs[s].c_str());
      
      bool ok = fparser::qi::phrase_parse( f, l, mydoparser, fparser::qi::space, result );
      
      
      fprintf(stdout, "Finished parse (into [%lu] elements)! Will print...\n", result.size());
      for(size_t x=0; x<result.size(); ++x)
	{
	  fprintf(stdout, "[%s]\n", result[x].c_str() );
	}
      fprintf(stdout, "\n" );
    }

  return 0;
}
