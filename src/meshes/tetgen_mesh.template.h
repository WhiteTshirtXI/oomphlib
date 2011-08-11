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

#ifndef OOMPH_TETGEN_MESH_HEADER
#define OOMPH_TETGEN_MESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif


#ifdef OOMPH_HAS_MPI
//mpi headers
#include "mpi.h"
#endif

#include "../generic/tetgen_scaffold_mesh.h"
#include "../generic/tet_mesh.h"

namespace oomph
{

//=========start of TetgenMesh class======================================
/// \short  Unstructured tet mesh based on output from Tetgen:
/// http://tetgen.berlios.de/
//========================================================================
template <class ELEMENT>
class TetgenMesh : public virtual TetMeshBase
{

public:

 /// \short Empty constructor
 TetgenMesh()
  {
  }

 /// \short Constructor with the input files
 TetgenMesh(const std::string& node_file_name,
            const std::string& element_file_name,
            const std::string& face_file_name,
            TimeStepper* time_stepper_pt=
            &Mesh::Default_TimeStepper,
            const bool &use_attributes=false)
              
  {
   //Store the attributes
   Use_attributes = use_attributes;

   // Store timestepper used to build elements
   Time_stepper_pt = time_stepper_pt;

   // Build scaffold
   Tmp_mesh_pt= new 
    TetgenScaffoldMesh(node_file_name,element_file_name,face_file_name);
 
   // Convert mesh from scaffold to actual mesh
   build_from_scaffold(time_stepper_pt,use_attributes);

   // Kill the scaffold
   delete Tmp_mesh_pt;
   Tmp_mesh_pt=0;
  }


 /// \short Constructor with tetgenio data structure
 TetgenMesh(tetgenio& tetgen_data,
            TimeStepper* time_stepper_pt=
            &Mesh::Default_TimeStepper,
            const bool &use_attributes=false)
              
  {
   //Store the attributes
   Use_attributes = use_attributes;

   // Store timestepper used to build elements
   Time_stepper_pt = time_stepper_pt;

   //We do not have a tetgenio representation
   Tetgenio_exists = false;
   Tetgenio_pt = 0;

   // Build scaffold
   Tmp_mesh_pt= new 
    TetgenScaffoldMesh(tetgen_data);
 
   // Convert mesh from scaffold to actual mesh
   build_from_scaffold(time_stepper_pt,use_attributes);

   // Kill the scaffold
   delete Tmp_mesh_pt;
   Tmp_mesh_pt=0;
  }


 /// \short Constructor with the input files. Setting the boolean
 /// flag to true splits "corner" elements, i.e. elements that
 /// that have at least three faces on a domain boundary. The 
 /// relevant elements are split without introducing hanging
 /// nodes so the sons have a "worse" shape than their fathers.
 /// However, this step avoids otherwise-hard-to-diagnose
 /// problems in fluids problems where the application of
 /// boundary conditions at such "corner" elements can
 /// overconstrain the solution. 
 TetgenMesh(const std::string& node_file_name,
            const std::string& element_file_name,
            const std::string& face_file_name,
            const bool& split_corner_elements,
            TimeStepper* time_stepper_pt=
            &Mesh::Default_TimeStepper,
            const bool &use_attributes=false) : Outer_boundary_pt(0)
              
