/** @file cfl_abaseset.h @brief Class TaBaseSet */


/* FAU Discrete Event Systems Library (libfaudes)

   Copyright (C) 2008  Thomas Moor
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



#ifndef FAUDES_ABASESET_H
#define FAUDES_ABASESET_H

#include "cfl_baseset.h"

namespace faudes {

/** @addtogroup ContainerClasses */
/** @{*/

/**
 * Set with nontrivial attributes.
 *
 * The TaBaseSet implements the attribute interface introduced
 * in TBaseSet. 
 *
 * Note: while TaBaseSet is indirectly derived from Type, we avoid the FAUDES_TYPE_DECLARATION
 * macro and define the Type interface explicitely. This is for cosmetic reasons only.
 *
 */


template<class T, class Attr, class Cmp=std::less<T> >
class TaBaseSet : virtual public TBaseSet<T,Cmp> {


public:

  /**
   * Constructor. 
   */
  TaBaseSet(void);

  /**
   * Copy-constructor. 
   * Uses DoAssign copy the set incl attributes.
   *
   * @param rOtherSet 
   *    Source to copy from
   */
  TaBaseSet(const TaBaseSet& rOtherSet);

  /**
   * Copy-constructor. Sets attributes to default.
   * Uses the DoAssign method to convert attributes if possible.
   *
   * @param rOtherSet 
   *    Source to copy from
   */
  TaBaseSet(const TBaseSet<T,Cmp>& rOtherSet);

  /**
   * Constructor from file.
   * This constructor indicates the intended interface for derived classes. The base set 
   * itself cannot read from token streams.
   *
   * @param rFilename
   *   Name of File
   * @param rLabel
   *   Section for the set in the file; 
   */
  TaBaseSet(const std::string& rFilename, const std::string& rLabel = "BaseSet");

  /**
   * Virtual destructor
   */
  virtual ~TaBaseSet(void);

  /**
   * Faudes Type interface. Construct object of same type on heap.
   */
  //virtual TaBaseSet* New(void) const { return new TaBaseSet(); };

  /**
   * Faudes Type interface. Construct copy on heap.
   */
  //virtual TaBaseSet* Copy(void) const { return new TaBaseSet(*this); };

  /**
   * Faudes Type interface. Cast object to my type.
   */
  //virtual const Type* Cast(const Type* pType) const { return dynamic_cast<const TaBaseSet*>(pType); };

  /**
   * Copy from a TaBaseSet with matching attributes.
   *
   * @param rSourceSet 
   *    Set to copy from
   */
  virtual TaBaseSet& Assign(const TaBaseSet& rSourceSet);

  /**
   * Copy from a TBaseSet with attributes, provided that they
   * can be casted acciordingly.
   *
   * @param rSourceSet 
   *    Set to copy from
   */
  virtual TaBaseSet& Assign(const TBaseSet<T,Cmp>& rSourceSet);

  /**
   * Copy from a TBaseSet without attributes. This method
   * clears all attributes even if they cound be casted.
   *
   * @param rSourceSet 
   *    Set to copy from
   */
  virtual TaBaseSet& AssignWithoutAttributes(const TBaseSet<T,Cmp>& rSourceSet);

  /**
   * Faudes Type interface. Assignment operator with strict type matching. 
   * @param rSource 
   *    Object to assign from
   */
  virtual TaBaseSet& operator=(const TaBaseSet& rSource) { DoAssign(rSource);return *this; };

  /**
   * Faudes Type interface (extension). Assignment operator incl attribute cast.
   * @param rSource 
   *    Object to assign from
   */
  virtual TaBaseSet& operator=(const TBaseSet<T,Cmp>& rSource) { return Assign(rSource); }

  /** 
   * Clear all set.
   */
  virtual void Clear(void);
      
  /**
   * Attribute typeinfo.
   *
   * @return
   *    Pointer to some attribute of this sets attribute type.
   */
  const Attr* Attributep(void) const;

  /**
   * Attribute typeinfo
   *
   * @return
   *    Reference to some attribute of this sets attribute type
   */
  const Attr& Attribute(void) const;

  /** 
   * Attribute typeinfo.
   * An TaBaseSet accepts all attributes that we can cast to our default attribute's type.
   * The implementation uses the virtual function Cast of the default attribute to perform
   * the test. Thus, it is crucial that Cast is re-implemented for attribute classes.
   * @param rAttr
   *     Attribute type to test.
   * @return True, if attribute type is accepted.
   */
  virtual bool AttributeTry(const Type& rAttr) const;

  /**
   * Get number of explicit (aka non-default) attributes.
   *
   * @return 
   *   Number of entries in mAttributeMap
   */
  virtual Idx AttributesSize(void) const ;

  /** 
   * Clear all attributes to default value.
   *
   */
  virtual void ClearAttributes(void);

  /**
   * Test whether attributes match with other set,
   * Return true if attributes match for shared elements. It uses
   * the equality test of individual attributes and, hence, requires the type
   * match too.
   * @param rOtherSet 
   *    Other set to compare with.
   * @return 
   *   True on match.
   */
  virtual bool EqualAttributes(const TBaseSet<T,Cmp>& rOtherSet) const;

