//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.90. August 3, 2009.
//LIC// 
//LIC// Copyright (C) 2006-2009 Matthias Heil and Andrew Hazel
//LIC// 
//LIC// This library is free software; you can redistribute it and/or
//LIC// modify it under the terms of the GNU Lesser General Public
//LIC// License as published by the Free Software Foundation; either
//LIC// version 2.1 of the License, or (at your option) any later version.
//LIC// 
//LIC// This library is distributed in the hope that it will be useful,
//LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
//LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//LIC// Lesser General Public License for more details.
//LIC// 
//LIC// You should have received a copy of the GNU Lesser General Public
//LIC// License along with this library; if not, write to the Free Software
//LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
//LIC// 02110-1301  USA.
//LIC// 
//LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
//LIC// 
//LIC//====================================================================
// Header file for elements that allow the imposition of a "constant volume"
// constraint in free surface problems.

//Include guards, to prevent multiple includes
#ifndef CONSTRAINED_FLUID_VOLUME_ELEMENTS_HEADER
#define CONSTRAINED_FLUID_VOLUME_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

//OOMPH-LIB headers
#include "generic/Qelements.h"
#include "generic/spines.h"

using namespace std;

//-------------------------------------------
// hierher This is still (tidy up) work 
// in progress. Need to create versions
// for axisymmetric and cartesian 3D
// bulk equations, as well as spine
// version for all of these (resulting
// in six elements in total). These
// will gradually replace the fix_*.h
// files that currently live in 
// various demo driver codes. 
//-------------------------------------------

namespace oomph
{

//==========================================================================
/// A class that is used to implement the constraint that the fluid volume
/// in a region bounded by associated FaceElements of type 
/// LineVolumeConstraintElement (attached, e.g., to the
/// mesh boundaries that enclose a bubble) must take a specific value. 
/// This GeneralisedElement is used only to store the desired volume and
/// a pointer to the (usually pressure) freedom that must be traded 
/// for the volume constraint.
//=========================================================================
class VolumeConstraintElement : public GeneralisedElement
 {
  private:

 /// Pointer to the desired value of the volume
 double *Prescribed_volume_pt;
  
 /// \short The Data that contains the traded pressure is stored
 /// as external or internal Data for the element. What is its index
 /// in these containers?
 unsigned External_or_internal_data_index_of_traded_pressure;

 /// \short The Data that contains the traded pressure is stored
 /// as external or internal Data for the element. Which one?
 bool Traded_pressure_stored_as_internal_data;

 /// Index of the value in traded pressure data that corresponds to the
 /// traded pressure
 unsigned Index_of_traded_pressure_value; 
 
 /// \short The local eqn number for the traded pressure
 inline int ptraded_local_eqn()
  {
   if (Traded_pressure_stored_as_internal_data)
    { 
     return 
      this->internal_local_eqn(
       External_or_internal_data_index_of_traded_pressure,
       Index_of_traded_pressure_value);
    }
   else 
    {
     return 
      this->external_local_eqn(
       External_or_internal_data_index_of_traded_pressure,
       Index_of_traded_pressure_value);     
    }
  }
 
 
 /// \short Fill in the residuals for the volume constraint
 void fill_in_generic_contribution_to_residuals_volume_constraint(
  Vector<double> &residuals)
 {
  // Note: This element can only be used with the associated 
  // hierher [fill in name] elements which compute the actual
  // enclosed volume; here we only add the contribution to the
  // residual; everything else, incl. the derivatives of this
  // residual w.r.t. the nodal positions of the hierher [fill in name]
  // elements is handled by them
  int local_eqn = ptraded_local_eqn();
  if(local_eqn >= 0)
   {
    residuals[local_eqn] += *Prescribed_volume_pt;
   }
 }
  
   public:
 
