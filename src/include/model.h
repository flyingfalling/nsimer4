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

  gAMPA.add_hole( "gSyn" );
  //gAMPA.add_model( "gSyn",  spksyn ); //May get from multiple GRPS (models?) of synapse, implementations at least. And furthermore, from many INDIVIDUALS
  
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
  
  
  
  
  EVENT_MODEL spksyn;
  //For each guy, need to have a hit_gAMPA , and hit_gNMDA etc.!!!!
  spksyn.add_intern_var( "hitweight" );
  spksyn.add_intern_var( "weight" );
  spksyn.add_intern_var( "t_hit" ); //this is time of hit? Or time of spike?
  spksyn.add_intern_var( "delay" ); //this is time of hit? Or time of spike?
  
  spksyn.add_intern_var( "transmitter_type" ); //GABA or Glutamate, etc.? Do I need to know this? Choose all syns that have this as postsyn, and find
  //the ones that are of (transmitter) type, and add those to add to me. Etc.
  
  spksyn.add_hole( "postsyn_receptors" );

  //spksyn.add_model( "postsyn_receptors", gAMPA );
  //spksyn.add_model( "postsyn_receptors", gNMDA ); //automatically do this for all types that receive "glu". Every time, normalize for all of them add add that amount or whatever.

  spksyn.add_hole( "presyn_firetime" ); //name is like "tspk" or some shit.
  //spksyn.add_model( "presyn_firetime", adex ); //or spiker or some shit?

  spksyn.specify_event_adder( "presyn_firetime" ); //There is only one of these? It is outside? Update function specifies it. Based on that, it updates
  //every turn, to "schedule" events I gues.

  spksyn.specify_scheduled_var( "delay" );
  
  //Need to specify e.g. how to build it based on "delay", etc. Like, how many "holes", what is max number, what is min/max delay, etc.

  //spksyn.specify_event_trigger( "t_hit" );
  
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
  //lm.add_to_model( "adex1", "conductance", gAMPA, "gAMPA1" ); 
  //lm.add_to_model( "adex1", "conductance", gNMDA, "gNMDA1" ); 
  lm.add_to_model( "adex1", "current", Iinj, "Iinj1" );
  

  lm.add( adex, "adex2" );

    
  

  //Add to model, add to model hole, add to model hole (?!?!!), model type that I will forcefully add, name of forcefully added
  //This part is already "connecting" them, by specifying this model as the "glue" between them.
  //Why not do that? Much easier ;)
  //Here was old way (specify each)
  //lm.add_to_model( "syn2-1", "postsyn_receptors", "adex1->gNMDA1" );
  
  lm.add( spksyn, "syn2-1" );
  lm.add_to_model( "syn2-1", "postsyn_receptors", "adex1->conductance", gAMPA, "syn2-1_gAMPA" ); //REV: Is that the name IN the model or what? Maybe?
  lm.add_to_model( "syn2-1", "postsyn_receptors", "adex1->conductance", gNMDA, "syn2-1_gNMDA" );
  lm.add_existing_model( "syn2-1", "presyn_firetime", "adex2" );
  
  lm.add( spksyn, "syn1-1" );
  lm.add_to_model( "syn1-1", "postsyn_receptors", "adex1->conductance", gAMPA, "syn1-1_gAMPA" ); //REV: Is that the name IN the model or what? Maybe?
  lm.add_to_model( "syn1-1", "postsyn_receptors", "adex1->conductance", gNMDA, "syn1-1_gNMDA" );
  lm.add_existing_model( "syn1-1", "presyn_firetime", "adex1" );

  //OK, now I've specified:
  //1) Existence of adex grp adex1 and adex2. It has gLeak and gInj.
  //2) Existence of spksyn grp syn2-1. It uses a grp gAMPA added to adex1 as a "conductance" (which is a hole).
  //This is created in both groups, in postsyn as the conductance; and in the presyn (synapse) as the postsyn_receptors (which is a hole).
  //   Finally, it has adex2 fill the presyn_firetime.
  //3) Existence of spksyn grp syn1-1, with same, but now with presyn grp adex1. Furthermore, it creates new forced model to glue them together,
  //   in same way as syn2-1. Note in some cases, I can name them same thing (i.e. all AMPA writes to AMPA). If that is the case, it will detect it
  //   already exists, and do what? gAMPA model now has not only multiple items, but also multiple MODELS (grps) that it is receiving from. That should
  //   be fine as long as update function is written to handle it ;) And furthermore must be updated correctly...?

  
  
  //lm.add_existing_model( "adex1->gAMPA1", "gSyn", "syn2-1" ); 
  //lm.add_existing_model( "adex1->gNMDA1", "gSyn", "syn2-1" );
  
  //lm.add_existing_model( "adex1->gAMPA1", "gSyn", "syn1-1" );
  //lm.add_existing_model( "adex1->gNMDA1", "gSyn", "syn1-1" );

  //#########
  // ########   REV: I could have this model forcefully add a model type to the other guy of the "conductance" type. Ah, that works nicest!!!!


  //Is this correct (i.e. specify from postsyn receptor, to add a "guy" to me?) How do I know how much is "released". They have to compete to get
  //transmitter, then I need to "distributed" it? I need to have "access" to each one? It should be the "synapse" who writes to gNMDA and gAMPA for me..
  //Should I specify the connection from "both sides"??? Hitweight is what feeds "gSyn" in the update thing? Furthermore,
  //This is way better, in this way I know that syn2-1 has postsyn receptors are AMPA1, and furthermore AMPA1 has pre model that is spiking synapse...
  
 

  //Idea is that all models have "holes", or "transducer" type things, that can have one or more things....and our job is just to hook up those
  //things correctly...
  //but, the most natural way when making a CONN is to specify pre and post from the CONN. So, for example where I added existing model "adex!" as
  //the postsyn receptors, I know that I need to make some way to process "hitweight" from "postsyn receptors". Do I need to go through and check for
  //"both way" connections? For example, there are 2 "incoming" groups to gAMPA1. gAMPA1 will handle that though. I guess. It will iterate through ;)

  //Through the start and end of each. It will handle them equivalently, accessing the parts that it wants to. gAMPA in this case has some other
  //parameter, which is like "NMDA-AMPA relationship". Which is kind of like uh, strength of NMDA guys, relative amount of conductance they get etc.
  //This could be per-guy, or not. I could add those parts as part of the connection model, and then add to the other guy "together". Problem is where
  //the N-to-1 hits. Do it this way for now I guess...

  //IF I were to basically "add" those bits to it, and say, yea, postsyn receptors are AMPA1 and NMDA1 over there, or, I could include the postsyn
  //receptors HERE (are they a model?), and create them here? No need...
  
}
