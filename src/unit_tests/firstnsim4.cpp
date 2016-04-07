

#include <symmodel.h>

void test_build()
{
  auto pos3d = symmodel::Create("pos3d", "3dposition|location|um" );
  //symmodel pos3d("pos3d", "3dposition|location|um" );
  pos3d->addvar( "x", "xdimension|um" );
  pos3d->addvar( "y", "ydimension|um" );
  pos3d->addvar( "z", "zdimension|um" );

  //Needs to know what to "read from" to see if I increase?
  //symmodel gAMPA("gAMPA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
  auto gAMPA = symmodel::Create("gAMPA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
  gAMPA->addvar( "E", "reversal-potential|mV" );
  gAMPA->addvar( "g", "conductance|nS" );
  gAMPA->addvar( "tau1", "exp-rise-time-constant|ms" );
  gAMPA->addvar( "tau2", "exp-decay-time-constant|ms" );
  gAMPA->addvar( "affinity", "Glu-affinity|transmitter-affinity" );
  gAMPA->addhole( "membrane" );

  auto gNMDA = symmodel::Create("gNMDA", "conductance|GluR-mediated-conductance|synaptically-mediated-conductance" );
  gAMPA->addvar( "E", "reversal-potential|mV" );
  gAMPA->addvar( "g", "conductance|nS" );
  gAMPA->addvar( "g2", "ungated-conductance|nS" );
  gAMPA->addvar( "tau1", "exp-rise-time-constant|ms" );
  gAMPA->addvar( "tau2", "exp-decay-time-constant|ms" );
  gAMPA->addvar( "affinity", "Glu-affinity|transmitter-affinity" );
  gAMPA->addhole( "membrane" );

  auto gLeak = symmodel::Create("gLeak", "conductance");
  //I need to tell it that the V used in the update equation of gLeak is the V of adex!!!
  gLeak->addvar( "E", "reversal-potential|mV" );
  gLeak->addvar( "g", "conductance|nS" );
  gLeak->addhole( "membrane" ); //do I always need to tell it this? Do I need to explicitly connect all of these? Do I automatically view all guys "up"?

  auto adex = symmodel::Create("adex", "spiking|neuron");
  adex->addvar( "V", "membrane-potential|mV" ); //membrane potential
  adex->addvar( "W", "recovery-potential|mV" ); //recovery potential
  adex->addvar( "tspk", "spike-time|time|ms" ); //spiketime, time,

  adex->addhole( "currents" );
  adex->addhole( "conductances" ); //Could separate these into synapses etc? E.g. presyn and postsyn? Inhib/excit. etc.
  adex->addhole( "postsyn" );
  adex->addhole( "presyn" );

  adex->addmodel( gLeak, "gL", "" );

  adex->addmodel( pos3d, "position", "" );
  
  auto Iinj = symmodel::Create( "Iinj", "current" );
  Iinj->addvar( "I", "current|uA" );

  auto spksyn = symmodel::Create( "spksyn", "synapse");
  spksyn->addvar( "delay", "delay|ms" );
  spksyn->addvar( "weight", "synaptic-efficacy|nS" );
  spksyn->addvar( "hitweight", "spike-efficacy|nS" );
  spksyn->addhole( "presyn-neuron" );
  spksyn->addhole( "postsyn-neuron" ); //conductance?
  spksyn->addhole( "presyn-spiketimer" ); //must be of type "spiker" or something?
  spksyn->addhole( "postsyn-conductances" ); //How do I know which one corresponds to gAMPA, and GNMDA. Oh, I make them with same name so fine. Problem is, how much do I add to
  //each? E.g. does each differnet synapse model have differnet weightings for the different receptor types. Does it normalize automatically between them?
  //Uh, add it at the postsyn level? I.e. how strong gNMDA/gAMPA is for each...? Like some may be clustered? Do it per synapse? In which case we'd add multiple weightings if
  //we want to do it here? How about every single one has its own weight? Is there a way to e.g. add a thing for every model, but outside it...

  //Fill-hole-with-holes (by type) seems like definitely something I want
  //Or, fill-by-type, but where it searchers holes? fuck...


  //So, I connect this guy to other guy as a postsyn guy. And tell it, to only add to GLU type guys. OK, and then it goes and adds. Or I manually specify which ones to add to.
  //If I only generate e.g. a hole, and tell it to "point to" those guys, it will get in trouble because 

  //It will "generate" hitweights in here based on which guys I tell it is connected to!



  //This is an empty model. Need to know "root" of my reference in order to get it. There must always be a path...so that's fine...
  //Updates all of neuron type, then updates all of synapse type, etc.? No, there is no "explicitly update model X", it is all implicit... So this has no update function.
  auto sc = symmodel::Create("sc", "circuit");

  //Neurons and synapses are "holes"? They're just types haha.
  //Only vars and holes can be referenced directly...? Nah, models can too, they are just "variables" of this model...? Shit. How to update?
  //All models are updated...
  sc->addmodel( adex, "adex1", "" ); //Has no specific "local functions"
  sc->addmodel( adex, "adex2", "" );
  sc->addmodel( spksyn, "syn2-1", "" );
  sc->addmodel( spksyn, "syn1-1", "" ); //Specify type of synapse? Or might have different receptor to each postsyn target?

  //sc->connect( "syn2-1", "adex1" );
  //sc->connect( "adex2", "syn2-1" );



  //Where do I want to put the conductances? They should be where they are "size"
  //Best way is to add as postsyn, and "force" all presyn guys to add the right thing for me (how do I know when to do this?). In other words, if I add gNMDA, all presyn guys
  //know to automatically add variable for it.
  sc->addmodel( gAMPA, "adex1/gAMPA1", "" );
  sc->addmodel( gNMDA, "adex1/gNMDA1", "" );


  sc->addmodel( Iinj, "adex1/Iinj1", "" );
  sc->fillhole( "adex1/currents", "adex1/Iinj1" );

  sc->fillhole( "adex1/conductances", "adex1/gL" );
  sc->fillhole( "adex1/conductances", "adex1/gAMPA1" );
  sc->fillhole( "adex1/conductances", "adex1/gNMDA1" );

  //Fill hole holes? I.e. for all conductances, fill V with me?
  //Do "names" appear in models? What becomes the local name??? Is it in "hole" after all?
  //I.e. holes are first-order, just as models are...no I.e. blah/conductances/blah
  //It might have MANY postsynaptic connections...shit.
  sc->fillhole( "adex1/gAMPA1/membrane", "adex1" );
  sc->fillhole( "adex1/gNMDA1/membrane", "adex1" );
  sc->fillhole( "adex1/gL/membrane", "adex1" );


  //REV: I can literally artificially make a list of all postsyn grps (by making a list of postsyn_synapses or something?)
  //Will it automatically fill everything from those conductances?
  sc->fillhole( "syn2-1/postsyn-neuron", "adex1" );
  sc->fillhole( "syn2-1/presyn-neuron", "adex2" );
  sc->fillhole( "syn2-1/presyn-spiketimer", "adex2" );

  //Alternatively, use sc.fillhole_bytype( "syn2-1/postsyn-conductance", "GluR-mediated-conductance" );
  sc->fillhole( "syn2-1/postsyn-conductance", "adex1/gAMPA1" );
  sc->fillhole( "syn2-1/postsyn-conductance", "adex1/gNMDA1" );




  sc->fillhole( "adex1/conductances", "adex1/gL" );
  sc->fillhole( "adex2/gL/membrane", "adex2" );


  sc->fillhole( "syn1-1/postsyn-neuron", "adex1" );
  sc->fillhole( "syn1-1/presyn-neuron", "adex1" );

  sc->fillhole( "syn1-1/presyn-spiketimer", "adex1" );

  //Alternatively, use sc->fillhole_bytype( "syn2-1/postsyn-conductance", "GluR-mediated-conductance" );
  sc->fillhole( "syn1-1/postsyn-conductance", "adex1/gAMPA1" );
  sc->fillhole( "syn1-1/postsyn-conductance", "adex1/gNMDA1" );



  //Note, some groups may NOT have presyn (!!!) or postsyn or something ;) E.g. "non-source axons"
  sc->fillhole( "adex2/postsyn", "syn2-1" );
  sc->fillhole( "adex1/postsyn", "syn1-1" );
  sc->fillhole( "adex1/presyn", "syn1-1" );
  sc->fillhole( "adex1/presyn", "syn2-1" );
}

int main()
{
  test_build();
}