  {
   //Throw an error if you try to split corner elements
   //and use attributes
   if(split_corner_elements && use_attributes)
    {
     std::ostringstream error_stream;
     error_stream <<
      "Using region attributes and split_corner_elements simultaneously\n"
                  << 
      "is not guaranteed to work. The elements adjacent to boundaries\n"
                  << 
      "accessed by region will not be set up correctly.\n"
                  << 
      "\n Please fix this!\n\n";
     
     throw OomphLibError(error_stream.str(),
                         "TetgenMesh::TetgenMesh()",
                         OOMPH_EXCEPTION_LOCATION);
    }

   

   //Store the attributes
   Use_attributes = use_attributes;

   // Store timestepper used to build elements
   Time_stepper_pt = time_stepper_pt;

   // We do not have a tetgenio representation
   this->Tetgenio_exists = false;;
   this->Tetgenio_pt = 0;

   // Build scaffold
   Tmp_mesh_pt= new 
    TetgenScaffoldMesh(node_file_name,element_file_name,face_file_name);
 
   // Convert mesh from scaffold to actual mesh
   build_from_scaffold(time_stepper_pt,use_attributes);

   // Kill the scaffold
   delete Tmp_mesh_pt;
   Tmp_mesh_pt=0;

   // Split corner elements
   if (split_corner_elements)
    {
     split_elements_in_corners();
    }
  }

 /// \short Constructor with tetgen data structure Setting the boolean
 /// flag to true splits "corner" elements, i.e. elements that
 /// that have at least three faces on a domain boundary. The 
 /// relevant elements are split without introducing hanging
 /// nodes so the sons have a "worse" shape than their fathers.
 /// However, this step avoids otherwise-hard-to-diagnose
 /// problems in fluids problems where the application of
 /// boundary conditions at such "corner" elements can
 /// overconstrain the solution. 
 TetgenMesh(tetgenio &tetgen_data,
            const bool& split_corner_elements,
            TimeStepper* time_stepper_pt=
            &Mesh::Default_TimeStepper,
            const bool &use_attributes=false) : Outer_boundary_pt(0)
              
  {
   //Throw an error if you try to split corner elements
   //and use attributes
   if(split_corner_elements && use_attributes)
    {
     std::ostringstream error_stream;
     error_stream <<
      "Using region attributes and split_corner_elements simultaneously\n"
                  << 
      "is not guaranteed to work. The elements adjacent to boundaries\n"
                  << 
      "accessed by region will not be set up correctly.\n"
                  << 
      "\n Please fix this!\n\n";
     
     throw OomphLibError(error_stream.str(),
                         "TetgenMesh::TetgenMesh()",
                         OOMPH_EXCEPTION_LOCATION);
    }
   

   //Store the attributes
   Use_attributes = use_attributes;

   // Store timestepper used to build elements
   Time_stepper_pt = time_stepper_pt;

   // We do not have a tetgenio representation
   this->Tetgenio_exists = false;;
   this->Tetgenio_pt = 0;

   // Build scaffold
   Tmp_mesh_pt= new TetgenScaffoldMesh(tetgen_data);
 
   // Convert mesh from scaffold to actual mesh
   build_from_scaffold(time_stepper_pt,use_attributes);

   // Kill the scaffold
   delete Tmp_mesh_pt;
   Tmp_mesh_pt=0;

   // Split corner elements
   if (split_corner_elements)
    {
     split_elements_in_corners();
    }
  }


  /// \short Build mesh, based on a TetgenMeshClosedSurface that specifies
  /// the outer boundary of the domain and any number of internal
  /// closed curves, also specified by TriangleMeshClosedSurfaces.
  /// Also specify target area for uniform element size.
  TetgenMesh(TetgenMeshFacetedSurface* const &outer_boundary_pt,
             Vector<TetgenMeshFacetedSurface*>& 
             internal_closed_surface_pt,
             const double &element_volume,
             TimeStepper* time_stepper_pt=
             &Mesh::Default_TimeStepper,
             const bool &use_attributes=false) 
   {
    //Store the attributes
    Use_attributes = use_attributes;
    
    // Store timestepper used to build elements
    Time_stepper_pt = time_stepper_pt;
    
    //Store the boundary
    Outer_boundary_pt = outer_boundary_pt;

    //Store the internal boundary
    Internal_surface_pt = internal_closed_surface_pt;

    //Tetgen data structure for the input and output
    tetgenio in;

    this->build_tetgenio(outer_boundary_pt,
                         internal_closed_surface_pt,
                         in);

    //Now tetrahedralise
    std::stringstream input_string;
    input_string << "pAa" << element_volume;
    char tetswitches[100];
    sprintf(tetswitches,"%s",input_string.str().c_str());

    //Make a new tetgen representation
    this->Tetgenio_exists = true;
    Tetgenio_pt = new tetgenio;

    tetrahedralize(tetswitches, &in, this->Tetgenio_pt);

    // Build scaffold
    Tmp_mesh_pt= new TetgenScaffoldMesh(*this->Tetgenio_pt);
    
    // Convert mesh from scaffold to actual mesh
    build_from_scaffold(time_stepper_pt,use_attributes);
    
    // Kill the scaffold
    delete Tmp_mesh_pt;
    Tmp_mesh_pt=0;
   }
    