  /**
   * Test set equality.
   * The test ignores attributes. It is implemented by the virtual method DoEqual().
   * @param rOtherSet 
   *    Other set to compare with.
   * @return 
   *   True on match.
   */
  virtual bool operator==(const TBaseSet<T,Cmp>& rOtherSet) const;

  /**
   * Test whether attributes match with other set,
   * Return true if attributes match for shared elements. It uses
   * the equality test of individual attributes and, hence, requires the type
   * match too.
   * @param rOtherSet 
   *    Other set to compare with.
   * @return 
   *   True on match.
   */
  virtual bool EqualAttributes(const TaBaseSet& rOtherSet) const;

  /** 
   * Get attribute reference by element. Note that in order to produce a non-const 
   * reference this method will insert an explicit default attribute if necessary. 
   * If a const reference is sufficient, you should use Attribute(rElem) const instead.
   *
   * @param rElem
   *    Specify element
   * @exception Exception
   *   - Element does not exist (60)
   * @return 
   *    Pointer to attribute
   */
  virtual Attr* Attributep(const T& rElem);

  /** 
   * Get attribute by element. This function returns a const reference to the attribute
   * of the specified element. 
   * @param rElem
   *    Specify element
   * @exception Exception
   *   - Element does not exist (63)
   * @return 
   *    Reference to attribute
   */
  virtual const Attr& Attribute(const T& rElem) const;

  /** 
   * Set attribute. Provided that the attribute can be casted to the
   * appropriate type, this method sets the attribute of the sepcified element to the given value. 
   *
   * @param rElem
   *    Specify element
   * @param attr
   *    Attribute value.
   * @exception Exception
   *   - Element does not exist (60)
   *   - Cannot cast attribute type (63)
   */
  virtual void Attribute(const T& rElem, const Type& attr);

  /** 
   * Set attribute. This method sets the attribute of the sepcified element to the given value. 
   *
   * @param rElem
   *    Specify element
   * @param attr
   *    Attribute value.
   * @exception Exception
   *   - Element does not exist (60)
   *   - Cannot cast attribute type (63)
   */
  virtual void Attribute(const T& rElem, const Attr& attr);

  /** 
   * Set attribute. Provided that the attribute can be casted to the
   * appropriate type, this method sets the attribute of the sepcified element to the given value. 
   * If the cast fails, this method does nothing.
   *
   * @param rElem
   *    Specify element
   * @param attr
   *    Attribute value.
   * @exception Exception
   *   - Element does not exist (60)
   */
  virtual void AttributeTry(const T& rElem, const Type& attr);

  /**
   * Set attributes. Provided that rOtherSet has attributes that can be
   * casted to the appropriate type, attributes are copied per element from rOtherSet. 
   * Elements of this set which are not in rOtherSet maintain their attribute. 
   *
   * @param rOtherSet
   *   Other BaseSet
   * @exception Exception
   *   - Cannot cast attribute type (63)
   */
  virtual void Attributes(const TBaseSet<T,Cmp>& rOtherSet);

  /**
   * Set attributes. Attributes are copied per element from rOtherSet. 
   * Elements of this set which are not in rOtherSet maintain their attribute. 
   *
   * @param rOtherSet
   *   Other BaseSet
   */
  virtual void Attributes(const TaBaseSet& rOtherSet);

  /** 
   * Clear attribute to default value. 
   *
   * @param rElem
   *    Specify element
   */
  virtual void ClrAttribute(const T& rElem);

  /** 
   * Use TBaseSet iterators. 
   */
  using typename TBaseSet<T,Cmp>::Iterator;

  /**
   * Insert element. If the element exists, the attribute is maintained.
   * If the element does not exist, it is inserted with default attribute.
   *
   * @param rElem
   *   Specify element
   * @return
   *   True if element was new to set 
   */
  bool Insert(const T& rElem);

  /** 
   * Insert element with attribute. 
   *
   * @param rElem
   *   Specify element
   * @param attr
   *   Specify attribute of (new) element
   * @return
   *   True if element was new to set 
   */
  bool Insert(const T& rElem, const Attr& attr);

  /**
   * Inserts elements of rOtherSet.
   * Attributes of this set are maintained, newly inserted elements have default attribute.
   *
   *
   * @param rOtherSet
   *   Other BaseSet
   */
  void InsertSet(const TBaseSet<T,Cmp>& rOtherSet);

  /**
   * Inserts elements of rOtherSet.
   * Attributes of this set are maintained, new elements are inserted with attribute.
   *
   * @param rOtherSet
   *   Other BaseSet
   */
   void InsertSet(const TaBaseSet& rOtherSet);


  /** 
   * Erase Element (incl its attribute) 
   *
   * @param rElem
   *    Specify element
   * @return
   *    True if element used to exist
   */
  bool Erase(const T& rElem);

