//REV: Final best impl
//Model, not including generators.
//I.e. assumes everything is already allocated.
//Generators will be implemented separately...
//We need a way to "reset" models to a target (distribution) though.
//That will also only matter later, after "model" is running...

//Basic struct of simulation is we have:
//1) symbolic models, which define update of an individual "element" of a model
//   symbolic models may contain many "parts", such as conductances, etc.
//   These may be defined by "lists" of parts, i.e. may be different number of these per neuron (like presynaptic)



//How was I doing it?

//BASE_MODEL (structural model?)

//MODEL. This is a "base" model.
//Have INTERNAL var (i.e. things that I have)
//Have "named holes", which are other models that are included.
//Then, models can be added to "holes", by add_intern_model

//Note, we do not know the "type" of intern variables yet...


//SYMB_MODEL ("logical" model)
//Next we build the "symbolic" model, which is e.g. the connectivity of the circuit and which variables go to which between base models.
//If that's the case, they are specified after I guess...by a generator etc.
//For now, generating 3d grid, and then um, jittering with 3D gaussian?

//Code stuff for model...figure out how I will generate though...everything is string is nasty.



//Abstract implementation of update equations etc. (for each element for example).
//SYMBOLIC (abstract) syntax tree like thing.
SYMB_MODEL adex_neuron;
adex_neuron.var( "V" );
adex_neuron.var( "W" );
adex_neuron.var( "tspk" );

adex_neuron.hole( "conductance" );
adex_neuron.hole( "current" );

adex_neuron.add_model_part( pos3d, "pos" ); //This could technically be a "hole"
adex_neuron.set_part_type( "pos", "position3d" );

adex_neuron.add_model_part( gLeak, "gL" );
adex_neuron.set_part_type( "gL", "conductance" );


SYMB_MODEL gAMPA;
gAMPA.var("g");
gAMPA.var("E");
//Tau, affinity, etc.? Need to make a "postsyn-binder" model which specifies how a "hitweight" translates to input to the conductances.
//I.e. a model of the specific affinities of GLU etc. This just needs to know how many (more?) nS I get this turn...?

SYMB_MODEL gNMDA;
gNMDA.var("g");
gNMDA.var("E"); //and some other vars...g2 g1 etc.

SYMB_MODEL gLeak;
gLeak.var("g");
gLeak.var("E");


SYMB_MODEL spiking_synapse; //event model?
//REV: THIS IS FIRING SCHEDULER, BE CAREFUL.
spiking_synapse.multi_hole( "postsyn_receptors" );
spiking_synapse.unique_hole( "presyn_firetime" ); //specify that this must be UNIQUE (i.e. only 1?)



//Actual connections in *this* circuit.
//Tell which "holes" (?) parts of the SYMB MODELs are connected to which.
//Including neuron connections, what type of schedulers there are etc,
//But doesn't actually generate them yet.
CIRCUIT_MODEL my_circuit_model;

//REV: I could specify all connections here, however it is useful to have "preconstructed" symbolic models...e.g. they contain conductance.
//Otherwise I am adding to "instances", I.e. making a new symbolic model. However, I can't modify the update equation at that point, I think.

my_circuit_model.add_model( adex_neuron, "adex1" );
my_circuit_model.add_model( adex_neuron, "adex2" );
my_circuit_model.add_model( spiking_synapse, "syn2-1");
my_circuit_model.add_model( spiking_synapse, "syn2-2");


//REV: specify which parts are connected to which parts...
//A current injector to adex1
my_circuit_model.add_model_part( Iinj, "Iinj1", "adex1" );
my_circuit_model.set_part_type( "adex1->Iinj1", "current" ); //fills "hole"?

// syn2-1 connects to adex1 via 2 connection types
my_circuit_model.add_model_part( gAMPA, "gAMPA", "syn2-1" );
my_circuit_model.add_model_part( gNMDA, "gNMDA", "syn2-1" );
my_circuit_model.set_part_type( "syn2-1->gAMPA", "postsyn_receptors" ); //fills "hole"?
my_circuit_model.set_part_type( "syn2-1->gNMDA", "postsyn_receptors" );
my_circuit_model.set_connection( "syn2-1->postsyn_receptors", "adex1->conductance" ); //so...cond will know to look through "me"...OK

// syn2-1 connects to adex2 via 1 connection type (presyn spike time)
my_circuit_model.set_connection( "syn2-1->presyn_firetime", "adex1->tspk" );


