

#include <symmodel.h>


//REV: Great, now the reason for this is that I need to reference "connection". I want to generate whole "synapses" array type thing,
//which requires me making presyn and postsyn at same time?
//As I had the idea before, basically now, for every "model to model" (i.e. top level), I have a correspondence? Hm, what if I have self-to-self thing?
//Only top-level have "sizes"...
//what about stuff like "axons"...meh.
//Those would be in each guy, but it would be a hole
//and then that would be separate model, which would have arbitrary size...just like syns type thing.
//At any rate, I can generate model sizes based on correspondences.
//Easiest way to do it is e.g..
//I generate syn2-1. I do it based on co-generation of syn2-1->adex2 and syn2-1->adex1. They will be same size.
//All models have interface to each other guy. I must specify a variable to generate though. All models have a "correspondence"
//for each other model they are connected to.


//Note all (base-level) model have "index" or "indices" variable


//For models that are "larger", it tells which of the smaller model I am connected to. E.g. I am 23, but I am #2 of smaller model. That's what I "point" to.
//Thus, it is literally just IDX of guy that I "connect" to. There may be many in this array that are #2.



//For models that are "smaller", it tells which members of the larger model are "mine"? There must be more than one, so I literally would have to search through all, to find
//those that correspond to me. Easier to have a sub-section of a second array, which tells the start point and number, and then that array contains indices into it.
//So, literally, large-to-small contains one-to-one (of large size).
//               small-to-large contains start/size index (of small size), which indicates start idx and size of section containing the connection indices of mine.
//best idea: contain both? Select based on size? Nah.
//If it's just a re-ordering, we have to do it in both directions?
//How about, make it always same. In other words, in all cases, they have a startpr and endptr, which tells the one I point to. For larger ones, it will always just be one
//size (hah). For smaller, might be larger.


//SO, all guys have such refs. And they are all of same form.
//Specifically, they are referenced by model->model, which gets the correct guy (for real)
//All are of form "correspondence" and from X->Y.
//They all have:
//1) array mystartidx;
//2) array mynumidxs;
//3) array corresponding_elements;

//These are first-level citizens, and can be accessed by GENERATORS (not by update equations I assume).

//However, for update equations, eventually I need to tell how to handle these.
//best/easiest way is that it will literally access all variables VIA the correspondence.
//In other way, I say, (for all X) read blah and sum. I can directly access, but it will error if it is not the same model.
//If it is different model, it will go to the indices. I can always call single result on it, but it will error if size is not zero (i.e. if I am many-to-one or one-to-many).


//Eventually we want to sort everything so they are sorted together, and offset by warpsize or something, but do that later ;)

//So, easiest way is to make index, which contains MY index if that guy corresponds to me.
//In update function, I will literally (?) just do a FORALL type loop of those guys (which correspond to me).
//Of course, it is better/easier to have a "list" of the guys, rather than searching for them.
//So, it will have a list of the indices of the guys that correspond to me. It's not a hole with multiple components, but something like that.
//If we assume they're all together (not always true), we could do start/end.
//Instead, we literally have a list of 

//At any rate, I will specify between two possibly submodels/holes, and it will find the "global" (highest level) between them, and *that* is the *real* "model" that it is
//addressing (real variable?)


//Note, when I construct something, I will specify root symmodel, e.g. sc1 in this case.