  ///\short Build tetgenio object from the TetgenMeshClosedSurfaces
  void build_tetgenio(TetgenMeshFacetedSurface* const &outer_boundary_pt,
                      Vector<TetgenMeshFacetedSurface*> 
                      &internal_closed_surface_pt,
                      tetgenio& tetgen_io)
  {
   //Pointer to Tetgen facet
   tetgenio::facet *f;
   //Pointer to Tetgen polygon
   tetgenio::polygon *p;

   //Start all indices from 1 (it's a choice and we've made it
   tetgen_io.firstnumber = 1;
   ///ALH: This may not be needed
   tetgen_io.useindex = true;

   //Find the number of internal surfaces
   const unsigned n_internal = internal_closed_surface_pt.size();

   //Find the number of points on the outer boundary
   const unsigned n_outer_vertex = outer_boundary_pt->nvertex();
   tetgen_io.numberofpoints = n_outer_vertex;

   //Find the number of points on the inner boundaries and add to the totals
   Vector<unsigned> n_internal_vertex(n_internal);
   Vector<unsigned> internal_vertex_offset(n_internal);
   for(unsigned h=0;h<n_internal;++h)
    {
     n_internal_vertex[h] = internal_closed_surface_pt[h]->nvertex();
     internal_vertex_offset[h] = tetgen_io.numberofpoints;
     tetgen_io.numberofpoints += n_internal_vertex[h];
    }
   
   //Read the data into the point list
   tetgen_io.pointlist = new double[tetgen_io.numberofpoints * 3];
   tetgen_io.pointmarkerlist = new int[tetgen_io.numberofpoints];
   unsigned counter=0;
   for(unsigned n=0;n<n_outer_vertex;++n)
    {
     for(unsigned i=0;i<3;++i)
      {
       tetgen_io.pointlist[counter] = 
        outer_boundary_pt->vertex_coordinate(n,i);
       ++counter;
      }
    }
   for(unsigned h=0;h<n_internal;++h)
    {
     const unsigned n_inner = n_internal_vertex[h];
     for(unsigned n=0;n<n_inner;++n)
      {
       for(unsigned i=0;i<3;++i)
        {
         tetgen_io.pointlist[counter] = 
          internal_closed_surface_pt[h]->vertex_coordinate(n,i);
         ++counter;
        }
      }
    }


   //Set up the pointmarkers
   counter=0;
   for(unsigned n=0;n<n_outer_vertex;++n)
    {
     tetgen_io.pointmarkerlist[counter] = 
      outer_boundary_pt->vertex_boundary_id(n);
     ++counter;
    }
   for(unsigned h=0;h<n_internal;++h)
    {
     const unsigned n_inner = n_internal_vertex[h];
     for(unsigned n=0;n<n_inner;++n)
      {
       tetgen_io.pointmarkerlist[counter] = 
        internal_closed_surface_pt[h]->vertex_boundary_id(n);
       ++counter;
      }
    }


   //Now the facets
   unsigned n_outer_facet = outer_boundary_pt->nfacet();
   tetgen_io.numberoffacets = n_outer_facet;
   Vector<unsigned> n_inner_facet(n_internal);
   for(unsigned h=0;h<n_internal;++h)
    {
     n_inner_facet[h] = internal_closed_surface_pt[h]->nfacet();
     tetgen_io.numberoffacets += n_inner_facet[h];
    }

   tetgen_io.facetlist = new tetgenio::facet[tetgen_io.numberoffacets];
   tetgen_io.facetmarkerlist = new int[tetgen_io.numberoffacets];


   counter=0;
   for(unsigned n=0;n<n_outer_facet;++n)
    {
     //Set pointer to facet
     f = &tetgen_io.facetlist[counter];
     f->numberofpolygons = 1;
     f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
     f->numberofholes = 0;
     f->holelist = NULL;
     p = &f->polygonlist[0];

     Vector<unsigned> facet = outer_boundary_pt->facet(n);
     
     p->numberofvertices = facet.size();
     p->vertexlist = new int[p->numberofvertices];
     for(int i=0;i<p->numberofvertices;++i)
      {
       //The offset here is because we have insisted on 1-based indexing
       p->vertexlist[i] = facet[i] + 1;
      }

     //Set up the boundary markers
     tetgen_io.facetmarkerlist[counter] = 
      outer_boundary_pt->facet_boundary_id(n);
     //Increase the counter
     ++counter;
    }

   //Initialise the number of holes
   tetgen_io.numberofholes=0;

   //Loop over the internal stuff
   for(unsigned h=0;h<n_internal;++h)
    {
     for(unsigned n=0;n<n_inner_facet[h];++n)
      {
       //Set pointer to facet
       f = &tetgen_io.facetlist[counter];
       f->numberofpolygons = 1;
       f->polygonlist = new tetgenio::polygon[f->numberofpolygons];
       f->numberofholes = 0;
       f->holelist = NULL;
       p = &f->polygonlist[0];
       
       Vector<unsigned> facet = internal_closed_surface_pt[h]->facet(n);

       p->numberofvertices = facet.size();
       p->vertexlist = new int[p->numberofvertices];
       for(int i=0;i<p->numberofvertices;++i)
        {
         //Add the 1-based and vertex offsets to get these number correct
         p->vertexlist[i] = facet[i] +  internal_vertex_offset[h] + 1;
        }
       //Set up the boundary markers
       tetgen_io.facetmarkerlist[counter] = 
        internal_closed_surface_pt[h]->facet_boundary_id(n);
       ++counter;
      }

     //If it's a hole add it
     if(internal_closed_surface_pt[h]->is_hole()) 
      {
       ++tetgen_io.numberofholes;
      }
    }

   //Set storage for the holes
   tetgen_io.holelist = new double[3*tetgen_io.numberofholes];
   //Loop over all the internal boundaries again
   counter=0;
   for(unsigned h=0;h<n_internal;++h)
    {
     if(internal_closed_surface_pt[h]->is_hole())
      {
       for(unsigned i=0;i<3;++i)
        {
         tetgen_io.holelist[counter] = 
          internal_closed_surface_pt[h]->internal_point(i);
         ++counter;
        }
      }
    }

 /*in.numberofregions = 1;
 in.regionlist = new double[5*in.numberofregions];
 in.regionlist[0] = 0.0;
 in.regionlist[1] = 0.0;
 in.regionlist[2] = 0.0;
 in.regionlist[3] = 1;
 in.regionlist[4] = 1;*/
  }