//After specifying all of that, generators will iteratively generate actual implementations of the circuit.
//This specifies the generators, sizes, and variables.
CIRCUIT_INSTANCE my_circuit_inst;



//Add generators and their parameters?
//First "make" new generators.


//First, place neurons in space, given densities, according to some function. If it is an arbitrary function, we do it using monte carlo....?
//But, then it must be normalized to zero/discretized? Fuck. Or integrated. Fuck.

//Basic problem:
// some generators are "global", i.e. I draw from something and place members.
// Other generators are "local", i.e. each member does its thing regardless of the whole.
// Force user to specify these differences?
//"Grow" in some iterative algorithm? Cool... I.e. a simulation/model in itself to determine the positions. Or, neuron activity itself causes
//them to cluster in a "nice" grid. More parameters....



//Generators can depend on:
// other generators
// other parameters (of models)
// atomic values etc.

//GRID:

//Give basic random generators to user...E.g. uniform, etc.

//Leave it up to user to figure out integral? Force them to give integral function...OK. Like they must sum to 1 integral.
//Can each dimension rely on each other? I.e. correlation

//I.e. can I rely on value of function in Y dim or something? I guess so...

//This is a global thing...it tells "how" a variable is generated....? No, now I make a more complex one...

GLOBAL_GEN grid3d_gen; // this would make a "cube" grid, which is not realistic? Set a gaussian at each center, and do it that way?

grid3d_gen.var( "dim1" );
grid3d_gen.var( "dim2" );
grid3d_gen.var( "dim3" );

grid3d_gen.param( "dim1_density" ); //REV: are params always constant? Not necessarily. Assumption is that everything relies on const generators...?
grid3d_gen.param( "dim2_density" );
grid3d_gen.param( "dim3_density" );

grid3d_gen.param( "dim1_min" );
grid3d_gen.param( "dim1_max" );

grid3d_gen.param( "dim2_min" );
grid3d_gen.param( "dim2_max" );

grid3d_gen.param( "dim3_min" );
grid3d_gen.param( "dim3_max" );

//This generates the points, just a vector of 3d pos???

//Then, I generate "new" 3d points from those gaussian centers.

//These could rely on other aspects, such as a function of dim1, dim2, dim3 etc.
//So, it generates std and mean from the gen3d_gen?
LOCAL_GEN grid3dgauss_gen;

grid3dgauss_gen.var("dim1");
grid3dgauss_gen.var("dim2");
grid3dgauss_gen.var("dim3");

grid3dgauss_gen.param("d1std");
grid3dgauss_gen.param("d1mean");

grid3dgauss_gen.param("d2std");
grid3dgauss_gen.param("d2mean");

grid3dgauss_gen.param("d3std");
grid3dgauss_gen.param("d3mean");

//SOMETHING, must generate dim1mean, dim2mean, dim3mean.
grid3dgauss_gen.required_gen( "d1meangen", "dim1mean" );
grid3dgauss_gen.required_gen( "d2meangen", "dim2mean" );
grid3dgauss_gen.required_gen( "d3meangen", "dim3mean" );

grid3dgauss_gen.set_param_gen("d1meangen", grid3d_gen, "dim1");
grid3dgauss_gen.set_param_gen("d2meangen", grid3d_gen, "dim2");
grid3dgauss_gen.set_param_gen("d3meangen", grid3d_gen, "dim3");



//Density is not a function of 
grid3d_gen.funct()
{
  dim1wid = dim1max - dim1min;
  dim2wid = dim2max - dim2min;
  dim3wid = dim3max - dim3min;
  dim1size = dim1density * dim1wid; //density is ITEMS/SPACE. We have SPACE
  dim2size = dim2density * dim2wid; //density is ITEMS/SPACE. We have SPACE
  dim3size = dim3density * dim3wid; //density is ITEMS/SPACE. We have SPACE

  //To generate an individual item, it must iterate in some way. Through all 3 dims?
  //I.e. generate 1 dim, then next, then next, using them as starting positions. Crap.

  //Spacing is basically density...?
  dim1start = dim1density/2; //dim1wid / dim1size; //E.g. 100 items at 100 wid, gives 1 spacing between. So, start at 0...end at 99. Should start at spacing/2, end at spacing/2
  dim1spacing = dim1density;
  dim2start = dim2density/2;
  dim2spacing = dim2density;
  dim3start = dim3density/2;
  dim3spacing = dim3density;

  dim1locs = seq( dim1density, dim1size, dim1spacing );
  dim2locs = seq( dim2density, dim2size, dim2spacing );
  dim3locs = seq( dim3density, dim3size, dim3spacing );

  for( n1 in dim1locs )
    {
      for( n2 in dim2locs )
	{
	  for( n3 in dim3locs )
	    {
	      add( dim1, n1 );
	      add( dim2, n2 );
	      add( dim3, n3 );
	    }
	}
    }

  //done
}

