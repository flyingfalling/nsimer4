//REV: 4 Apr 2016
//Contains symmodel, for constructing symbolic models

#include <symmodel.h>

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


//real_t DOCMD( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(DOCMD)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    {
      exit(1);
    }
  
  vector<string> functparse = cmds.fparse( arg );
  string fname = functparse[0];
  functparse.erase( functparse.begin() );
  vector<string> fargs = functparse;
  
  //Lol, just re-parse them?
  string newarg = CAT( fargs, "," );
  
  cmd_functtype execfunct;
  bool found = cmds.findfunct( fname, execfunct );
  real_t retval;
  if( found )
    {
      retval = execfunct( newarg, model, cmds, myidx, targidx );
    }
  else
    {
      bool isnumer = checknumeric( fname, retval );

      if( isnumer == false )
	{
	  //This will call recursively if fname is not local to model.
	  retval = READ( fname, model, cmds, myidx, targidx);
	}
    }
  
  return retval;
}


//REV: this is "fake" read...I want to compile it into a "real" read, which requires actually looking at correspondence! :)
//If correspondences are not made yet, then whatever?
//So, yea, they should take "symvar" after all ;)
//Which variable I "read" will depend on my "local idx" in the model (i.e. what is my thread?)
//Can I pass that? I.e. in reality it will iterate each one in turn, calling "model" with "index in model".
//Literally a for loop through the model indexes? :) Calling update function.
//Need to check variable update dependencies to know if we need to automatically generate new variables?

//REV: I am making this over-complex. But, I don't want to "reprogram" these, so better to have it actually access
//the variable with offset, based on calling model?
//But, I'm making it do too many things. Best idea is to have differnet structs to do the actual lookup etc.
//Problem is that, it always must know member index in it?
//Which means all variables must pass the index through ;)
//"MODEL" is the model I am "calling" from (?)
//In which case it might go inside a "sub" (hole) different model at some point. In which case, index basis will change.
//real_t READ( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(READ)
{
  //This parses into variable name!!!
  vector<string> parsed = cmds.doparse( arg );
  //Set some "var_counter" in model to be read.
  if( parsed.size() != 1 )
    {
      exit(1);
    }

  //May still be blah/blah/blah
  string varname = parsed[0];

  //"I" am the reading model. SO, I need to do model ->get
  std::shared_ptr<corresp> c;
  bool gotcorresp = model->getcorresp( model->readvar( varname ), c );
  if( !gotcorresp )
    {
      fprintf(stderr, "REV: in READ, error, could not get correspondence of varname [%s] from model [%s]\n", varname.c_str(), model->buildpath().c_str());
      exit(1);
    }
  //symvar& v = model->readvar( varname );
  
  //REV: HERE -- at model.getvar, we need to recursively search
  //This will do the recursive search
  //This is a dummy call!
  real_t val;
  if( model->readvar( varname ).valu.size() == 0 )
    {
      val = 0;
    }
  else //this is a real call!
    {
      size_t idx = 0; //is idx of calling guy?
      
      //Need to like "get nth guy", or need to "for all guys, do x"? Where it will use an iterator variable?
      //Whenever you read a variable, you need to make sure that you are going through the correct correspondence? At each point, it will go through a "correspondence"
      //REV; FUCK FUCK FUCK problem is if we go "through" hole, then "back" it will go many-to-one one way (which is fine), but then when I go "back", it won't know
      //which one of the many was the original one I accessed? So I need to keep an index variable to remember all models what my index in that model was?
      //Like, a model "trace"
      //This is final problem I need to solve...which is how/what index to pass with?
      //val = get_via_corresp( model->readvar( parsed[0] ), idx) );
    }
  
  return val;
}


//REV ERROR is parsed[0] is not a variable!!
//real_t SET( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(SET)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 2 )
    {
      exit(1);
    }

  
  
  string toexec = parsed[1];
  real_t val = DOCMD( toexec, model, cmds, myidx, targidx );

  string varname = parsed[0];

  //"I" am the reading model. SO, I need to do model ->get
  std::shared_ptr<corresp> c;
  bool gotcorresp = model->getcorresp( model->getvar( varname ), c );
  if( !gotcorresp )
    {
      fprintf(stderr, "REV: in SET, error, could not get correspondence of varname [%s] from model [%s]\n", varname.c_str(), model->buildpath().c_str());
      exit(1);
    }
  
  //Could have been read and set separately?
  model->setvar( varname, val );
  
  return 0;
}