  /** 
   * Erase element by iterator (incl attribute) 
   *
   * @param pos
   *    Iterator to specify element
   * @return 
   *    Iterator to next element or End()
   */
  // declare&define for cl_win issues
  typename TBaseSet<T,Cmp>::Iterator Erase(const typename TBaseSet<T,Cmp>::Iterator& pos){
    ClrAttribute(*pos);
    return TBaseSet<T,Cmp>::Erase(pos); 
  }; 
  

  /** 
   * Erase elements given by other set. This function
   * ignores the attributes of the other set and maintains the attributes 
   * of the remaining elements in this set.
   *
   * @param rOtherSet 
   *    Elements to erase
   */
  void EraseSet(const TBaseSet<T,Cmp>& rOtherSet);

  /** 
   * Restrict to specified subset. Erases any elements no in
   * the specified set. This function
   * ignores the attributes of the other set and maintains the attributes 
   * of the remaining elements in this set.
   *
   * @param rOtherSet 
   *    Elements to erase
   */
   void RestrictSet(const TBaseSet<T,Cmp>& rOtherSet);


  /**
   * Set union. The union is wrt the set of indices, the result is accumulated in this set.
   * Attributes are set to default. See also InsertSet(const TaBaseSet&).
   *
   * @param rOtherSet
   *   Other BaseSet
   */
  void SetUnion(const TBaseSet<T,Cmp>& rOtherSet);

  /**
   * Set intersection. The intersection is  wrt set of indices, the result is stored in this set.
   * Attributes are set to default.
   * 
   * @param rOtherSet
   *   Other BaseSet
   */
  void SetIntersection(const TBaseSet<T,Cmp>& rOtherSet);


  /** Detach from extern storage (reimplement base) */
  virtual void Detach(void) const;

 protected:

  /** use TBaseSet STL iterators */
  typedef typename TBaseSet<T,Cmp>::iterator iterator;

  /** use TBaseSet STL iterators */
  typedef typename TBaseSet<T,Cmp>::const_iterator const_iterator;

  /** use TBaseSet STL iterators */
  typedef typename TBaseSet<T,Cmp>::aiterator aiterator;

  /** use TBaseSet STL iterators */
  typedef typename TBaseSet<T,Cmp>::const_aiterator const_aiterator;


  /** default attribute */ 
  Attr* mpDefAttribute; 
  
  /** assignment from source with matching attributes */
  virtual void DoAssign(const TaBaseSet& rSourceSet);

  /** test equality, ignore attributes */
  virtual bool DoEqual(const TBaseSet<T,Cmp>& rOtherSet) const;

  /** set attribute in map (assume elem exists in set, NULL <=> set to default) */
  virtual void DoAttributep(const T& rElem, const Type* pAttr);

  /** set attribute in map (assume elem exists in set, NULL <=> set to default) */
  virtual void DoAttributep(const T& rElem, const Attr* pAttr);

  /** get attribute from map (return null if elem does not exist in map) */
  virtual const Attr* DoAttributep(const T& rElem) const;

  /** get attribute from map (insert explicit default if elem does not exist in map) */
  virtual Attr* DoAttributep(const T& rElem);


};



/** @} doxygen group */



/*
******************************************************************************************
******************************************************************************************
******************************************************************************************

Implementation of TaBaseSet

******************************************************************************************
******************************************************************************************
******************************************************************************************
*/

/* convenience access to relevant scopes */
#define THIS TaBaseSet<T,Attr,Cmp> 
#define TEMP template<class T, class Attr, class Cmp>
#define BASE TBaseSet<T,Cmp> 

// TaBaseSet()
TEMP THIS::TaBaseSet(void) :
  BASE(),
  mpDefAttribute(new Attr())
{
  FD_DC("TaBaseSet(" << this << ")::TaBaseSet()");
}

  
// TaBaseSet(rOtherSet)
TEMP THIS::TaBaseSet(const TaBaseSet& rOtherSet) : 
  BASE(),
  mpDefAttribute(new Attr())
{
  FD_DC("TaBaseSet(" << this << ")::TaBaseSet(TaBaseSet " << &rOtherSet << "): copy construct");
  DoAssign(rOtherSet);
}

// TaBaseSet(rOtherSet)
TEMP THIS::TaBaseSet(const TBaseSet<T,Cmp>& rOtherSet) : 
  BASE(),
  mpDefAttribute(new Attr())
{
  FD_DC("TaBaseSet(" << this << ")::TaBaseSet(TBaseSet " << &rOtherSet << "): copy construct");
  Assign(rOtherSet);
}

// TaBaseSet(filename)
TEMP THIS::TaBaseSet(const std::string& rFileName, const std::string& rLabel)  :
  BASE(),
  mpDefAttribute(new Attr())
{
  FD_DC("TaBaseSet(" << this << ")::TaBaseSet()");
  // do read etc ... this is a dummy anyway
  BASE::Read(rFileName,rLabel);  
}

