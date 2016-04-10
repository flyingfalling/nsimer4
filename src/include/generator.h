//REV: 4 Apr 2016
//generator.h
//generator class for user to make generators to generate actual models (implemented symmodels)



//TODO:
//I need to generate the size.
//Generators can be:
//  GLOBAL (i.e. this will generate "the whole thing" themselves)
//  LOCAL (they MAY generate some part locally ). What determines number of local to call? Num of pre-post pairs, etc.? I.e. a GLOBAL-inside-LOCAL

//  In local case, it simply re-combines all LOCAL guys together at the end.
//  Can we nest, local in local?
//  Basically, GLOBAL is a local with size=1.

//REV: how can I reference a "constant" variable? I can do it, but it requires it to know it is a global variable haha.
//Just mark varible "isglobal" or some shit? Or have a subset that are certain values? In that way, make corresp...do it later.
//Just generate single variable, it's "constant", and I mark that it is the case.
//Also mark "param" (i.e. can't be modified by update function), etc.? Problem is e.g. some previous params like weight might be turned into vars by
//additional models?




#pragma once

#include <symmodel.h>

struct generator
{
  //A generator is something that is dependent on other things
  //Ah, basically this is exactly the same as symmodel with update functs...but it will "compute" or "add" each guy..shit.
  //So, it will target a given model, and a given variable in the model (or the size_t guy I guess shit ).
  //And so it will be either LOCAL or GLOBAL?
  
};