grid3dgauss_gen.funct()
{
  dim1 = gaussian( d1mean, d1std ); //check if it's outside allowed MIN/MAX
  dim2 = gaussian( d2mean, d2std );
  dim3 = gaussian( d3mean, d3std );
}


LOCAL_GEN gauss1d;
gauss1d.var("val");
gauss1d.param("mean");
gauss1d.param("std");

gauss1d.funct()
{
  val = gaussian( mean, std );
}



//REV: OK, how to do 1) connections 2) draws from multiple groups.

//DRAWS FROM MULTI GRPS (i.e. "placing" two groups by drawing from target collection with probability). Could make it a function of X/Y etc.?
//Shit, if I already made the "grid" positions, that is fine...but I need to make it in multiple groups.
//Make a generator generate multiple groups possibly.
//That's fine. I use a single generator to generate multiple groups. That is fine. I need to tell how to select between the groups though.

//Just make it so that they can naturally do it.
//Select with some probability.

//Is used to select from a number of groups to put "this" value to.
struct multi_generator
{
  std::string selgrp()
  {
    grp = selector_function(); //calls a selector function. e.g. 20% grp1, 40% grp2, etc.
    //This may be dependent on other parameters, such as x, y, z of the given item in question? In which case need integral...
    
    return grp;
  }

  //Somehow use the grp selected as the guy to do it with ;)
};


//How to do connections?
//E.g. we can have probability between, or what? Must be global? Must be local? I can create a local "store" for each presyn or postsyn neuron or some shit...
//How do I do this, there is no "pre and post", but just models.
//Global seems best, but I can do local. If I do local how do I know how "many" to do? I must make sure they don't run into problems. What is an 'item' in that case?
//E.g. for each possible pair? Makes, or it doesn't.

//Need to add to arrays or something. If I set the number, and simply distribute them, that is much easier... but in this case it is actually creating the number of
//models...difficult. In that case, it needs to be done serially...i.e. draw each one from the target. E.g. generate connections from multiple groups. Setting pre and post.
//Or some shit?

//This is difficult. I need to go through all possible neuron models, and all possible post models. and generate. So, e.g. adex1, adex2. Something based on distance.
//I.e. connection probability is based on distance. I add to main guy, independently, computing probability to each other postsynaptic target of each presynaptic neuron.
//I can then combine these into an individual group I guess? So, local generators can uh, create local lists?

//I could generate synapses by:
//1) specifying a neuron-to-neuron probability function. This is easy but will leave "holes". This must be done one-by-one or each one must report its result.
//2) specifying a density, and drawing (attempting to draw) for each neuron, its own specific density... (i.e. normalize probability for each neuron). Again, will make its own local vector, which needs to be made into the final vectors.
//3) specify a max number of synapses, and choose pre/post neuron by drawing from multinomial (?). First, uniform, then, based on probability density of that one.

//At any rate, it needs to be able to grow unlimited vector for each "item". How to do this on GPU is beyond me, but easiest to specify a fixed maximum number first, then do it.
//only up to the maximum number? Or just let it grow without bound? Do it same as spike list...heh. We need to make it large enough so that on average, it will not overflow.

//If it overflows, we just don't add that synapse? We print a warning, or exit? How often will it happen? We need to go through and clear the memory. Yea...so in GPU memory,
//is there a "push back" type thing? :)

//So, I need to write an "emergency" reallocating type thing....which can never return to host (actually it can, only in same "context" though...?).

//Seriously, I need to write boilerplate for a simple vector-like thing to handle outside-of-memory references? That is such a pain in the ass...