//Easier to just make sure it's 2...
//real_t SUM( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(SUM)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }
  
  real_t val=0;
  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val += DOCMD( toexec, model, cmds, myidx, targidx );
    }

  return val;
}


//product
//real_t MULT( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(MULT)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  real_t val=1.0;

  for( size_t tosum=0; tosum<parsed.size(); ++tosum)
    {
      string toexec = parsed[tosum];
      val *= DOCMD( toexec, model, cmds, myidx, targidx );
    }

  return val;
}

//div
//real_t DIV( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(DIV)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];

  real_t val=DOCMD( toexec, model, cmds, myidx );
  
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val /= DOCMD( toexec, model, cmds, myidx );
    }

  return val;
}

//subtract
//real_t DIFF( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(DIFF)
{
  //subtract all from first one?
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() < 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val=DOCMD( toexec, model, cmds, myidx );
  for( size_t tosum=1; tosum<parsed.size(); ++tosum)
    {
      toexec = parsed[tosum];
      val -= DOCMD( toexec, model, cmds, myidx, targidx );
    }
  
  return val;
}


//real_t NEGATE( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(NEGATE)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val= DOCMD( toexec, model, cmds, myidx, targidx );
  return (-1.0 * val);
}

//real_t EXP( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(EXP)
{
  vector<string> parsed = cmds.doparse( arg );
  if( parsed.size() != 1 )
    { exit(1); }

  string toexec = parsed[0];
  real_t val = DOCMD( toexec, model, cmds, myidx, targidx );

  return exp( val );
}

//PROBLEM: How to know what it is talking about "inside" the hole? I assume in this case, it will be referring to the base variables (as local to the returned model)
//Which means...how do I know about correspondence etc.?
//Might be better to do like, sumall( current/V ), which will iterate through all current/V for each member.
//Or, pass it all the way through, like SUMALL( current, SUM(current/V, current/E) ). So, in other words, there is no need to explicitly know it's a hole?
//Although holes contain multiple, which we know. We could do like a "for all by type", which is basically what currents are?
//Note, some current/cond models might be LOCAL, others foreign?
//I guess, in this case, all local...? Shit.
FUNCDECL( SUMFORALL )
//real_t SUMFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if(parsed.size() == 2 )
    {
      //DO through holes
      return SUMFORALLHOLES( arg, model, cmds, myidx, targidx );
    }
  else if(parsed.size() == 1)
    {
      return SUMFORALLCONNS( arg, model, cmds, myidx, targidx );
      //Do through single model (with multiple or one guy)
    }
  else
    {
      fprintf(stderr, "REV: SUMFORALL is not appropriately spread...\n");
      exit(1);
    }
  
  
}


//real_t SUMFORALLHOLES( const string& arg, std::shared_ptr<symmodel>& model, const::shared_ptrconst cmdstore& cmds, const size_t& myidx)
FUNCDECL( SUMFORALLHOLES )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if( parsed.size() != 2 )
    {
      fprintf(stderr, "REV: some big mistake SUMFORALLHOLES, parsed size != 2\n");
      exit(1);
    }
  
  string holename = parsed[0];
  string toexec = parsed[1];
  
  hole myhole = model->gethole( holename );
  
  real_t val=0;
  for( size_t h=0; h<myhole.members.size(); ++h)
    {
      std::shared_ptr<symmodel> holemod = myhole.members[h];
      val += DOCMD( toexec, holemod, cmds, myidx, targidx ); //does DOCMD return a real_t val? Fuck...
    }
  return val;
}

