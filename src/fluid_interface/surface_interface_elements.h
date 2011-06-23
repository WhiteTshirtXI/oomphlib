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
//Header file for specific (two-dimensional) fluid free surface elements

//Include guards, to prevent multiple includes
#ifndef OOMPH_SURFACE_INTERFACE_ELEMENTS_HEADER
#define OOMPH_SURFACE_INTERFACE_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

//OOMPH-LIB headers
#include "../generic/Qelements.h"
#include "../generic/spines.h"
#include "../generic/hijacked_elements.h"
#include "interface_elements.h"

namespace oomph
{

//======================================================================
/// Two-dimensional interface elements that are used with a spine mesh,
/// i.e. the mesh deformation is handled by Kistler & Scriven's "method
/// of spines". These elements are FaceElements are attached to 3D bulk
/// Fluid elements and the particular type of fluid element is passed 
/// as a template parameter to the element. It 
/// shouldn't matter whether the passed 
/// element is the underlying (fixed) element or the templated 
/// SpineElement<Element>.
/// Optionally, an external pressure may be specified, which must be
/// passed to the element as external data. If there is no such object,
/// the external pressure is assumed to be zero.
//======================================================================
 template<class ELEMENT>
  class SpineSurfaceFluidInterfaceElement : 
 public virtual Hijacked<SpineElement<FaceGeometry<ELEMENT> > >, 
  public virtual SurfaceFluidInterfaceElement 
  {
    private:
   
   /// \short In spine elements, the kinematic condition is the equation 
   /// used to determine the unknown spine heights. Overload the
   /// function accordingly
   int kinematic_local_eqn(const unsigned &n) 
   {return this->spine_local_eqn(n);}
   
   // hierher clarify this
   /// \short Hijacking the kinematic condition corresponds to hijacking the
   /// spine heights.
   void hijack_kinematic_conditions(const Vector<unsigned> &bulk_node_number)
   {
    //Loop over all the passed nodes
    for(Vector<unsigned>::const_iterator it=bulk_node_number.begin();
        it!=bulk_node_number.end();++it)
     {
      //Hijack the spine heights. (and delete the returned data object)
      delete this->hijack_nodal_spine_value(*it,0);
     }
   }
   
    public:
   
   
   /// \short Constructor, the arguments are a pointer to the  "bulk" element 
   /// and the index of the face to be created
   SpineSurfaceFluidInterfaceElement(FiniteElement* const &element_pt, 
                                     const int &face_index) : 
    Hijacked<SpineElement<FaceGeometry<ELEMENT> > >(), 
    SurfaceFluidInterfaceElement()
     {
#ifdef PARANOID
      //Check that the element is not refineable
      ELEMENT* elem_pt = new ELEMENT;
      if(dynamic_cast<RefineableElement*>(elem_pt))
       {
        //Issue a warning
        OomphLibWarning(
         "This interface element will not work correctly if nodes are hanging\n",
         "SpineSurfaceFludInterfaceElement::Constructor",
         OOMPH_EXCEPTION_LOCATION);
       }
#endif
      
      //Attach the geometrical information to the element, by
      //making the face element from the bulk element
      element_pt->build_face_element(face_index,this);
      
      //Find the index at which the velocity unknowns are stored 
      //from the bulk element
      ELEMENT* cast_element_pt = dynamic_cast<ELEMENT*>(element_pt);
      this->U_index_interface.resize(3);
      for(unsigned i=0;i<3;i++)
       {
        this->U_index_interface[i] = cast_element_pt->u_index_nst(i);
       }
     }
    
    /// Calculate the contribution to the residuals and the jacobian
    void fill_in_contribution_to_jacobian(Vector<double> &residuals, 
                                          DenseMatrix<double> &jacobian)
    {
     //Call the generic routine with the flag set to 1
     fill_in_generic_residual_contribution_interface(residuals,jacobian,1);

     //Call the generic routine to handle the shape derivatives
     this->fill_in_jacobian_from_geometric_data(jacobian);
    }
    
    /// Overload the output function
    void output(std::ostream &outfile) {FiniteElement::output(outfile);}
    
    /// Output the element
    void output(std::ostream &outfile, const unsigned &n_plot)
    {SurfaceFluidInterfaceElement::output(outfile,n_plot);}
    
    ///Overload the C-style output function
    void output(FILE* file_pt) {FiniteElement::output(file_pt);}
    
    ///C-style Output function
    void output(FILE* file_pt, const unsigned &n_plot)
    {SurfaceFluidInterfaceElement::output(file_pt,n_plot);}
    
    

    /// \short Create an "edge" element (here actually a 2D line element
    /// of type  SpineLineFluidInterfaceEdgeElement<ELEMENT> that allows
    /// the application of a contact angle boundary condition on the
    /// the specified face.
    virtual FluidInterfaceEdgeElement* make_edge_element(
     const int &face_index)
    {
     //Create a temporary pointer to the appropriate FaceElement
     SpineLineFluidInterfaceEdgeElement<ELEMENT> *face_el_pt = 
      new SpineLineFluidInterfaceEdgeElement<ELEMENT>;
     
     //Attach the geometrical information to the new element
     this->build_face_element(face_index,face_el_pt);
     
     //Set the value of the nbulk_value, the node is not resized
     //in this problem, so it will just be the actual nvalue
     face_el_pt->nbulk_value(0) = face_el_pt->node_pt(0)->nvalue();
     
     //Set of unique geometric data that is used to update the bulk,
     //but is not used to update the face
     std::set<Data*> unique_additional_geom_data;

     //Get all the geometric data for this (bulk) element
     this->assemble_set_of_all_geometric_data(unique_additional_geom_data);
     
     //Now assemble the set of geometric data for the face element
     std::set<Data*> unique_face_geom_data_pt;
     face_el_pt->assemble_set_of_all_geometric_data(unique_face_geom_data_pt);
     
     //Erase the face geometric data from the additional data
     for(std::set<Data*>::iterator it=unique_face_geom_data_pt.begin();
         it!=unique_face_geom_data_pt.end();++it)
      {unique_additional_geom_data.erase(*it);}
     
     //Finally add all unique additional data as external data
     for(std::set<Data*>::iterator it = unique_additional_geom_data.begin();
         it!= unique_additional_geom_data.end();++it)
      {
       face_el_pt->add_external_data(*it);
      }
     
     //Return the value of the pointer
     return face_el_pt;
    }
    

    
    /// \short Helper function to calculate the additional contributions
    /// to be added at each integration point. Empty as there's nothing
    /// to be done
    void add_additional_residual_contributions(
     Vector<double> &residuals, 
     DenseMatrix<double> &jacobian,
     const unsigned &flag,
     const Shape &psif,
     const DShape &dpsifds,
     const Vector<double> &interpolated_x, 
     const Vector<double> &interpolated_n, 
     const double &W, 
     const double &J){}
    
  };
 
 

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



//=======================================================================
/// Two-dimensional interface elements that are used when the mesh
/// deformation is handled by a set of equations that modify the nodal
/// positions. These elements are FaceElements attached to 3D bulk fluid 
/// elements and the fluid element is passed as a template parameter to 
/// the element.
/// Optionally an external pressure may be specified, which must be
/// passed to the element as external data. The default value of the external
/// pressure is zero.
//=======================================================================
 template<class ELEMENT>
  class ElasticSurfaceFluidInterfaceElement : 
 public virtual Hijacked<FaceGeometry<ELEMENT> >, 
  public SurfaceFluidInterfaceElement
  {
    private:
      
   /// \short ID of the Lagrange Lagrange multiplier (in the collection of 
   /// nodal values accomodated by resizing)
   unsigned Id;
   
   /// \short Equation number of the kinematic BC associated with node j.
   /// (This is the equation for the Lagrange multiplier) 
   int kinematic_local_eqn(const unsigned &j)
   {return this->nodal_local_eqn(j,Nbulk_value[j]);}
   
   
   /// hierher clarify this
   void hijack_kinematic_conditions(const Vector<unsigned> &bulk_node_number)
   {    
    //Loop over all the passed nodes
    for(Vector<unsigned>::const_iterator it=bulk_node_number.begin();
        it!=bulk_node_number.end();++it)
     {
      //Make sure that we delete the returned value
      delete this->hijack_nodal_value(*it,Nbulk_value[*it]); // hierher generalise?
     }
   }
   
    public:
   


   /// \short Constructor, pass a pointer to the bulk element and the face 
   /// index of the bulk element to which the element is to be attached to.
   /// The optional identifier can be used
   /// to distinguish the additional nodal value (Lagr mult) created by 
   /// this element from those created by other FaceElements.
   ElasticSurfaceFluidInterfaceElement(FiniteElement* const &element_pt, 
                                       const int &face_index,
                                       const unsigned &id=0) : 
    FaceGeometry<ELEMENT>(), SurfaceFluidInterfaceElement(), Id(id)
    {
#ifdef PARANOID
     //Check that the element is not refineable
     ELEMENT* elem_pt = new ELEMENT;
     if(dynamic_cast<RefineableElement*>(elem_pt))
      {
       //Issue a warning
       OomphLibWarning(
        "This interface element will not work correctly if nodes are hanging\n",
        "ElasticSurfaceFludInterfaceElement::Constructor",
        OOMPH_EXCEPTION_LOCATION);
      }
#endif
     
     //Attach the geometrical information to the element
     //This function also assigned nbulk_value from required_nvalue of the
     //bulk element
     element_pt->build_face_element(face_index,this);
     
     //Find the index at which the velocity unknowns are stored 
     //from the bulk element
     ELEMENT* cast_element_pt = dynamic_cast<ELEMENT*>(element_pt);
     this->U_index_interface.resize(3);
     for(unsigned i=0;i<3;i++)
      {
       this->U_index_interface[i] = cast_element_pt->u_index_nst(i);
      }
     
     //Read out the number of nodes on the face
     unsigned n_node_face = this->nnode();
     
     //Set the additional data values in the face
     //There is one additional values at each node --- the Lagrange multiplier
     Vector<unsigned> additional_data_values(n_node_face);
     for(unsigned i=0;i<n_node_face;i++) additional_data_values[i] = 1;
     
     // Now add storage for Lagrange multipliers and set the map containing 
     // the position of the first entry of this face element's 
     // additional values.
     add_additional_values(additional_data_values,id);
     
     // hierher kill
     // //Resize the data arrays accordingly 
     //resize_nodes(additional_data_values);
    }
    
    /// \short The "global" intrinsic coordinate of the element when 
    /// viewed as part of a geometric object should be given by 
    /// the FaceElement representation, by default
    double zeta_nodal(const unsigned &n, const unsigned &k,
                      const unsigned &i) const                              
    {return FaceElement::zeta_nodal(n,k,i);}  
    
    /// Return the lagrange multiplier at local node j
    double &lagrange(const unsigned &j)
     {
      // Get the index of the nodal value associated with Lagrange multiplier
      unsigned lagr_index=dynamic_cast<BoundaryNodeBase*>(node_pt(j))->
       index_of_first_value_assigned_by_face_element(Id);
      
      // hierher Andrew: Why dereference the value_pt?
      return *node_pt(j)->value_pt(lagr_index); 
      
      //return *node_pt(j)->value_pt(Nbulk_value[j]);  // hierher genearlise
     }
    

    /// Fill in contribution to residuals and Jacobian
    void fill_in_contribution_to_jacobian(Vector<double> &residuals, 
                                          DenseMatrix<double> &jacobian)
    {
     //Call the generic routine with the flag set to 1
     fill_in_generic_residual_contribution_interface(residuals,jacobian,1);

     //Call the generic finite difference routine for the solid variables
     this->fill_in_jacobian_from_solid_position_by_fd(jacobian);
    }
    
    /// Overload the output function
    void output(std::ostream &outfile) {FiniteElement::output(outfile);}
    
    /// Output the element
    void output(std::ostream &outfile, const unsigned &n_plot)
    {SurfaceFluidInterfaceElement::output(outfile,n_plot);}
    
    ///Overload the C-style output function
    void output(FILE* file_pt) {FiniteElement::output(file_pt);}
    
    ///C-style Output function
    void output(FILE* file_pt, const unsigned &n_plot)
    {SurfaceFluidInterfaceElement::output(file_pt,n_plot);}
    
    
    /// \short Helper function to calculate the additional contributions
    /// to be added at each integration point. This deals with 
    /// Lagrange multiplier contribution
    void add_additional_residual_contributions(
     Vector<double> &residuals, 
     DenseMatrix<double> &jacobian,
     const unsigned &flag,
     const Shape &psif,
     const DShape &dpsifds,
     const Vector<double> &interpolated_x, 
     const Vector<double> &interpolated_n, 
     const double &W, 
     const double &J)
    {
     //Assemble Lagrange multiplier by loop over the shape functions
     unsigned n_node = this->nnode();
     double interpolated_lagrange = 0.0;
     for(unsigned l=0;l<n_node;l++)
      {
       //Note same shape functions used for lagrange multiplier field
       interpolated_lagrange += lagrange(l)*psif(l);
      }
     
     int local_eqn=0, local_unknown = 0;
     //Loop over the shape functions
     for(unsigned l=0;l<n_node;l++)
      {
       //Loop over the directions
       for(unsigned i=0;i<3;i++)
        {
         //Now using the same shape functions for the elastic equations,
         //so we can stay in the loop
         local_eqn = this->position_local_eqn(l,0,i);
         if(local_eqn >= 0)
          {
           //Add in the Lagrange multiplier contribution
           //(The normal vector includes the appropriate area contribution)
           // hierher check statement in brackets -- is interpolated_n
           // not normalised?
           residuals[local_eqn] -= 
            interpolated_lagrange*interpolated_n[i]*psif(l)*W;
           
           //Do the Jacobian calculation
           if(flag)
            {
             //Loop over the nodes 
             for(unsigned l2=0;l2<n_node;l2++)
              {
               // Dependence on solid positions will be handled by FDs
               //That leaves the Lagrange multiplier contribution
               local_unknown = kinematic_local_eqn(l2);
               if(local_unknown >= 0)
                {
                 jacobian(local_eqn,local_unknown) -=
                  psif(l2)*interpolated_n[i]*psif(l)*W;
                }
              }
            } //End of Jacobian calculation
          } 
        } 
       
      }
    }
    
    
 
    /// \short Create an "edge" element (here actually a 2D line element
    /// of type ElasticLineFluidInterfaceEdgeElement<ELEMENT> that allows
    /// the application of a contact angle boundary condition on the
    /// the specified face. 
    virtual FluidInterfaceEdgeElement* make_edge_element(const int &face_index)
    {
     //Create a temporary pointer to the appropriate FaceElement
     ElasticLineFluidInterfaceEdgeElement<ELEMENT> *face_el_pt = 
      new ElasticLineFluidInterfaceEdgeElement<ELEMENT>;
     
     //Attach the geometrical information to the new element
     this->build_face_element(face_index,face_el_pt);
     
     //Find the nodes
     std::set<SolidNode*> set_of_solid_nodes;
     unsigned n_node = this->nnode();
     for(unsigned n=0;n<n_node;n++)
      {
       set_of_solid_nodes.insert(static_cast<SolidNode*>(this->node_pt(n)));
      }
     
     //Delete the nodes from the face
     n_node = face_el_pt->nnode();
     for(unsigned n=0;n<n_node;n++)
      {
       //Set the value of the nbulk_value, from the present element
       face_el_pt->nbulk_value(n) = 
        this->nbulk_value(face_el_pt->bulk_node_number(n));
       
       //Now delete the nodes from the set
       set_of_solid_nodes.erase(static_cast<SolidNode*>(
                                 face_el_pt->node_pt(n)));
      }
     
     //Now add these as external data
     for(std::set<SolidNode*>::iterator it=set_of_solid_nodes.begin();
         it!=set_of_solid_nodes.end();++it)
      {
       face_el_pt->add_external_data((*it)->variable_position_pt());
      }
     
     
     //Return the value of the pointer
     return face_el_pt;
    }
    
  };
 
}

#endif