// destructor
TEMP THIS::~TaBaseSet(void) {
  FD_DC("TaBaseSet(" << this << ")::~TaBaseSet()");
  // free my members
  if(mpDefAttribute) delete mpDefAttribute;
  // note: attributes will be deleted in BASE after relink
}


// copy, matching attributes
TEMP void THIS::DoAssign(const TaBaseSet& rSourceSet) {
  FD_DC("TaBaseSet(" << this << ")::DoAssign(TaBaseSet " << &rSourceSet << ")");
  // bail out on selfref
  if(this==&rSourceSet) return;
  // call base (fake copy, sets to empty attributes)
  BASE::DoAssign(rSourceSet);
  // link to shared attributes
  BASE::pAttributes=rSourceSet.pAttributes;
}  



// public wrapper
TEMP THIS& THIS::Assign(const TaBaseSet<T,Attr,Cmp>& rSourceSet) {
  DoAssign(rSourceSet);
  return *this;
}

// assign, try attributes
TEMP THIS& THIS::Assign(const TBaseSet<T,Cmp>& rSourceSet) {
  FD_DC("TaBaseSet(" << this << ")::Assign(..)");
  FD_DC("TaBaseSet(" << this << ")::Assign(..): dst type " << typeid(*this).name());
  FD_DC("TaBaseSet(" << this << ")::Assign(..): src type " << typeid(rSourceSet).name());
  // call base (fake copy, results in effectively empty attributes)
  BASE::DoAssign(rSourceSet);
  // detach, since we dont want to share mixed attributes
  Detach();
  // if there are no attributes, we dont worry
  if(rSourceSet.AttributesSize()==0) return *this;
  // if we have void attributes, we cannot cast (TODO: known at compiletime!)
  FD_DC("TaBaseSet(" << this << ")::Assign(..): dst attribute type " << typeid(*mpDefAttribute).name());
  if( typeid(*mpDefAttribute) == typeid(AttributeVoid) ) return  *this;
  // if source has void attributes, we cannot cast
  FD_DC("TaBaseSet(" << this << ")::Assign(..): src attribute type " << typeid(*rSourceSet.Attributep()).name());
  if( typeid(*rSourceSet.Attributep()) == typeid(const AttributeVoid) ) return  *this;
  // if attributes cannot be casted, we dont need to loop either
  FD_DC("TaBaseSet(" << this << ")::Assign(..): try attribute cast");
  if(!mpDefAttribute->Cast(rSourceSet.Attributep())) return  *this;
  // try to be smart on attributes 
  BASE::mpAttributes= new std::map<T,AttributeVoid*>();
  BASE::pAttributes=BASE::mpAttributes;
  FD_DC("TaBaseSet(" << this << ")::Assign(..): mind attributes");
  const_aiterator ait=rSourceSet.pAttributes->begin();
  for(;ait!=rSourceSet.pAttributes->end(); ++ait) {
    Attr* attr= new Attr;
    attr->Assign(*ait->second);
    (*BASE::pAttributes)[ait->first]=attr;
  }
  // done
  FD_DC("TaBaseSet(" << this << ")::Assign(..): done");
  return *this;
}



// public wrapper
TEMP THIS& THIS::AssignWithoutAttributes(const TBaseSet<T,Cmp>& rSourceSet) {
  // call base (fake copy, results in effectively empty attributes)
  BASE::DoAssign(rSourceSet);
  // detach, since we dont want to share mixed attributes
  Detach();
  // done
  return *this;
}

// test equality, ignore attributes 
TEMP bool THIS::DoEqual(const TBaseSet<T,Cmp>& rOtherSet) const {
  FD_DC("TaBaseSet::DoEqual()");
  // require set elements to match
  if(!BASE::DoEqual(rOtherSet)) 
    return false;
  // ok, equal
  return true;
  /*
  For reference: version that requires attributes either to match or 
  to be of default value

  // true, if we share attubute data
  FD_DC("TaBaseSet::DoEqual(TBaseSet): 2");
  if(BASE::pAttributes==rOtherSet.pAttributes)
    return true;
  // true, if there are no attributes
  FD_DC("TaBaseSet::EqualAttributes(TBaseSet): 3");
  if(rOtherSet.AttributesSize()==0)
  if(this->AttributesSize()==0)
    return true;
  // iterate over attributes
  aiterator ait1 = BASE::pAttributes->begin();
  aiterator ait2 = rOtherSet.pAttributes->begin();
  while ((ait1 != BASE::pAttributes->end()) && (ait2 != rOtherSet.pAttributes->end())) {
    if (ait1->first < ait2->first) {
      ++ait1;
    }
    else if (ait1->first == ait2->first) {
      FD_DC("TaBaseSet::EqualAttributes(TBaseSet): cmp " << ait1->second->ToString() 
	      << " vs " << ait2->second->ToString());
      if( (!ait1->second->IsDefault()) || (!ait2->second->IsDefault()))  
        if( ! ait1->second->Equal(*ait2->second)) return false;
      ++ait1;
      ++ait2;
    }
    else { // (*it1 > *it2)
      ++ait2;
    }
  }
  // passed
  FD_DC("TaBaseSet::EqualAttributes(TBaseSet): pass");
  return true;
  */
}

