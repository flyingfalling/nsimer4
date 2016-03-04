#pragma once



struct MODEL
{
  virtual void update() = 0;
  
  std::vector< std::string > internvars;
  std::vector< std::string > externvars; //has all external as well as internals? Could be a combination of external and internals, wow...
  
  std::vector< std::string > models_types;
  std::vector< std::vector< MODEL* > > models_vect;

  void add_extern_var( const std::string& var )
  {
    externvars.push_back( var );
  }
  
  void add_intern_var( const std::string& var )
  {
    internvars.push_back( var );
  }

};


//We simply tell it which "variables" are external etc., and then they must always be accessed if they're equivalent. Via iteration through MODELS ;)
//Models might be internal. User doesn't care at some point if g[x](t) is from internal or external etc. But, at iteration time, when we go through
//to use it, it must be referenced in a forall(models) loop.

//I need to add model to "fill" that g(x) spot or something? Or that E(x) spot? Does it "automatically" recognize which are which? It looks through all
//for models that have that? Or must be models of a "type"? Like, forall conductance models, etc.
//Easier to reference them in update function... how do I know "which" i iterate through, for Ex and gX. They know. Go through the cond models.
//User KNOWS, because he adds for "that" variable, he adds guys? Nah, he just adds, with some name, like "conductance" or some shit.
//Ah, tell what it is, cond->g or cond->E, or some shit?

//Literally, they have internal/external models...duh.



//Which gX goes with which EX?
//We basically give each "model" that is iterated through, so they correspond... "nth" model's item. Better to have a name or
//smthing to make sure they're the same, ugh. Need a "for all models", so that all accessed items are from that model. Like m[x]->g and m[x]->E

//example:

//REV: This is being too specific.
//The "add model" things would make an individual guy.
//Like a model "instantiation" with gAMPA and gNMDA. But, easier to construct it at each group level?
//Some groups might not have it for example.
//I want to add the group as a "chunk"

