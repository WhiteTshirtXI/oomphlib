//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.85. June 9, 2008.
//LIC// 
//LIC// Copyright (C) 2006-2008 Matthias Heil and Andrew Hazel
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
// Templated functions for MeshAsGeomObject
// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

//oomph-lib headers
#include "mesh.h"
#include "mesh_as_geometric_object.h"

#include <stdio.h>


#ifndef OOMPH_MESH_AS_GEOMETRIC_OBJECT_CC
#define OOMPH_MESH_AS_GEOMETRIC_OBJECT_CC


namespace oomph
{

//========================================================================
/// Helper function for constructor: Oass the pointer to the mesh, 
/// communicator and boolean
///to specify whether to calculate coordinate extrema or not
//========================================================================
 template<unsigned DIM_LAGRANGIAN, unsigned DIM_EULERIAN, class ELEMENT>
 void MeshAsGeomObject<DIM_LAGRANGIAN,DIM_EULERIAN,ELEMENT>::construct_it
 (Mesh* const &mesh_pt, OomphCommunicator* comm_pt,
  const bool& compute_extreme_bin_coords)
 {
#ifdef OOMPH_HAS_MPI
   // Set communicator
   Communicator_pt=comm_pt;
#endif

   // Create temporary storage for geometric Data (don't count 
   // Data twice!
   std::set<Data*> tmp_geom_data;
   
   //Copy all the elements in the mesh into local storage
   //N.B. elements must be able to have a geometric object representation.
   unsigned n_sub_object = mesh_pt->nelement();
   Sub_geom_object_pt.resize(n_sub_object);
   for(unsigned e=0;e<n_sub_object;e++)
    {

     // (Try to) cast to an ELEMENT:
     Sub_geom_object_pt[e]=dynamic_cast<ELEMENT*>(mesh_pt->element_pt(e));

#ifdef PARANOID
     if (Sub_geom_object_pt[e]==0)
      {
       std::ostringstream error_message;
       error_message << "Unable to dynamic cast element: " << std::endl
                     << "into an ELEMENT\n";
       throw OomphLibError(
        error_message.str(),
        "MeshAsGeomObject::MeshAsGeomObject(...)",
        OOMPH_EXCEPTION_LOCATION);
      }
#endif

     // Add the geometric Data of each element into set
     unsigned ngeom=Sub_geom_object_pt[e]->ngeom_data();
     for (unsigned i=0;i<ngeom;i++)
      {
       tmp_geom_data.insert(Sub_geom_object_pt[e]->geom_data_pt(i));
      }
    }

   // Now copy unique geom Data values across into vector
   unsigned ngeom=tmp_geom_data.size();
   Geom_data_pt.resize(ngeom);
   typedef std::set<Data*>::iterator IT;
   unsigned count=0;
   for (IT it=tmp_geom_data.begin();it!=tmp_geom_data.end();it++)
    {
     Geom_data_pt[count]=*it;
     count++;
    }

   // Set storage for minimum and maximum coordinates
   Minmax_coords.resize(DIM_LAGRANGIAN*2);

   // Get the default parameters for the number of bins in each 
   // dimension from the Multi_domain_functions namespace
   
   // Parameters are at Nx_bin, Ny_bin, Nz_bin
   Nbin_x=Multi_domain_functions::Nx_bin;
   Nbin_y=Multi_domain_functions::Ny_bin;
   Nbin_z=Multi_domain_functions::Nz_bin;
   
   // Are we computing the extreme bin coordinates here?
   if (compute_extreme_bin_coords)
    {
     // Find the maximum and minimum coordinates for the mesh
     get_min_and_max_coordinates(mesh_pt);

     // Create the bin structure
     create_bins_of_objects();
    }
  }






//========================================================================
/// \short Find the sub geometric object and local coordinate therein that
/// corresponds to the intrinsic coordinate zeta. If sub_geom_object_pt=0
/// on return from this function, none of the constituent sub-objects 
/// contain the required coordinate.
/// Setting the optional bool argument to true means that each
/// time the sub-object's locate_zeta function is called, the coordinate
/// argument "s" is used as the initial guess
//========================================================================
 template<unsigned DIM_LAGRANGIAN, unsigned DIM_EULERIAN, class ELEMENT>
 void MeshAsGeomObject<DIM_LAGRANGIAN,DIM_EULERIAN,ELEMENT>::locate_zeta
 (const Vector<double>& zeta,GeomObject*& sub_geom_object_pt,
  Vector<double>& s, const bool &called_within_spiral)
  {
   // Initialise return to null -- if it's still null when we're
   // leaving we've failed!
   sub_geom_object_pt=0;

   // Does the zeta coordinate lie within the current bin structure?
   // If not then modify Minmax_coords and re-populate the bin structure
   if (DIM_LAGRANGIAN==1)
    {
     if (zeta[0]<Minmax_coords[0])
      {
       Minmax_coords[0]=zeta[0];
       create_bins_of_objects();
      }
     else if (zeta[0]>Minmax_coords[1])
      {
       Minmax_coords[1]=zeta[0];
       create_bins_of_objects();
      }
    }
   else if (DIM_LAGRANGIAN==2)
    {
     if (zeta[0]<Minmax_coords[0])
      {
       Minmax_coords[0]=zeta[0];
       create_bins_of_objects();
      }
     else if (zeta[0]>Minmax_coords[1])
      {
       Minmax_coords[1]=zeta[0];
       create_bins_of_objects();
      }
     // and in the second direction...
     if (zeta[1]<Minmax_coords[2])
      {
       Minmax_coords[2]=zeta[1];
       create_bins_of_objects();
      }
     else if (zeta[1]>Minmax_coords[3])
      {
       Minmax_coords[3]=zeta[1];
       create_bins_of_objects();
      }
    }
   else if (DIM_LAGRANGIAN==3)
    {
     if (zeta[0]<Minmax_coords[0])
      {
       Minmax_coords[0]=zeta[0];
       create_bins_of_objects();
      }
     else if (zeta[0]>Minmax_coords[1])
      {
       Minmax_coords[1]=zeta[0];
       create_bins_of_objects();
      }
     // and in the second direction...
     if (zeta[1]<Minmax_coords[2])
      {
       Minmax_coords[2]=zeta[1];
       create_bins_of_objects();
      }
     else if (zeta[1]>Minmax_coords[3])
      {
       Minmax_coords[3]=zeta[1];
       create_bins_of_objects();
      }
     // and in the third direction...
     if (zeta[2]<Minmax_coords[4])
      {
       Minmax_coords[4]=zeta[2];
       create_bins_of_objects();
      }
     else if (zeta[2]>Minmax_coords[5])
      {
       Minmax_coords[5]=zeta[2];
       create_bins_of_objects();
      }
    }

   unsigned bin_number=0;
   // Get the min and max coords of the bin structure, and find
   // the bin structure containing the current zeta cooordinate
   if (DIM_LAGRANGIAN==1)
    {
     double x_min=Minmax_coords[0];
     double x_max=Minmax_coords[1];

     bin_number=int(Nbin_x*((zeta[0]-x_min)/(x_max-x_min)));
     if (bin_number==Nbin_x) {bin_number=Nbin_x-1;}
    }
   else if (DIM_LAGRANGIAN==2)
    {
     double x_min=Minmax_coords[0];
     double x_max=Minmax_coords[1];
     double y_min=Minmax_coords[2];
     double y_max=Minmax_coords[3];

     bin_number=int(Nbin_x*((zeta[0]-x_min)/(x_max-x_min)))
      +Nbin_x*int(Nbin_y*((zeta[1]-y_min)/(y_max-y_min)));
     if (bin_number==Nbin_x*Nbin_y) {bin_number=Nbin_x*Nbin_y-1;}
    }
   else if (DIM_LAGRANGIAN==3)
    {
     double x_min=Minmax_coords[0];
     double x_max=Minmax_coords[1];
     double y_min=Minmax_coords[2];
     double y_max=Minmax_coords[3];
     double z_min=Minmax_coords[4];
     double z_max=Minmax_coords[5];

     bin_number=int(Nbin_x*((zeta[0]-x_min)/(x_max-x_min)))
      +Nbin_x*int(Nbin_y*((zeta[1]-y_min)/(y_max-y_min)))
      +Nbin_y*Nbin_x*int(Nbin_z*((zeta[2]-z_min)/(z_max-z_min)));
     if (bin_number==Nbin_x*Nbin_y*Nbin_z) 
      {
       bin_number=Nbin_x*Nbin_y*Nbin_z-1;
      }
    }

   if (called_within_spiral)
    {
     // Current "spiral" level
     unsigned i_level=current_spiral_level();

     // Call helper function to find the neighbouring bins at this level
     Vector<unsigned> neighbour_bin;
     get_neighbouring_bins_helper(bin_number,i_level,neighbour_bin);
     unsigned n_nbr_bin=neighbour_bin.size();

     // Set bool for finding zeta
     bool found_zeta=false;
     for (unsigned i_nbr=0;i_nbr<n_nbr_bin;i_nbr++)
      {
       // Get the number of element-sample point pairs in this bin
       unsigned n_sample=
        Bin_object_coord_pairs[neighbour_bin[i_nbr]].size();

       // Don't do anything if this bin has no sample points
       if (n_sample>0)
        {
         for (unsigned i_sam=0;i_sam<n_sample;i_sam++)
          {
           // Get the element
           ELEMENT* el_pt=Bin_object_coord_pairs
            [neighbour_bin[i_nbr]][i_sam].first;

           // Get the local coordinate
           s=Bin_object_coord_pairs[neighbour_bin[i_nbr]][i_sam].second;

           // Use this coordinate as the initial guess
           bool use_coordinate_as_initial_guess=true;

           // Attempt to find zeta within a sub-object
           el_pt->locate_zeta(zeta,sub_geom_object_pt,s,
                              use_coordinate_as_initial_guess);

#ifdef OOMPH_HAS_MPI
           // Dynamic cast the result to an ELEMENT
           ELEMENT* test_el_pt=dynamic_cast<ELEMENT*>(sub_geom_object_pt);
           if (test_el_pt!=0)
            {
             // We only want to exit if this is a non-halo element
             if (test_el_pt->is_halo()) {sub_geom_object_pt=0;}
            }
#endif

           // If the ELEMENT is non-halo and has been located, exit
           if (sub_geom_object_pt!=0)
            {
             found_zeta=true;
             break;
            }
          } // end loop over sample points
        }

       if (found_zeta)
        {
         break;
        }

      } // end loop over bins at this level

    }
   else
    {
     // Not called from within a spiral procedure
     // (i.e. the loop in multi_domain.h), so do the spiralling here

     // Loop over all levels... maximum of N*_bin
     unsigned n_level=Nbin_x;
     if (DIM_LAGRANGIAN>=2)
      {
       if (n_level < Nbin_y) { n_level=Nbin_y; }
      }
     if (DIM_LAGRANGIAN==3)
      {
       if (n_level < Nbin_z) { n_level=Nbin_z; }
      }

     // Set bool for finding zeta
     bool found_zeta=false;
     for (unsigned i_level=0;i_level<n_level;i_level++)
      {
       // Call helper function to find the neighbouring bins at this level
       Vector<unsigned> neighbour_bin;
       get_neighbouring_bins_helper(bin_number,i_level,neighbour_bin);
       unsigned n_nbr_bin=neighbour_bin.size();

       // Loop over neighbouring bins
       for (unsigned i_nbr=0;i_nbr<n_nbr_bin;i_nbr++)
        {
         // Get the number of element-sample point pairs in this bin
         unsigned n_sample=
          Bin_object_coord_pairs[neighbour_bin[i_nbr]].size();

         // Don't do anything if this bin has no sample points
         if (n_sample>0)
          {
           for (unsigned i_sam=0;i_sam<n_sample;i_sam++)
            {
             // Get the element
             ELEMENT* el_pt=Bin_object_coord_pairs
              [neighbour_bin[i_nbr]][i_sam].first;

             // Get the local coordinate
             s=Bin_object_coord_pairs[neighbour_bin[i_nbr]][i_sam].second;

             // Use this coordinate as the initial guess in locate_zeta
             bool use_coordinate_as_initial_guess=true;

             // Attempt to loacte the correct sub-object
             el_pt->locate_zeta(zeta,sub_geom_object_pt,s,
                                use_coordinate_as_initial_guess);

             // If it was found then break
             if (sub_geom_object_pt!=0)
              {
               found_zeta=true;
               break;
              }
            } // end loop over sample points
          }

         // Break out of the bin loop if locate was successful
         if (found_zeta)
          {
           break;
          }

        } // end loop over bins at this level

       // Break out of the spiral loop if locate was successful
       if (found_zeta)
        {
         break;
        }

      } // end loop over levels

    } // end if (called_within_spiral)

  }

//========================================================================
///Get the min and max coordinates for the mesh, in each dimension
//========================================================================
 template<unsigned DIM_LAGRANGIAN, unsigned DIM_EULERIAN, class ELEMENT>
 void MeshAsGeomObject<DIM_LAGRANGIAN,DIM_EULERIAN,ELEMENT>::
 get_min_and_max_coordinates(Mesh* const &mesh_pt)
  {
   // Storage locally (i.e. in parallel on each processor)
   double x_min_local=DBL_MAX; double x_max_local=-DBL_MAX;
   double y_min_local=DBL_MAX; double y_max_local=-DBL_MAX;
   double z_min_local=DBL_MAX; double z_max_local=-DBL_MAX;

   // Loop over the elements of the mesh
   unsigned n_el=mesh_pt->nelement();
   for (unsigned e=0;e<n_el;e++)
    {
     ELEMENT* el_pt=dynamic_cast<ELEMENT*>(mesh_pt->element_pt(e));

     // Get the number of vertices (nplot=2 does the trick)
     unsigned n_plot=2;
     unsigned n_plot_points=el_pt->nplot_points(n_plot);

     // Loop over the number of plot points
     for (unsigned i=0;i<n_plot_points;i++)
      {
       Vector<double> el_local(DIM_LAGRANGIAN);
       Vector<double> el_global(DIM_LAGRANGIAN);

       // Get the local s
       el_pt->get_s_plot(i,n_plot,el_local);

       // Now interpolate to global coordinates
       el_pt->interpolated_zeta(el_local,el_global);

       // Check the max and min in each direction
       if (el_global[0] < x_min_local) {x_min_local=el_global[0];}
       if (el_global[0] > x_max_local) {x_max_local=el_global[0];}
       if (DIM_LAGRANGIAN>=2)
        {
         if (el_global[1] < y_min_local) {y_min_local=el_global[1];}
         if (el_global[1] > y_max_local) {y_max_local=el_global[1];}
        }
       if (DIM_LAGRANGIAN==3)
        {
         if (el_global[2] < z_min_local) {z_min_local=el_global[2];}
         if (el_global[2] > z_max_local) {z_max_local=el_global[2];}
        }
      }
    }

   // Global extrema - in parallel, need to get max/min across all processors
   double x_min=0.0, x_max=0.0, y_min=0.0, y_max=0.0, z_min=0.0, z_max=0.0;
#ifdef OOMPH_HAS_MPI
   // If the mesh has been distributed...
   if (mesh_pt->mesh_has_been_distributed())
    {
     // .. we need a non-null communicator!
     if (Communicator_pt!=0)
      {
       int n_proc=Communicator_pt->nproc();
       if (n_proc>1)
        {
         MPI_Allreduce(&x_min_local,&x_min,1,MPI_DOUBLE,MPI_MIN,
                       Communicator_pt->mpi_comm());
         MPI_Allreduce(&x_max_local,&x_max,1,MPI_DOUBLE,MPI_MAX,
                       Communicator_pt->mpi_comm());
         if (DIM_LAGRANGIAN>=2)
          {
           MPI_Allreduce(&y_min_local,&y_min,1,MPI_DOUBLE,MPI_MIN,
                         Communicator_pt->mpi_comm());
           MPI_Allreduce(&y_max_local,&y_max,1,MPI_DOUBLE,MPI_MAX,
                         Communicator_pt->mpi_comm());
          }
         if (DIM_LAGRANGIAN==3)
          {
           MPI_Allreduce(&z_min_local,&z_min,1,MPI_DOUBLE,MPI_MIN,
                         Communicator_pt->mpi_comm());
           MPI_Allreduce(&z_max_local,&z_max,1,MPI_DOUBLE,MPI_MAX,
                         Communicator_pt->mpi_comm());
          }
        }
      }
     else // Null communicator - throw an error
      {
       std::ostringstream error_message_stream;                           
       error_message_stream                                        
        << "Communicator not set for a MeshAsGeomObject\n"
        << "that was created from a distributed Mesh";
       throw OomphLibError(error_message_stream.str(),                  
                           "MeshAsGeomObject::get_min_and_max_coordinates(..)",
                           OOMPH_EXCEPTION_LOCATION);
      }
    }
   else // If the mesh hasn't been distributed then the 
        // max and min are the same on all processors
    {
     x_min=x_min_local;
     x_max=x_max_local;
     if (DIM_LAGRANGIAN>=2)
      {
       y_min=y_min_local;
       y_max=y_max_local;
      }
     if (DIM_LAGRANGIAN==3)
      {
       z_min=z_min_local;
       z_max=z_max_local;
      }
    }
#else // If we're not using MPI then the mesh can't be distributed
   x_min=x_min_local;
   x_max=x_max_local;
   if (DIM_LAGRANGIAN>=2)
    {
     y_min=y_min_local;
     y_max=y_max_local;
    }
   if (DIM_LAGRANGIAN==3)
    {
     z_min=z_min_local;
     z_max=z_max_local;
    }
#endif

   // Decrease/increase min and max to allow for any overshoot in
   // meshes that may move around
   // There's no point in doing this for DIM_LAGRANGIAN==1
   double percentage_offset=5.0;
   if (DIM_LAGRANGIAN>=2)
    {
     double x_length=x_max-x_min;
     double y_length=y_max-y_min;
     x_min=x_min-((percentage_offset/100.0)*x_length);
     x_max=x_max+((percentage_offset/100.0)*x_length);
     y_min=y_min-((percentage_offset/100.0)*y_length);
     y_max=y_max+((percentage_offset/100.0)*y_length);
    }
   if (DIM_LAGRANGIAN==3)
    {
     double z_length=z_max-z_min;
     z_min=z_min-((percentage_offset/100.0)*z_length);
     z_max=z_max+((percentage_offset/100.0)*z_length);
    }

   // Add these entries to the Minmax_coords vector
   Minmax_coords[0]=x_min;
   Minmax_coords[1]=x_max;
   if (DIM_LAGRANGIAN>=2)
    {
     Minmax_coords[2]=y_min;
     Minmax_coords[3]=y_max;
    }
   if (DIM_LAGRANGIAN==3)
    {
     Minmax_coords[4]=z_min;
     Minmax_coords[5]=z_max;
    }

  }

//========================================================================
///Initialise the "bin" structure for locating coordinates
//========================================================================
 template<unsigned DIM_LAGRANGIAN, unsigned DIM_EULERIAN, class ELEMENT>
 void MeshAsGeomObject<DIM_LAGRANGIAN,DIM_EULERIAN,ELEMENT>::
 create_bins_of_objects()
  {
   // Output message regarding bin structure setup
   oomph_info << "==============================================" << std::endl;
   oomph_info << " MeshAsGeomObject: setting up bin search with:" << std::endl;
   oomph_info << "   Nbin_x=" << Nbin_x << "  ";
   if (DIM_LAGRANGIAN>=2)
    {
     oomph_info << "Nbin_y=" << Nbin_y << "  ";
    }
   if (DIM_LAGRANGIAN==3)
    {
     oomph_info << "Nbin_z=" << Nbin_z;
    }
   oomph_info << std::endl;
   oomph_info << "   Xminmax=" << Minmax_coords[0] << " " << Minmax_coords[1] 
              << "  ";
   if (DIM_LAGRANGIAN>=2)
    {
     oomph_info << "Yminmax=" << Minmax_coords[2] << " " << Minmax_coords[3]
              << "  ";
    }
   if (DIM_LAGRANGIAN==3)
    {
     oomph_info << "Zminmax=" << Minmax_coords[4] << " " << Minmax_coords[5] 
              << "  ";
    }
   oomph_info << std::endl;
   oomph_info << "==============================================" << std::endl;

   /// Flush all objects out of the bin structure
   flush_bins_of_objects();

   ///The storage for these bins is of size Nbin_x*Nbin_y*Nbin_z
   unsigned ntotalbin=Nbin_x;
   if (DIM_LAGRANGIAN==2)
    {
     ntotalbin=Nbin_x*Nbin_y;
    }
   else if (DIM_LAGRANGIAN==3)
    {
     ntotalbin=Nbin_x*Nbin_y*Nbin_z;
    }
   Bin_object_coord_pairs.resize(ntotalbin);

   ///Loop over subobjects to decide which bin they belong in...
   unsigned n_sub=Sub_geom_object_pt.size();

   oomph_info << "There are " << n_sub << " elements to be put into bins"
              << std::endl << std::endl;

   for (unsigned e=0;e<n_sub;e++)
    {
     // Cast to the element (sub-object) first
     ELEMENT* el_pt=dynamic_cast<ELEMENT*>(Sub_geom_object_pt[e]);

     // Get specified number of points within the element
     unsigned n_plot_points=
      el_pt->nplot_points(Multi_domain_functions::Nsample_points);

     for (unsigned i=0;i<n_plot_points;i++)
      {
       // Storage for local and global coordinates
       Vector<double> local_coord(DIM_LAGRANGIAN,0.0);
       Vector<double> global_coord(DIM_LAGRANGIAN,0.0);

       // Get local coordinate and interpolate to global
       el_pt->get_s_plot(i,Multi_domain_functions::Nsample_points,local_coord);
       el_pt->interpolated_zeta(local_coord,global_coord);

       //Which bin are the global coordinates in?
       unsigned bin_number=0;

       //Get max coordinates of bin structure in 1st dimension
       double x_min=Minmax_coords[0];
       double x_max=Minmax_coords[1];

       //Work out bin number in this dimension
       unsigned bin_number_x=int(Nbin_x*(global_coord[0]-x_min)/(x_max-x_min));
       // Buffer the case where global_coord[0]==x_max
       if (bin_number_x==Nbin_x) {bin_number_x=Nbin_x-1;}

       //Work out the bin number (in higher dimensions if required)
       if (DIM_LAGRANGIAN==1)
        {
         bin_number=bin_number_x;
        }
       else // DIM_LAGRANGIAN=2,3
        {
         double y_min=Minmax_coords[2];
         double y_max=Minmax_coords[3];

         // Bin number along second dimension
         unsigned bin_number_y=
          int(Nbin_y*(global_coord[1]-y_min)/(y_max-y_min));
         if (bin_number_y==Nbin_y) {bin_number_y=Nbin_y-1;}

         if (DIM_LAGRANGIAN==2)
          {
           // Total bin number
           bin_number=bin_number_x+(Nbin_x*bin_number_y);
          }
         else if (DIM_LAGRANGIAN==3)
          {
           double z_min=Minmax_coords[4];
           double z_max=Minmax_coords[5];

           // Bin number along third dimension
           unsigned bin_number_z=
            int(Nbin_z*(global_coord[2]-z_min)/(z_max-z_min));
           if (bin_number_z==Nbin_z) {bin_number_z=Nbin_z-1;}

           // Total bin number
           bin_number=bin_number_x+(Nbin_x*bin_number_y)
            +(Nbin_y*Nbin_x*bin_number_z);
          }
        }

       //Add element-sample local (?) coord pair to the calculated bin
       Bin_object_coord_pairs[bin_number].push_back
        (std::make_pair(el_pt,local_coord));
      }
    }

  }

//========================================================================
///Calculate the bin numbers of all the neighbours to "bin" given the level
//========================================================================
 template<unsigned DIM_LAGRANGIAN, unsigned DIM_EULERIAN, class ELEMENT>
 void MeshAsGeomObject<DIM_LAGRANGIAN,DIM_EULERIAN,ELEMENT>::
 get_neighbouring_bins_helper(const unsigned& bin, const unsigned& level,
                              Vector<unsigned>& neighbour_bin)
  {
   // This will be different depending on the value of DIM_LAGRANGIAN
   if (DIM_LAGRANGIAN==1)
    {
     // Single "loop" in one direction - always a vector of max size 2
     unsigned nbr_bin_left=bin-level;
     if ((nbr_bin_left>=0) && (nbr_bin_left<Nbin_x))
      {
       unsigned nbr_bin=nbr_bin_left;
       neighbour_bin.push_back(nbr_bin);
      }
     unsigned nbr_bin_right=bin+level;
     if ((nbr_bin_right>=0) && (nbr_bin_right<Nbin_x) && 
         (nbr_bin_right!=nbr_bin_left))
      {
       unsigned nbr_bin=nbr_bin_right;
       neighbour_bin.push_back(nbr_bin);
      }
    }
   else if (DIM_LAGRANGIAN==2)
    {
     unsigned n_total_bin=Nbin_x*Nbin_y;

     // Which row of the bin structure is the current bin on?
     // This is just given by the integer answer of dividing bin
     // by Nbin_x (the number of bins in a single row)
     // e.g. in a 6x6 grid, bins 6 through 11 would all have bin_row=1
     unsigned bin_row=bin/Nbin_x;

     // The neighbour_bin vector contains all bin numbers at the 
     // specified "distance" (level) away from the current bin

     // Row/column length
     unsigned n_length=(level*2)+1;

     // Loop over the rows
     for (unsigned j=0;j<n_length;j++)
      {
       // Loop over the columns
       for (unsigned i=0;i<n_length;i++)
        {
         // Only do this for all the first & last row, and the
         // end points of every other row
         if ((j==0) || (j==n_length-1) || (i==0) || (i==n_length-1))
          {
           unsigned nbr_bin=bin-level+i-((level-j)*Nbin_x);
           // This number might fall on the wrong
           // row of the bin structure; this needs to be tested

           // Which row is this number on? (see above)
           unsigned nbr_bin_row=nbr_bin/Nbin_x;

           // Which row should it be on?
           unsigned row=bin_row-level+j;

           // These numbers for the rows must match; 
           // if it is then add nbr_bin to the neighbour scheme
           // (The bin number must also be greater than zero
           //  and less than the total number of bins)
           if ((row==nbr_bin_row) && (nbr_bin>=0) && (nbr_bin<n_total_bin))
            {
             neighbour_bin.push_back(nbr_bin);
            }  
          }
        }

      }

    }
   else if (DIM_LAGRANGIAN==3)
    {
     unsigned n_total_bin=Nbin_x*Nbin_y*Nbin_z;

     // Which layer of the bin structure is the current bin on?
     // This is just given by the integer answer of dividing bin
     // by Nbin_x*Nbin_y (the number of bins in a single layer
     // e.g. in a 6x6x6 grid, bins 72 through 107 would all have bin_layer=2
     unsigned bin_layer=bin/(Nbin_x*Nbin_y);

     // Which row in this layer is the bin number on?
     unsigned bin_row=(bin/Nbin_x)-(bin_layer*Nbin_y);

     // The neighbour_bin vector contains all bin numbers at the 
     // specified "distance" (level) away from the current bin

     // Row/column/layer length
     unsigned n_length=(level*2)+1;

     // Loop over the layers
     for (unsigned k=0;k<n_length;k++)
      {
       // Loop over the rows
       for (unsigned j=0;j<n_length;j++)
        {
         // Loop over the columns
         for (unsigned i=0;i<n_length;i++)
          {
           // Only do this for the end points of every row/layer/column
           if ((k==0) || (k==n_length-1) || (j==0) || 
               (j==n_length-1) || (i==0) || (i==n_length-1))
            {
             unsigned nbr_bin=bin-level+i-((level-j)*Nbin_x)-
              ((level-k)*Nbin_x*Nbin_y);
             // This number might fall on the wrong
             // row or layer of the bin structure; this needs to be tested

             // Which layer is this number on?
             unsigned nbr_bin_layer=nbr_bin/(Nbin_x*Nbin_y);

             // Which row is this number on? (see above)
             unsigned nbr_bin_row=(nbr_bin/Nbin_x)-(nbr_bin_layer*Nbin_y);

             // Which layer and row should it be on, given level?
             unsigned layer=bin_layer-level+k;
             unsigned row=bin_row-level+j;

             // These layers and rows must match up:
             // if so then add nbr_bin to the neighbour schemes
             // (The bin number must also be greater than zero
             //  and less than the total number of bins)
             if ((row==nbr_bin_row) && (layer==nbr_bin_layer)
                 && (nbr_bin>=0) && (nbr_bin<n_total_bin))
              {
               neighbour_bin.push_back(nbr_bin);
              }  
            }

          }
        }
      }

    }
  }
 

}

#endif