 /// \short Constructor: Pass pointer to target volume. "Pressure" value that
 /// "traded" for the volume contraint is created internally (as a Data
 /// item with a single pressure value)
 VolumeConstraintElement(double* prescribed_volume_pt)
  {
   // Store pointer to prescribed volume
   Prescribed_volume_pt = prescribed_volume_pt;
      
   // Create data, add as internal data and record the index
   // (gets deleted automatically in destructor of GeneralisedElement)
   External_or_internal_data_index_of_traded_pressure=
    add_internal_data(new Data(1)); 

   // We've created it as internal data
   Traded_pressure_stored_as_internal_data=true;
   
   // ...and stored the "traded pressure" value as first value
   Index_of_traded_pressure_value=0; 
  } 
 
 

 /// \short Constructor: Pass pointer to target volume, pointer to Data
 /// item whose value specified by index_of_traded_pressure represents
 /// the "Pressure" value that "traded" for the volume contraint.
 /// The Data is stored as external Data for this element.
 VolumeConstraintElement(double* prescribed_volume_pt,
                         Data* p_traded_data_pt,
                         const unsigned& index_of_traded_pressure)
  {
   // Store pointer to prescribed volume
   Prescribed_volume_pt = prescribed_volume_pt;
   
   // Add as external data and record the index
   External_or_internal_data_index_of_traded_pressure=
    add_external_data(p_traded_data_pt);
   
   // We've created it as external data
   Traded_pressure_stored_as_internal_data=false;
   
   // Record index
   Index_of_traded_pressure_value=index_of_traded_pressure;
  } 
 
 /// Access to Data that contains the traded pressure
 Data* p_traded_data_pt()
  {
   if (Traded_pressure_stored_as_internal_data)
    { 
     return internal_data_pt(
      External_or_internal_data_index_of_traded_pressure);
    }
   else 
    {
     return external_data_pt(
      External_or_internal_data_index_of_traded_pressure);
    }
  }

 /// The traded pressure value
 double p_traded()
  {
   return p_traded_data_pt()->value(Index_of_traded_pressure_value);
  }
 
 /// Which value in traded pressure Data corresponds to the traded pressure?
 unsigned index_of_traded_pressure()
 {
  return Index_of_traded_pressure_value;
 }
 
 
 /// \short Fill in the residuals for the volume constraint
 void fill_in_contribution_to_residuals( Vector<double> &residuals)
 {
  this->fill_in_generic_contribution_to_residuals_volume_constraint(
   residuals);
 }
 
 
 /// \short Fill in the residuals and jacobian for the volume constraint
 void fill_in_contribution_to_jacobian(Vector<double> &residuals,
                                       DenseMatrix<double> &jacobian)
 {
  //No contribution to jacobian; see comment in that function
  this->fill_in_generic_contribution_to_residuals_volume_constraint(
   residuals);
 }
 