void make_adex()
{
  MODEL adex;
  //3d position? etc.
  adex.add_intern_var( "V" );
  adex.add_intern_var( "W" );
  adex.add_intern_var( "Cm" );
  adex.add_intern_var( "Vpeak" );
  adex.add_intern_var( "Vreset" );
  adex.add_intern_var( "Wa" );
  adex.add_intern_var( "Wb" );
  adex.add_intern_var( "tspk" );

  //define variable "holes"?
  adex.add_hole( "conductance" );
  adex.add_hole( "current" );

  adex.add_model( "conductance", gLeak );
  adex.add_model( "conductance", gAMPA );
  adex.add_model( "conductance", gNMDA );

  adex.add_model( "current", Iinj );

  //Define ADEX UPDATE FUNCT, as iterating through "conductance" modeli, doing modeli->g and modeli->E, etc.
  //Force user to name them same thing haha...easier. Could do them by "type" too...oh well.
  //What if we are iterating through "elements" too? Shit?! If it's a model, it has elements. Basic, is just single 1-to-1, but
  //we must e.g. have a "correspondence", if size is different.
  
  MODEL gLeak;
  gLeak.add_intern_var( "g" );
  gLeak.add_intern_var( "E" );

  //Recall there may be multiple gAMPA or gNMDA *implemented* groups which are feeding to this single conductance here...? Or many of these individually?
  //Might have different tau, different E? Etc.? So, I need a way to specify not just different "models" but different "groups" or "implementations" etc.?
  MODEL gAMPA; //could be gGABA etc. too!!!
  gAMPA.add_intern_var( "g" );
  gAMPA.add_intern_var( "E" );
  gAMPA.add_intern_var( "tau1" );
  gAMPA.add_intern_var( "tau2" );
  gNMDA.add_intern_var( "GluAffinity" );

  //REV: FUCK?!! What if I have both um, gNMDA to add, and gNMDA to add?
  //in which case there is like gHit for each and every POSTSYN TYPE.
  //Which I need to update...
  MODEL gNMDA;
  gNMDA.add_intern_var( "g" );
  gNMDA.add_intern_var( "E" );
  gNMDA.add_intern_var( "tau1" );
  gNMDA.add_intern_var( "tau2" );
  gNMDA.add_intern_var( "g2" ); //?? Need a gated variable, like "bound" guys versus "gated" guys.
  gNMDA.add_intern_var( "GluAffinity" );
  
  MODEL Iinj;
  Iijn.add_intern_var( "I" );
  
  
  
  gAMPA.add_hole( "gSyn" );
  gAMPA.add_model( "gSyn",  spksyn ); //May get from multiple GRPS (models?) of synapse, implementations at least. And furthermore, from many INDIVIDUALS
  //within there. Do we care difference between individuals and models? I guess...we need to make sure for my "N" of this "group" I am updating, that
  //I am (always?) grabbing the correct "X" from the target. Thus, every time we do model->g or whatever, we don't just directly access it, but we're
  //iterating not only through models, but also through individuals. Every model-model interface *ALWAYS* has a correspondence, of which (group of?)
  //Ns correspond to me. I can of course remove the loop if it is just 1, or if it is equivalent, etc.
  //OK, so how to "actually" make. If there is a single group, it always "adds" guys, etc.

  
  EVENT_MODEL spksyn;
  //For each guy, need to have a hit_gAMPA , and hit_gNMDA etc.!!!!
  spksyn.add_intern_var( "hitweight" );
  spksyn.add_intern_var( "weight" );
  spksyn.add_intern_var( "t_hit" ); //this is time of hit? Or time of spike?
  
  spksyn.add_intern_var( "transmitter_type" ); //GABA or Glutamate, etc.? Do I need to know this? Choose all syns that have this as postsyn, and find
  //the ones that are of (transmitter) type, and add those to add to me. Etc.
  
  spksyn.add_hole( "postsyn_receptors" );

  spksyn.add_model( "postsyn_receptors", gAMPA );
  spksyn.add_model( "postsyn_receptors", gNMDA ); //automatically do this for all types that receive "glu". Every time, normalize for all of them add add that amount or whatever.

  spksyn.add_hole( "presyn_firetime" ); //name is like "tspk" or some shit.
  //spksyn.add_model( "presyn_firetime", adex ); //or spiker or some shit?

  spksyn.specify_event_adder( "presyn_firetime" ); //There is only one of these? It is outside? Update function specifies it. Based on that, it updates
  //every turn, to "schedule" events I gues.

  //Need to specify e.g. how to build it based on "delay", etc. Like, how many "holes", what is max number, what is min/max delay, etc.

  spksyn.specify_event_trigger( "t_hit" );
  
  //spksyn.add_intern_var( "pren" ); //Do this by "add model type", which will be a presyn/postsyn neuron? there must be only one of each etc.?
  //May not have a presyn neuron though haha...
  //spksyn.add_intern_var( "postn" );

  //MODEL spk_sched;
  //spk_sched.add_hole( "event_time" );
  //spk_sched.add_model( "event_time", spksyn ); //gets presyn neuron of spk.
  //For all spikes, check if presyn neuron fired, and add to my thing.
  //Wait what, for each neuron, I compute adding all postsyn guys...that is still iterating through all SYNAPSES (fuck...).
  //my UPDATE is to check and add. And then, to check hits this turn, and
  //process. And call "update" of spksyn...ok.

  //REV: HOW TO SPECIFY TO UPDATE SPKSYN SECOND THING WHEN THIS HITS?!
  //checks its fire time or shit.
  //REV: Do I need to explicitly make correspondences. Of every pair of grps? I guess so, pass it an algorithm (or results)? They can be based on
  //any parameters of the target. Of a certain "kind"? They may also be based on some "global" guy...


  //User doesn't know what it is now. that only happens when we actaully "build" them ;)

  //To build them, I specify "implementations" of them?
  
}