//real_t SUMFORALLCONNS( const string& arg, std::shared_ptr<symmodel>& model, const::shared_ptrconst cmdstore& cmds, const size_t& myidx)
FUNCDECL( SUMFORALLCONNS )
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.
  
  if( parsed.size() != 1 )
    {
      fprintf(stderr, "REV: some big mistake SUMFORALLCONNS, parsed size != 1\n");
      exit(1);
    }

  //REV: TODO make it so it adds corresopnding opposite-side correspondoncence when I add to model.
  
  //REV: shit, this is a direct variable read? I need a way to go through all members of a connected model and get correct corresponding guy?
  //E.g. we get both g and E of model. And back to corresponding V of me.
  //So, basically "inside" of that model, it uses correspondence from "me" to that model???? Fuck, but once I'm "inside" that model...how will I get
  //the correspondence?

  //When I read or write, it returns a vector. I need to tell it what to do with all the vectors it returns.
  //Problem is functs don't return vectors. They return single values ;)

  //So...I need to pass "index-within-index" also. So that I get back a single value for each, and THEN I sum those ;)
  //Shouganai...

  //OK, so now I have this, but it still has a recursive thing. So, in this case, I must STRONGLY ASSUME that all guys are accessed via a SINGLE hole (?)
  //However, we have a problem because they may refer to me?!?!?!?
  //It's fine if I assume all guys in there are referring to a single hole? The hole is a model. I will only invoke this for a single model? Unless my model
  //internally has something? I.e. if I want to sum multiple variables within my own model? In that case, that is just a fucking straight correspondence.
  //I.e. that is what happens when I update for model. Independnetly set for each element. Basically update heh.

  //OK, so basic assumption is that all things are happening WITHIN a given hole? I.e. for a given HOLEINDEX? Or something?
  //Ah, I like that (?). Within a hole, could the function in that hole also happen for something? For example, calling within each, it returns a value. We must tell how
  //to handle WITHIN the holes! So, that takes an index too. The index in *that* case is the **HOLE** index!!
  //In the other case, it is the **element** index.
  //No, that doesn't work, because we must have passed through? "myindex". Ah, in that case, "targidx" is used as "holeidx"...and then we iterate through?
  //Once we have the model, we execute directly? Passing that as the model? SO e.g. we get the model on the other side of the hole. Now, we can access its vars.
  //So, now we want to access for MYIDX, all postsyn vars. So, to do that, we get CORRESP between pre-hole and post-hole. Oh fuck but we have already lost pre-hole,
  //because we passed through the model and executed the next with MODEL as argument. SO, I'm fucked.
  //I need to always keep around "starting" model. Which tells how to access. Problem is, if there is a 3rd model involved (e.g. presyn, to syn, to postsyn), then it is
  //necessary to keep track of the index in each that I am updating.
  //How many models could I possibly be accessing directly? I could go through some unending string of guys for accessing. But, for each CALL, it is a single one? However,
  //I need to know what source I am calling it from? User is sure as fuck not going to specify in update function, so, I will
  //I could make him do that though, like FORALL(X)(do BLAH)


  //Right now I pass a model, and a STRING of thing to run. All references to things in that model are done *FROM* the model.
  //Including recursive calls to jumping in holes of THOSE models.
  //That is fine. Assuming only one jump, what will happen. I want to, for all X, do BLAH. So, it goes inside X, and executes the thing with X as the model.
  //That is fine, the problem is that now MODEL points to X. I want to always have a corresp carried with me. When I go through a hole, the corresp is left as the
  //old one. Otherwise, it is done with "this" one.

  //So, if I'm inside a hole, and there is a call to a var. I guess now I read that variable directly, using (old) correspondence???? FUCK. No, I need to know "which"
  //index of variables to read. If there's only one, I just read the one. Only gone "through" holes. 

  //REV: I call update from a model, right.
  //The function updates this model. I.e. my variables. (none of my holes?)
  //It calls the update function using this model as an argument.
  //Variable lookups are done from this model.

  //If there is a sub-model, that is fine, I don't need to use any lookups.

  //However, I go through a hole.
  //From the point of view of the hole, it is now calling an update function FROM that model.
  //For holes, I iterate through each model of the hole, for each model, I call an update string.
  //In other words, for each sub-model, I am evaluating the update function and getting the result, and doing something with it.
  //It is not an update/write, it is just reads.
  //However, the problem is that, if I am updating the Nth member of "this" model, I only want to reference corresponding members of the
  //other models

  //Furthermore, if we want to interlace other parts of models (i.e. iterate through multi holes of the same size -- fuck!!!)
  //Anyway, don't worry about that.
  //It's really simple Richard, there's 2 things I want to do
  //1) iterate through holes (i.e. models) and sum their results
  //2) iterate through "items" within a hole, and do something to their results.
  //Same thing? Iterating through "items", be they models, or items in a model. Yea but syntax is SUM(1, 2, 3, 4), not taking some vector of "things", which is what would be req.

  //For holes, it's easy because it's iter through model.
  //Problem is with correspondences.

  //Let's run a simulation
  //I am adex1. I am updating gAMPA1.

  //gAMPA1 has presyn, which is the presynaptic syn model. Fine.
  //That will have e.g. hitweight etc.
  //So, I do FORALLHOLES( presyn ).
  //We are updating neuron IDX=1
  //MODEL=gAMPA1
  //This returns:
  //syn2-1 (for example, let's say there's only 1 hole).
  //DO EXAMPLE WITH TRIANGLE, oR DOUBLE DEPTH HOLE
  
  //for that, I execute:
  //  MULT(hitweight, postsyn-gNMDA/affinity)
  //MODEL=syn2-1. We are still updating IDX=1. But, now we want to update it for CORRESP(gAMPA1 -> syn2-1), for hitweight. And also for postsyn-gNMDA/affinity.
  //so, by IDX lookup, we found that our IDX in syn2-1 is 15. (ooh mark each idx owning model?)
  //Great, we might find that we need to do, 13, 15, and 23
  //Fine.
  //Now, we are doing syn2-1[13]. Now we find we need to reference syn2-1[13]/postsyn-gNMDA/affinity
  //So, now we need to know, syn2-1[13]/postsyn-gNMDA/affinity, what idx should we get?
  //Recall that in one way, (gAMPA1[1] -> syn2-1[13][15][23]).
  //So, [13][15][23] -> [1].
  //So it will get that value. It works in this case, fine.
  //However, what about other way.
  //How about case, where we are updating
  //gAMPA1[1], and it references syn2-1? Not possible.
  //(this is fine in this case, but 15 could correspond to multiple back in original?) So we need to know which original we came from?
  //Keep around a "trace" of index to access for ALL possible models? And when we go "back" to a model, we get the value? but might be diff for diff corresps. fuck
  //Ok, how about triangle? Triangle is also fine I guess?
  //How about 2 depth? 2 depth might cause problems if we go: small->big->small->big->small.
  //So, (adex1[1] -> syn1[13][15] -> adex2[3].
  //Let's say for some reason I need to know property of adex[2] to update syn1[13].
  //So, adex1[1], we grab syn[13], fine. Then, syn[13] for some reason, references presyn/tspk for example. In that case, adex2[3]. OK, we now have adex2[3] to return.
  //But let's say that for some crazy reason, (possible?) the string called in syn[13], got some hole in adex2. That is right. But let's say it required it to update
  //something from adex2, from syn[13]. For example, it needs to know um, delay. So, now it goes like, what the fuck:
  //   adex1[1] -> syn[13][15] -> adex2[3] -> syn[13] (somehow it knows which hole it is, and also it knows blah).

  //In other words, I need to be able to "bubble through" arbitrary parts of my model inside it. Which I guess must be totally resolved first.
  //This solves all sorts of nasty problems with not keeping "global" scope when accessing things? Like, name local var, and pass through. Like user says,
  //SUMFORALL( presyn, (var1=localvarX), "MULT( var1, remotevar2 )" )
  //OK, fine. I just tell which variables to maintain. It will resolve those down before going to the next step. But user has no idea what variables will be referenced?
  //OH FUCK just leave the "back track" open for variable and hole referencing? In other words, when searching for the hole, it lets it search the previous model (and
  //any models that led to that). I.e. pass "deep" an ever-expanding list of models, which I "pop back".
  //And, for each model, I assume I must keep an "index" around for what I am currently "computing"
  //Will I ever be able to visit a model while another guy is using its index?
  //"usually" it finds the varible in the last element of the model vector. However, if it cannot find it there, it will pop back.
  //Furthermore, for each one, it keeps the corresponding "index" with it for each model. And it will iterate through, and add a reference with that model index next.
  //This may be somewhat elegant...
  //As I go "back", I pop off for each model. When I reference a variable, I use the one for that model (if its not in current, I go back until it matches that model)

  //Oh, only problem is when I go through a "hole"
  //I.e. model precession represents holes. Got it.
  //If I just go in sub-model, it doesn't count.
  //For example, if I iterate through holes, some may be um, non-local. That is fine.
  //At any rate, after checking "my"  model, then I check "back through models"?
  //Or, do I explicitly name it, like ../../../blah.
  //Nah, just go back through hole and compare at each step.

  //When do I use correspondence then? At each point in time, it must always access it directly if one-to-many.
  //if many to one, it implies that I am going "back" and so should use that one?
  //Right, in other words I never go back (I shouldn't go back?)
  //But, it some cases I *would* go back? In every case, go through models to make sure it is not a model I have been to yet! i.e. bread crumbs ;)
  //If it *is* (that is fine?), just make sure the hole doesn't reference me, which references same hole in me! Haha user is fucked for that you get what you deserve.
  //OK, so:
  //I pass VECT<modelptr>
  //  I iterate through holes and do something for each result, just as it is now. Might be single access...
  //  Whenver I literally go through a hole, e.g. whenever mysubmod/externhole/var, at externhole point, it needs to add to a model-pointer, and I need to handle that
  //  appropriately.
  //  whenever I access a var through it, it needs to only get the corresp guy. If there is more than one, it must be handled in some way.
  //  what if there are many holes, which ref many guys? Not possible?

  //E.g.      mysubmod/externhole[13][15]/nestedexternhole[75][34]. How do I handle that? If externhole is wrapped in a SUM-FOR-ALL, that is fine. However, the second
  //extern hole is not. So, it will error out at this point I guess? Only if direct references like blah/hole/hole. If it is a function on them, that is fine. OK, great
  //just exit out at FILLHOLE or FINDMODEL time, if I am inside a hole, and then I try to go in another (extern) hole? Or should I allow it? In some cases, it could work...
  //As long as it is not returning.
  //That is fine. In cases where user is going through a hole without an appropriate way of handling multiple members, I exit.
  //I will only know that after construction time, I guess? In some cases they may be one-to-one, but without same ordering? Why would that happen?
  //So, always external model, that is the only marker. Without specification, it is fine, but it expects only one.
  //How does the search funct know if all members were appropriately handled?


  //It knows when I "go through" to get the next model?


  //OK, so I am now passing VEC<modelptr> models
  //Inside this function thing, I need to execute "STRING"
  //I resolve variables directly. First, I check if X/Y/Z can be resolved in models[end], indices[end]
  //If not, I go from x in models[x=end-1] to models[x=begin] to check if it can be resolved in indices[x].
  //If not, I exit.

  //At each point, if I try to resolve the variable X/Y/Z, note that it will try to resolve it up to the base level of that model.
  //At each point, it finds the model right. The model may actually be a named hole. If it is a named hole, it would return a HOLE (???!)
  //**NO**. It would return a symmodel directly ONLY if hole.members.size() == 1. OK.
  //So, it returns a model NEWMOD
  // I check, NEWMOD==models[x]? If it is, it is current model, I can continue with with indices[x]
  // If NEWMOD!=models[x], I need to make sure that the model is not one I have already passed through.
  //Note, I store non-base models for indices, but I store non-base models for blah. I check if basemodel of it is same as my basemodel.

  //Anyway, if NEWMOD!=models[x], I need to check all models I have been through to make sure none of them are NEWMOD.

  //So, I check all models[x], first to find if I can match the variable/hole, and then, once having gone through the hole, I need to make sure
  //if it is one of my already passed through models. In that case, I just use that index.
  //Otherwise, I need to get the correspondence from model[x] and NEWMOD. And use that as the new index in NEWMOD, which I push back to models, and return.


  //OK, to code this, I need to modify:
  //Functions: now they take vec<models> and vec<indices>
  //           now when I call a function recursively, I need to be careful about the vec<models> and vec<indicies> I pass.
  //           First, when finding a hole or variable *name*, it needs to try to resolve it in models[list] first (as a hole or submodel)
  //           Furthermore, when going through a hole (or entering a submodel...?) it needs to check that it is not currently in models.


  //Note, if it is, I use current index. I.e. this is a problem of indexing, not a problem of finding models. I get it, OK.
  //Everything works as is without indices. The problem is, I need to make sure to always pass through a "trace" of models and indices. This determines
  //whether a new index needs to be added.
  //In other words, I never return a symlink. I just push to the ongoing vector, that is the return value.
  //Is that OK? yea, e.g. if I "dive" into a model, e.g. adex/blah/blah, that is fine. Problem is how/when I pop them?
  //For example...if, from the function, I go indo adex/presyn/gAMPA/g, where presyn is a hole. In that case, what should I return?
  //I'm just trying to "read" g. Fuck, but I'm "looking" into a model.
  //Remember, the whole point is that, once inside a model, it doesn't matter? Yes, yes it does. blah/blah/blah[X], where X is index in first model,
  //should properly resolve through all holes by passing through correspondences, by "getting" model. Of course it will error if it is a hole.
  //Locally, it will of course be going through and building a model trace, including index. When will it pop that though?
  //The first thing I do is provide an index list, and a models list to it. And it makes sure its not in any of those. But, it starts for finding in last of models.
  //Say, for example, it finds it in one back. So, now it goes into that hole, but it uses the index from the one-back model. Then it goes into next one, and it tries to
  //find it. OOHH here, is the question. Do I "always" append to models/index? Only if it never happened before? Only if it is external? Fuck. Fuck fuck. If it is not
  //external, I just append same index. If it is, I append corresp[idx]. Great.

  //This is additional machinery that is written into:
  //getmodel, getvar, getcontainingmodel, gethole, etc.

  //This is all fine. Main question, at what point do I erase the thing? Oh, it only passes it appended. When the call ends, it returns without overwriting the vector.
  //But only for accesses? No, for everything. I.e. when I go in, I append to the vector but it is a copy. OK. Inefficient but fine.

  //So, how do um, iters work.
  //For example, I iter through a hole, executing a phrase "PHRASE". At this point, I am going into a given hole of the model. So, I use hole.member[x], and go into that model,
  //and for that sub-call, I've appeneded hole.member[x] to the vect list. When it returns, it is gone.

  //Next, OK, given that I am going into a model, I check the model. I am always passing an index. A single index. And it changes based on crresp/same model. Fine.
  //So, now I'm in a hole or not. The corresp has MULTIPLE GUYS. I need to know how to handle it.
  //I will go through, and with my current IDX set to 0, 1, 2, 3, 4, etc., I will execute recursively. With just my model added on with that as the index.
  //When I say my model of course I mean the model I am about to dive into. And the index, is the index I computed as the index in "that" model to dive into, computed
  //from the correspondence from "this" to "that".

  //Note, when I dive in from a previous index, e.g. [1][5][4][15], and I dive in from one back e.g. [4], I still keep the full [1][5][4][15], and I append, probably not
  //necessary though.. Those models are only used for finding ****index****, and it can't hurt. It's also used for finding models refered to,
  //in which case it should only go "backwards"
  //Fuck, could that ever mess something up? Hm, yea I guess if in that case I was iterating through another guy separately...yea, get rid of it. Both for finding idx, and etc.?
  //Is that right? So, I *don't* keep it fine.
  //In case where it is used for finding model, would that ever happen? Only where it's appended when I do recursive function calls in update function.
  //Other than that under normal circumstances, e.g. when filling, it does not happen, because we don't/rarely go through holes.

  //So, need to write the functions for get-model etc., so that they take vec<modelptr> models and vec<size_t> idxs. The purpose is that, whenever I get a model, I also am getting
  //the corresponding index in that model (for what I am now). By default, I don't care about it, e.g. it is zero. If a correspondence doesn't exist (at the beginning, e.g
  //for "get model", it needs to be made?). E.g. for example, in "fill model", I am passing 0, from current model, it tries to find for idx 0. I add a new model. Like,
  //for "get-containing-model", it will try to get "idx". Fuck..so just make a whole new series of get and set, which also refer to INDICES.
  //I like that much better I guess.
  


  //it would have be be nested something right?

  //Problem here is that, for hitweight, we are fine. Our 

}