 /// \short Fill in the residuals, jacobian and mass matrix for the volume
 /// constraint
  void fill_in_contribution_to_jacobian_and_mass_matrix(
    Vector<double> &residuals,
    DenseMatrix<double> &jacobian,
    DenseMatrix<double> &mass_matrix)
    {
     //No contribution to jacobian or mass matrix; see comment in that function
     this->fill_in_generic_contribution_to_residuals_volume_constraint(
      residuals);
    }



}; 


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////



//=======================================================================
/// Base class for interface elements that allow the application of 
/// a volume constraint on the region bounded by these elements. The
/// elements must be used together with the associated
/// VolumeConstraintElement which stores the value of the
/// target volume. To enforce that a fluid volume has a
/// certain volume, attach these elements to all faces of the
/// bulk fluid elements (of type ELEMENT) that bound that region
/// and specify the "pressure" value that is traded for the constraint.
//=======================================================================
template<class ELEMENT>
 class VolumeConstraintBoundingElement : 
 public virtual FaceGeometry<ELEMENT>, 
  public virtual FaceElement
{
  protected:

 /// \short The Data that contains the traded pressure is stored
 /// as external Data for this element. Which external Data item is it?
 unsigned External_data_number_of_traded_pressure;
 
 /// Index of the value in traded pressure data that corresponds to the
 /// traded pressure
 unsigned Index_of_traded_pressure_value;

 /// \short The local eqn number for the traded pressure
 inline int ptraded_local_eqn()
  {
   return this->external_local_eqn(External_data_number_of_traded_pressure,
                                   Index_of_traded_pressure_value);
  }

 /// \short Helper function to fill in contributions to residuals
 /// (remember that part of the residual is added by the the 
 /// associated VolumeConstraintElement). This is dimension/geometry
 /// specific and must be implemented in derived classes for
 /// 1D line, 2D surface and axisymmetric fluid boundaries
 virtual void fill_in_generic_residual_contribution_volume_constraint(
  Vector<double> &residuals)=0;
 
 
  public:
 
 
 /// \short Contructor: Specify bulk element and index of face to which
 /// this face element is to be attached and the VolumeConstraintElement
 /// that is in charge of the "traded pressure"
 VolumeConstraintBoundingElement(FiniteElement* const &element_pt, 
                                 const int &face_index,
                                 VolumeConstraintElement* 
                                 vol_constraint_el_pt)
  {
   //Attach the geometrical information to the element, by
   //making the face element from the bulk element
   element_pt->build_face_element(face_index,this);
   
   // Add "traded" pressure data as external data to this element
   External_data_number_of_traded_pressure=
    this->add_external_data(vol_constraint_el_pt->p_traded_data_pt());
   
   // Which value corresponds to the traded pressure
   Index_of_traded_pressure_value=vol_constraint_el_pt->
    index_of_traded_pressure();
  }
 
 /// Fill in contribution to residuals and Jacobian
 void fill_in_contribution_to_residuals(Vector<double> &residuals)
 {
  //Call the generic routine
  this->fill_in_generic_residual_contribution_volume_constraint(residuals);
 }
 

 /// \short The "global" intrinsic coordinate of the element when
 /// viewed as part of a geometric object should be given by
 /// the FaceElement representation, by default
 double zeta_nodal(const unsigned &n, const unsigned &k,
                   const unsigned &i) const
 {return FaceElement::zeta_nodal(n,k,i);}
 
   
};

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////



//=======================================================================
/// One-dimensional interface elements that allow the application of 
/// a volume constraint on the region bounded by these elements. The
/// elements must be used together with the associated
/// VolumeConstraintElement which stores the value of the
/// target volume. To enforce that a fluid volume has a
/// certain volume, attach these elements to all faces of the
/// (2D cartesian) bulk fluid elements (of type ELEMENT) that bound that region
/// and specify the "pressure" value that is traded for the constraint.
//=======================================================================
template<class ELEMENT>
 class LineVolumeConstraintBoundingElement : public virtual 
 VolumeConstraintBoundingElement<ELEMENT>
{

  protected:
 
 /// \short Helper function to fill in contributions to residuals
 /// (remember that part of the residual is added by the
 /// the associated VolumeConstraintElement). This is specific for
 /// 1D line elements that bound 2D cartesian fluid elements.
 void  fill_in_generic_residual_contribution_volume_constraint(
  Vector<double> &residuals)
 {
  //Add in the volume constraint term if required
  int local_eqn=this->ptraded_local_eqn(); 
  if(local_eqn >=0)
   {    
    //Find out how many nodes there are
    unsigned n_node = this->nnode();
    
    //Set up memeory for the shape functions
    Shape psif(n_node);
    DShape dpsifds(n_node,1);
    
    //Set the value of n_intpt
    unsigned n_intpt = this->integral_pt()->nweight();
    
    //Storage for the local cooridinate
    Vector<double> s(1);
    
    //Loop over the integration points
    for(unsigned ipt=0;ipt<n_intpt;ipt++)
     {
      //Get the local coordinate at the integration point
      s[0] = this->integral_pt()->knot(ipt,0);
      
      //Get the integral weight
      double W = this->integral_pt()->weight(ipt);
      
      //Call the derivatives of the shape function at the knot point
      this->dshape_local_at_knot(ipt,psif,dpsifds);
      
      // Get position and tangent vector
      Vector<double> interpolated_t1(2,0.0);
      Vector<double> interpolated_x(2,0.0);
      for(unsigned l=0;l<n_node;l++)
       {
        //Loop over directional components
        for(unsigned i=0;i<2;i++)
         {
          interpolated_x[i] += this->nodal_position(l,i)*psif(l);
          interpolated_t1[i] += this->nodal_position(l,i)*dpsifds(l,0);
         }
       }
      
      //Calculate the length of the tangent Vector
      double tlength = interpolated_t1[0]*interpolated_t1[0] + 
       interpolated_t1[1]*interpolated_t1[1];
      
      //Set the Jacobian of the line element
      double J = sqrt(tlength);
      
      //Now calculate the normal Vector
      Vector<double> interpolated_n(2);
      this->outer_unit_normal(ipt,interpolated_n);
      
      // Assemble dot product
      double dot = 0.0;
      for(unsigned k=0;k<2;k++) 
       {
        dot += interpolated_x[k]*interpolated_n[k];
       }

      // Add to residual // hierher don't we need a sign here for cases
      // when the outer unit normal points into or out of the 
      // constrained volume?
      residuals[local_eqn] += 0.5*dot*W*J;
     }
   }
 }

 
  public:

 /// \short Contructor: Specify bulk element and index of face to which
 /// this face element is to be attached and the VolumeConstraintElement
 /// that is in charge of the "traded pressure"
 LineVolumeConstraintBoundingElement(FiniteElement* const &element_pt, 
                                          const int &face_index,
                                          VolumeConstraintElement* 
                                          vol_constraint_el_pt) :
  VolumeConstraintBoundingElement<ELEMENT>(element_pt, face_index,
                                           vol_constraint_el_pt) 
  {}
  
  

  

};


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////




//=======================================================================
/// One-dimensional interface elements that allow the application of 
/// a volume constraint on the region bounded by these elements. The
/// elements must be used together with the associated
/// VolumeConstraintElement which stores the value of the
/// target volume. To enforce that a fluid volume has a
/// certain volume, attach these elements to all faces of the
/// (2D cartesian) bulk fluid elements (of type ELEMENT) that bound that region
/// and specify the "pressure" value that is traded for the constraint.
//=======================================================================
 template<class ELEMENT>
 class LineVolumeConstraintBoundingSolidElement : public virtual 
 LineVolumeConstraintBoundingElement<ELEMENT>
{

  public:

 /// \short Contructor: Specify bulk element and index of face to which
 /// this face element is to be attached and the VolumeConstraintElement
 /// that is in charge of the "traded pressure"
 LineVolumeConstraintBoundingSolidElement(FiniteElement* const &element_pt, 
                                          const int &face_index,
                                          VolumeConstraintElement* 
                                          vol_constraint_el_pt) :
  VolumeConstraintBoundingElement<ELEMENT>(element_pt, face_index,
                                           vol_constraint_el_pt),
  LineVolumeConstraintBoundingElement<ELEMENT>(element_pt, face_index,
                                               vol_constraint_el_pt)
  {}
  


  /// Fill in contribution to residuals and Jacobian. This is specific
  /// to solid-based elements in which derivatives w.r.t. to nodal
  /// positions are evaluated by finite differencing
  void fill_in_contribution_to_jacobian(Vector<double> &residuals, 
                                        DenseMatrix<double> &jacobian)
  {
   //Call the generic routine
   this->fill_in_generic_residual_contribution_volume_constraint(residuals);
   
   // Shape derivatives
   //Call the generic finite difference routine for the solid variables
   this->fill_in_jacobian_from_solid_position_by_fd(jacobian);
   
// hierher This is for spines 
/*    { */
/*     //Call the generic routine to evaluate shape derivatives */
/*     this->fill_in_jacobian_from_geometric_data(jacobian); */
/*    } */
  }


};






/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

}
#endif