// operator version of DoEqual
TEMP bool THIS::operator==(const TBaseSet<T,Cmp>& rOtherSet) const {
  return DoEqual(rOtherSet);
}

// Detach()
TEMP void THIS::Detach(void) const {
  FD_DC("TaBaseSet(" << this << ")::Detach(void)");

  // nothing todo
  if(BASE::mDetached) return;

  // provide fake const
  THIS* fake_const = const_cast< THIS* >(this);

  // do allocation and copy of attributes, 
  std::map<T,AttributeVoid*>* acopy = new std::map<T,AttributeVoid*>();
  for(aiterator ait=BASE::pAttributes->begin(); ait!=BASE::pAttributes->end(); ++ait) {
    Attr* attr= new Attr();
    attr->Assign(*ait->second);
    (*acopy)[ait->first]=attr;
  }

  // call base for core set, effectively empty attributes
  BASE::Detach();
 
  // stragie A or B: others got the origianal attributes, i take the copy
  fake_const->mpAttributes=acopy;
  fake_const->pAttributes=acopy;

}


//Clear
TEMP void THIS::Clear(void) {
  FD_DC("TaBaseSet(" << this << ")::Clear()");
  // call base (incl streamlined pseudo detach and empty attributes)
  BASE::Clear();
}


//Insert(elem)
TEMP bool THIS::Insert(const T& rElem) {
  FD_DC("TvIndexSet(" << this << ")::Insert(elem)");
  bool ret=BASE::Insert(rElem);
  return ret;
}

//Insert(idx,attr)
TEMP bool THIS::Insert(const T& rElem, const Attr& rAttr) {
  FD_DC("TaIndexSet(" << this << ")::Insert(elem,attr)");
  Detach();
  bool ret=BASE::pSet->insert(rElem).second;
  DoAttributep(rElem,&rAttr);
  return ret;
}

// InsertSet(set)
TEMP void THIS::InsertSet(const TBaseSet<T,Cmp>& rOtherSet) {
  bool doattr=AttributeTry(rOtherSet.Attribute()); 
  FD_DC("TaBaseSet(" << this << ")::InsertSet( [v] " << &rOtherSet << "): doattr=" << doattr);
  Detach();
  iterator it1 = BASE::pSet->begin();
  iterator it2 = rOtherSet.pSet->begin();
  while ((it1 != BASE::pSet->end()) && (it2 != rOtherSet.pSet->end())) {
    if (*it1 < *it2) {
      ++it1;
    }
    else if (*it1 == *it2) {
      ++it1;
      ++it2;
    }
    else { // (*it1 > *it2)
      Insert(*it2);
      if(doattr) DoAttributep(*it2,&rOtherSet.Attribute(*it2));
      ++it2;
    }
  }
  while (it2 != rOtherSet.pSet->end()) {
    Insert(*it2);
    if(doattr) DoAttributep(*it2,&rOtherSet.Attribute(*it2));
    ++it2;
  }
}

// InsertSet(set)
TEMP void THIS::InsertSet(const TaBaseSet& rOtherSet) {
  FD_DC("TaBaseSet(" << this << ")::InsertSet( [a] " << &rOtherSet << ")");
  Detach();
  iterator it1 = BASE::pSet->begin();
  iterator it2 = rOtherSet.pSet->begin();
  while ((it1 != BASE::pSet->end()) && (it2 != rOtherSet.pSet->end())) {
    if (*it1 < *it2) {
      ++it1;
    }
    else if (*it1 == *it2) {
      ++it1;
      ++it2;
    }
    else { // (*it1 > *it2)
      Insert(*it2,rOtherSet.Attribute(*it2));
      ++it2;
    }
  }
  while (it2 != rOtherSet.pSet->end()) {
    Insert(*it2,rOtherSet.Attribute(*it2));
    ++it2;
  }
}


//Erase(idx)
TEMP bool THIS::Erase(const T& rElem) {
  Detach();
  DoAttributep(rElem,(const Attr*) 0);
  return (BASE::pSet->erase(rElem)!=0);
}


//Erase(pos)
/*
// cl_win issue: has been moved to declaration
TEMP typename BASE::Iterator THIS::Erase(const typename BASE::Iterator& pos) { 
#ifdef FAUDES_CHECKED
  if(pos == BASE::End()) {
    std::stringstream errstr;
    errstr << "iterator out of range " << std::endl;
    throw Exception("TaBase::Erase", errstr.str(), 60);
  }
#endif
  ClrAttribute(*pos);
  return BASE::Erase(pos);
}
*/

