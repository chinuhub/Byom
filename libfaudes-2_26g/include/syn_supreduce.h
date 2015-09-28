/** @file syn_reduced.h Supervisor Reduction */

/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2006  Bernd Opitz
   Copyright (C) 2007  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

#ifndef FAUDES_SUPREDUCE_H
#define FAUDES_SUPREDUCE_H

#include "corefaudes.h"

namespace faudes {
  
 /** 
  * Data structure for identifying states in the same coset for supervisor reduction
  */
struct ReductionStateInfo{
    StateSet mPlantStates;
    EventSet mEnabledEvents;
    EventSet mDisabledEvents;
    bool mMarkedState;
    bool mPlantMarked;
};

/**
 * Supervisor Reduction algorithm
 *
 * Computes a reduced supervisor from a given potentially non-reduced supervisor and the plant. 
 * This algorithm implements the results obtained in 
 *
 * R. Su and W. M. Wonham. Supervisor Reduction for Discrete-Event Systems.
 * Discrete Event Dynamic Systems vol. 14, no. 1, January 2004.
 *
 * Both, plant and supervisor MUST be deterministic and share the same alphabet!!!
 *
 * @param rPlantGen
 *   Plant generator
 * @param rSupGen
 *   Supervisor generator
 * @param rReducedSup
 *   Reduced supervisor generator
 *
 * @return
 *   True if a reduction was achieved
 *
 * @exception Exception
 *   - alphabets of generators don't match (id 100)
 *   - plant nondeterministic (id 201)
 *   - supervisor nondeterministic (id 203)
 *   - plant and supervisor nondeterministic (id 204)
 *
 * @ingroup SynthesisPlugIn
 */
bool SupReduce(const System& rPlantGen, const System& rSupGen, System& rReducedSup);


/**
 * Supervisor Reduction mergibility algorithm
 *
 * This recursive algorithm determines if two supervisor states can be merged to the same coset. 
 * It is called by the main procedure SupReduce. 
 *
 * @param stateI
 *   First state to be checked
 * @param stateJ
 *   second state to be checked
 * @param rWaitList
 *   list of waiting state pairs
 * @param cNode
 *   remembers first state to be checked
 * @param rSupGen
 *   constant reference to supervisor 
 * @param rSupStateInfo
 *   constant reference to state info data structure
 * @param rState2Class
 *   constant reference to data structure that maps states to their coset
 * @param rClass2State
 *   constant reference to data structure that maps cosets to their states
 *
 * @return
 *   True if the classes of statep and stateq can be merged
 *
 */
bool CheckMergibility(Idx stateI, Idx stateJ, std::vector<std::set<Idx> >& rWaitList, Idx cNode, const System& rSupGen,
			 const std::map<Idx,ReductionStateInfo>& rSupStateInfo, const std::map<Idx,Idx>& rState2Class, const std::vector<StateSet>& rClass2States);
			 

} // namespace faudes

#endif 
