/** @file tp_timeinterval.h  Class TimeInterval */


/* 
   Time plug-in for FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2007  Ruediger Berndt
   Copyright (C) 2007  Thomas Moor
   Exclusive copyright is granted to Klaus Schmidt

*/

#ifndef FAUDES_TP_TIMEINTERVAL_H
#define FAUDES_TP_TIMEINTERVAL_H

#include "corefaudes.h"
#include <cstdlib>
#include <cstring>
#include <cfloat>
#include <climits>
#include <iostream>


namespace faudes {

/** 
 * Type to represent time. 
 *
 * The time plugin will compile with integer or float time type. However,
 * we insist in discrete time in the sense that there is a smallest
 * time step. So float time would be cosmetic only. 
 * 
 * Note that timed generators do not explicitely refer to a physical time unit like e.g.
 * hours or seconds. However, it is assumed, that a fixed conversion factor
 * from values of type tpTime to some physical unit exists. Thus, we say tpTime
 * variables hold time in faudes-time units. We feel that this is consistent with
 * Alur's timed automata.
 *
 * @ingroup TimedPlugin 
 */
class tpTime {
 public:
   /** Datatype for point on time axis */
   typedef Int Type;
   /** Maximum time, associated with infinitiy */
   const static Type Max=FAUDES_INT_MAX;
   /** Minimum time, associated with minus infinitiy */
   const static Type Min=FAUDES_INT_MIN+1;
   /** Undefined time value */
   const static Type UnDef=FAUDES_INT_MIN;
   /** Smallest representable time step */
   const static Type Step=1;
   /** convert to string */
   std::string static Str(Type time) { 
     if(time == UnDef) return "undef"; 
     if(time < Min) return "err"; 
     if(time == Min) return "-inf"; 
     if(time > Max) return "err"; 
     if(time == Max) return "inf"; 
     return ToStringInteger(time); 
   };
};

/*
Alternative time representation: cosmetic float
class tpTime {
 public:
   typedef double Type;
   const static Type Max=FLT_MAX;
   const static Type Min=FLT_MIN*0.5;
   const static Type UnDef=FLT_MIN;
   const static Type Step=0.0001;
};
*/



/** 
 * Model of a time interval. An interval consisits of a lower and upper bound,
 * plus flags to indicate whether the bounds are inclusive or not.
 * The maximum and minimum values of the time data tyle are interpreted as infinity.
 * 
 *
 * @ingroup TimedPlugin 
 */
 class TimeInterval {
	
 public:
		
   /** 
    * Default constructor - sets time interval to (-inf; +inf)
    */
   TimeInterval(void) {SetFull();};

   /**
    * Default destructor
    */
   ~TimeInterval(void) {};

   /** 
    * Set the lower bound to a given value.
    * If the value is minus infinity, it is set exclusive. Otherwise,
    * the exclusive/inclusive flag is kept.
    *
    *	@param time
    *		The new value of lower bound
    */
   void LB(tpTime::Type time) {
     if(time <= tpTime::Min) time=tpTime::Min;
     if(time >= tpTime::Max) time=tpTime::Max;
     mstLB=time;
     if(mstLB==tpTime::Min)  mbLBincl=false; 
   }


   /** 
    * Set the upper bound to a given value.
    * If the value is infinity, it is set exclusive. Otherwise,
    * the exclusive/inclusive flag is kept.
    *
    *	@param time
    *		The new value of upper bound
    */
   void UB(tpTime::Type time) {
     if(time <= tpTime::Min) time=tpTime::Min;
     if(time >= tpTime::Max) time=tpTime::Max;
     mstUB=time;
     if(mstUB==tpTime::Max)  mbUBincl=false;                                  
   }


   /** 
    * Configures the upper bound to be inclusive.
    * If the upper bound is infinity, it stays exclusive.
    *
    *	@param incl
    */
   void UBincl(bool incl) {
     mbUBincl=incl;
     if(mstUB>=tpTime::Max) mbUBincl=false;
   }


   /** 
    * Configures the lower bound to be inclusive.
    * If the lower bound is minus infinity, it stays exclusive.
    *
    *	@param incl
    */
   void LBincl(bool incl) {
     mbLBincl=incl;
     if(mstLB<=tpTime::Min) mbLBincl=false;
   }


   /** 
    * Set upper bound to infinity
    */
   void setUBinf(void) {
     mstUB=tpTime::Max; mbUBincl=false; }

   /** 
    * Set lower bound to infinity
    */
   void setLBinf(void) {
     mstLB=tpTime::Min; mbLBincl=false; }

   /** 
    * Return upper bound.
    *
    *	@return upper bound
    */
   tpTime::Type UB(void) const {return mstUB;}

   /** 
    * Return lower bound.
    *
    *	@return lower bound
    */
   tpTime::Type LB(void) const {return mstLB;}

   /** 
    * Test for lower bound inclusive
    *
    *	@return true for inclusive
    */
   bool LBincl(void) const {return mbLBincl;}

   /** 
    * Test for upper bound inclusive
    *
    *	@return true for inclusive
    */
   bool UBincl(void) const {return mbUBincl;}