//EraseSet(set)
TEMP void THIS::EraseSet(const TBaseSet<T,Cmp>& rOtherSet) {
  FD_DC("TaBaseSet(" << this << ")::EraseSet(" << rOtherSet.ToString() << ")");
  Detach();
  // todo: test and optimize 
  iterator tmpit;
  iterator it = BASE::pSet->begin();
  iterator oit = rOtherSet.pSet->begin();
  while ((it != BASE::pSet->end()) && (oit != rOtherSet.pSet->end())) {
    if (*it < *oit) {
      it=BASE::pSet->lower_bound(*oit); // alt: ++it;
    }
    else if (*it == *oit) { 
      DoAttributep(*it,(const Attr*) 0);
      tmpit=it;
      ++it;
      ++oit;
      BASE::pSet->erase(tmpit);
    }
    else { // (*it > *oit)
      oit=rOtherSet.pSet->lower_bound(*it); // ++it2;
    }
  }
}


//RestrictSet(set)
TEMP void THIS::RestrictSet(const TBaseSet<T,Cmp>& rOtherSet) {
  FD_DC("TaIndexSet(" << this << ")::RestrictSet(" << rOtherSet.ToString() << ")");
  Detach();
  // todo: test and optimize 
  iterator tmpit;
  iterator it = BASE::pSet->begin();
  iterator oit = rOtherSet.pSet->begin();
  while ((it != BASE::pSet->end()) && (oit != rOtherSet.pSet->end())) {
    if (*it < *oit) {
      DoAttributep(*it,(const Attr*) 0);
      tmpit=it;
      ++it;
      BASE::pSet->erase(tmpit);
    }
    else if (*it == *oit) { 
      ++it;
      ++oit;
    }
    else { // (*it > *oit)
      ++oit;
    }
  }
  while(it != BASE::pSet->end()) {
    DoAttributep(*it,(const Attr*) 0);
    tmpit=it;
    ++it;
    BASE::pSet->erase(tmpit);
  }

}


// SetUnion(set)
 TEMP void THIS::SetUnion(const TBaseSet<T,Cmp>& rOtherSet) {
  BASE::SetUnion(rOtherSet);
  ClearAttributes();
}

// SetUnion(set)
TEMP void THIS::SetIntersection(const TBaseSet<T,Cmp>& rOtherSet) {
  BASE::SetIntersection(rOtherSet);
  ClearAttributes();
}


// attribute typeinfo
TEMP const Attr* THIS::Attributep(void) const { 
  return mpDefAttribute; 
}

// attribute typeinfo
TEMP const Attr& THIS::Attribute(void) const { 
  return *mpDefAttribute;
}

// test attribute type
TEMP bool THIS::AttributeTry(const Type& rAttr) const {
  return mpDefAttribute->Cast(&rAttr)!=NULL;
}


// get attribute wrapper
TEMP Attr* THIS::Attributep(const T& rElem) {
#ifdef FAUDES_CHECKED
  if(!THIS::Exists(rElem)) {
    std::stringstream errstr;
    errstr << "element \"" << this->Str(rElem) << "\" not member of set " << this->Name() << std::endl;
    throw Exception("TaBaseSet::Attributep(elem)", errstr.str(), 60);
  }
#endif
  // must detach
  Detach();
  // find in map (incl insert explicit default)
  Attr* res=DoAttributep(rElem);
  // done
  return res;
}

// get attribute wrapper
TEMP const Attr& THIS::Attribute(const T& rElem) const {
#ifdef FAUDES_CHECKED
  if(!THIS::Exists(rElem)) {
    std::stringstream errstr;
    errstr << "element \"" << this->Str(rElem) << "\" not member of set " << this->Name() << std::endl;
    throw Exception("TaBaseSet::Attribute(elem)", errstr.str(), 60);
  }
#endif
  // find in map
  const Attr* res=DoAttributep(rElem);
  // use default
  if(!res) res=mpDefAttribute;
  // done
  return *res;
}

// set attribute wrapper
TEMP void THIS::Attribute(const T& rElem, const Type& attr) {
#ifdef FAUDES_CHECKED
  if (!THIS::Exists(rElem)) {
    std::stringstream errstr;
    errstr << "element \"" << this->Str(rElem) << "\" not member of set " << this->Name() << std::endl;
    throw Exception("TaBaseSet::Attribute(elem,attr)", errstr.str(), 60);
  }
#endif
  if(!AttributeTry(attr)) {
    std::stringstream errstr;
    errstr << "cannot cast attribute " << std::endl;
    throw Exception("TaBaseSet::Attribute(elem,attr)", errstr.str(), 63);
  }
  Detach();
  DoAttributep(rElem, dynamic_cast<const Attr*>(&attr)); 
}

// set attribute wrapper
TEMP void THIS::AttributeTry(const T& rElem, const Type& attr) {
  if(!AttributeTry(attr)) return;
#ifdef FAUDES_CHECKED
  if (!THIS::Exists(rElem)) {
    std::stringstream errstr;
    errstr << "element \"" << this->Str(rElem) << "\" not member of set " << this->Name() << std::endl;
    throw Exception("TaBaseSet::Attribute(elem)", errstr.str(), 60);
  }
#endif
  Detach();
  DoAttributep(rElem, dynamic_cast<const Attr*>(&attr)); 
}

