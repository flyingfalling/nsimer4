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

  adex.add_intern_model( pos3d, "position3d", "pos" );
  adex.add_intern_model( gLeak, "conductance" "gL" );
  
  MODEL pos3d; //x,y,z etc.
  pos3d.add_intern_var( "x" );
  pos3d.add_intern_var( "y" );
  pos3d.add_intern_var( "z" );
  
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
  //lm.add_to_model( "adex1", "conductance", gLeak, "gL1" );
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

  lm.add_symbolic( "neurons" );
  lm.add_to_symbolic( "neurons", "adex1" );  //one is excitatory, other's not.
  lm.add_to_symbolic( "neurons", "adex2" );

  //OK, now I've specified:
  //1) Existence of adex grp adex1 and adex2. It has gLeak and gInj.
  //2) Existence of spksyn grp syn2-1. It uses a grp gAMPA added to adex1 as a "conductance" (which is a hole).
  //This is created in both groups, in postsyn as the conductance; and in the presyn (synapse) as the postsyn_receptors (which is a hole).
  //   Finally, it has adex2 fill the presyn_firetime.
  //3) Existence of spksyn grp syn1-1, with same, but now with presyn grp adex1. Furthermore, it creates new forced model to glue them together,
  //   in same way as syn2-1. Note in some cases, I can name them same thing (i.e. all AMPA writes to AMPA). If that is the case, it will detect it
  //   already exists, and do what? gAMPA model now has not only multiple items, but also multiple MODELS (grps) that it is receiving from. That should
  //   be fine as long as update function is written to handle it ;) And furthermore must be updated correctly...?


  //Then, after I have specified all this, I need to generate the groups, and the parameters.
  //The groups and parameters can be co-generated, using "generators".
  //Generators can be quite complex.
  //Group size can be drawn from either a global distribution, or they can be local.
  //For example, I can manually specify size, etc.
  //Just offer many distributions.
  //Local e.g. connectivity functions are nasty.

  //I need to specify the "generator" for # of members etc. in each group.
  //Can some groups have iterms that are not exactly N size? Crap....

  //Every "model" has a "natural" implementation of a single "element" as a struct. This can (is?) compiled into array-format.

  //================ REV: MAKING/PLACING NEURONS ===============//
  
  size_t width, height, depth;
  //post3d is an actual implementation of it with variables etc. SET. This way it knows it is of the correct type...( need to check that).
  //It copies those into optimize array-first shape for GPU computation later.
  double edgesize;
  std::vector<pos3d> grid = make_grid( width, height, depth, edgesize ); //This will tile it.
  //Note: shit, we don't know how many there are of each grp beforehand, since they will be shuffled here. So we need to separate it into the 2 grps
  //from which each grp will be generated. In other words, this is "above" and "before" grp construction.
  
  double xdensity1=0.05, ydensity1=0.05, zdensity1=0.05;
  double xdensity2=0.05, ydensity2=0.05, zdensity2=0.05;
  density1dfunct xfunct1 = uniform_funct( -100, 100, xdensity1 ); //Need to tell "density" as well? E.g. are same number in -50 to +50 as there are in -100 to +100?
  //If we want the same total number but over a larger region, it needs to be lower density of course.
  density1dfunct yfunct1 = uniform_funct( 0, 0,  ydensity1 );
  density1dfunct zfunct1 = uniform_funct( -100, 100, zdensity1 );

  density1dfunct xfunct2 = uniform_funct( -100, 100, xdensity2 );
  density1dfunct yfunct2 = uniform_funct( 0, 0, ydensity2 ); //in some cases, it could depend on the y position...by some scaling or non-linear function etc. These
  //are pre programmed and user can select them by name.
  density1dfunct zfunct2 = uniform_funct( -100, 100, zdensity2 );
  
  density3dfunct grp1funct( xfunct1, yfunct1, zfunct1 ); //some function that can access other variables? Tells probability of a neuron being at this position, given the X Y Z position. It tries to access other variables maybe? For example, relative to some "input". In this case, it is only relative to independently, where in Y we are.
  //Leave it up to user to ensure correct overall scaling of probabilities. Compute it directly from density.
  density3dfunct grp2funct( xfunct2, yfunct2, zfunct2 );

  shuffled_generator sg;
  sg.add_grp( "adex1", grp1funct );
  sg.add_grp( "adex2", grp2funct );
  sg.distribute( grid ); //will have an error if all probs are zero at this point?

  implmodel im( lm );
  //Now I want to distribute grid among it.
  im.implement( "neurons->pos", sg); //this will find corresopnding one of each group and do it.
  //ROFL, these are already all manaully set, so no point.
  //How to do the rest of them.

  //REV: I wanted to set the size of grp and everything manually, which helps. So, somehow size of "neurons" is now set! I can access it.
  gaussgen adex1Vresetgen( -65.0, 1.0 );
  gaussgen gLgen( 1.0, 0.1 );
  gaussgen ELgen( -65.0, 1.0 );
  im.draw_parameter_values( "adex1->Vreset",  adex1Vresetgen ); //does it this way, easiest. We know size from blah.
  im.draw_parameter_values( "adex1->gL->g",  gLgen ); //does it this way, easiest. We know size from blah.
  im.draw_parameter_values( "adex1->gL->E",  ELgen ); //does it this way, easiest. We know size from blah.

  //This "draws" it, how about resetting to some initialized values too (i.e. we need to "remember" init values. Whatever.

  //=============== MAKING CONNECTION GRPS ===================//
  double sxdensity21=0.05, sydensity21=0.05, szdensity21=0.05; //number of synapses per unit area. (uniform in hyperblock)
  double sxdensity11=0.05, sydensity11=0.05, szdensity11=0.05; //number of synapses per unit area. (uniform in hyperblock)
  double sxdensityALL=0.05, sydensityALL=0.05, szdensityALL=0.05; //number of synapses per unit area. (uniform in hyperblock)
  //This times size of area should yield number of synapses. It's much easier to "draw" synapses this way... We know total number of synapses over
  //all, but not which group they're from(!). So we can do a "synapses" group type thing...and draw them all I guess? But then it's not space they're
  //drawn from, just normalized/uniform density of all synapses of whichever type.
  density1dfunct sxfunct21 = uniform_funct( -100, 100, sxdensity21 ); //Need to tell "density" as well? E.g. are same number in -50 to +50 as there are in -100 to +100?
  //If we want the same total number but over a larger region, it needs to be lower density of course.
  density1dfunct syfunct21 = uniform_funct( 0, 0,  sydensity21 );
  density1dfunct szfunct21 = uniform_funct( -100, 100, szdensity21 );

  density1dfunct sxfunct11 = uniform_funct( -100, 100, sxdensity11 ); //Need to tell "density" as well? E.g. are same number in -50 to +50 as there are in -100 to +100?
  //If we want the same total number but over a larger region, it needs to be lower density of course.
  density1dfunct syfunct11 = uniform_funct( 0, 0,  sydensity11 );
  density1dfunct szfunct11 = uniform_funct( -100, 100, szdensity11 );

  density1dfunct sxfunctALL = uniform_funct( -100, 100, sxdensityALL );
  density1dfunct syfunctALL = uniform_funct( 0, 0, sydensityALL );
  density1dfunct szfunctALL = uniform_funct( -100, 100, szdensityALL );
  //Randomly distribute them, not on grid, and without caring about x/y/z pos...

  //std::vector<> ;//what is this a collection of? It's the collection of pre/post basically hahahaha. I.e. which ones from each "group" are selected.
  //This is weird, what if I want to select pre/post neuron based on x/y/z pos?
  //So, what is this doing? Distributing the total density to the groups as I wish. And their relative "positions"? I.e. upper neurons more likely
  //to have connections, but I don't care. Why not co-generate. So, I now also actually generate for pre/post group! Based on being selected from this
  //group, and possibly based on something else such as "x/y" position of pre/post. Like, drawing from a multinomial, but with weights based on the
  //(post) synaptic neuron's x/y/z pos? Yea, that's what I'm doing anyway. For each neuron I make a postsynaptic distribution, and draw some number
  //of synapses from that (how many are allocated for me? Must be related to my integral compared to total my density). In this case I specify the
  //pre/post CORRESPONDENCE directly, but that's a dirty word (to refer to the correspondence). But, all model-model interfaces must have a corresp
  //that defines how they are connected. So, what are we defining? First, we choose the presyn, and then based on presyn, we choose the postsyn?
  //Hm, I don't think that will work. We need to "co-choose" them?

  //I like the "choose pre, then choose post". So, first, we choose (randomly) the pre from some function. Then, we choose post from some function,
  //which may depend on the chosen pre. This is great and fine, but it requires us to "know" about the implicit "pre" and "post" correspondences that
  //exist for every var-var correspondence. It's easiest to simply force every single variable to have a correspondence.

  //So, we are doing something like selecting a correspondence! Obviously we need a way to distribute that. We already have the "abstract" model.
  //Do we have a pointer to a "pre" model and "post" model? I guess so...what we are doing is selecting that. Problem is, it is an integer...the
  //index. So, selecting it as a "variable" per se might not be the best approach since we assume all variables are real numbers...
  //But, we could assign distributions? O variables? My goal was to make it so that such things were totally invisible to the user? I guess I could do
  //that...but then how would user specify it? And more importantly how to specify it efficiently? I kind of want to do it in parallel. Which is
  //possible only if I know the number beforehand. And each guy, it chooses the pre, then it chooses the post. Fine. The other guy, it did them all in
  //parallel. So, the other guy could do them all in parallel too. Problem is for choosing positions, we don't know if they're "taken" or not.
  //OK great, so for synaptic density, we know the # of synapses, and we just need to choose pre and post. So, first we choose pre (for each in parallel),
  //from a uniform, or evenly distributed in some natural way. Then, for each neuron, we compute the probability for each post as a distribution
  //based on all other neurons' probability (distance). Probability of the synapse going to ONE of them is 1.0, so I just scale it. Do cumsum etc. Then
  //drawn N from that. Either way same amount of work, possible postsyns are always the same...though I'm doing the same work for each synapse with
  //same presynaptic ugh...better to choose a number for each neuron (from the total? or something?) around the mean, and then pre-compute only for
  //each cell separately...

  //No, just have it access the same matrix each time separately, great.

  //Still need to actually "do" it. If all I have is an abstract model, connections between them, and ways to distribute individual parameters/variables,
  //that is fine. but it only distributes variables that are "explicit". For connections I need to effectively distribute what i had considered an
  //"implicit" variable, i.e. one that is an implementation detail. Knowing the "index" of pre/post model is bad, I need a way of referring to it
  //selecting them. Not the variable, but the target. So, make that a possiblity, make some function to do it, and the function will know that it is
  //selecting discrete distributions. Good. So, example:
  //first I need to make the "number" of synapses hard coded beforehand!!!
  //2 things, GIVEN that I select a pre, what is prob of post?
  //and prob of selecting PRE.
  //The total density, and relative density, above should tell what is happening? Better to select for each group separately from pre-computed
  //sizes...? Or, should I select pre/post in real time as I go? I could do that too, but really quite inefficient, and then I have to manually
  //express that I have drawn "pre" and "post".

  //E.g. presynaptic connections are more likely from more "upper" neurons...
  //How could I literally manually split the neurons into Nsyn/Npreneurons? I would have to actually refer to "number" of neurons, size I do want a
  //"size" reference after all...
  //  correspondence_distribution synselector21( "adex2", uniform_selector( "adex2", density3dfunct(sxfunct21, syfunct21, szfunct21) ),
  //					     "adex1", distance_selector( "adex1" ) );
  //Does it *always* go from arg1->arg2? What if arg1 depends on arg2? That's fine too. Dependency? I.e. can I select "post" first? Can I co-select them?
  //separate them out first.

  //Selects pre neurons...
  //correspondence_distribution synselector21pre( "adex2", uniform_selector( "adex2->pos", density3dfunct(sxfunct21, syfunct21, szfunct21) ) );
  //First I generate the "number" of synapses, problem is I'm only setting "correspondece" now.
  double integral = integrated_density; //integrated density of synapses of this type over the space...will depend on min/max of area of course, fuck.
  //I can approximate it numerically, by computing and dividing by the space. Will take forever though...
  //This will determine, for each synapse, WHERE it came from. Problem is I need to know # of synapses lol...assuming I know the integral,
  //I can always compute that raw, if the densities represent actual probabilities.
  //Symbolic integration is like, sometimes impossible. I need to know scale in which to compute the integral for approximation purposes...
  //If user will do it. Haha force user to integrate his own function (i.e. provide # of synapses)
  //Give a way to do that if he is too lazy (by literally integrating on grid, adaptively, or non-adaptively with some h). Integral will be h/space.
  std::vector< correspondence_t > corrs = generate( density3dfunct(sxfunct21, syfunct21, szfunct21), integral );
  //Now I need to distribute these to actual presyns ;) All of these need to "grab" a value from neurons.

  //REV: WHAT THE FUCK TO DO.
  //DEFINE PRE? DEFINE POST? MIGHT BE CO-GENERATED? I CANT IMAGINE BUT POSSIBLE? FUCK IT?! INDEPENDENT?
  //HOW TO GET #? FROM INTEGRAL? OK. NOW I LITERALLY HAVE A VECTOR OF IT. I DONT LIKE THAT. I DONT WANT THAT.
  //HOW TO RE-DO NEURAL GEN? THERES SOME SET, AND ALL NEURONS ARE GRABBED FROM EITHER OR. IN OTHER WORDS, ITS CORR INTO THAT ARRAY, FUCK.
  //NEED TO KNOW HOW MANY OF EACH THERE ARE. NOT POSSIBLE IN PARALLEL.

  //I refer to each neuron, not the index. Great. User might want to refer to the index though. Give him that.
  //Like, each one has 1/10 of total or some shit. Need to know size then. Co-generate size and other variables, fuck the world.
  //Co-generate multiple groups, pain in the ass. In that case, generate size first. Do I already know size? Drawing params always requires size first.
  //But why, why not allow user to generate size adaptively. E.g. based on a target group or some shit. E.g. for each neuron, make 40 synapses. How to say
  //"for each neuron"? Not possible? Implicit knowledge of what a neuron is, that they are "one" neuron, etc., counting.

  //FUCK, if I have some sub-items kn for each n in N, we have a problem, because group has N items, but overall there are sum(kn) components, for each N,
  //etc. Which is a problem, because what does a correspondence point to? Like, I want to point to the nth guy, but I want to know that this is neuron N,
  //which has someparts. So are those models, holes? If they are holes, what? We iterate through them implicitly. Fine, how do we know to point to the
  //Nth one though/ How can we know which guy it is? Can things overlap those borders? It's a pointer to those guys inside it. No way to discover which
  //N my guys is a part of? FUck the world.


  //Write out state. Deconstruct.
  //I have nothing. I might not even have neural sets yet. What if i want to construct neuronsf rom synapses? Possible? Lewlw.
  //Synapses from neurons, OK. I could make them have a size beforehand. Then hook them up. Fine. But whatever. Still need to generate them.
  //In this case, based on neurons.
  //What is a neuron? only a pos right now? Lol...force groups to be constructed through iterators. No explicit presyn definition.
  //Refer to items, not numbers. As a set. Presynaptic density is a distribution too. Fine. So I select from presynaptic guys, of that, based on density.
  //And from that, I generate the correspondence.

  //Integral defines "scale" of drawing.
  //NO, based on the model it "knows" what the connection is! fuck the world.
  //It knows the logical connection, but we're generating based on possibly other parameters (we could manually specify them).
  //In this case it is literally generating the correspondence between them also. Where is it stored? Does every group have a correspondence to
  //every other fucking group? Always? If there is a connection. Here I am generating, but possibly not connecting. WHy can (t I co-do it.
  //Define CO GENERATE type function? Generate CORRESPONDENCE. I.e. define the fucking corresopndence as I generate. Great.
  generate_correspondence( generate_from_density( "syn2-1", "adex2", density3dfunct( sxfunct21, syfunct21, szfunct21, integral) ) );

  //Or some shit. Great, just let it be haha.

  sampler sss( "syn2-1",  dist(pre, post ) ); //samples based on the function of distance.
  //Bases on the distance to "pre". What the fuck is pre, NO one fucking knows. It's POS of INDEX of 
  
  
  //Requires xyz pos of pre,e tc. Asusming it knows what fucking pre is, etc. who the fuck defines this shit.
  generate_correspondence( generate_from_density( "syn2-1", "adex1", draw_from_distance( ) ) );
  
  //This generates individual syn21, based on adex, and density3d funct. So it generates "items", and how they correspond. This literally generates just
  //a correspondence to individual items of adex1. But, how many to generate? That depends on size of adex1, and integral? I know integral is total number
  //of synapses (or density per unit space?). Total number. How about if I literally just want to randomly draw some number per guy... Will be generated
  //from the density at that "region", the density funct which depends on neruron number or position or some shit. For example in this case I check how
  //many postsyn I should create for each fucking adex1 neuron I go through, based on adex1 neuron's xyz.

  //REV: what if I want to distribute otherwise, like distribute some number among adex1 neurons fucking neruons hahaha.
  //In that case, I define number, and define a way to draw random adex1 for each synapse. Same fucking thing, in one number is told beforehand.
  //Problem is what about like, multi-group type shit. I need to know numbers to separate them. Can't grow tihout fucking bound.
  //generate_from_density( "syn2-1", "adex1",

  //Might want to keep around a "global" count of number generated or somes hit. ugh.
  //OK, this just generates the number. It generates specifically the correspondence. How do I know to generate the correspondence? This just generates
  //the items. Might as well tell it which presyn guy there is too. In other words, I'm generating the "group", but a specific element of the group.
  //Specifically, the presyn_blah. Not the value, THE FUCKING INDEX OF IT I.E> WHICH OF THOSE FUCKING ARRAY FUCKING ITEMS TO FUCKING POINT TO.


  //REV: NO, corresponding way to do it would be to select PRE neurons
  //of correct size from PRE, and then "distribute" those to syngrp.
  //Will distribute "these" (generating them) among the targets, one each.
  //note they will overlap in this case.
  generator g;
  g.add_grp( "syn2-1", predensityfunct );
  g.distribute( "adex1" ); //This will try to distribute adex1 "targets"
  //for each syn2-1 item. No, I provide the "grid" first. Grid is syns.
  //So, need to generate syns. Then I distribute the presyn neuron idxs
  //based on that. This is too complex for the user rofl. If I have to think about it, I can make mistakes rofl.

  //REV: Look, I'm generating connections. I *know* they're between groups. They need to refer to the presyn item and postsyn item. That's all they
  //refer to. Not the index, the *item*.
  
  
  //Density describes a FUNCTION. This function
  //will be based on the integral basically, which is a super pain in the ass lol... unless the integral is constant. I *must* provide the integral
  //or risk having arbitrary non-integratable functions.
  
  correspondence_distribution synselector21pre( "adex2", "presyn_firetime" );

  correspondence_distribution synselector21post( "adex1", distance_selector( "adex2->pos", "adex1->pos", distparams() ) );

  correspondence_distribution synselector21( synselector21pre, synselector21post );
  
  //How does it know to select A first, then connect to B? One of them will be conditional other not. Dependencies. Fuck.
  im.implement( "syn2-1", synselector21 ); //This will only generate size (i.e. pre/post correspondences). User knows nothing about that.
  //But, he should...fuck.
  
  //REV: Just let user do size_t finding!
  //Have a special variable for every "interface" variable, which is the corresponedence... Name it specially? Like "set correspondence" type things.
  //Great. SEt the corresopndence based on e.g. pre x, pre y, etc. And post correspondence might depend on pre correspodnence, or vice versa.
  //Good...I like that.

  //For e.g. normally distributing as I tlaked about, that's fine.
  //But what about for e.g. dividing normally. I need to know "size" of synapses, etc. I can do that if it is distributed. problem is that until now,
  //"size" is only distributed on a per-variable basis. There is no group "size".
  //So, I obviously need to make one. For correspondence purposes.
  //Do correspondences one at a time? Or in parallel? Or something. meh, go.
  
  //I need to think about how "correspondence" will work. These two functions can reference each other I guess...
  
  
  
    
  //implementer im;

  //generator nposgen;

  //Basically the problem is this.
  //For every variable, I want to specify a function to generate it. This function must be able to depend on the variable values of ANY OTHER
  //variable in the whole simulation.
  //Furthermore, to "start it out", I want to be able to separately specify some skeleton on which to grow.
  //For example, to determine the number of elements. Neurons are specified, synapses may not be initially.
  //It should be possible to "co-generate" both parameters and elements. Like, probability of there being certain parameters with certain values.
  //I want to basically say, like setparam( "blah", paramgen( PARAMS ) ).
  //And "blah" could be multiple guys from multiple groups, In which case, it must have a way of specifically accessing the guys of each group by a
  //local variable, which is shit difficult. Or is it that difficult.
  //At any rate, at some point we need to actually generate a group from a non-relative distribution. For example, determining # of members.
  //In this case, we provide a function that is not reliant on external things. A "grid function". And, this will generate members, and choose
  //their positions. Which, in the end, will be a vector/vectors. Is there a growing vector size? For connections, I may literally want to
  //grow the number of elements. If this is done in parallel, that is a problem ;) But, I don't think there is any other ways to do it ;)
  //Than in serial. of course, we could do each one in serial, but no. Each neuron's postsyn in parallel? But, in that case um, prob of connection
  //might be 2-way lololol. I'd still need to "grow" for each neuron...it's generation of e.g. connections which is expensive. If I pre-specify the
  //number, it is very easy to do. I could just leave holes, but that's a waste of memory/space. Does actual number matter? Probably not so much,
  //when it is very large. Set number, and distribute them among the neurons. That way I can do it in parallel. Or, pre-specify a MAXIMUM number,
  //and a now number, so I can grow later. Oohhh. What if user wants to "grow" a number? Nah, for now, just force user to always specify a (maximum)
  //size, not all of which may be used?
  

  //REV: So, how to do it. Specify size somehow, and from that it generates size. If there are any that are initialized, it uses that N size?
  //Force user to specify a size? I assume that's the only way. Models have some natural size of actual elements, or do they need to? Assume so. if they
  //rely on e.g. POSITIONS to tell number of elements, that works.
  

  //SO, we have the LOGICAL MODEL, which specifies actual connectivity of each piece. We assume all models must have same thing. Let us specify now
  //how to generate the number of elements in the model. All models will have this (unless it is connection in which case it is pre-specified).
  //All will have dependencies.
  //Could use a "generator" to (co-) specify model size.
  
  //First, generate a uniform grid into which the neurons will be "place". Grids can be generated based on several factors, including "relation" to other
  //groups that we pass as arguments? Do I want to define such things as "3d grid" or something? Position should be a base type I think...
  //Generate the impl group of N "positions" first? Or use them as a gaussian at each pos? Or grow them in some way...?
  //Easier to generate as well go... Could draw them based on some "property", such as Vthresh of particular neuron...,
  //i.e. correlation. Ugh, difficult. Here I just manually set the value...
  
  //REV: I want to use this for liquid topology construction too, so be careful...
  //Better to take (extract) mathematical form only and operate on those.
  //Everything is a vector (an impl group?). So, first I generate it from what?  I could have some function that e.g. reduces probability as
  //Y decreases between min and max. Do I want to arbitrarily define that function? To place the number of them. I could uniformly place them
  //also, but with drawing decreased across the domain of the function. What do I need, integral function?

  //REV: Need a function to sample from an arbitrary distribution. Use MCMC.
  //Need to know the integral from the function, that's the problem...
  //Just use rejection sampling
  //Crap, even to use metropolis algorithm (rejection), we need to know integral because we need to know it integrates to 1.0.
  //Just force user to provide an integral, OR let user specify that it will be discretized (numerically), and use cumsum to get it.
  //Cumulsum is easiest, but difficult for aribtary guys (e.g. long-tail). Um, also user needs to choose "value". Also, values will be
  //chosen in "chunks" so it won't represent the actual distribution in that.... I.e. value will be at the "edges" of the grid...

  //Whatever, forget it for now... just have user pass in the function that does the drawing in each dimension (or in all dimensions). That makes
  //it much easier. If dimensions are correlated, that's fine, they will happen in serial.
  //The other possibliity is to "distribute" the actual "elements" to pre-defined "values". This is much easier... We can call this "shuffling".
  //Or "allocation". Problem is generating the pre-defined values in the first place hehe. They could be generated in real timem by some algo that
  //interacts with itself. I.e. does it generate the grid in real time and hide how it does it, or do I generate grid, and then place items in the grid.
  //Way too complex though, that's the problem.

  //REV: Just a choose a way to do it....pre-define distirbutions user can use/parameters (programatically), and just do those. Way easier.
  //Allow user to have functions for some dimensions that reference/correlate with other dimensions? This will limit e.g. the number of
  //dimensions that we can use...but user shouldn7t want more than 3 dimensions, so code up to 3 I guess... Or code 3, and of course others are
  //special cases of it. This works best. And for each, we basically pass the function of each? Which may reference the others. And of course,
  //a draw will draw one first, then do the other 2. Even if they are correlated...we can always choose one first right?
  //Do I want to/need to choose total number? Crap.

  //For example, we are drawing from N groups, but probability of it being of a given type will depend on the e.g. X, Y, Z location.
  //Do we know the dependency? There can not be a circular dependency in there anywhere. So, for every specific choice of point X,Y,Z, the
  //probability of drawing from one group or other other will be modulated.

  //Can we also co-draw from a gaussian? E.g. place items, and also determine the thing? Like, should we PLACE a neuron here (maybe)? Need to
  //draw in some way from the SUMS of all the group's functions at this point. That would require us basically constructing an overall PDF in the
  //3d space by combining those of each of the groups independently. Which is a pain in the ass... For example I define the X,Y,Z thing of each group,
  //which can interact. Furthermore, they can rely on OTHER groups, (either those being co-constructed, or already-construcdted groups, to e.g. form
  //the correct topology). This is nasty. What if it depends on co-constructed group, like we want to place inhib neurons around excit? Forget that,
  //too complex. Or if we want to place guys wehre there are few already placed or something along htose likes??? Like an interactive placing? Will
  //try to maximize it based on "maximal" distance from other guys. In other words, basically a "growing" system. Seems pretty complex.
  //But doing things like, referencing it to a certain target, this is needed. For determining connectivity, or for placing in the first place?
  //So it can depend on arbitrary variables etc.

  //Can we make "composite" groups, for example the 3d positions of the COMBINATION of 2 groups...yea I guess so....Figure it out when you need it,
  //not before!!!!!

  //How can user define how to place them in relation to another group? Have "holes", fine, but then user needs to define the function to "use"
  //that too. Basically give a way to do "transforms" of space or something. First, just make the 3d grid lol. Based on arbitrary guys...
  
  nposgen.type( "shuffled_grid3d" );
  //nposgen.setvar( "ndim", 3 );
  //nposgen.setvar( "dimvarnames", {"x", "y", "z"} ); //uses these variables
  nposgen.setvar( "dim1_var", "x" );
  nposgen.setvar( "dim2_var", "y" );
  nposgen.setvar( "dim3_var", "z" );
  
  nposgen.setvar( "dim1_min_max", {-100, 100} );
  nposgen.setvar( "dim2_min_max", {-100, 100} );
  nposgen.setvar( "dim3_min_max", {0, 0} );
  
  nposgen.togen( "adex1->pos" );
  nposgen.togen( "adex2->pos" ); //adding multiple groups to generate with same generator in this case...

  //Relative strength of each group? E.g. 0.4, 0.6 or something?
  nposgen.genfunct( "adex1->pos", "x", uniform_params( nposgen.getvar("dim1_min_max") ) ); //REv: this can literally take a "params" argument that it references at runtime, i.e. it is not simplified to the parameter beforehand...
  nposgen.genfunct( "adex1->pos", "y", uniform_params( nposgen.getvar("dim2_min_max") ) ); 
  nposgen.genfunct( "adex1->pos", "z", uniform_params( nposgen.getvar("dim3_min_max") ) ); 

  nposgen.genfunct( "adex2->pos", "x", uniform_params( nposgen.getvar("dim1_min_max") ) ); 
  nposgen.genfunct( "adex2->pos", "y", uniform_params( nposgen.getvar("dim2_min_max") ) ); 
  nposgen.genfunct( "adex2->pos", "z", uniform_params( nposgen.getvar("dim3_min_max") ) ); 

  //REV: Figure out how to generate groups later. First, assume the groups are made, and make some simple way to "manually" set their parameters?
  //This will make it much easier. OK, so for now, I will literaly just separately distribute them. But make shape so that I can specify after the fact.
  
  
  //nposgen.setvar( "dims_min_max", { {-100, 100}, {0, 0}, {-200, 200} } ); //assumes density is same on any dimension...
  //nposgen.setvar( "dims_min_max", { {-100, 100}, {0, 0}, {-200, 200} } ); //assumes density is same on any dimension...
  //Assume they are always on a "grid". it will compute min-max for each. So we have, ((max-min)/gridsize) + 1 for the number
  //of elements arranged on each dimension. How do we compute gridsize though. Force user to specify I guess... I need to get relative sizes of them.
  //Right now it's basically: 2 to 1 to 0 (?). First, assume smallest is "1". So, that is free. Might have none fit there. Start with one fitting on smallest. Of course, size 0 is too small, so we immediately go to the next one (?). How do we multiply grids.
  //We already do 1000 / 200. So that's 5. Then we do 5 / 100, that's 0.05. So, if we do 0.05*0.05, then that makes um:
  //2000 fit in a 100 space. So, that's not right lol.

  //Can I just solve the system of equations, N=xexpanse/L * yexpanse/L * zexpanse/L
  //N/(z/L) = x/L * y/L
  //N/(z/L) = x*1/L * y*1/L = x*y*(1/L)^2
  //N*(1/(z/L)) = x*y*(1/L)^2
  //N*(L/z) = x*y*(1/L)^2
  //N/z * L = x*y*(1/L)^2
  //1/z * N * L = x*y*(1/L)^2
  //x * y * (1/L)^2 / (N * L)

  //1/z =  x * y * L^-2 * L^-1 * N^-1
  //1/z = x*y*L^-3 * 1/N

  //N, x, y, z \in N
  //N = x*y*z
  //x*L <= xe
  //y*L <= ye
  //z*L <= ze
  //x,y,z >= 1
  //L = min(i) { (xe*ye*ze) - (x*i*y*i*z*i) }

  //Does one of the dimensions always have to "fit" exactly? In which case we can roughly compute the correct L, then fix that (?) and fill in the other
  //dims. Smallest dim has number equal to cube root of it or smthing?
  
  
  
  //adex1_gen.set_type( "uniform_grid" ); //by probability? or, size? Or, grid? I could grid them but why? 2d sheet, whatever heh.
  //Need to set "number" of variables and shit.
  //Add anonymous functions to specify density in X dim, Y dim, etc. Ugly.
  //What if I want interaction between generators, for example, I want a total of 1000 neurons, randomly drawn between 2 different groups. Or, I want to
  //randomly place neurons from multiple different groups. And generate them or some shit. Generating "size" before "blah"
  //Let's do the hardest case of co-generating them, in a space, out of possible spaces, until all spaces are met!?! So "shuffling" them.

  //All this has dependencies of course ;)


  
  
  //Needs to use random number gens, etc.
  //Let's imagine I don't care about the positions (though I should). Can a 3d pos be a more complex model. Yes, it is. It7s a 3d pos model and shit haha.
  
  //We haven't chosen which "side" owns the correspondences yet I guess... "ownership" means it must be "inside" the model, although other guys can have
  //pointers to it. Note that, always the larger "larger" group will have the correspondence, for both sides. Not larger, but rather, the variable grp.
  im.build_implementation( lm );
  
  
  
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