//This defines an abstract "connectivity", however in the abstract case, I need to know there can be "many" to me. Again as top, OK, all have corresp
//so its all for loops. I erase if there is always 1, and it's 1-by-1.

//Great, so that is how we update those guys ;)
//We then define update equations for them, and it determines dependencies based on those.
//Need to get actually, um, build single "groups". So, I've constructed the "abstract" models (????), which I can make individual ones of.
//E.g. has NMDA, doesn't have NMDA., etc.

//Now, actually build the groups
void build_adex_grps()
{
  modelgroup adexgrp1( adex ); //Will this recursively make the conductances etc. too? I need to "fill" all the "internal" models? I need to tell how to
  //"connect" everythign together. Do it manually for now?

  modelgroup adexgrp1_gL( gLeak );
  modelgroup adexgrp1_gAMPA1( gAMPA ); //from conn1_2
  modelgroup adexgrp1_gNMDA1( gNMDA ); //from conn1_2
  modelgroup adexgrp1_Iinj1( Iinj ); //just inject?

  
  modelgroup adexgrp2( adex );
  //modelgroup adexgrp1_Iinj1( Iinj );

  modelgroup conn1_2( spksyn );

  modelgroup conn1_2sched( spk_sched );
  
  //Do I want to do this, make them all independent? or access them from inside the guy after?
  //How do I refer to the gAMPA of conngrp1 after the fact? does it have some name? Can I get it from outside? I guess so.
  
  
  
}






void adex_thing()
{
  MODEL adex;
  //3d position? etc.
  adex.add_intern_var( "V" );
  adex.add_intern_var( "W" );
  adex.add_intern_var( "Cm" );
  adex.add_intern_var( "Vpeak" );
  adex.add_intern_var( "Vreset" );
  adex.add_intern_var( "Wa" );
  adex.add_intern_var( "Wb" );
  adex.add_intern_var( "tspk" );

  //define variable "holes"?
  adex.add_hole( "conductance" );
  adex.add_hole( "current" );

  MODEL gLeak;
  gLeak.add_intern_var( "g" );
  gLeak.add_intern_var( "E" );

  //Add them together to adex model, fine.
  //Can add gLeak now bc it doesn't have any other guys like, writing to it.
  //However, for e.g. gAMPA, it needs to "read" from a bunch of places. Which can be added later I guess... if it's only one gAMPA. Might be more
  //than 1. User can tell us haha.
}


void make_circuit()
{
  //Logical  models are "generated" into groups.
  logicalmodel lm;
  
  lm.add( adex, "adex1" );
  lm.add_to_model( "adex1", "conductance", gLeak, "gL1" );
  lm.add_to_model( "adex1", "conductance", gAMPA, "gAMPA1" ); 
  lm.add_to_model( "adex1", "conductance", gNMDA, "gNMDA1" ); 
  lm.add_to_model( "adex1", "current", Iinj, "Iinj1" );

  lm.add( adex, "adex2" );



  lm.add( spksyn, "syn2-1" );
  lm.add_existing_model( "syn2-1", "postsyn_receptors", gAMPA, "adex1->gAMPA1" ); //REV: Is that the name IN the model or what? Maybe?
  lm.add_existing_model( "syn2-1", "postsyn_receptors", gNMDA, "adex1->gNMDA1" );
  lm.add_existing_model( "syn2-1", "presyn_firetime", adex, "adex1" );
  
  lm.add( spksyn, "syn1-1" );
  lm.add_existing_model( "syn1-1", "postsyn_receptors", gAMPA, "adex1->gAMPA1" );
  lm.add_existing_model( "syn1-1", "postsyn_receptors", gNMDA, "adex1->gNMDA1" );
  lm.add_existing_model( "syn1-1", "presyn_firetime", adex, "adex1" );


  //SPKSYN are *EVENT* models (update only when a certain variable happens). We need to specify that variable.
  //All others are *NORMAL* models (update every dt)
    
  //Now I literally make all the logical models out of those...
  //Some of them automatically make variables?
}