 /// Empty destructor 
 ~TetgenMesh() 
  {
   if(Tetgenio_exists)
    {
     delete Tetgenio_pt;
    }
  }



 /// Boolen defining whether tetgenio object has been built or not
 bool &tetgenio_exists() {return Tetgenio_exists;}


 /// Access to the triangulateio representation of the mesh
 tetgenio* &tetgenio_pt() {return Tetgenio_pt;}

 /// Set the tetgen pointer by a deep copy
 void set_deep_copy_tetgenio_pt(tetgenio* const &tetgenio_pt)
 {
  //Delete the existing data
  if(Tetgenio_exists) {delete Tetgenio_pt;}
  this->Tetgenio_pt= new tetgenio;
  //Create a deep copy of tetgenio_pt and store the result in
  //Tetgenio_pt
  this->deep_copy_of_tetgenio(tetgenio_pt,this->Tetgenio_pt);
 }

 /// Transfer tetgenio data from the input to the output
 /// The output is assumed to have been constructed and "empty"
 void deep_copy_of_tetgenio(tetgenio* const &input_pt,
                            tetgenio* &output_pt);

 /// \short Setup boundary coordinate on boundary b which is
 /// assumed to be planar. Boundary coordinates are the
 /// x-y coordinates in the plane of that boundary with the
 /// x-axis along the line from the (lexicographically)
 /// "lower left" to the "upper right" node. The y axis
 /// is obtained by taking the cross-product of the positive
 /// x direction with the outer unit normal computed by
 /// the face elements.
 void setup_boundary_coordinates(const unsigned& b)
 {
  // Dummy file
  std::ofstream some_file;
  
  // Don't switch the normal
  bool switch_normal=false;

  setup_boundary_coordinates(b,switch_normal,some_file);
 }
 