// set attribute wrapper
TEMP void THIS::Attribute(const T& rElem, const Attr& attr) {
#ifdef FAUDES_CHECKED
  if (!THIS::Exists(rElem)) {
    std::stringstream errstr;
    errstr << "element \"" << this->Str(rElem) << "\" not member of set " << this->Name() << std::endl;
    throw Exception("TaBaseSet::Attribute(elem)", errstr.str(), 60);
  }
#endif
  Detach();
  DoAttributep(rElem, &attr); 
}


// set attribute wrapper
TEMP void THIS::Attributes(const TBaseSet<T,Cmp>& rOtherSet) {
  if(!AttributeTry(rOtherSet.Attribute())) {
    std::stringstream errstr;
    errstr << "cannot cast attribute " << std::endl;
    throw Exception("TaBaseSet::Attribute(elem,attr)", errstr.str(), 63);
  }
  Detach();
  iterator it1 = BASE::pSet->begin();
  iterator it2 = rOtherSet.pSet->begin();
  while ((it1 != BASE::pSet->end()) && (it2 != rOtherSet.pSet->end())) {
    if (*it1 < *it2) {
      ++it1;
    }
    else if (*it1 == *it2) {
      DoAttributep(*it1,&rOtherSet.Attribute(*it2)); 
      ++it1;
      ++it2;
    }
    else { // (*it1 > *it2)
      ++it2;
    }
  }
}

// set attribute wrapper
TEMP void THIS::Attributes(const TaBaseSet& rOtherSet) {
  Detach();
  iterator it1 = BASE::pSet->begin();
  iterator it2 = rOtherSet.pSet->begin();
  while ((it1 != BASE::pSet->end()) && (it2 != rOtherSet.pSet->end())) {
    if (*it1 < *it2) {
      ++it1;
    }
    else if (*it1 == *it2) {
      DoAttributep(*it1,&rOtherSet.Attribute(*it2)); 
      ++it1;
      ++it2;
    }
    else { // (*it1 > *it2)
      ++it2;
    }
  }
}

// clr attributes wrapper  
TEMP void THIS::ClrAttribute(const T& rElem) {
  Detach();
  DoAttributep(rElem,(const Attr*) 0);
}

// Implement attributes: clear
TEMP void THIS::ClearAttributes(void) {
  // bail out if there are no attributes anyway
  if(BASE::pAttributes->size()==0) return;
  // detach (incl empty attributes if used to share)
  BASE::Detach();
  // clear if we (still) own attributes FIXME ...DOUBLE CHECK
  if(BASE::mpAttributes) {
    aiterator ait=BASE::mpAttributes->begin();
    for(;ait!=BASE::mpAttributes->end();++ait)
      delete ait->second;
    BASE::mpAttributes->clear();
  }
}

// Implement attributes: equality
TEMP bool THIS::EqualAttributes(const TBaseSet<T,Cmp>& rOtherSet) const {
  FD_DC("TaBaseSet::EqualAttributes(TBaseSet)");
  // true, if we share attubute data
  FD_DC("TaBaseSet::EqualAttributes(TBaseSet): 1");
  if(BASE::pAttributes==rOtherSet.pAttributes)
    return true;
  // false, if type does not match
  FD_DC("TaBaseSet::EqualAttributes(TBaseSet): 2");
  if(typeid(*rOtherSet.Attributep())!=typeid(*this->Attributep())) 
    return false;
  // true if there are no attributes
  FD_DC("TaBaseSet::EqualAttributes(TBaseSet): 3");
  if(rOtherSet.AttributesSize()==0)
  if(this->AttributesSize()==0)
    return true;
  // figure shared elements
  aiterator ait1 = BASE::pAttributes->begin();
  aiterator ait2 = rOtherSet.pAttributes->begin();
  while ((ait1 != BASE::pAttributes->end()) && (ait2 != rOtherSet.pAttributes->end())) {
    if (ait1->first < ait2->first) {
      ++ait1;
    }
    else if (ait1->first == ait2->first) {
      FD_DC("TaBaseSet::EqualAttributes(TBaseSet): cmp " << ait1->second->ToString() 
	      << " vs " << ait2->second->ToString());
      if( ! ait1->second->Equal(*ait2->second)) return false;
      ++ait1;
      ++ait2;
    }
    else { // (*it1 > *it2)
      ++ait2;
    }
  }
  // passed
  FD_DC("TaBaseSet::EqualAttributes(TBaseSet): pass");
  return true;
}

// Implement attributes: equality
TEMP bool THIS::EqualAttributes(const TaBaseSet& rOtherSet) const {
  FD_DC("TaBaseSet::EqualAttributes(TaBaseSet)");
  // true, if we share attubute data
  if(BASE::pAttributes==rOtherSet.pAttributes)
    return true;
  // true if there are no attributes
  if(rOtherSet.AttributesSize()==0)
  if(this->AttributesSize()==0)
    return true;
  // figure shared elements
  aiterator ait1 = BASE::pAttributes->begin();
  aiterator ait2 = rOtherSet.pAttributes->begin();
  while ((ait1 != BASE::pAttributes->end()) && (ait2 != rOtherSet.pAttributes->end())) {
    if (ait1->first < ait2->first) {
      ++ait1;
    }
    else if (ait1->first == ait2->first) {
      if( *(static_cast<const Attr*>(ait1->second)) != 
	  *(static_cast<const Attr*>(ait2->second)) ) return false;
      ++ait1;
      ++ait2;
    }
    else { // (*it1 > *it2)
      ++ait2;
    }
  }
  // passed
  return true;
}