//OK, so allow local guys to allocate up to whatever, and then finally, a guy gets their "size" and "pointers" and copies them all back. However, this must be something only
//after all have finished executing....Fine, so device kernel. In that case, would I free the old memory, or what? I want it contiguous? I actually want it "interlaced" rofl...
//so inefficient. But allocating/doing random in parallel, is probably the best way to do it. I.e. build it on the freaking GPU, so much better that way... I could do build
//and execute in separate kernels (of course), and copy it back in parallel for sanity/safety...

//OK, that all works. So, now how to do it. Specify "local", but each "local" generates an unknown number, which are all compressed at the end. This is fine...but how does it
//know which is pre/post etc.?

//For example, in case I want each pre/post to generate unknown number from distribution. I say OK, fine, for each pre, it first generates the things for each possible post,
//and then computes cumulative, and then draws N from there. All guys should be doing same thing, but large memory differences. Imagine e.g. 10k post, each one will need 10k,
//so 10k*10k doubles...800MB heh. That is the most expensive operation I think though...

//And then some "main" guy copies them all back somehow...basically all guys are treated like "threads", which compute their own guy.

//So, it's a LOCAL_GEN, but each guy instead of generating "val" (a single val), it generates N values (possibly 0), which are the results. If it's just a true/false, then
//it exists, and then parameters are drawn otherwise. That is just to select the "number".

//So, in some cases, that is effectively selecting the "correspondences", before selecting the contents of the correspondence targets? No, not the correspondence.
//But rather the "size". But it has pre and post selected...so those correspondences are set somehow (shit). WHat is a correspondence? Literally a vector array index...
//But it will point to what...a postsynaptic conductance? A neuron model? Etc.? Yea, I guess, the neuron named thing.

//So, literally that is what it will generate, a correspondence. So, correspondences are first-class citizens (types), not just backend stuff. Do it also for neurons,
//e.g. I will just generate "existence" of neuron or not. And then it will be filled in later (?). So like "pre" or "post" target is set. So, for connection group,
//it will select the size of it based on drawing FOR each pre, SOME number of posts. That's all well and good...but what am I selecting? I am not setting parameters (or am I?)
//should I always force it to set a parameter based on something? A constant parameter? Or some other parameter. But the parameter is only set (created) if the connection
//would exist. There are no totally free correspondences...I have to choose some parameter to distribute first. But it might be e.g. a constant parameter, in which case nothing
//is even made, all we care about is exists, or not. Just make correspondences first-class citizens. It's that easy. Name it something like, a connection. This thing has a
//pre connection to the pre neuron, and a post connection to the post neuron. So I'm co-generating the "pre" connection and the "post" connection. Which are really only
//a correspondence for many variables? Correspondences are between MODELS (whole models!). Not per-variable I assume.

//The question is, HOW do I specify the correspondence with a generator? I'm not generating a variable. The generator is a generator of "pre" and "post". But, what do it tell
//to do? Make it a "connection" generator! I guess I like taht. A special/different type of generator that generates models, and the base type is of type connection, before
//variables are even specified. Fine. All model-to-model connections are mediated by a CONNECTION, which may simply be the one-to-one type, or may be one-to-many, or many-to-one.
//many-to-many is to come. OK. So 3 types of connections. We build arrays for both directions. So, a connection gen could be the base dependency for the type. But it will depend
//on the pre and post being generated (or maybe just the pre). It generates both at a time, wow. OK. Remember, we can generate multiple guys with same generator!! Which we need
//if e.g. synapses CREATE neurons. Fine. That is great. Just like one guy creating multiple neuron groups or something.

//At any rate, once there is a generator that "creates" for a group, other generators will depend on it (can another generator modify size of group later, reduce it?)
//I guess so...shit that causes some problems. E.g. pruning connections or some shit. At any rate, let's go with this.