//REV: OK, time to get serious. I was figuring out how to place in a volume in 3D space using some parameters.
//More importantly, how do we specify "size", i.e. what do we generate from?

//We need to "generate". Some parts had an XYZ position, other's didn't. I was assuming a regular reality, in which all can be transformed to same coordinate space...fine.


//I had things worked out. So user specifies models, then specifies how to generate them separately.
//I.e. draw N neurons from X. need to separate the groups. Painful.

//Problem with axon retro, is we need to possibly transfer at time slices of less than DT. Which is the issue.

//Do I need to specify a total number, or what? If I route the axons and dendrites, I will know what is hit by the electrical field... I can mostly assume its axons going in.
//How do I specify a "node" that is activated though? I could have a single guy, and have it change from GABA to AMPA etc.? I.e. make positive to negative by not specifying
//whether it was GABA or AMPA...? But, it's postsyn response, so kind of weird...


//Trans-dimensinal is difficult. But, we could specify that they are 3 groups, and somehow let it vary between GABA and AMPA? I.e. don't specify GABA/AMPA, but rather let all
//types vary between negative and positive, and vary their postsyn responses...? Assume 2 types, but NMDA will have that gating thing that makes it complex...?

//Anyway, give myself a way to make models. We can name all presyn and postsyn the same thing (like all use a single transmitter, or two types, or something). Problem is we
//need to try/hypothesize different STRUCTURAL models from a possibly infinite model space...shit.