//How to deal with numerals? Just parse them as base vectors...
//real_t MULTFORALL( const string& arg, std::shared_ptr<symmodel>& model, const cmdstore& cmds, const size_t& myidx )
FUNCDECL(MULTFORALL)
{
  vector<string> parsed = cmds.doparse( arg ); //For sumforall, it will only expect 2 arguments.

  if(parsed.size() != 2 )
    {
      exit(1);
    }

  hole myhole = model->gethole( parsed[0] );
  
  string toexec = parsed[1];
  real_t val=1.0;
  for( size_t h=0; h<myhole.members.size(); ++h)
    {
      std::shared_ptr<symmodel> holemod = myhole.members[h];
      val *= DOCMD( toexec, holemod, cmds, myidx, targidx ); //does DOCMD return a real_t val? Fuck...
    }
  
  return val;
}



void hole::add( const std::shared_ptr<symmodel>& h )
  {
    members.push_back(h);
    
    if( parent->is_submodel( h ) ) // is *NOT* external.
      {
	external.push_back( false );
      }
    else
      {
	external.push_back( true );
      }
    //set external or not?
    //by checking whether h is a submodel of this? E.g. iterate h until it hits null or this.
  }



#define ADDFUNCT(fname)							\
  {									\
    cmd_functtype fa = fname;						\
    add( #fname, fa );							\
  }

cmdstore::cmdstore()
  {
    ADDFUNCT( DOCMD );
    ADDFUNCT( READ );
    ADDFUNCT( SET );
    ADDFUNCT( SUM );
    ADDFUNCT( MULT );
    ADDFUNCT( DIV );
    ADDFUNCT( DIFF );
    ADDFUNCT( EXP );
    ADDFUNCT( NEGATE );
    ADDFUNCT( SUMFORALL );
    ADDFUNCT( MULTFORALL );
  }