 /// \short Setup boundary coordinate on boundary b which is
 /// assumed to be planar. Boundary coordinates are the
 /// x-y coordinates in the plane of that boundary with the
 /// x-axis along the line from the (lexicographically)
 /// "lower left" to the "upper right" node. The y axis
 /// is obtained by taking the cross-product of the positive
 /// x direction with the outer unit normal computed by
 /// the face elements. 
 void setup_boundary_coordinates(const unsigned& b, const bool& switch_normal)
 {
  // Dummy file
  std::ofstream some_file;
  
  setup_boundary_coordinates(b,switch_normal,some_file);
 }
 

 /// \short Setup boundary coordinate on boundary b which is
 /// assumed to be planar. Boundary coordinates are the
 /// x-y coordinates in the plane of that boundary with the
 /// x-axis along the line from the (lexicographically)
 /// "lower left" to the "upper right" node. The y axis
 /// is obtained by taking the cross-product of the positive
 /// x direction with the outer unit normal computed by
 /// the face elements. Doc faces in output file.
 void setup_boundary_coordinates(const unsigned& b,
                                 std::ofstream& outfile)
  {
   // Don't switch the normal
   bool switch_normal=false;
   
   setup_boundary_coordinates(b,switch_normal,outfile);
  }


 /// \short Setup boundary coordinate on boundary b which is
 /// assumed to be planar. Boundary coordinates are the
 /// x-y coordinates in the plane of that boundary with the
 /// x-axis along the line from the (lexicographically)
 /// "lower left" to the "upper right" node. The y axis
 /// is obtained by taking the cross-product of the positive
 /// x direction with the outer unit normal computed by
 /// the face elements (or its negative if switch_normal is set
 /// to true). Doc faces in output file.
 void setup_boundary_coordinates(const unsigned& b,
                                 const bool& switch_normal,
                                 std::ofstream& outfile);


  /// Return the number of elements adjacent to boundary b in region r
  inline unsigned nboundary_element_in_region(const unsigned &b,
                                        const unsigned &r) const
   {
    //Need to use a constant iterator here to keep the function "const"
    //Return an iterator to the appropriate entry, if we find it
    std::map<unsigned,Vector<FiniteElement*> >::const_iterator it =
     Boundary_region_element_pt[b].find(r);
    if(it!=Boundary_region_element_pt[b].end())
     {
      return (it->second).size();
     }
    //Otherwise there are no elements adjacent to boundary b in the region r
    else
     {
      return 0;
     }
   }

  /// Return pointer to the e-th element adjacent to boundary b in region r
  FiniteElement* boundary_element_pt_in_region(const unsigned &b, 
                                               const unsigned &r,
                                               const unsigned &e) const
  {
   //Use a constant iterator here to keep function "const" overall
   std::map<unsigned,Vector<FiniteElement*> >::const_iterator it =
    Boundary_region_element_pt[b].find(r);
   if(it!=Boundary_region_element_pt[b].end())
    {
     return (it->second)[e];
    }
   else
    {
     return 0;
    }
  }