//REV: I need to decide how to code this...

//As is I build it in functions whicch add/blah to models. Fine. Question is how to compile that to CPP? Or CUDA? I.e. I basically give it C++? Or is there limited way?
//Yea, I think I specified it in some specific way...

//And I need to determine dependencies from models.

//Build "group" from "abstract model". And connect them up. Using synapse models. But stuff like "presyn" and "postsyn" are what? Integers. How was I handling that?
//They are correspondences? Everything is a correspondence? Hmmm...

//REV: I guess to make GPU, I need to be careful. Every X can be done as an item, but keeping same group within same block is ideal? Or do one group at a time? What if there
//aren't enough? if they just access differnet pointers, that works fine...

//Only serial things are basically where I update spike pointers. These must be stored per-block so that I can use synchthreads to ensure that all presyn/postsyn threads
//are done updating first? Oh...shit. If I call device functions, there is no guarantee that all blocks will be finished computing before the next device function is called.
//Thus, I need to ensure that all models are barriered before I go to the next group? Some groups don't have dependencies, but for example, I can't check whether a neuron has
//fired this turn until I am sure the neuron is finished updating...so those must be in same block at the very least.

//So...presyn neuron groups, must do that. Is there a way to synchronize the whole CUDA device within kernel calls? Or do I want to make recursive cuda calls?
//For example between time points, I assume that all guys are updated by the time next timepoint come. And that DT is only updated...not while other guys are going? Do all guys
//keep a copy of dt? Copied at beginning of turn? Basically use BLOCKS as mini-SMP. This is fine, but since there will at some point be cross-block communication, that must
//either happen between kernel calls... Or something. Or use kernel-inside kernels. That seems like the best bet.

//Note rand generation can be done in device functions, so that solves that problem...need to convert from user code though...
//I can get normal, uniform, lognormal, poisson...

//Reduction type things are fine. But, at any rate, seems using KDP will work best... just be careful about where I launch kernels from... I.e. only X=0 controls launches,
//although I could do memory transfers too I guess...?

//Assume that we do connections after neurons/elements. In that case, we effectively only need to launch 2 "kernels" per time step, which each may call many device functions.
//If they're independent, we are OK. If not, we have a problem...? Within each kernel launch, I have many device functions, which are called one at a time? by each thread,
//in parallel...knowing its start/end time...? In order. Fine. That forces me to compute "offset" for each guy or something though..? Nah, no need to synch within threads...
//Same update function, different data (params etc.?)?

//For shit like um, gap junctions, we need to keep V-1 around, which is the problem...if the other guy's Vt is updated and v-1 is erased before this guy reads it, it is fucked.\
//So, be careful about that...i.e. when Vt-1 is actually updated...