std::shared_ptr<symmodel> test_build()
{
  auto pos3d = symmodel::Create("pos3d", "3dposition|location|um" );
  pos3d->addvar( "x", "xdimension|um" );
  pos3d->addvar( "y", "ydimension|um" );
  pos3d->addvar( "z", "zdimension|um" );

  
  auto gAMPA = symmodel::Create("gAMPA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
  gAMPA->addvar( "E", "reversal-potential|mV" );
  gAMPA->addvar( "g", "conductance|nS" );
  gAMPA->addvar( "tau1", "exp-rise-time-constant|ms" );
  gAMPA->addvar( "tau2", "exp-decay-time-constant|ms" );
  gAMPA->addvar( "affinity", "Glu-affinity|transmitter-affinity" );
  gAMPA->addhole( "membrane" );
  gAMPA->addhole( "presyn" );
  
  gAMPA->add_to_updatefunct( "SET(g, SUM(g, SUMFORALL(presyn, MULT(Glu_syn/hitweight, Glu_syn/postsyn-gAMPA/affinity) ) ) )" );
  gAMPA->add_to_updatefunct( "SET(g, MULT(g, EXP( NEGATE( tau1 ) ) ) )" );
  
  auto gNMDA = symmodel::Create("gNMDA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
  gNMDA->addvar( "E", "reversal-potential|mV" );
  gNMDA->addvar( "g", "conductance|nS" );
  gNMDA->addvar( "g2", "ungated-conductance|nS" );
  gNMDA->addvar( "tau1", "exp-rise-time-constant|ms" );
  gNMDA->addvar( "tau2", "exp-decay-time-constant|ms" );
  gNMDA->addvar( "affinity", "Glu-affinity|transmitter-affinity" );
  gNMDA->addhole( "membrane" );
  gNMDA->addhole( "presyn" );
  

  //gNMDA->add_to_updatefunct( "SET(g, SUM(g, SUMFORALL(presyn, SUMFORALL(MULT(Glu_syn/hitweight, Glu_syn/postsyn-gNMDA/affinity) ) ) ) )" );
  gNMDA->add_to_updatefunct( "SET(g, SUM(g, SUMFORALL(presyn, SUMFORALL(MULT(Glu_syn/hitweight, Glu_syn/postsyn-gNMDA/affinity) ) ) ) )" );
  gNMDA->add_to_updatefunct( "SET(g, MULT(g, EXP( NEGATE( tau1 ) ) ) )" );
  
  
  
  auto gLeak = symmodel::Create("gLeak", "conductance");
  gLeak->addvar( "E", "reversal-potential|mV" );
  gLeak->addvar( "g", "conductance|nS" );
  gLeak->addhole( "membrane" );

  
  auto adex = symmodel::Create("adex", "spiking|neuron");
  adex->addvar( "V", "membrane-potential|mV" ); //membrane potential
  adex->addvar( "W", "recovery-potential|mV" ); //recovery potential
  adex->addvar( "tspk", "spike-time|time|ms" ); //spiketime, time,

  adex->addhole( "currents" );
  adex->addhole( "conductances" ); //Could separate these into synapses etc? E.g. presyn and postsyn? Inhib/excit. etc.
  //adex->addhole( "postsyn" );
  //adex->addhole( "presyn" );
  
  adex->addmodel( gLeak, "gL" );
  adex->addmodel( pos3d, "position"  );



  //REV: need to look in "my" holes first for "membrane"?
  adex->add_to_updatefunct( "SET(V, DIFF( SUM( V, SUMFORALL(currents, I), SUMFORALL( conductances, MULT(g, DIFF( membrane/V, E ) ) ) ), W ) )" );
  
    
  auto Iinj = symmodel::Create( "Iinj", "current" );
  Iinj->addvar( "I", "current|uA" );
  
  

  
  auto spksyn = symmodel::Create( "spksyn", "spksyn");
  spksyn->addvar( "delay", "delay|ms" );

  
  auto syn = symmodel::Create( "syn", "synapse");
  syn->addvar( "weight", "synaptic-efficacy|nS" );
  syn->addvar( "hitweight", "spike-efficacy|nS" );
  
  auto Glu_syn = syn->makederived( "Glu_syn", "glutamatergic-synapse" );
  Glu_syn->addhole( "postsyn-gNMDA" );
  Glu_syn->addhole( "postsyn-gAMPA" );
  
  
  auto synapse = symmodel::Create( "synapse", "synapse" );
  synapse->addhole( "postsyn-neuron" );
  synapse->addhole( "presyn-neuron" );
  synapse->addmodel( spksyn, "spksyn" );
    
  
  auto Glu_synapse = synapse->makederived( "Glu_synapse", "glutamatergic-spiking-synapse" );
  Glu_synapse->addmodel( Glu_syn, "Glu_syn" );
  
  
  
  auto sc = symmodel::Create("sc", "circuit", "sc1");
  
  /////// ADEX1
  sc->addmodel( adex, "adex1" ); //Has no specific "local functions"
  
  sc->addmodel( gAMPA, "adex1/gAMPA1" );
  sc->addmodel( gNMDA, "adex1/gNMDA1" );

  sc->addmodel( Iinj, "adex1/Iinj1" );
  sc->fillhole( "adex1/currents", "adex1/Iinj1" );

  sc->fillhole( "adex1/conductances", "adex1/gL" );
  sc->fillhole( "adex1/conductances", "adex1/gAMPA1" );
  sc->fillhole( "adex1/conductances", "adex1/gNMDA1" );

  sc->fillhole( "adex1/gAMPA1/membrane", "adex1" );
  sc->fillhole( "adex1/gNMDA1/membrane", "adex1" );
  sc->fillhole( "adex1/gL/membrane", "adex1" );


  //////// ADEX2
  sc->addmodel( adex, "adex2" );
  sc->fillhole( "adex2/gL/membrane", "adex2" );


  
 
  //////// SYN2-1
  sc->addmodel( Glu_synapse, "syn2-1" ); //contains both spksyn, and Glu_syn
  
  sc->fillhole( "syn2-1/presyn-neuron", "adex2" );
  sc->fillhole( "syn2-1/postsyn-neuron", "adex1" );
  //sc->fillhole( "adex2/postsyn", "syn2-1" );
  //sc->fillhole( "adex1/presyn", "syn2-1" );

  //REV: If we don't explicitly fill this with presyn, it would find PRESYN in the place above, which would fuck everything up.
  sc->fillhole( "adex1/gAMPA1/presyn", "syn2-1"); //Does it know, in there to look in Glu_syn/hitweight? Shit...
  sc->fillhole( "adex1/gNMDA1/presyn", "syn2-1");

  sc->fillhole( "syn2-1/Glu_syn/postsyn-gAMPA", "adex1/gAMPA1" );
  sc->fillhole( "syn2-1/Glu_syn/postsyn-gNMDA", "adex1/gNMDA1" );
  
  
  
  //////// SYN1-1
  sc->addmodel( Glu_synapse, "syn1-1" ); //contains both spksyn, and Glu_syn
  
  sc->fillhole( "syn1-1/presyn-neuron", "adex1" );
  sc->fillhole( "syn1-1/postsyn-neuron", "adex1" );
  //sc->fillhole( "adex1/postsyn", "syn1-1" );
  //sc->fillhole( "adex1/presyn", "syn1-1" );
  sc->fillhole( "adex1/gAMPA1/presyn", "syn1-1"); //Does it know, in there to look in Glu_syn/hitweight?
  sc->fillhole( "adex1/gNMDA1/presyn", "syn1-1");

  sc->fillhole( "syn1-1/Glu_syn/postsyn-gAMPA", "adex1/gAMPA1" );
  sc->fillhole( "syn1-1/Glu_syn/postsyn-gNMDA", "adex1/gNMDA1" );
  
    
  sc->check_and_enumerate(0, false);
  sc->check_and_enumerate(0, true);
  fprintf(stdout, "Done checking/enumerating: SUCCESS\n");

  return sc;
  
}



int main()
{
  auto sc = test_build();


  //I will now specify generators for all SC variables.
  
}