//1) Generators can generate multiple groups at a time.
//2) Generators can be local and also "create" multiple things per thread, in which case they are coalesced later
//3) All inter-model things are mediated by connections of one of the 3 types, just a size_t array or something. These can be referenced just like any other varible, by referring
//   to the pre-model post-model name (?). Will it deduce which is the many-to-one etc.? E.g. generate syn2-1->adex1 (what? fuck...). Or do I have to name the "connected"
//   variables/holes, which may be multiple in number? No, just name it conn(syn2-1, adex1). If I co-generate conn(adex2, syn2-1). Nice. It adds them in lock-step though,
//   one is added to each in turn. So it's actually generating the same number of guys, and specifically, it is the syn2-1 model size!!!! So, it's actually generating MODELS
//   huh...? Co-generating is differnet from selecting (I think?). Of course only the first guy sets the size, but it might set values for other variables... Co-generating
//   is important.
//4) So, are there no global generators? Global generators are just local generators with a single thread....heh. Fine.
//   local generators just call the function repeatedly on a pre-existing scaffold.
//   Some specify just "set" a given (existing) parameter/variable array, others are "push-back".
//   How do I differentiate? There are "generators" which generate size, and then there are just "fillers" which fill values. They are the same...just use different
//   syntax inside to specify when to add and when to set? Like, for each guy, do blah (draw from a distr)
//   Versus, for each guy, do a test, and push back/add one (with the given value).
//   Do an automatic dependency check, for which generators are dependent on which other variables/models (?!?!?!!!). This determines the actual order in which things are
//   generated.
//5) specify for all models (variables?) a generator. For "update" variables, the generator is used to generate "reset" values. For "parameter" variables, it is used to
//   draw initially.

//   If we want to "reset" temporarily, we can temporarily specify a new generator for a given variable in-time.

//   Every variable, EVEN connection variables (i.e. correspondences) *must* have a generator specified or it will exit. Furthermore, all models MUST have a size-generating
//   generator specified (something that "adds" i.e. sets the size).

//   Size-generating variables are first-class citizens? I.e. can I generate/specify size of something without specifying values for any variable? I could have TWO generators
//   for a variable, one of which is just a dummy to generate size? Seems like a hack. Thus, all models also have implicit "index" variabls, which go from 0 to N-1. And that
//   can be generated although not much use. So, instead of CONN(X, Y), we can generate INDEX(X).



// Syntactically, how do I want to reference the connection/interface between two models. Does the order matter? I assume it will have "both directions" of indices. Wait,
// which am I generating though? I'm co-generating the connections on "this side" I guess... so it is ordered ME-HIM type thing. Wait, in that case I'm also generating
// "me-him" for the presyn guys...heh. Which will be um, many-few. OK... Just always do it that way.
//  CONN(me, him).
// Specifically, name it grp->grp, or use a special symbol for it? Yea, / is for containment, -> is for connection.
//The thing is, *what* do we add? We add specifically the INDEX!!! How do we specify the index of a group? All groups have an index variable, e.g. adex->index.


//Let's build the full thing.







//Once all things have been generated, we can actually do things.
//I like the connections and index idea.



//For a pre-conn guy, to a post-conn guy, we must know which is 1-to-many, and which is many-to-one. We need to eventually order/sort with a warp-offset stride for processing?





//Will depend on xyz location of pre and postsynaptic, so need access to that info ;)
//Might make total synapses all together, but type is different? Not really realistic...?
//What if we have a multi-compartmental guy? How do we determine then which compartment/which neuron it's connected to...?

//Yea, "symbolic" groups etc. needed after all... "named" groups.

struct GLOBAL_GEN
{
  //calls funct once.
}


struct LOCAL_GEN
{
  //calls funct for every element...
  void generate()
  {
    
  }

  //varlist of variables an values?
  //list of pointers to models (??)
  
};














//Main issue is how hard I want to push the generators for connections, where they can be specified. They could all be local.
//At any rate, for determining SIZE and CORRESPONDENCES, that is the most important part.



//Generators for: XYZ position, Vms, etc.


//How to do VM update? Figure out all requirements of all variables left/right, and figure whether I need to keep it around for 1 more timestep.
//If so,


//I need to generate:

//Locations of neurons (?). Is this the "first variable"? We need some way to distribute a variable "first" that is not dependent on anything.
//Variables' values
//Variables' restart values (?) (generators)
//Connections

//What about: Growing axons. Placing dendrites. Etc. Curving can be determined by polynomial spline..








//REV: If everything for generation is local, how can I do it on GPU? I need to e.g. "connect" to there, would I add to my local memory?
//Any contention is handled how? Like, global types are fucked up...

//REV: Shit, need to handle VARt and VARt+1.
//Need to handle generation.
//Parse it for variable references. External models we keep list of what should exist and figure what it is later. Grab correct vector from there.

//REV: Main problem is we need to figure out how hard/easy it is to debug stuff too...I.e. print what it is doing at every time point...
//I.e. we need to be able to add a "print" funct or smthing (?), which stores in mem, sends it back, and logs it.

