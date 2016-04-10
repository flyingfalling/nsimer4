//REV: 4 Apr 2016
//generator.h
//generator class for user to make generators to generate actual models

//Note, models must have connection, which is a specific variable (?) which is value of pre/post.
//these are "connections"

//Do these exist for each "HOLE" (element!!?)
//And also for each SUB model (those are always identity)
//Let's assume for each model-model pair (different for pre-post).
//For each model, for each other model (that it connects to?), it has a:
//  1)  him-to-me, i.e. if I want to access "my" value, it is at conn[me] idx.
//  X)  Just offset and size. Might not be in order? Offset and size in CONN!!!
//  Note, these are variables of PRE side? In other words, they are same length as pre-side.
//  so, I will have "start" and "size" if it is one-to-many (that is pointer into what, a larger conn array?)
//  or, I will have "src" if it is many-to-one
//  2)  me-to-him, i.e. if I want to
//  At any rate, do only HOLES need it? Can things be connected without filling holes? For example, if I am a model that is filling a hole of another,
//  E.g. I am the postsyn neuron.
//  I assume they are only referenced for "reading" purposes. In other words, for gAMPA, I iter through all the presyn-gAMPA. To know which elements to actually access,
//  I need to use connection model. Which is the connection between me and him. It goes not through holes, but through variables. But don't know how to sum them?
//  Need to iter through all hitweights of all presyn.

//Fine. So, I select to "generate" a variable. E.g. for example, I generate synapse connections. For example, syns2-1. Its connection to adex1. I need to check both adex1
//and adex2, and that is that. So I need to set presyn and postsyn both for the spiker and the synapse? Ugh.. Make it parallel?

//At any rate, where are connections existing? Are they only at holes (I assume so?).
//In that case, I need to specify the hole to generate from as the thing to generate.
//But, in that case, e.g. if I want to generate from conductances, I need to say, the hole from ME to POSTSYN.
//Are all other guys of size? How can I create "from group" of same size? They're a single model I guess...
//Like, a "synapse" model.
//That makes more sense, I can do it either way though.

//Make a way (in future) to generate parallel model A from model B. For now, add both spksyn and syn to same submodel.
//This means, I would derive normal synapse, which has both spksyn, and syn in it, and instead make it so that it has
//spksyn and Glusyn. Yea, it's a new type, and it has both of them in it. Fine.


#pragma once

#include <symmodel.h>

struct generator
{
  //A generator is something that is dependent on other things
  //Ah, basically this is exactly the same as the other guy...but it will "compute" or "add" each guy..shit.
  //So, it will target a given model, and a given variable in the model (or the size_t guy I guess shit ).
  //And so it will be either LOCAL or GLOBAL?
  
};