  /// Return face index of the e-th element adjacent to boundary b in region r
  int face_index_at_boundary_in_region(const unsigned &b, 
                                       const unsigned &r,
                                       const unsigned &e) const
   {
    //Use a constant iterator here to keep function "const" overall
    std::map<unsigned,Vector<int> >::const_iterator it =
     Face_index_region_at_boundary[b].find(r);
    if(it!=Face_index_region_at_boundary[b].end())
     {
      return (it->second)[e];
     }
    else
     {
      std::ostringstream error_message;
      error_message << "Face indices not set up for boundary " 
                    << b << " in region " << r << "\n";
      error_message 
       << "This probably means that the boundary is not adjacent to region\n";
      throw OomphLibError(error_message.str(),
                          "TetgenMesh::face_index_at_boundary_in_region()",
                          OOMPH_EXCEPTION_LOCATION);
     }
   }


  /// Return the number of regions specified by attributes
  unsigned nregion() {return Region_element_pt.size();}
  
  /// Return the number of elements in region i
  unsigned nregion_element(const unsigned &i) 
  {return Region_element_pt[i].size();}
  
  /// Return the attribute associated with region i
  double region_attribute(const unsigned &i)
  {return Region_attribute[i];}
  
  /// Return the e-th element in the i-th region
  FiniteElement* region_element_pt(const unsigned &i,
                                   const unsigned &e)
  {return Region_element_pt[i][e];}
 
 /// \short Snap boundaries specified by the IDs listed in boundary_id to
 /// a quadratric surface, specified in the file 
 /// quadratic_surface_file_name. This is usually used with vmtk-based
 /// meshes for which oomph-lib's xda to poly conversion code produces the files
 /// "quadratic_fsi_boundary.dat" and "quadratic_outer_solid_boundary.dat"
 /// which specify the quadratic FSI boundary (for the fluid and the solid)
 /// and the quadratic representation of the outer boundary of the solid. 
 /// When used with these files, the flag switch_normal should be
 /// set to true when calling the function for the outer boundary of the
 /// solid. The DocInfo object can be used to label optional output
 /// files. (Uses directory and label).
 void snap_to_quadratic_surface(const Vector<unsigned>& boundary_id,
                                const std::string& quadratic_surface_file_name,
                                const bool& switch_normal,
                                DocInfo& doc_info);

 /// \short Snap boundaries specified by the IDs listed in boundary_id to
 /// a quadratric surface, specified in the file 
 /// quadratic_surface_file_name. This is usually used with vmtk-based
 /// meshes for which oomph-lib's xda to poly conversion code produces the files
 /// "quadratic_fsi_boundary.dat" and "quadratic_outer_solid_boundary.dat"
 /// which specify the quadratic FSI boundary (for the fluid and the solid)
 /// and the quadratic representation of the outer boundary of the solid. 
 /// When used with these files, the flag switch_normal should be
 /// set to true when calling the function for the outer boundary of the
 /// solid. 
 void snap_to_quadratic_surface(const Vector<unsigned>& boundary_id,
                                const std::string& quadratic_surface_file_name,
                                const bool& switch_normal)
 {
  // Dummy doc info
  DocInfo doc_info;
  doc_info.disable_doc();
  snap_to_quadratic_surface(boundary_id,quadratic_surface_file_name,
                            switch_normal,doc_info);
  
 }
 

 /// \short Non-Delaunay split elements that have three faces on a boundary
 /// into sons. Timestepper species timestepper for new nodes; defaults
 /// to to steady timestepper.
 void split_elements_in_corners(TimeStepper* time_stepper_pt=
                                &Mesh::Default_TimeStepper);
 

  protected:

 /// Build mesh from scaffold
 void build_from_scaffold(TimeStepper* time_stepper_pt, 
                          const bool &use_attributes);


  protected:

 /// \short Boolean to indicate whether a tetgenio representation of the
 /// mesh exists
 bool Tetgenio_exists;
 
 /// \short Tetgen representation of mesh
 tetgenio* Tetgenio_pt;

 /// Temporary scaffold mesh
 TetgenScaffoldMesh* Tmp_mesh_pt;

 /// Vectors of elements in each region differentiated by attribute
 Vector<Vector<FiniteElement*> > Region_element_pt;

 /// Vector of attributes associated with the elements in each region
 Vector<double> Region_attribute;

 /// Storage for elements adjacent to a boundary in a particular region
 Vector<std::map<unsigned,Vector<FiniteElement*> > > 
  Boundary_region_element_pt;
 
 /// \short Storage for the face index adjacent to a boundary in a particular
 /// region
 Vector<std::map<unsigned,Vector<int> > > Face_index_region_at_boundary;
 
 /// Faceted surface that defines outer boundaries
 TetgenMeshFacetedSurface* Outer_boundary_pt;
 
 /// Vector to faceted surfaces that define internal boundaries
 Vector<TetgenMeshFacetedSurface*> Internal_surface_pt;


 /// Timestepper used to build elements
 TimeStepper* Time_stepper_pt;

 /// Boolean flag to indicate whether to use attributes or not
 /// (required for multidomain meshes)
 bool Use_attributes;
 
}; //end class




//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////


//==============start_mesh=================================================
/// Tetgen-based mesh upgraded to become a solid mesh. Automatically
/// enumerates all boundaries
//=========================================================================
template<class ELEMENT>
class SolidTetMesh : public virtual TetgenMesh<ELEMENT>,
                     public virtual SolidMesh 
{
 
public:
 
 /// \short Constructor. Boundary coordinates are setup 
 /// automatically.
  SolidTetMesh(const std::string& node_file_name,
               const std::string& element_file_name,
               const std::string& face_file_name,
               const bool& split_corner_elements,
               TimeStepper* time_stepper_pt=
               &Mesh::Default_TimeStepper,
               const bool &use_attributes=false) : 
 TetgenMesh<ELEMENT>(node_file_name, element_file_name,
                     face_file_name, split_corner_elements, 
                     time_stepper_pt, use_attributes)
  {
   //Assign the Lagrangian coordinates
   set_lagrangian_nodal_coordinates();
   
   // Find out elements next to boundary
   setup_boundary_element_info();
   
   // Setup boundary coordinates for all boundaries without switching
   // direction of normal
   bool switch_normal=false;
   unsigned nb=this->nboundary();
   for (unsigned b=0;b<nb;b++) 
    {
     this->setup_boundary_coordinates(b,switch_normal);
    }
  }

 /// \short Constructor. Boundary coordinates are setup 
 /// automatically, with the orientation of the outer unit
 /// normal determined by switch_normal.
  SolidTetMesh(const std::string& node_file_name,
               const std::string& element_file_name,
               const std::string& face_file_name,
               const bool& split_corner_elements,
               const bool& switch_normal,
               TimeStepper* time_stepper_pt=
               &Mesh::Default_TimeStepper,
               const bool &use_attributes=false) : 
 TetgenMesh<ELEMENT>(node_file_name, element_file_name,
                     face_file_name, split_corner_elements, 
                     time_stepper_pt, use_attributes)
  {
   //Assign the Lagrangian coordinates
   set_lagrangian_nodal_coordinates();
   
   // Find out elements next to boundary
   setup_boundary_element_info();
   
   // Setup boundary coordinates for all boundaries
   unsigned nb=this->nboundary();
   for (unsigned b=0;b<nb;b++) 
    {
     this->setup_boundary_coordinates(b,switch_normal);
    }
  }

 /// Empty Destructor
 virtual ~SolidTetMesh() { }

};
 




}

#endif