// Implement attributes: get pointer
TEMP const Attr* THIS::DoAttributep(const T& rElem) const {
  const_aiterator ait;
  ait=BASE::pAttributes->find(rElem);
  if(ait==BASE::pAttributes->end()) return NULL;
  return static_cast<const Attr*>(ait->second);
} 

// Implement attributes: get pointer (must detach)
TEMP Attr* THIS::DoAttributep(const T& rElem) {
  FD_DC("TaBaseSet::DoAttributep(Elem)");
#ifdef FAUDES_DEBUG_CODE
  if(BASE::pAttributes!=BASE::mpAttributes) {
    FD_ERR("TaBaseSet::DoAttributep(Elem): attributes not detached");
    abort();
  }
#endif
  aiterator ait;
  ait=BASE::pAttributes->find(rElem);
  // explicit default
  if( ait==BASE::pAttributes->end() ) {
    Attr* attr = new Attr();
    attr->Assign(*mpDefAttribute);
    (*BASE::pAttributes)[rElem]=attr;
    return attr;
  }
  return static_cast<Attr*>(ait->second);
} 

// Implement attributes: set  (must detach)
TEMP void THIS::DoAttributep(const T& rElem, const Type* pAttr) {
  FD_DC("TaBaseSet::DoAttributep([v] " << this->Str(rElem) << ", ...)");
#ifdef FAUDES_DEBUG_CODE
  if(BASE::pAttributes!=BASE::mpAttributes) {
    FD_ERR("TaBaseSet::DoAttributep([v] set): attributes not detached");
    abort();
  }
#endif
  // find element in map  
  aiterator ait;
  AttributeVoid* oldattr=NULL;
  const AttributeVoid* newattr=dynamic_cast<const AttributeVoid*>(pAttr);
  ait=BASE::pAttributes->find(rElem);
  if(ait!=BASE::pAttributes->end() ) 
    oldattr=ait->second;
  // set to default, case 1
  if(newattr==NULL) {
    FD_DC("TaBaseSet::DoAttributep([v] " << this->Str(rElem) << ", ...): default 1");
    if(oldattr==NULL) return;
    delete oldattr;
    BASE::pAttributes->erase(ait);
    return;
  }  
  // set to default, case b
  if(newattr->IsDefault()) {
    FD_DC("TaBaseSet::DoAttributep([v] " << this->Str(rElem) << ", ...): default 2");
    if(oldattr==NULL) return;
    delete oldattr;
    BASE::pAttributes->erase(ait);
    return;
  }  
  FD_DC("TaBaseSet::DoAttributep([v] " << this->Str(rElem) << ", ...): " << newattr->ToString());
  // prepare attribute and set
  if(oldattr==NULL) {
    Attr* attr = new Attr();
    attr->Assign(*newattr);
    (*BASE::pAttributes)[rElem]=attr;
    return;
  }
  // plain set     
  oldattr->Assign(*pAttr);
}

// Implement attributes: set (must detach)
TEMP void THIS::DoAttributep(const T& rElem, const Attr* pAttr) {
  FD_DC("TaBaseSet::DoAttributep([a] " << this->Str(rElem) << ", ...)");
#ifdef FAUDES_DEBUG_CODE
  if(BASE::pAttributes!=BASE::mpAttributes) {
    FD_ERR("TaBaseSet::DoAttributep([a] set): attributes not detached");
    abort();
  }
#endif
  // find element in map  
  aiterator ait;
  AttributeVoid* oldattr=NULL;
  ait=BASE::pAttributes->find(rElem);
  if(ait!=BASE::pAttributes->end() ) 
    oldattr=ait->second;
  // set to default, case a
  if(pAttr==NULL) {
    if(oldattr==NULL) return;
    delete oldattr;
    BASE::pAttributes->erase(ait);
    return;
  }  
  // set to default, case b
  if(pAttr->IsDefault()) {
    if(oldattr==NULL) return;
    delete oldattr;
    BASE::pAttributes->erase(ait);
    return;
  }  
  // prepare attribute and set
  if(oldattr==NULL) {
    Attr* attr = new Attr();
    *attr=*pAttr;
    (*BASE::pAttributes)[rElem]=attr;
    return;
  }
  // plain set     
  *static_cast<Attr*>(oldattr)=*pAttr;
}

// Implement attributes
TEMP Idx THIS::AttributesSize(void) const {
  return BASE::pAttributes->size();
}

/* undefine local shortcuts */
#undef THIS
#undef TEMP
#undef BASE


} // namespace faudes

#endif 
