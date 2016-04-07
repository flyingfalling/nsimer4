#include <fparser.h>


int main()
{
  string str1 = "FUNC1( VAR1, NFUNC1( NVAR1, NVAR2 ), NFUNC2( NVAR3 ), VAR2 )";

  auto f(std::begin(str1));
  auto l(std::end(str1));
  vector<string> result;
  const static fparser::parser<decltype(f)> myparser;

  fprintf(stdout, "Parsing [%s]\n", str1.c_str());
  
  bool ok = fparser::qi::phrase_parse( f, l, myparser, fparser::qi::space, result );


  fprintf(stdout, "Finished parse (into [%lu] elements)! Will print...\n", result.size());
  for(size_t x=0; x<result.size(); ++x)
    {
      fprintf(stdout, "[%s]\n", result[x].c_str() );
    }

  return 0;
}
