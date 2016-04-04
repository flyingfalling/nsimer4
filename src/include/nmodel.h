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



global_generator X;
local_generator Y;

my_circuit_inst.gen( );













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