   /** 
    * Test for lower bound infinity
    *
    *	@return true for infinity
    */
   bool LBinf(void) const {return mstLB == tpTime::Min; }

   /** 
    * Test for upper bound infinity
    *
    *	@return true for infinity
    */
   bool UBinf(void) const {return mstUB == tpTime::Max;}


   /** 
    * Convert to canonical representation.
    *
    * Uses tpTime::Step to convert to aleft closed / right opened
    * representation
    */
   void Canonical(void);

   /** 
    * Set to full  (-inf, +inf)
    */
   void SetFull(void) {
     mstUB=tpTime::Max; mstLB=tpTime::Min; mbUBincl=false; mbLBincl=false; };

   /** 
    * Set to empty  (1, -1)
    */
   void SetEmpty(void) {
     mstUB=-1; mstLB=1; mbUBincl=false; mbLBincl=false; };

   /** 
    * Set to positive  [0, inf)
    */
   void SetPositive(void) {
     mstUB=tpTime::Max; mstLB=0; mbUBincl=false; mbLBincl=true; };


   /** 
    * Set to negative  (-inf, 0]
    */
   void SetNegative(void) {
     mstLB=tpTime::Min; mstUB=0; mbUBincl=false; mbLBincl=true; };

   /** 
    *  Test whether interval is full
    *
    *  @return
    *	   True if interval is (-inf,+inf)
    */		
   bool Full(void) const { return (mstUB==tpTime::Max) && (mstLB==tpTime::Min) ;};
			
   /** 
    *  Test interval for empty set
    *
    *  @return
    *	   True if interval is empty
    */		
   bool Empty(void) const;
			
   /**
    * Test whether interval includes [0,inf)
    *
    * @return
    *	   True if interval includes [0,inf)
    */
   bool IncludesPositive(void) const {  
      return ( (mstLB<0) || ((mstLB ==0) && mbLBincl) ) && (mstUB==tpTime::Max);};

   /**
    * Test whether interval includes (-inf,0]
    *
    * @return
    *	   True if interval includes (-inf,0]
    */
   bool IncludesNegative(void) const {  
      return ( (mstUB>0) || ((mstUB ==0) && mbUBincl) ) && (mstLB==tpTime::Min);};

   /**
    * Test whether a point satisfies interval
    * @return
    *	   True if interval includes the point
    */
   bool In(tpTime::Type time) const;


   /**
    * Test whether two intervals are equal.
    *
    * Note that the test is strictly based on the
    * internal representation, ie [0,10] is not equal to [0,11). This
    * may change in future implementations to consider the tpTime::TypeStep
    *
    * @return
    *	   True on equality
    */
   bool operator == (const TimeInterval& rOtherInterval) const {
     if(mstUB!=rOtherInterval.mstUB) return false;
     if(mstLB!=rOtherInterval.mstLB) return false;
     if(mbUBincl!=rOtherInterval.mbUBincl) return false;
     if(mbLBincl!=rOtherInterval.mbLBincl) return false;
     return true;
   }

   /**
    * Test whether two intervals not equal.
    *
    * Note that the test is strictly based on the
    * internal representation, ie [0,10] is not equal to [0,11). This
    * may change in future implementations to consider the tpTime::TypeStep
    *
    * @return
    *	   False on equality
    */
   bool operator != (const TimeInterval& rOtherInterval) const {
     return ! (*this == rOtherInterval) ;
   }

   /**
    * Transform by left shift and intersection with [0, inf)
    * @param time
    *    Amount to shift left
    *
    */
   void PositiveLeftShift(tpTime::Type time);

   /** 
    * Intersect this interval with other interval
    *
    *	@param rOtherInterval
    *		
    */
   void Intersect(const TimeInterval& rOtherInterval);
			
   /** 
    * Intersection of two time intervals.
    *
    *	@param rInterval1
    *		Reference of first time interval
    *	@param rInterval2
    *		Reference of second time interval
    *		
    *	@return Intersection of both intervals.
    */
   static TimeInterval Intersect(const TimeInterval& rInterval1, const TimeInterval& rInterval2);
			

   /** 
    * Merge this interval with other interval.
    * I.e. find smallest superset of the union of the two intervals.
    *
    *	@param rOtherInterval
    *		Reference to other time interval
    *		
    */
   void Merge(const TimeInterval& rOtherInterval);

   /** 
    * Merge this interval with other interval.
    * I.e. find smallest superset of the union of the two intervals.
    *
    *	@param rInterval1
    *		Reference of first time interval
    *	@param rInterval2
    *		Reference of second time interval
    *		
    *   @return Superset of both intervals.	
    */
   static TimeInterval Merge(const TimeInterval& rInterval1, const TimeInterval& rInterval2);
						
   /** 
    * Pretty printable string.
    *
    *	@return printable string.
    */
   std::string Str(void) const;
			
		
 private:

   /** Upper bound */    
   tpTime::Type mstUB;

   /** Lower bound */
   tpTime::Type mstLB;

   /** Flag to indicate that the upper bound is part of the interval */
   bool mbUBincl;

   /** Flag to indicate that lower boundary is part of the interval. */
   bool	mbLBincl;

};


} // end namespace faudes


#endif
