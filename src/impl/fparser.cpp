
#pragma once

#include <fparser.h>

namespace fparser
{

  //REV: impl of parser constructor
  template <typename It, typename Skipper>
  parser<It,Skipper>::parser()
    : parser::base_type(vecform)
  {
    using qi::lit;
    using qi::lexeme;
    using qi::raw;
    using ascii::char_;
    using ascii::string;
    using namespace qi::labels;
	 
    using phoenix::at_c;
    using phoenix::push_back;


    //only the first level parses. So basically, it parses one or more things separated by ","
    //However, after that, everything is a "chunk". Problem is it will include nested functions' commas.
    //So, only parse "stringform" separated by commas.
    //A "stringform" can be any number of things
    //However, it just returns it as a string.
    //It includes specifically

	 
    //Does the star work here? Can I have just a literal for the vect form? What will it parse if there is nothing? It will infinitely parse?
    vecform = (
	       fname [push_back(ref(_val), _1)]  
	       >>
	       *(
		 '('
		 >>
		 stringform [push_back( ref(_val), _1) ]
		 >>
		 *( ',' >> stringform [push_back( ref(_val), _1) ] )
		 >>
		 ')'
		 )
	       );
	  

    //Same as vect, but it just makes a single string.
    //Need to specify to return it e.g. lexeme[ ]
    //Easier to parse it and turn it into a single string, but how to do that?
    //I could do it with nested parse lol...
    stringform = (
		   qi::repeat(0, 1)
		   [fname
		  >>
		  *(
		    char_('(')
		    >>
		    stringform
		    >>
		    *(
		      char_(',')
		      >>
		      stringform
		      )
		    >>
		    char_(')')
		    )
			
		  
		   ]
		  );


    //Read until (, or if no (, then just read and exit...
    //Am I required to have parens? I guess I can have "optinal" parens ;0
    //How do I tell it to go "until it ends"?
    fname = (
	     lexeme[+(char_ - '(' - ')' - ',')]
	     );

    //A stringform simply returns itself. It only reads up until ) basically (if it had an opener)
	  
  } //end implementation of parser CONSTRUCTOR


  
  //REV: impl of parser constructor
  template <typename It, typename Skipper>
  doparser<It,Skipper>::doparser()
    : doparser::base_type(vecform)
  {
    using qi::lit;
    using qi::lexeme;
    using qi::raw;
    using ascii::char_;
    using ascii::string;
    using namespace qi::labels;
    
    using phoenix::at_c;
    using phoenix::push_back;

    //Does the star work here? Can I have just a literal for the vect form? What will it parse if there is nothing? It will infinitely parse?
    vecform = (
	       *(
		 estringform [push_back( ref(_val), _1) ]
		 >>
		 *( ',' >> estringform [push_back( ref(_val), _1) ] )
		 )
	       );
	  

    //Same as vect, but it just makes a single string.
    //Need to specify to return it e.g. lexeme[ ]
    //Easier to parse it and turn it into a single string, but how to do that?
    //I could do it with nested parse lol...
    estringform = (
		   fname
		  >>
		  *(
		    char_('(')
		    >>
		    stringform
		    >>
		    *(
		      char_(',')
		      >>
		      stringform
		      )
		    >>
		    char_(')')
		    )
		   );
    
    stringform = (
		   qi::repeat(0, 1)
		   [fname
		  >>
		  *(
		    char_('(')
		    >>
		    stringform
		    >>
		    *(
		      char_(',')
		      >>
		      stringform
		      )
		    >>
		    char_(')')
		    )
			
		  
		   ]
		  );




    //Read until (, or if no (, then just read and exit...
    //Am I required to have parens? I guess I can have "optinal" parens ;0
    //How do I tell it to go "until it ends"?
    fname = (
	     lexeme[+(char_ - '(' - ')' - ',')]
	     );

  }
  
} // end fparser namespace
