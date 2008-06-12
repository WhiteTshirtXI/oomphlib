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
#include<list>
#include<algorithm>

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

#include "oomph_utilities.h"
#include "problem.h"
#include "timesteppers.h"
#include "refineable_mesh.h"
#include "linear_solver.h"
#include "eigen_solver.h"
#include "assembly_handler.h"

namespace oomph
{

//////////////////////////////////////////////////////////////////
//Non-inline functions for the problem class
//////////////////////////////////////////////////////////////////

//================================================================
/// Constructor: Allocate space for one time stepper
/// and set all pointers to NULL and set defaults for all
/// parameters.
//===============================================================
 Problem::Problem() : 
  Mesh_pt(0), Time_pt(0), Saved_dof_pt(0), 
  Jacobian_reuse_is_enabled(false), Jacobian_has_been_computed(false),
  Newton_solver_tolerance(1.0e-8),
  Max_newton_iterations(10), Max_residuals(10.0),
  Problem_is_nonlinear(true),
  Sparse_assembly_method(Perform_assembly_using_vectors_of_pairs), 
  Pause_at_end_of_sparse_assembly(false),
  Numerical_zero_for_sparse_assembly(0.0),
  Minimum_dt(1.0e-12), Maximum_dt(1.0e12), DTSF_max_increase(4.0),
  Scale_arc_length(true), Desired_proportion_of_arc_length(0.5),
  Theta_squared(1.0), Sign_of_jacobian(0), Continuation_direction(1.0), 
  Parameter_derivative(1.0), Parameter_current(0.0),
  Ds_current(0.0), Desired_newton_iterations_ds(5), 
  Minimum_ds(1.0e-10), Bifurcation_detection(false), 
  First_jacobian_sign_change(false), Arc_length_step_taken(false),
#ifdef OOMPH_HAS_MPI
  Problem_has_been_distributed(false),
  Max_permitted_error_for_halo_check(1.0e-14),
#endif
  Shut_up_in_newton_solve(false)
 {
  // By default no submeshes:
  Sub_mesh_pt.resize(0);
  // No timesteppers
  Time_stepper_pt.resize(0);
  
  //Set the linear solvers, eigensolver and assembly handler
  Linear_solver_pt = Default_linear_solver_pt = new SuperLU;
  
  Eigen_solver_pt = Default_eigen_solver_pt = new ARPACK;
  
  Assembly_handler_pt = Default_assembly_handler_pt = new AssemblyHandler;
 }

//================================================================
/// Destructor to clean up memory
//================================================================
Problem::~Problem()
{
//  // Delete the (global) mesh pointer: This also kills the associated 
//  // nodes and elements. 
//  delete Mesh_pt; 
//  Mesh_pt=0;

//  // Now wipe the submeshes -- careful: their element and node pointers
//  // still point to nodes and elements that have just been deleted.
//  // Need to reset their pointers to NULL so the submeshes themselves 
//  // can be deleted again without any side effects.

//  // Number of submeshes
//  unsigned nsub_mesh=Sub_mesh_pt.size();
 
//  // Loop over submeshes
//  for (unsigned imesh=0;imesh<nsub_mesh;imesh++)
//   {
   
//    // NULL the element pointers
//    unsigned long n_element=Sub_mesh_pt[imesh]->nelement();
//    for (unsigned long e=0;e<n_element;e++)
//     {
//      Sub_mesh_pt[imesh]->element_pt(e)=0;
//     }
   
//    // NULL the node pointers
//    unsigned long n_node=Sub_mesh_pt[imesh]->nnode();
//    for (unsigned long n=0;n<n_node;n++)
//     {
//      Sub_mesh_pt[imesh]->node_pt(n)=0;
//     }
   
//    // Now the submesh itself can safely be deleted
//    delete Sub_mesh_pt[imesh];
//    Sub_mesh_pt[imesh]=0;
//   }

//  //Find number of time steppers
//  unsigned n_time_steppers = Time_stepper_pt.size();

//  //Loop over and delete the memory assigned
//  for(unsigned i=n_time_steppers;i>0;i--)
//   {
//    delete Time_stepper_pt[i-1]; Time_stepper_pt[i-1] = 0;
//   }

//  //Delete the memory assigned for the global time
//  delete Time_pt; Time_pt = 0;

//  //Find the number of global data values
//  unsigned Nglobal_data = Global_data_pt.size();
//  //Loop over and delete the memory assigned
//  for(unsigned i=Nglobal_data;i>0;i--)
//   {
//    delete Global_data_pt[i-1]; Global_data_pt[i-1] = 0;
//   }


 // Hang on: If we're not using the default linear solver,
 // somebody else must have built it, so that person 
 // must be in charge of killing it. 

 // We can safely delete the defaults, however
 delete Default_linear_solver_pt;
 delete Default_eigen_solver_pt;
 delete Default_assembly_handler_pt;

 //Clean up the memory allocated for the linear solver,
 //if it is not the default
 //if(Linear_solver_pt != &Default_LinearSolver){delete Linear_solver_pt;} 
 //Set the pointer to the linear solver to zero
 //Linear_solver_pt = 0;
}


#ifdef OOMPH_HAS_MPI

 //==================================================================
 /// Distribute the problem and doc
 //==================================================================
 void Problem::distribute(DocInfo& doc_info, const bool& report_stats)
 {
  int n_element=mesh_pt()->nelement();
  if (MPI_Helpers::Nproc==1)
   {
    oomph_info << "WARNING: You've tried to distribute a problem over only\n"
               << "one processor: this would make METIS crash.\n" 
               << "Ignoring your request for distribution."
               << std::endl << std::endl;
   }
  else if (MPI_Helpers::Nproc>n_element)
   {
    std::ostringstream error_stream;
    error_stream << "ERROR: You've tried to distribute a problem with\n"
                 << n_element << " elements over " << MPI_Helpers::Nproc
                 << " processors; if you want to\n"
                 << "use this many processors then refine the mesh first!\n"
                 << std::endl;
    throw OomphLibError(error_stream.str(),
                        "Problem::distribute()",
                        OOMPH_EXCEPTION_LOCATION);
   }
  else
   {
    // Distribute the mesh
    mesh_pt()->distribute(doc_info,report_stats);
  
    // Now the problem has been distributed
    Problem_has_been_distributed=true;
  
    // Re-assign the equation numbers (incl synchronisation if reqd)
    assign_eqn_numbers();
   }
 }


 //==================================================================
 /// (Irreversibly) redistribute elements and nodes, usually
 /// after another round of refinement, to get rid of
 /// excessively wide halo layers. Note that the current
 /// mesh will be now regarded as the base mesh and no unrefinement
 /// relative to it will be possible once this function 
 /// has been called.
 //==================================================================
 void Problem::redistribute(DocInfo& doc_info, const bool& report_stats)
 {
  
  // Distribution required?
  if (!Problem_has_been_distributed)
   {
    oomph_info 
     << " Problem::redistribute() was called on non-distributed Problem" 
     << std::endl;
    oomph_info << "Calling Problem::distribute() first..." << std::endl;
    distribute(doc_info,report_stats);
   } 

  // Redistribute the mesh
  mesh_pt()->redistribute(doc_info,report_stats);
  
  // Re-assign the equation numbers (incl synchronisation if reqd)
  assign_eqn_numbers();
 }



 
#endif


//===================================================================
/// Build a single (global) mesh from a number
/// of submeshes which are passed as a vector of pointers to the
/// submeshes. The ordering is not necessarily optimal.
//==============================================================
void Problem::build_global_mesh()
{
#ifdef PARANOID
 //Has a global mesh already been built
 if(Mesh_pt!=0)
  {
   std::string error_message = 
    "Problem::build_global_mesh() called,\n";
   error_message += " but a global mesh has already been built:\n";
   error_message += "Problem::Mesh_pt is not zero!\n";

   throw OomphLibError(error_message,
                       "Problem::build_global_mesh()",
                       OOMPH_EXCEPTION_LOCATION);
  }
 //Check that there are submeshes
 if(Sub_mesh_pt.size()==0)
  {
   std::string error_message = 
    "Problem::build_global_mesh() called,\n";
   error_message += " but there are no submeshes:\n";
   error_message += "Problem::Sub_mesh_pt has no entries\n";
   
   throw OomphLibError(error_message,
                       "Problem::build_global_mesh()",
                       OOMPH_EXCEPTION_LOCATION);
  }
#endif

 //Create an empty mesh
 Mesh_pt = new Mesh();

 //Call the rebuild function to construct the mesh
 rebuild_global_mesh();
}

//====================================================================
/// If one of the submeshes has changed (e.g. by
/// mesh adaptation) we need to update the global mesh.
/// \b Note: The nodes boundary information refers to the
/// boundary numbers within the submesh!
/// N.B. This is essentially the same function as the Mesh constructor
/// that assembles a single global mesh from submeshes
//=====================================================================
void Problem::rebuild_global_mesh()
{

 //Number of submeshes
 unsigned nsub_mesh=Sub_mesh_pt.size();

 // Initialise element, node and boundary counters for global mesh
 unsigned long n_element=0;
 unsigned long n_node=0;
 unsigned n_bound=0;

 // Loop over submeshes and get total number of elements, nodes and
 // boundaries
 for(unsigned imesh=0;imesh<nsub_mesh;imesh++)
  {
   n_element += Sub_mesh_pt[imesh]->nelement();
   n_node += Sub_mesh_pt[imesh]->nnode();
   n_bound += Sub_mesh_pt[imesh]->nboundary();
  }  
 
 // Reserve storage for element and node pointers 
 Mesh_pt->Element_pt.clear();
 Mesh_pt->Element_pt.reserve(n_element);
 Mesh_pt->Node_pt.clear();
 Mesh_pt->Node_pt.reserve(n_node);
 //Resize vector of vectors of nodes
 Mesh_pt->Boundary_node_pt.clear();
 Mesh_pt->Boundary_node_pt.resize(n_bound);
 

 // Sets of pointers to elements and nodes (to exlude duplicates -- they
 // shouldn't occur anyway but if they do, they must only be added
 // once in the global mesh to avoid trouble in the timestepping)
 std::set<GeneralisedElement*> element_set_pt;
 std::set<Node*> node_set_pt;

 //Counter for total number of boundaries in all the submeshes
 unsigned ibound_global=0;   
 //Loop over the number of submeshes 
 for(unsigned imesh=0;imesh<nsub_mesh;imesh++)
  {
   //Loop over the elements of the submesh and add to vector
   //duplicates are ignored
   unsigned nel_before=0;
   unsigned long n_element=Sub_mesh_pt[imesh]->nelement();
   for (unsigned long e=0;e<n_element;e++)
    {
     GeneralisedElement* el_pt=Sub_mesh_pt[imesh]->element_pt(e);
      element_set_pt.insert(el_pt);
     // Was it a duplicate?
     unsigned nel_now=element_set_pt.size();
     if (nel_now==nel_before)
      {
       std::ostringstream warning_stream;
       warning_stream  <<"WARNING: " << std::endl
                       <<"Element " << e << " in submesh " << imesh 
                       <<" is a duplicate \n and was ignored when assembling " 
                       <<"global mesh." << std::endl;
       OomphLibWarning(warning_stream.str(),
                       "Problem::rebuild_global_mesh()",
                       OOMPH_EXCEPTION_LOCATION);
      }
     else
      {
       Mesh_pt->Element_pt.push_back(el_pt);
      }
     nel_before=nel_now;
    }

   //Loop over the nodes of the submesh and add to vector
   //duplicates are ignored
   unsigned nnod_before=0;
   unsigned long n_node=Sub_mesh_pt[imesh]->nnode();
   for (unsigned long n=0;n<n_node;n++)
    {
     Node* nod_pt=Sub_mesh_pt[imesh]->node_pt(n);
     node_set_pt.insert(nod_pt);
     // Was it a duplicate?
     unsigned nnod_now=node_set_pt.size();
     if (nnod_now==nnod_before)
      {
       std::ostringstream warning_stream;
       warning_stream << "WARNING: " << std::endl
                      << "Node " << n << " in submesh " << imesh 
                      << " is a duplicate \n and was ignored when assembling " 
                      << "global mesh." << std::endl;
       OomphLibWarning(warning_stream.str(),
                       "Problem::rebuild_global_mesh()",
                       OOMPH_EXCEPTION_LOCATION);
      }
     else
      {
       Mesh_pt->Node_pt.push_back(nod_pt);
      }
     nnod_before=nnod_now;
    }

   //Loop over the boundaries of the submesh
   unsigned n_bound=Sub_mesh_pt[imesh]->nboundary();
   for (unsigned ibound=0;ibound<n_bound;ibound++)
    {
     //Loop over the number of nodes on the boundary and add to the 
     //global vector
     unsigned long n_bound_node=Sub_mesh_pt[imesh]->nboundary_node(ibound);
     for (unsigned long n=0;n<n_bound_node;n++)
      {
       Mesh_pt->Boundary_node_pt[ibound_global].push_back(
        Sub_mesh_pt[imesh]->boundary_node_pt(ibound,n));
      }
     //Increase the number of the global boundary counter
     ibound_global++;
    }
  } //End of loop over submeshes

}





//================================================================
///  Add a timestepper to the problem. The function will automatically
/// create or resize the Time object so that it contains the appropriate
/// number of levels of storage.
//================================================================
void Problem::add_time_stepper_pt(TimeStepper* const &time_stepper_pt) 
{
 //Add the timestepper to the vector
 Time_stepper_pt.push_back(time_stepper_pt);
 //Find the number of timesteps required by the timestepper
 unsigned ndt = time_stepper_pt->ndt();
 //If time has not been allocated, create time object with the 
 //required number of time steps
 if(Time_pt==0) 
  {
   Time_pt = new Time(ndt);
   oomph_info << "Created Time with " << ndt << " timesteps" << std::endl;
  }
 else
  {
   //If the required number of time steps is greater than currently stored
     //resize the time storage
   if(ndt > Time_pt->ndt()) 
    {
     Time_pt->resize(ndt);
     oomph_info << "Resized Time to include " << ndt << " timesteps" 
               << std::endl;
    }
   //Otherwise report that we are OK
   else
    {
     oomph_info << "Time object already has storage for " << ndt 
               << " timesteps" << std::endl;
    }
  }
 
 //Pass the pointer to time to the timestepper
 time_stepper_pt->time_pt() = Time_pt;
}




//================================================================
/// Assign all equation numbers for problem: Deals with global
 /// data (= data that isn't attached to any elements) and then
 /// does the equation numbering for the elements.
//================================================================
unsigned long Problem::assign_eqn_numbers()
{

#ifdef OOMPH_HAS_MPI

 // If the problem has been distributed we first have to 
 // classify any potentially newly created nodes as
 // halo or haloed (or neither)
 if (Problem_has_been_distributed)
  {
   // Classify any so-far unclassified nodes
   mesh_pt()->classify_halo_and_haloed_nodes();

   // Check the synchronicity of hanging nodes for a refineable mesh
   // - In a multi-physics case this should be called for every mesh
   // - It is also possible that a single mesh contains different elements 
   //   (with different values of ncont_interpolated_values);
   //   in this instance, this routine needs a rethink.
   if(RefineableMeshBase* mmesh_pt = 
      dynamic_cast<RefineableMeshBase*>(mesh_pt(0))) 
    {
     unsigned ncont_interpolated_values=dynamic_cast<RefineableElement*>
      (mmesh_pt->element_pt(0))->ncont_interpolated_values();
     mmesh_pt->synchronise_hanging_nodes(ncont_interpolated_values);
    }
  }
 
#endif

 //(Re)-set the dof pointer to zero length because entries are 
 //pushed back onto it -- if it's not reset here then we get into
 //trouble during mesh refinement when we reassign all dofs
 Dof_pt.resize(0);
 unsigned long n_dof=0;

 //Reset the equation number
 unsigned long equation_number=0;

 // Loop over all elements in the mesh and set up any additional 
 // dependencies that they may have (e.g. storing the geometric
 // Data, i.e. Data that affects an element's shape in elements
 // with algebraic node-update functions
 unsigned nel=Mesh_pt->nelement();
 for (unsigned e=0;e<nel;e++)
  {
   Mesh_pt->element_pt(e)->complete_setup_of_dependencies();
  }

 //Now set equation numbers for the global Data
 unsigned Nglobal_data = nglobal_data();
 for(unsigned i=0;i<Nglobal_data;i++)
  {Global_data_pt[i]->assign_eqn_numbers(equation_number,Dof_pt);}

 //Check that the Mesh_pt has been assigned
 if(Mesh_pt==0)
  {
   std::string error_message =
    "(Global) Mesh_pt must be assigned before calling ";
   error_message += " assign_eqn_numbers()\n";;

   throw OomphLibError(error_message,
                       "Problem::assign_eqn_numbers()",
                       OOMPH_EXCEPTION_LOCATION);
  }
   
 // Loop over the submeshes: Note we need to call the submeshes' own
 // assign_*_eqn_number() otherwise we miss additional functionality
 // that is implemented (e.g.) in SolidMeshes!
 unsigned nsub_mesh=Sub_mesh_pt.size();
 if (nsub_mesh==0)
  {
   n_dof=Mesh_pt->assign_global_eqn_numbers(Dof_pt);
   Mesh_pt->assign_local_eqn_numbers();
   //Clear out the temporary global storage for hijacked equation numbers
   //HijackedElementBase::reset_hijacked_data_pt();
  }
 else
  {
   //Assign global equation numbers first
   for (unsigned i=0;i<nsub_mesh;i++)
    {
     Sub_mesh_pt[i]->assign_global_eqn_numbers(Dof_pt);
    }
   for (unsigned i=0;i<nsub_mesh;i++)
    {
     Sub_mesh_pt[i]->assign_local_eqn_numbers();
    }
   //Clear out the temporaray global storage for hijacked equation numbers
   //HijackedElementBase::reset_hijacked_data_pt();
   n_dof=Dof_pt.size();
  }

#ifdef OOMPH_HAS_MPI

 // Only synchronise if the problem has actually been
 // distributed.
 if (Problem_has_been_distributed)
  {
   // Wait until all processes have assigned their eqn numbers
   MPI_Barrier(MPI_COMM_WORLD);
   
   // Synchronise the equation numbers and return the total
   // number of degrees of freedom in the overall problem
   n_dof=synchronise_eqn_numbers();
  }
 
#endif

 return n_dof;

}

//================================================================
/// Get the vector of dofs, i.e. a vector containing the current
/// values of all unknowns.
//================================================================
void Problem::get_dofs(Vector<double>& dofs)
{
 //Find number of dofs
 unsigned long n_dof = ndof();

 //Copy dofs into vector
 for(unsigned long l=0;l<n_dof;l++)
  {
   dofs[l] = *Dof_pt[l];
  }
}


//================================================================
/// Get the total residuals Vector for the problem
//================================================================
void Problem::get_residuals(Vector<double> &residuals)
{
 // Three different cases; if MPI_Helpers::MPI_has_been_initialised=true 
 // this means MPI_Helpers::setup() has been called.  This could happen on a
 // code compiled with MPI but run serially; in this instance the
 // get_residuals function still works on one processor.
 //
 // Secondly, if a code has been compiled with MPI, but MPI_Helpers::setup()
 // has not been called, then MPI_Helpers::MPI_has_been_initialised=false 
 // and the code calls...
 //
 // Thirdly, the serial version (compiled by all, but only run when compiled
 // with MPI if MPI_Helpers::MPI_has_been_initialised=false
 //
 // The only case where an MPI code cannot run serially at present
 // is one where the distribute function is used (i.e. METIS is called)

#ifdef OOMPH_HAS_MPI
 if (MPI_Helpers::MPI_has_been_initialised)
  {
   // Number of dofs
   int n_dof=ndof();
 
   // Vector to hold the partial residuals if Nproc>1
   Vector<double> partial_residuals;

   // pointer to the Vector used in the residuals assembly
   Vector<double>* residuals_pt;

   // set residuals_pt 
   if (MPI_Helpers::Nproc>1)
    {
     residuals_pt = &partial_residuals;
    }
   else
    { 
     residuals_pt = &residuals;
    }

   // number of elements
   int n_el=mesh_pt()->nelement();
 
   // Default assignments for distributed problem
   unsigned j_lo=0;
   unsigned j_hi=n_el;
 
   // Otherwise just loop over fractions of the elements
   if (!Problem_has_been_distributed)
    {
     // Distribute work evenly
     unsigned range=unsigned(double(n_el)/double(MPI_Helpers::Nproc));
     j_lo=MPI_Helpers::My_rank*range;
     j_hi=(MPI_Helpers::My_rank+1)*range;
    
     // Last one needs to incorporate any dangling elements
     if (MPI_Helpers::My_rank==MPI_Helpers::Nproc-1) j_hi=n_el;
    }
 
   // Initialise the partial_residuals Vector to zero
   residuals_pt->clear();
   residuals_pt->resize(n_dof,0.0);

   // Assemble the partial residual vector: Note that this
   // is a full-length vector but only contributions from 
   // a sub-set of elements are filled in, other values
   // are set to zero
 
   //Loop over all the elements for this processor
   for(unsigned long e=j_lo;e<j_hi;e++)
    {
     // Get element
     GeneralisedElement* el_pt=mesh_pt()->element_pt(e);
   
     // Is it a halo?
     if (!el_pt->is_halo())
      {
       //Find number of dofs in the element
       const unsigned n_el_dofs = el_pt->ndof();
     
       //Set up a Vector
       Vector<double> element_residuals(n_el_dofs);
     
       //Fill the array
       el_pt->get_residuals(element_residuals);
       
       //Now loop over the dofs and assign values to global Vector
       for(unsigned l=0;l<n_el_dofs;l++)
        {
         (*residuals_pt)[el_pt->eqn_number(l)]+=element_residuals[l];
        }
      }
    }
 
   // Receive from the other processors and assemble if required
   if (MPI_Helpers::Nproc>1)
    {
     // clear and resize residuals
     residuals.clear();
     residuals.resize(n_dof, 0.0);
     MPI_Allreduce(&partial_residuals[0], &residuals[0], n_dof,
                   MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    }
  

  }
 else // !MPI_Helpers::MPI_has_been_initialised
#endif // OOMPH_HAS_MPI
  {
   //Find number of dofs
   unsigned long n_dof = ndof();
   residuals.resize(n_dof);

   //Initialise the residuals Vector to zero
   for(unsigned long l=0;l<n_dof;l++) residuals[l] = 0.0;

   //Locally cache pointer to assembly handler
   AssemblyHandler* const assembly_handler_pt = Assembly_handler_pt;

   //Loop over all the elements
   unsigned long Element_pt_range = Mesh_pt->nelement();
   for(unsigned long e=0;e<Element_pt_range;e++)
    {
     //Get the pointer to the element
     GeneralisedElement* elem_pt = Mesh_pt->element_pt(e);
     //Find number of dofs in the element
     unsigned n_element_dofs = assembly_handler_pt->ndof(elem_pt); 
     //Set up an array
     Vector<double> element_residuals(n_element_dofs);
     //Fill the array
     assembly_handler_pt->get_residuals(elem_pt,element_residuals);
     //Now loop over the dofs and assign values to global Vector
     for(unsigned l=0;l<n_element_dofs;l++)
      {
       residuals[assembly_handler_pt->eqn_number(elem_pt,l)] 
        += element_residuals[l];
      }
    }
  }
}

//================================================================
/// Get the fully assembled residual vector and Jacobian matrix
/// in dense storage.
//================================================================
void Problem::get_jacobian(Vector<double> &residuals, 
                           DenseDoubleMatrix& jacobian)
{
 // Resize the matrices -- this cannot always be done externally
 // because get_jacobian exists in many different versions for
 // different storage formats -- resizing a CC or CR matrix doesn't
 // make sense.
 unsigned n_dof=ndof(); 
 residuals.resize(n_dof);
 jacobian.resize(n_dof,n_dof);
 residuals.initialise(0.0);
 jacobian.initialise(0.0);
 
 //Locally cache pointer to assembly handler
 AssemblyHandler* const assembly_handler_pt = Assembly_handler_pt;

 //Loop over all the elements
 unsigned long n_element = Mesh_pt->nelement();
 for(unsigned long e=0;e<n_element;e++)
  {
   //Get the pointer to the element
   GeneralisedElement* elem_pt = Mesh_pt->element_pt(e);  
   //Find number of dofs in the element
   unsigned n_element_dofs = assembly_handler_pt->ndof(elem_pt); 
   //Set up an array
   Vector<double> element_residuals(n_element_dofs);
   //Set up a matrix
   DenseMatrix<double> element_jacobian(n_element_dofs);
   //Fill the array
   assembly_handler_pt->get_jacobian(elem_pt,
                                     element_residuals,element_jacobian);
   //Now loop over the dofs and assign values to global Vector
   for(unsigned l=0;l<n_element_dofs;l++)
    {
     unsigned long eqn_number = assembly_handler_pt->eqn_number(elem_pt,l);
     residuals[eqn_number] += element_residuals[l];
     for(unsigned l2=0;l2<n_element_dofs;l2++)
      {
       jacobian(eqn_number ,
                assembly_handler_pt->eqn_number(elem_pt,l2)) += 
        element_jacobian(l,l2);
      }
    }
  }
}

//======================================================================
/// Return the fully-assembled Jacobian and residuals for the problem,
/// in the case where the Jacobian matrix is in a row compressed storage
/// format.
//======================================================================
void Problem::get_jacobian(Vector<double> &residuals, CRDoubleMatrix &jacobian)
{
 // Three different cases; if MPI_Helpers::MPI_has_been_initialised=true 
 // this means MPI_Helpers::setup() has been called.  This could happen on a
 // code compiled with MPI but run serially; in this instance the
 // get_residuals function still works on one processor.
 //
 // Secondly, if a code has been compiled with MPI, but MPI_Helpers::setup()
 // has not been called, then MPI_Helpers::MPI_has_been_initialised=false 
 // and the code calls...
 //
 // Thirdly, the serial version (compiled by all, but only run when compiled
 // with MPI if MPI_Helpers::MPI_has_been_initialised=false
 //
 // The only case where an MPI code cannot run serially at present
 // is one where the distribute function is used (i.e. METIS is called)

 //Allocate storage for the matrix entries
 //The generalised Vector<Vector<>> structure is required
 //for the most general interface to sparse_assemble() which allows
 //the assembly of multiple matrices at once.
 Vector<Vector<int> > column_index(1);
 Vector<Vector<int> > row_start(1);
 Vector<Vector<double> > value(1); 
 //Allocate generalised storage format for passing to sparse_assemble()
 Vector<Vector<double>*> residuals_vector(1);
 //Set the residuals passed to sparse assemble to be those passed
 //into this function
 residuals_vector[0] = &residuals;
 
 //The matrix is in compressed row format
 bool compressed_row_flag=true;

#ifdef OOMPH_HAS_MPI
 if (MPI_Helpers::MPI_has_been_initialised)
  {
   // Get matrix rows and residual in CR format
   global_matrix_sparse_assemble(column_index,
                                 row_start,
                                 value,
                                 residuals_vector,
                                 compressed_row_flag);
  }
 else
#endif
  {
   //Call the helper function sparse_assemble
   sparse_assemble_row_or_column_compressed(column_index,
                                            row_start,
                                            value,
                                            residuals_vector,
                                            compressed_row_flag);
  }

 //The jacobian is the first (and only) matrix assembled by
 //the helper function
 //Get the number of dofs (the size of the matrix)
 unsigned long n_dof = ndof();
 //Build the jacobian
 jacobian.build(value[0],column_index[0],row_start[0],n_dof,n_dof);

}


//=======================================================================
/// Return the fully-assembled Jacobian and residuals for the problem,
/// in the case when the jacobian matrix is in column-compressed storage
/// format.
//=======================================================================
void Problem::get_jacobian(Vector<double> &residuals, CCDoubleMatrix &jacobian)
{
 // Three different cases; if MPI_Helpers::MPI_has_been_initialised=true 
 // this means MPI_Helpers::setup() has been called.  This could happen on a
 // code compiled with MPI but run serially; in this instance the
 // get_residuals function still works on one processor.
 //
 // Secondly, if a code has been compiled with MPI, but MPI_Helpers::setup()
 // has not been called, then MPI_Helpers::MPI_has_been_initialised=false 
 // and the code calls...
 //
 // Thirdly, the serial version (compiled by all, but only run when compiled
 // with MPI if MPI_Helpers::MPI_has_been_initialised=false
 //
 // The only case where an MPI code cannot run serially at present
 // is one where the distribute function is used (i.e. METIS is called)

 //Allocate storage for the matrix entries
 //The generalised Vector<Vector<>> structure is required
 //for the most general interface to sparse_assemble() which allows
 //the assembly of multiple matrices at once.
 Vector<Vector<int> > row_index(1);
 Vector<Vector<int> > column_start(1);
 Vector<Vector<double> > value(1); 
 //Allocate generalised storage format for passing to sparse_assemble()
 Vector<Vector<double>*> residuals_vector(1);
 //Set the residuals passed to sparse assemble to be those passed
 //into this function
 residuals_vector[0] = &residuals;
 
 //The matrix is in compressed column format
 bool compressed_row_flag=false;
 
#ifdef OOMPH_HAS_MPI
 if (MPI_Helpers::MPI_has_been_initialised)
  {
   // Get matrix rows and residual in CC format
   global_matrix_sparse_assemble(row_index,
                                 column_start,
                                 value,
                                 residuals_vector,
                                 compressed_row_flag);
  }
 else
#endif
  {
   //Call the helper function sparse_assemble
   sparse_assemble_row_or_column_compressed(row_index,
                                            column_start,
                                            value,
                                            residuals_vector,
                                            compressed_row_flag);
  }
 
 //The jacobian is the first (and only) matrix assembled by
 //the helper function
 //Get the number of dofs (size of the matrix)
 unsigned long n_dof = ndof();
 //Build the Jacobian
 jacobian.build(value[0],row_index[0],column_start[0],n_dof,n_dof);

}

#ifdef OOMPH_HAS_MPI

//=============================================================================
/// Compute the fully-assembled Jacobian and residuals for the problem.
/// Interface for the case when the Jacobian is in distributed 
/// row-compressed storage format. 
/// Returns the residual vector as a distributed vector
//=============================================================================
void Problem::get_jacobian(DistributedVector<double> &residuals,
                           DistributedCRDoubleMatrix &jacobian)
{
 // Provide storage
 Vector<Vector<int> > column_index(1);
 Vector<Vector<int> > row_start(1);
 Vector<Vector<double> > value(1);
 Vector<Vector<double>*> residuals_vector(1);
 Vector<double> temp_residuals;
 residuals_vector[0] = &temp_residuals;
 unsigned long first_row;
 unsigned long n_row_local;
 unsigned long n_row_total;
 
 // Get my block of rows
 distributed_matrix_sparse_assemble(column_index,
                                    row_start,
                                    value,
                                    residuals_vector,
                                    first_row,
                                    n_row_local,
                                    n_row_total,
                                    true);

 // create the distribution to assemble the distributed jacobian and residal
 DistributionInfo my_distribution(MPI_COMM_WORLD,first_row,n_row_local,
                                  n_row_total);

 // and then assembled the distributed matrix
 jacobian.build(value[0],
                column_index[0],
                row_start[0],
                my_distribution,
                n_row_total);

 // and then assemble the distributed vector
 residuals.distribute(my_distribution);
 unsigned nrow_local = my_distribution.nrow_local();
 for (unsigned i = 0; i < nrow_local; i++)
  {
   residuals[i] = temp_residuals[i];
  }
}

#endif


//=====================================================================
/// This is a (private) helper function that is used to assemble system
/// matrices in compressed row or column format
/// and compute residual vectors.
/// The default action is to assemble the jacobian matrix and 
/// residuals for the Newton method. The action can be
/// overloaded at an elemental level by chaging the default
/// behaviour of the function Element::get_all_vectors_and_matrices().
/// column_or_row_index: Column [or row] index of given entry
/// row_or_column_start: Index of first entry for given row [or column]
/// value              : Vector of nonzero entries
/// residuals          : Residual vector
/// compressed_row_flag: Bool flag to indicate if storage format is 
///                      compressed row [if false interpretation of
///                      arguments is as stated in square brackets].
/// We provide four different assembly methods, each with different
/// memory requirements/execution speeds. The method is set by
/// the public flag Problem::Sparse_assembly_method.
//=====================================================================
void Problem::sparse_assemble_row_or_column_compressed(
 Vector<Vector<int> > &column_or_row_index, 
 Vector<Vector<int> > &row_or_column_start, 
 Vector<Vector<double> > &value, 
 Vector<Vector<double>*> &residuals,
 bool compressed_row_flag)
{

 // Choose the actual method
 switch(Sparse_assembly_method)
  {

  case Perform_assembly_using_vectors_of_pairs:

   sparse_assemble_row_or_column_compressed_with_vectors_of_pairs(
    column_or_row_index, 
    row_or_column_start, 
    value, 
    residuals,
    compressed_row_flag);

    break;

  case Perform_assembly_using_two_vectors:

   sparse_assemble_row_or_column_compressed_with_two_vectors(
    column_or_row_index, 
    row_or_column_start, 
    value, 
    residuals,
    compressed_row_flag);

   break;

  case Perform_assembly_using_maps:

   sparse_assemble_row_or_column_compressed_with_maps(
    column_or_row_index, 
    row_or_column_start, 
    value, 
    residuals,
    compressed_row_flag);

   break;

  case Perform_assembly_using_lists:

   sparse_assemble_row_or_column_compressed_with_lists(
    column_or_row_index, 
    row_or_column_start, 
    value, 
    residuals,
    compressed_row_flag);

   break;

  default:

   std::ostringstream error_stream;
   error_stream
    << "Error: Incorrect value for Problem::Sparse_assembly_method" 
    << Sparse_assembly_method << std::endl
    << "It should be one of the enumeration Problem::Assembly_method" 
    << std::endl;
   throw OomphLibError(error_stream.str(),
                       "Problem::sparse_assemble_row_or_column_compressed",
                       OOMPH_EXCEPTION_LOCATION);
  }

}





//=====================================================================
/// This is a (private) helper function that is used to assemble system
/// matrices in compressed row or column format
/// and compute residual vectors, using maps
/// The default action is to assemble the jacobian matrix and 
/// residuals for the Newton method. The action can be
/// overloaded at an elemental level by chaging the default
/// behaviour of the function Element::get_all_vectors_and_matrices().
/// column_or_row_index: Column [or row] index of given entry
/// row_or_column_start: Index of first entry for given row [or column]
/// value              : Vector of nonzero entries
/// residuals          : Residual vector
/// compressed_row_flag: Bool flag to indicate if storage format is 
///                      compressed row [if false interpretation of
///                      arguments is as stated in square brackets].
//=====================================================================
void Problem::sparse_assemble_row_or_column_compressed_with_maps(
 Vector<Vector<int> > &column_or_row_index, 
 Vector<Vector<int> > &row_or_column_start, 
 Vector<Vector<double> > &value, 
 Vector<Vector<double>*> &residuals,
 bool compressed_row_flag)
{
 //Total number of elements
 const unsigned long n_elements = mesh_pt()->nelement();

 // Default range of elements for distributed problems
 unsigned long el_lo=0;
 unsigned long el_hi=n_elements;

#ifdef OOMPH_HAS_MPI
 // Otherwise just loop over a fraction of the elements
 if (!Problem_has_been_distributed)
  {
   // Distribute work evenly
   unsigned long range = 
    static_cast<unsigned long>(double(n_elements)/double(MPI_Helpers::Nproc));
   el_lo = MPI_Helpers::My_rank*range;
   el_hi = (MPI_Helpers::My_rank+1)*range;
   
   // Last one needs to incorporate any dangling elements
   if (MPI_Helpers::My_rank==MPI_Helpers::Nproc-1) el_hi = n_elements;
  }
#endif 

 //Total number of degrees of freedom
 const unsigned long n_dof = Problem::ndof();

 //Find the number of vectors to be assembled
 const unsigned n_vector = residuals.size();

 //Find the number of matrices to be assembled
 const unsigned n_matrix = column_or_row_index.size();

  //Locally cache pointer to assembly handler
 AssemblyHandler* const assembly_handler_pt = Assembly_handler_pt;

//Error check dimensions
#ifdef PARANOID
 if(row_or_column_start.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error: " << std::endl
    << "row_or_column_start.size() " << row_or_column_start.size() 
    << " does not equal "
    << "column_or_row_index.size() " 
    <<  column_or_row_index.size() << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_maps",
    OOMPH_EXCEPTION_LOCATION);
  }

if(value.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error in Problem::sparse_assemble_row_or_column_compressed " 
    << std::endl
    << "value.size() " << value.size() << " does not equal "
    << "column_or_row_index.size() " 
    << column_or_row_index.size() << std::endl<< std::endl
    << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_maps",
    OOMPH_EXCEPTION_LOCATION);
  }
#endif


//The idea behind this sparse assembly routine is to use a vector of
//maps for the entries in each row or column of the complete matrix.
//The key for each map is the global row or column number and
//the default comparison operator for integers means that each map 
//is ordered by the global row or column number. Thus, we need not
//sort the maps, that happens at each insertion of a new entry. The 
//price we pay  is that for large maps, inseration is not a 
//cheap operation. Hash maps can be used to increase the speed, but then
//the ordering is lost and we would have to sort anyway. The solution if
//speed is required is to use lists, see below.
 
 
//Set up a vector of vectors of maps of entries of each  matrix,
//indexed by either the column or row. The entries of the vector for
//each matrix correspond to all the rows or columns of that matrix. 
//The use of the map storage
//scheme, with its implicit ordering on the first index, gives
//a sparse ordered list of the entries in the given row or column. 
 Vector<Vector<std::map<unsigned,double> > > matrix_data_map(n_matrix);
 //Loop over the number of matrices being assembled and resize
 //each vector of maps to the number of rows or columns of the matrix
 for(unsigned m=0;m<n_matrix;m++) {matrix_data_map[m].resize(n_dof);}
 
 //Resize the residuals vectors
 for(unsigned v=0;v<n_vector;v++) {residuals[v]->resize(n_dof);}
   
 //----------------Assemble and populate the maps-------------------------
 {
  //Allocate local storage for the element's contribution to the
  //residuals vectors and system matrices of the size of the maximum
  //number of dofs in any element.
  //This means that the storage is only allocated (and deleted) once
  Vector<Vector<double> > el_residuals(n_vector);
  Vector<DenseMatrix<double> > el_jacobian(n_matrix);

  //Loop over the elements for this processor
  for(unsigned long e=el_lo;e<el_hi;e++)
   {
    //Get the pointer to the element
    GeneralisedElement* elem_pt = mesh_pt()->element_pt(e);

#ifdef OOMPH_HAS_MPI
    //Ignore halo elements
    if (!elem_pt->is_halo())
     {
#endif

    //Find number of degrees of freedom in the element
    const unsigned nvar = assembly_handler_pt->ndof(elem_pt);

    //Resize the storage for elemental jacobian and residuals
    for(unsigned v=0;v<n_vector;v++) {el_residuals[v].resize(nvar);}
    for(unsigned m=0;m<n_matrix;m++) {el_jacobian[m].resize(nvar);}
    
    //Now get the residuals and jacobian for the element
    assembly_handler_pt->
     get_all_vectors_and_matrices(elem_pt,el_residuals, el_jacobian);

    //---------------Insert the values into the maps--------------

    //Loop over the first index of local variables
    for(unsigned i=0;i<nvar;i++)
     {
      //Get the local equation number
      unsigned eqn_number = assembly_handler_pt->eqn_number(elem_pt,i);
      //Add the contribution to the residuals
      for(unsigned v=0;v<n_vector;v++)
       {
        //Fill in each residuals vector
        (*residuals[v])[eqn_number] += el_residuals[v][i];
       }

      //Now loop over the other index
      for(unsigned j=0;j<nvar;j++)
       {
        //Get the number of the unknown
        unsigned unknown = assembly_handler_pt->eqn_number(elem_pt,j);

        //Loop over the matrices
        for(unsigned m=0;m<n_matrix;m++)
         {
          //Get the value of the matrix at this point
          double value = el_jacobian[m](i,j);
          //Only bother to add to the map if it's non-zero
          if(std::abs(value) > Numerical_zero_for_sparse_assembly)
           {
            //If it's compressed row storage, then our vector of maps
            //is indexed by row (equation number)
            if(compressed_row_flag)
             {
              //Add the data into the map using the unknown as the map key
              matrix_data_map[m][eqn_number][unknown] += value;
             }
            //Otherwise it's compressed column storage and our vector is
            //indexed by column (the unknown)
            else
             {
              //Add the data into the map using the eqn_numbe as the map key
              matrix_data_map[m][unknown][eqn_number] += value;
             }
           }
         } //End of loop over matrices
       }
     }

#ifdef OOMPH_HAS_MPI
     } // endif halo element
#endif
   } //End of loop over the elements

 } //End of map assembly
 
 //-----------Finally we need to convert the beautiful map storage scheme
 //------------------------to the containers required by SuperLU
 
 //Loop over the number of matrices
 for(unsigned m=0;m<n_matrix;m++)
  {
   //Set the number of rows or columns
   row_or_column_start[m].resize(n_dof+1);
   //Counter for the total number of entries in the storage scheme
   unsigned long entry_count=0;
   row_or_column_start[m][0] = entry_count;
   
   //Now we merely loop over the number of rows or columns 
   for(unsigned long i_global=0;i_global<n_dof;i_global++)
    {
     //Start index for the present row
     row_or_column_start[m][i_global] = entry_count;
     //If there are no entries in the map then skip the rest of the loop
     if(matrix_data_map[m][i_global].empty()) {continue;}

     //Loop over all the entries in the map corresponding to the given
     //row or column. It will be ordered
     for(std::map<unsigned,double>::iterator 
          it = matrix_data_map[m][i_global].begin();
         it!=matrix_data_map[m][i_global].end();++it)
      {
       //The first value is the column or row index
       column_or_row_index[m].push_back(it->first);
       //The second value is the actual data value
       value[m].push_back(it->second);
       //Increase the value of the counter
       entry_count++;
      }
    }
     
   //Final entry in the row/column start vector
   row_or_column_start[m][n_dof] = entry_count;
  } //End of the loop over the matrices

 if (Pause_at_end_of_sparse_assembly)
  {
   oomph_info << "Pausing at end of sparse assembly." << std::endl;
   pause("Check memory usage now.");
  }
} 






//=====================================================================
/// This is a (private) helper function that is used to assemble system
/// matrices in compressed row or column format
/// and compute residual vectors using lists
/// The default action is to assemble the jacobian matrix and 
/// residuals for the Newton method. The action can be
/// overloaded at an elemental level by chaging the default
/// behaviour of the function Element::get_all_vectors_and_matrices().
/// column_or_row_index: Column [or row] index of given entry
/// row_or_column_start: Index of first entry for given row [or column]
/// value              : Vector of nonzero entries
/// residuals          : Residual vector
/// compressed_row_flag: Bool flag to indicate if storage format is 
///                      compressed row [if false interpretation of
///                      arguments is as stated in square brackets].
//=====================================================================
void Problem::sparse_assemble_row_or_column_compressed_with_lists(
 Vector<Vector<int> > &column_or_row_index, 
 Vector<Vector<int> > &row_or_column_start, 
 Vector<Vector<double> > &value, 
 Vector<Vector<double>*> &residuals,
 bool compressed_row_flag)
{
 //Total number of elements
 const unsigned long n_elements = mesh_pt()->nelement();

 // Default range of elements for distributed problems
 unsigned long el_lo=0;
 unsigned long el_hi=n_elements;

#ifdef OOMPH_HAS_MPI
 // Otherwise just loop over a fraction of the elements
 if (!Problem_has_been_distributed)
  {
   // Distribute work evenly
   unsigned long range = 
    static_cast<unsigned long>(double(n_elements)/double(MPI_Helpers::Nproc));
   el_lo = MPI_Helpers::My_rank*range;
   el_hi = (MPI_Helpers::My_rank+1)*range;
   
   // Last one needs to incorporate any dangling elements
   if (MPI_Helpers::My_rank==MPI_Helpers::Nproc-1) el_hi = n_elements;
  }
#endif 

 //Total number of degrees of freedom
 const unsigned long n_dof = Problem::ndof();

 //Find the number of vectors to be assembled
 const unsigned n_vector = residuals.size();

 //Find the number of matrices to be assembled
 const unsigned n_matrix = column_or_row_index.size();

 //Locally cache pointer to assembly handler
 AssemblyHandler* const assembly_handler_pt = Assembly_handler_pt;

//Error check dimensions
#ifdef PARANOID
 if(row_or_column_start.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error: " << std::endl
    << "row_or_column_start.size() " << row_or_column_start.size() 
    << " does not equal "
    << "column_or_row_index.size() " 
    <<  column_or_row_index.size() << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_lists",
    OOMPH_EXCEPTION_LOCATION);
  }

if(value.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error in Problem::sparse_assemble_row_or_column_compressed " 
    << std::endl
    << "value.size() " << value.size() << " does not equal "
    << "column_or_row_index.size() " 
    << column_or_row_index.size() << std::endl<< std::endl
    << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_lists",
    OOMPH_EXCEPTION_LOCATION);
  }
#endif

//The idea behind this sparse assembly routine is to use a vector of
//lists for the entries in each row or column of the complete matrix.
//The lists contain pairs of entries (global row/column number, value).
//All non-zero contributions from each element are added to the lists.
//We then sort each list by global row/column number and then combine
//the entries corresponding to each row/column before adding to the
//vectors column_or_row_index and value.
 
//Note the trade off for "fast assembly" is that we will require
//more memory during the assembly phase. Then again, if we can
//only just assemble the sparse matrix, we're in real trouble.

//Set up a vector of lists of paired entries of 
//(row/column index, jacobian matrix entry). 
//The entries of the vector correspond to all the rows or columns. 
//The use of the list storage scheme, should give fast insertion 
//and fast sorts later.
 Vector<Vector<std::list<std::pair<unsigned,double> > > > 
  matrix_data_list(n_matrix);
 //Loop over the number of matrices and resize
 for(unsigned m=0;m<n_matrix;m++) {matrix_data_list[m].resize(n_dof);}

 //Resize the residuals vectors
 for(unsigned v=0;v<n_vector;v++) {residuals[v]->resize(n_dof);}

 //------------Assemble and populate the lists-----------------------
 {
  //Allocate local storage for the element's contribution to the
  //residuals vectors and system matrices of the size of the maximum
  //number of dofs in any element.
  //This means that the stored is only allocated (and deleted) once
  Vector<Vector<double> > el_residuals(n_vector);
  Vector<DenseMatrix<double> > el_jacobian(n_matrix);


  //Pointer to a single list to be used during the assembly
  std::list<std::pair<unsigned,double> > *list_pt;
    
  //Loop over the all elements
  for(unsigned long e=el_lo;e<el_hi;e++)
   {
    //Get the pointer to the element
    GeneralisedElement* elem_pt = mesh_pt()->element_pt(e);

#ifdef OOMPH_HAS_MPI
    //Ignore halo elements
    if (!elem_pt->is_halo())
     {
#endif

    //Find number of degrees of freedom in the element
    const unsigned nvar = assembly_handler_pt->ndof(elem_pt);
    
    //Resize the storage for the elemental jacobian and residuals
    for(unsigned v=0;v<n_vector;v++) {el_residuals[v].resize(nvar);}
    for(unsigned m=0;m<n_matrix;m++) {el_jacobian[m].resize(nvar);}
    
    //Now get the residuals and jacobian for the element
    assembly_handler_pt->
     get_all_vectors_and_matrices(elem_pt,el_residuals, el_jacobian);
    
    //---------------- Insert the values into the lists -----------
      
    //Loop over the first index of local variables
    for(unsigned i=0;i<nvar;i++)
     {
      //Get the local equation number
      unsigned eqn_number = assembly_handler_pt->eqn_number(elem_pt,i);
      //Add the contribution to the residuals
      for(unsigned v=0;v<n_vector;v++)
       {
        //Fill in the residuals vector
        (*residuals[v])[eqn_number] += el_residuals[v][i];
       }

      //Now loop over the other index
      for(unsigned j=0;j<nvar;j++)
       {
        //Get the number of the unknown
        unsigned unknown = assembly_handler_pt->eqn_number(elem_pt,j);
          
        //Loop over the matrices
        for(unsigned m=0;m<n_matrix;m++)
         {
          //Get the value of the matrix at this point
          double value = el_jacobian[m](i,j);
          //Only add to theif it's non-zero
          if(std::abs(value) > Numerical_zero_for_sparse_assembly)
           {
            //If it's compressed row storage, then our vector is indexed
            //by row (the equation number)
            if(compressed_row_flag)
             {
              //Find the list that corresponds to the desired row
              list_pt = &matrix_data_list[m][eqn_number];
              //Insert the data into the list, the first entry
              //in the pair is the unknown (column index),
              //the second is the value itself.
              list_pt->
               insert(list_pt->end(),std::make_pair(unknown,value));
             }
            //Otherwise it's compressed column storage, and our
            //vector is indexed by column (the unknown)
            else
             {
              //Find the list that corresponds to the desired column
              list_pt = &matrix_data_list[m][unknown];
              //Insert the data into the list, the first entry
              //in the pair is the equation number (row index),
              //the second is the value itself.
              list_pt->
               insert(list_pt->end(),std::make_pair(eqn_number,value));
             }
           }
         }
       }
     }

#ifdef OOMPH_HAS_MPI
     } // endif halo element
#endif      
   } //End of loop over the elements
    
 } //list_pt goes out of scope

 //----Finally we need to convert the beautiful list storage scheme---
 //----------to the containers required by SuperLU--------------------

 //Loop over the number of matrices
 for(unsigned m=0;m<n_matrix;m++)
  {
   //Set the number of rows or columns
   row_or_column_start[m].resize(n_dof+1);
   //Counter for the total number of entries in the storage scheme
   unsigned long entry_count=0;
   //The first entry is 0
   row_or_column_start[m][0] = entry_count;
     
   //Now we merely loop over the number of rows or columns 
   for(unsigned long i_global=0;i_global<n_dof;i_global++)
    {
     //Start index for the present row is the number of entries so far
     row_or_column_start[m][i_global] = entry_count;
     //If there are no entries in the list then skip the loop
     if(matrix_data_list[m][i_global].empty()) {continue;}

     //Sort the list corresponding to this row or column by the
     //column or row index (first entry in the pair). 
     //This might be inefficient, but we only have to do the sort ONCE
     //for each list. This is faster than using a map storage scheme, where
     //we are sorting for every insertion (although the map structure 
     //is cleaner and more memory efficient)
     matrix_data_list[m][i_global].sort();
       
     //Set up an iterator for start of the list
     std::list<std::pair<unsigned,double> >::iterator it
      = matrix_data_list[m][i_global].begin();
       
     //Get the first row or column index in the list...
     unsigned current_index = it->first;
     //...and the corresponding value
     double current_value = it->second;
       
     //Loop over all the entries in the sorted list 
     //Increase the iterator so that we start at the second entry
     for(++it;it!=matrix_data_list[m][i_global].end();++it)
      {
       //If the index has not changed, then we must add the contribution
       //of the present entry to the value. 
       //Additionally check that the entry is non-zero
       if((it->first == current_index) && 
          (std::abs(it->second) > Numerical_zero_for_sparse_assembly))
        {
         current_value += it->second;
        }
       //Otherwise, we have added all the contributions to the index
       //to current_value, so add it to the SuperLU data structure
       else
        {
         //Add the row or column index to the vector
         column_or_row_index[m].push_back(current_index);
         //Add the actual value to the vector
         value[m].push_back(current_value);
         //Increase the counter for the number of entries in each vector
         entry_count++;
           
         //Set the index and value to be those of the current entry in the 
         //list
         current_index = it->first;
         current_value = it->second;
        }
      } //End of loop over all list entries for this global row or column
       
     //There are TWO special cases to consider. 
     //If there is only one equation number in the list, then it
     //will NOT have been added. We test this case by comparing the
     //number of entries with those stored in row_or_column_start[i_global]
     //Otherwise
     //If the final entry in the list has the same index as the penultimate
     //entry, then it will NOT have been added to the SuperLU storage scheme
     //Check this by comparing the current_index with the final index
     //stored in the SuperLU scheme. If they are not the same, then
     //add the current_index and value.
       
     //If single equation number in list
     if((static_cast<int>(entry_count) == row_or_column_start[m][i_global])
        //If we have a single equation number, this will not be evaluated.
        //If we don't then we do the test to check that the final
        //entry is added
        ||(static_cast<int>(current_index) != column_or_row_index[m].back()))
      {
       //Add the row or column index to the vector
       column_or_row_index[m].push_back(current_index);
       //Add the actual value to the vector
       value[m].push_back(current_value);
       //Increase the counter for the number of entries in each vector
       entry_count++;
      }
       
    } //End of loop over the rows or columns of the entire matrix
     
   //Final entry in the row/column start vector
   row_or_column_start[m][n_dof] = entry_count;
  } //End of loop over matrices

 if (Pause_at_end_of_sparse_assembly)
  {
   oomph_info << "Pausing at end of sparse assembly." << std::endl;
   pause("Check memory usage now.");
  }

}



//=====================================================================
/// This is a (private) helper function that is used to assemble system
/// matrices in compressed row or column format
/// and compute residual vectors using vectors of pairs
/// The default action is to assemble the jacobian matrix and 
/// residuals for the Newton method. The action can be
/// overloaded at an elemental level by chaging the default
/// behaviour of the function Element::get_all_vectors_and_matrices().
/// column_or_row_index: Column [or row] index of given entry
/// row_or_column_start: Index of first entry for given row [or column]
/// value              : Vector of nonzero entries
/// residuals          : Residual vector
/// compressed_row_flag: Bool flag to indicate if storage format is 
///                      compressed row [if false interpretation of
///                      arguments is as stated in square brackets].
//=====================================================================
void Problem::sparse_assemble_row_or_column_compressed_with_vectors_of_pairs(
 Vector<Vector<int> > &column_or_row_index, 
 Vector<Vector<int> > &row_or_column_start, 
 Vector<Vector<double> > &value, 
 Vector<Vector<double>*> &residuals,
 bool compressed_row_flag)
{
 //Total number of elements
 const unsigned long n_elements = mesh_pt()->nelement();

 // Default range of elements for distributed problems
 unsigned long el_lo=0;
 unsigned long el_hi=n_elements;
 
#ifdef OOMPH_HAS_MPI
 // Otherwise just loop over a fraction of the elements
 if (!Problem_has_been_distributed)
  {
   // Distribute work evenly
   unsigned long range = 
    static_cast<unsigned long>(double(n_elements)/double(MPI_Helpers::Nproc));
   el_lo = MPI_Helpers::My_rank*range;
   el_hi = (MPI_Helpers::My_rank+1)*range;
   
   // Last one needs to incorporate any dangling elements
   if (MPI_Helpers::My_rank==MPI_Helpers::Nproc-1) el_hi = n_elements;
  }
#endif 

 //Total number of degrees of freedom
 const unsigned long n_dof = Problem::ndof();
 
 //Find the number of vectors to be assembled
 const unsigned n_vector = residuals.size();
 
 //Find the number of matrices to be assembled
 const unsigned n_matrix = column_or_row_index.size();
 
 //Locally cache pointer to assembly handler
 AssemblyHandler* const assembly_handler_pt = Assembly_handler_pt;
 
//Error check dimensions
#ifdef PARANOID
 if(row_or_column_start.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error: " << std::endl
    << "row_or_column_start.size() " << row_or_column_start.size() 
    << " does not equal "
    << "column_or_row_index.size() " 
    <<  column_or_row_index.size() << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_vectors_of_pairs",
    OOMPH_EXCEPTION_LOCATION);
  }

if(value.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error: " 
    << std::endl
    << "value.size() " << value.size() << " does not equal "
    << "column_or_row_index.size() " 
    << column_or_row_index.size() << std::endl<< std::endl
    << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_vectors_of_pairs",
    OOMPH_EXCEPTION_LOCATION);
  }
#endif


// The idea behind this sparse assembly routine is to use a Vector of
// Vectors of pairs for each complete matrix.
// Each inner Vector stores pairs and holds the row (or column) index
// and the value of the matrix entry.
 
// Set up Vector of Vectors to store the entries of each matrix,
// indexed by either the column or row.
 Vector<Vector< Vector<std::pair<unsigned,double> > > > matrix_data(n_matrix);
 
 //Loop over the number of matrices being assembled and resize
 //each Vector of Vectors to the number of rows or columns of the matrix
 for(unsigned m=0;m<n_matrix;m++) {matrix_data[m].resize(n_dof);}
 
 //Resize the residuals vectors
 for(unsigned v=0;v<n_vector;v++) {residuals[v]->resize(n_dof);}
 
 //----------------Assemble and populate the vector storage scheeme--------
 {
  //Allocate local storage for the element's contribution to the
  //residuals vectors and system matrices of the size of the maximum
  //number of dofs in any element
  //This means that the storage is only allocated (and deleted) once
  Vector<Vector<double> > el_residuals(n_vector);
  Vector<DenseMatrix<double> > el_jacobian(n_matrix);

  //Loop over the elements
  for(unsigned long e=el_lo;e<el_hi;e++)
   {
    //Get the pointer to the element
    GeneralisedElement* elem_pt = mesh_pt()->element_pt(e);
    
#ifdef OOMPH_HAS_MPI
    //Ignore halo elements
    if (!elem_pt->is_halo())
     {
#endif

    //Find number of degrees of freedom in the element
    const unsigned nvar = assembly_handler_pt->ndof(elem_pt);

    //Resize the storage for elemental jacobian and residuals
    for(unsigned v=0;v<n_vector;v++) {el_residuals[v].resize(nvar);}
    for(unsigned m=0;m<n_matrix;m++) {el_jacobian[m].resize(nvar);}

    //Now get the residuals and jacobian for the element
    assembly_handler_pt->
     get_all_vectors_and_matrices(elem_pt,el_residuals, el_jacobian);
    
    //---------------Insert the values into the vectors--------------
    
    //Loop over the first index of local variables
    for(unsigned i=0;i<nvar;i++)
     {
      //Get the local equation number
      unsigned eqn_number = assembly_handler_pt->eqn_number(elem_pt,i);
      //Add the contribution to the residuals
      for(unsigned v=0;v<n_vector;v++)
       {
        //Fill in each residuals vector
        (*residuals[v])[eqn_number] += el_residuals[v][i];
       }
      
      //Now loop over the other index
      for(unsigned j=0;j<nvar;j++)
       {
        //Get the number of the unknown
        unsigned unknown = assembly_handler_pt->eqn_number(elem_pt,j);
        
        //Loop over the matrices
        //If it's compressed row storage, then our vector of maps
        //is indexed by row (equation number)
        for(unsigned m=0;m<n_matrix;m++)
         {
          //Get the value of the matrix at this point
          double value = el_jacobian[m](i,j);
          //Only bother to add to the vector if it's non-zero
          if(std::abs(value) > Numerical_zero_for_sparse_assembly)
           {
            //If it's compressed row storage, then our vector of maps
            //is indexed by row (equation number)
            if(compressed_row_flag)
             {
              //Find the correct position and add the data into the vectors
              const unsigned size = matrix_data[m][eqn_number].size();
              for(unsigned k=0; k<=size; k++)
               {
                if(k==size)
                 {
                  matrix_data[m][eqn_number].push_back(
                   std::make_pair(unknown,value));
                  break;
                 }
                else if(matrix_data[m][eqn_number][k].first == unknown)
                 {
                  matrix_data[m][eqn_number][k].second += value;
                  break;
                 }
               }
             }
            //Otherwise it's compressed column storage and our vector is
            //indexed by column (the unknown)
            else
             {
              //Add the data into the vectors in the correct position
              const unsigned size = matrix_data[m][unknown].size();
              for(unsigned k=0; k<=size; k++)
               {
                if(k==size)
                 {
                  matrix_data[m][unknown].push_back(
                   std::make_pair(eqn_number,value));
                  break;
                 }
                else if(matrix_data[m][unknown][k].first == eqn_number)
                 {
                  matrix_data[m][unknown][k].second += value;
                  break;
                 }
               }
             }
           }
         } //End of loop over matrices
       }
     }

#ifdef OOMPH_HAS_MPI
     } // endif halo element
#endif
   } //End of loop over the elements
 } //End of vector assembly
 
 //-----------Finally we need to convert this vector storage scheme
 //------------------------to the containers required by SuperLU
 
 //Loop over the number of matrices
 for(unsigned m=0;m<n_matrix;m++)
  {
   //Set the number of rows or columns
   row_or_column_start[m].resize(n_dof+1);
   
   // fill row_or_column_start and find the number of entries
   row_or_column_start[m][0] = 0;
   for(unsigned long i=0;i<n_dof;i++)
    {
     row_or_column_start[m][i+1] = row_or_column_start[m][i]
      + matrix_data[m][i].size();
    }
   const unsigned entries = row_or_column_start[m][n_dof];
   
   // resize vectors
   column_or_row_index[m].resize(entries);
   value[m].resize(entries);
   
   //Now we merely loop over the number of rows or columns
   for(unsigned long i_global=0;i_global<n_dof;i_global++)
    {
     //If there are no entries in the vector then skip the rest of the loop
     if(matrix_data[m][i_global].empty()) {continue;}
     
     //Loop over all the entries in the vectors corresponding to the given
     //row or column. It will NOT be ordered
     unsigned p = 0;
     for(int j=row_or_column_start[m][i_global];
         j<row_or_column_start[m][i_global+1]; j++)
      {
       column_or_row_index[m][j] = matrix_data[m][i_global][p].first;
       value[m][j] = matrix_data[m][i_global][p].second;
       ++p;
      }
    }
  } //End of the loop over the matrices

 if (Pause_at_end_of_sparse_assembly)
  {
   oomph_info << "Pausing at end of sparse assembly." << std::endl;
   pause("Check memory usage now.");
  }
} 








//=====================================================================
/// This is a (private) helper function that is used to assemble system
/// matrices in compressed row or column format
/// and compute residual vectors using two vectors.
/// The default action is to assemble the jacobian matrix and 
/// residuals for the Newton method. The action can be
/// overloaded at an elemental level by chaging the default
/// behaviour of the function Element::get_all_vectors_and_matrices().
/// column_or_row_index: Column [or row] index of given entry
/// row_or_column_start: Index of first entry for given row [or column]
/// value              : Vector of nonzero entries
/// residuals          : Residual vector
/// compressed_row_flag: Bool flag to indicate if storage format is 
///                      compressed row [if false interpretation of
///                      arguments is as stated in square brackets].
//=====================================================================
void Problem::sparse_assemble_row_or_column_compressed_with_two_vectors(
 Vector<Vector<int> > &column_or_row_index, 
 Vector<Vector<int> > &row_or_column_start, 
 Vector<Vector<double> > &value, 
 Vector<Vector<double>*> &residuals,
 bool compressed_row_flag)
{
 //Total number of elements
 const unsigned long  n_elements = mesh_pt()->nelement();

 // Default range of elements for distributed problems
 unsigned long el_lo=0;
 unsigned long el_hi=n_elements;
 
#ifdef OOMPH_HAS_MPI
 // Otherwise just loop over a fraction of the elements
 if (!Problem_has_been_distributed)
  {
   // Distribute work evenly
   unsigned long range = 
    static_cast<unsigned long>(double(n_elements)/double(MPI_Helpers::Nproc));
   el_lo = MPI_Helpers::My_rank*range;
   el_hi = (MPI_Helpers::My_rank+1)*range;
   
   // Last one needs to incorporate any dangling elements
   if (MPI_Helpers::My_rank==MPI_Helpers::Nproc-1) el_hi = n_elements;
  }
#endif 

 //Total number of degrees of freedom
 const unsigned long n_dof = Problem::ndof();

 //Find the number of vectors to be assembled
 const unsigned n_vector = residuals.size();

 //Find the number of matrices to be assembled
 const unsigned n_matrix = column_or_row_index.size();

 //Locally cache pointer to assembly handler
 AssemblyHandler* const assembly_handler_pt = Assembly_handler_pt;

//Error check dimensions
#ifdef PARANOID
 if(row_or_column_start.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error: " << std::endl
    << "row_or_column_start.size() " << row_or_column_start.size() 
    << " does not equal "
    << "column_or_row_index.size() " 
    <<  column_or_row_index.size() << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_lists",
    OOMPH_EXCEPTION_LOCATION);
  }

if(value.size() != n_matrix)
  {
   std::ostringstream error_stream;
   error_stream
    << "Error: " 
    << std::endl
    << "value.size() " << value.size() << " does not equal "
    << "column_or_row_index.size() " 
    << column_or_row_index.size() << std::endl<< std::endl
    << std::endl;
   throw OomphLibError(
    error_stream.str(),
    "Problem::sparse_assemble_row_or_column_compressed_with_two_vectors",
    OOMPH_EXCEPTION_LOCATION);
  }
#endif

// The idea behind this sparse assembly routine is to use Vectors of
// Vectors for the entries in each complete matrix. And a second
// Vector of Vectors stores the global row (or column) indeces. This
// will not have the memory overheads associated with the methods using
// lists or maps, but insertion will be more costly.
 
// Set up two vector of vectors to store the entries of each  matrix,
// indexed by either the column or row. The entries of the vector for
// each matrix correspond to all the rows or columns of that matrix.
 Vector<Vector<Vector<unsigned> > > matrix_row_or_col_indices(n_matrix);
 Vector<Vector<Vector<double> > > matrix_values(n_matrix);
 
 //Loop over the number of matrices being assembled and resize
 //each vector of vectors to the number of rows or columns of the matrix
 for(unsigned m=0;m<n_matrix;m++)
  {
   matrix_row_or_col_indices[m].resize(n_dof);
   matrix_values[m].resize(n_dof);
  }
 
 //Resize the residuals vectors
 for(unsigned v=0;v<n_vector;v++) {residuals[v]->resize(n_dof);}
 
 //----------------Assemble and populate the vector storage scheme-------
 {
  //Allocate local storage for the element's contribution to the
  //residuals vectors and system matrices of the size of the maximum
  //number of dofs in any element
  //This means that the storage will only be allocated (and deleted) once
  Vector<Vector<double> > el_residuals(n_vector);
  Vector<DenseMatrix<double> > el_jacobian(n_matrix);
  
  //Loop over the elements
  for(unsigned long e=el_lo;e<el_hi;e++)
   {
    //Get the pointer to the element
    GeneralisedElement* elem_pt = mesh_pt()->element_pt(e);

#ifdef OOMPH_HAS_MPI
    //Ignore halo elements
    if (!elem_pt->is_halo())
     {
#endif

    //Find number of degrees of freedom in the element
    const unsigned nvar = assembly_handler_pt->ndof(elem_pt);
    
    //Resize the storage for elemental jacobian and residuals
    for(unsigned v=0;v<n_vector;v++) {el_residuals[v].resize(nvar);}
    for(unsigned m=0;m<n_matrix;m++) {el_jacobian[m].resize(nvar);}

    //Now get the residuals and jacobian for the element
    assembly_handler_pt->
     get_all_vectors_and_matrices(elem_pt,el_residuals, el_jacobian);
    
    //---------------Insert the values into the vectors--------------
    
    //Loop over the first index of local variables
    for(unsigned i=0;i<nvar;i++)
     {
      //Get the local equation number
      unsigned eqn_number = assembly_handler_pt->eqn_number(elem_pt,i);
      //Add the contribution to the residuals
      for(unsigned v=0;v<n_vector;v++)
       {
        //Fill in each residuals vector
        (*residuals[v])[eqn_number] += el_residuals[v][i];
       }
      
      //Now loop over the other index
      for(unsigned j=0;j<nvar;j++)
       {
        //Get the number of the unknown
        unsigned unknown = assembly_handler_pt->eqn_number(elem_pt,j);
        
        //Loop over the matrices
        //If it's compressed row storage, then our vector of maps
        //is indexed by row (equation number)
        for(unsigned m=0;m<n_matrix;m++)
         {
          //Get the value of the matrix at this point
          double value = el_jacobian[m](i,j);
          //Only bother to add to the vector if it's non-zero
          if(std::abs(value) > Numerical_zero_for_sparse_assembly)
           {
            //If it's compressed row storage, then our vector of maps
            //is indexed by row (equation number)
            if(compressed_row_flag)
             {
              //Find the correct position and add the data into the vectors
              const unsigned size = 
               matrix_row_or_col_indices[m][eqn_number].size();
              
              for(unsigned k=0; k<=size; k++)
               {
                if(k==size)
                 {
                  matrix_row_or_col_indices[m][eqn_number].
                   push_back(unknown);
                  matrix_values[m][eqn_number].push_back(value);
                  break;
                 }
                else if(matrix_row_or_col_indices[m][eqn_number][k] == 
                        unknown)
                 {
                  matrix_values[m][eqn_number][k] += value;
                  break;
                 }
               }
             }
            //Otherwise it's compressed column storage and our vector is
            //indexed by column (the unknown)
            else
             {
              //Add the data into the vectors in the correct position
              const unsigned size = 
               matrix_row_or_col_indices[m][unknown].size();
              for (unsigned k=0; k<=size; k++)
               {
                if (k==size)
                 {
                  matrix_row_or_col_indices[m][unknown].
                   push_back(eqn_number);
                  matrix_values[m][unknown].push_back(value);
                  break;
                 }
                else if (matrix_row_or_col_indices[m][unknown][k] == 
                         eqn_number)
                 {
                  matrix_values[m][unknown][k] += value;
                  break;
                 }
               }
             }
           }
         } //End of loop over matrices
       }
     }

#ifdef OOMPH_HAS_MPI
     } // endif halo element
#endif
   } //End of loop over the elements
  
 } //End of vector assembly
 
   //-----------Finally we need to convert this lousy vector storage scheme
   //------------------------to the containers required by SuperLU
 
   //Loop over the number of matrices
 for(unsigned m=0;m<n_matrix;m++)
  {
   //Set the number of rows or columns
   row_or_column_start[m].resize(n_dof+1);
   
   // fill row_or_column_start and find the number of entries
   row_or_column_start[m][0] = 0;
   for (unsigned long i=0;i<n_dof;i++)
    {
     row_or_column_start[m][i+1] = row_or_column_start[m][i]
      + matrix_values[m][i].size();
    }
   const unsigned entries = row_or_column_start[m][n_dof];
   
   // resize vectors
   column_or_row_index[m].resize(entries);
   value[m].resize(entries);
   
   //Now we merely loop over the number of rows or columns
   for(unsigned long  i_global=0;i_global<n_dof;i_global++)
    {
     //If there are no entries in the vector then skip the rest of the loop
     if(matrix_values[m][i_global].empty()) {continue;}
     
     //Loop over all the entries in the vectors corresponding to the given
     //row or column. It will NOT be ordered
     unsigned p = 0;
     for(int j=row_or_column_start[m][i_global];
         j<row_or_column_start[m][i_global+1]; j++)
      {
       column_or_row_index[m][j] = matrix_row_or_col_indices[m][i_global][p];
       value[m][j] = matrix_values[m][i_global][p];
       ++p;
      }
    }
  } //End of the loop over the matrices

 if (Pause_at_end_of_sparse_assembly)
  {
   oomph_info << "Pausing at end of sparse assembly." << std::endl;
   pause("Check memory usage now.");
  }

}

#ifdef OOMPH_HAS_MPI
//================================================================
/// Assemble Jacobian matrix in compressed row or column format in
/// parallel. Each processor first assembles a block of rows of the
/// complete Jacobian and residual, and then assembles the whole
/// Jacobian and residual.
/// column_or_row_index : Column [or row] index of given entry
/// row_or_column_start : Index of first entry for given row [or column]
/// value               : Vector of nonzero entries
/// residuals           : Residual vector
/// compressed_row_flag : compressed row storage? Otherwise
///                       compressed column -- defaults to false as 
///                       this is the format required by the global memory
///                       version of SuperLU_dist
//================================================================
void Problem::global_matrix_sparse_assemble
(Vector<Vector<int> > &column_or_row_index,
 Vector<Vector<int> > &row_or_column_start,
 Vector<Vector<double> > &value,
 Vector<Vector<double>*> &residuals,
 bool compressed_row_flag)
{
 //---------------------------------------------------------------
 // Note: Main comments refer to compressed row storage, comments
 //       in square brackets refer to compressed column storage
 //---------------------------------------------------------------

 // deal with single processor case
 if (MPI_Helpers::Nproc==1)
  {
   // Call sparse_assemble
   sparse_assemble_row_or_column_compressed(column_or_row_index,
                                            row_or_column_start,
                                            value,
                                            residuals,
                                            compressed_row_flag);
  }
 // When MPI_Helpers::Nproc>1 assemble the global matrix
 // from the blocks returned by distributed_matrix_sparse_assemble
 else 
 {
   //Clear everything
   value[0].clear();
   row_or_column_start[0].clear();
   column_or_row_index[0].clear();
   (*residuals[0]).clear();
  
   // Total number of dofs in the problem
   unsigned long n_dof=0;
  
   // Contribution to the Jacobian matrix and residual vector
   unsigned long my_first_row_or_col=0;
   unsigned long my_n_row_or_col=0;
   Vector<Vector<double> > my_value(1);
   Vector<Vector<int> > my_col_or_row_index(1);
   Vector<Vector<int> > my_row_or_col_start(1);
   Vector<double> my_residuals;
   Vector<Vector<double>*> my_residuals_vector(1);
   my_residuals_vector[0] = &my_residuals; 
  
   // Call distributed_matrix_sparse_assemble on local processor
   distributed_matrix_sparse_assemble(my_col_or_row_index,
                                      my_row_or_col_start,
                                      my_value,
                                      my_residuals_vector,
                                      my_first_row_or_col,
                                      my_n_row_or_col,
                                      n_dof,
                                      compressed_row_flag);
  
   // Create a array to store the number of values held on each processor
   // and get these - communicate via a single Allgather
   unsigned long my_n_value = my_value[0].size();
   Vector<unsigned long> n_values(MPI_Helpers::Nproc);
   MPI_Allgather(&my_n_value, 1,
                 MPI_UNSIGNED_LONG,
                 &n_values[0], 1,
                 MPI_UNSIGNED_LONG,
                 MPI_COMM_WORLD);
  
   // Calculate the total number of values and the position of the
   // first value from each processor in the global matrix
   unsigned long total_n_value = 0;
   Vector<unsigned long> value_offsets(MPI_Helpers::Nproc);
   for (int i=0; i<MPI_Helpers::Nproc; i++)
    {
     value_offsets[i] = total_n_value;
     total_n_value += n_values[i];
    }
  
   // Create a array to store the number of rows held on each processor
   // and get those values - communicate via a single Allgather
   Vector<unsigned long> n_rows_or_cols(MPI_Helpers::Nproc);
   MPI_Allgather(&my_n_row_or_col, 1,
                 MPI_UNSIGNED_LONG,
                 &n_rows_or_cols[0],1,
                 MPI_UNSIGNED_LONG,
                 MPI_COMM_WORLD);
  
   // Calculate the position of the first row for each block
   Vector<unsigned long> row_or_col_offsets(MPI_Helpers::Nproc);
   row_or_col_offsets[0] = 0;
   for (int i=0; i<MPI_Helpers::Nproc-1; i++)
    {
     row_or_col_offsets[i+1] = row_or_col_offsets[i] + n_rows_or_cols[i];
    }
  
   // Resize arrays
   value[0].resize(total_n_value);
   row_or_column_start[0].resize(n_dof+1);
   column_or_row_index[0].resize(total_n_value);
   (*residuals[0]).resize(n_dof);
  
   // Set last value in row_or_column_start
   row_or_column_start[0][n_dof] = total_n_value;
  
   // Copy my_value and my_column_or_row_index to the global matrix
   unsigned long value_offset = value_offsets[MPI_Helpers::My_rank];
   for(unsigned long i=0; i<my_n_value; i++)
    {
      value[0][i+value_offset] = my_value[0][i];
      column_or_row_index[0][i+value_offset] = my_col_or_row_index[0][i];
    }
  
   // Copy my_row_or_column_start and my_residuals to the global values
   unsigned long row_or_col_offset = row_or_col_offsets[MPI_Helpers::My_rank];
   for(unsigned long i=0; i<my_n_row_or_col; i++)
    {
      row_or_column_start[0][i+row_or_col_offset] = 
       my_row_or_col_start[0][i]+value_offset;
      (*residuals[0])[i+row_or_col_offset] = my_residuals[i];
    }
    
   // loop over communications with other processors
   for (int comm=1; comm<MPI_Helpers::Nproc; comm++)
    {
     // Select processor to send data to
     int send_proc = MPI_Helpers::My_rank + comm;
     if (send_proc >= MPI_Helpers::Nproc)
      {
       send_proc -= MPI_Helpers::Nproc;
      }
    
     // Select processor to receive data from
     int recv_proc = MPI_Helpers::My_rank - comm;
     if (recv_proc < 0)
      {
       recv_proc += MPI_Helpers::Nproc;
      }

     // Get offsets for values from receive processor
     value_offset = value_offsets[recv_proc];
     row_or_col_offset = row_or_col_offsets[recv_proc];
  
     // Stuff needed for MPI communications
     MPI_Status status;
     MPI_Request request[4];

     // MPI communications using non-blocking sends and blocking receives
     // and receiving directly into the correct positions in the Vectors

     // communicate residuals 
     MPI_Isend(&my_residuals[0],
               my_n_row_or_col,
               MPI_DOUBLE,
               send_proc, 0,
               MPI_COMM_WORLD,
               &request[0]);

     MPI_Recv(&(*residuals[0])[row_or_col_offset],
              n_rows_or_cols[recv_proc],
              MPI_DOUBLE,
              recv_proc,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &status);

     // communicate column [row] indices 
     MPI_Isend(&my_col_or_row_index[0][0],
               my_n_value,
               MPI_INT,
               send_proc, 0,
               MPI_COMM_WORLD,
               &request[1]);

     MPI_Recv(&column_or_row_index[0][value_offset],
              n_values[recv_proc], 
              MPI_INT,
              recv_proc,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &status);

     // communicate row [column] starts remembering we only need to send
     // the first my_n_row_or_col values and only need to receive the
     // first n_row_or_cols[recv_proc] values
     MPI_Isend(&my_row_or_col_start[0][0],
               my_n_row_or_col,
               MPI_INT,
               send_proc, 0,
               MPI_COMM_WORLD,
               &request[2]);

     MPI_Recv(&row_or_column_start[0][row_or_col_offset],
              n_rows_or_cols[recv_proc],
              MPI_INT, 
              recv_proc,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &status);

     // communicate value
     MPI_Isend(&my_value[0][0],
               my_n_value,
               MPI_DOUBLE,
               send_proc, 0,
               MPI_COMM_WORLD,
               &request[3]);

     MPI_Recv(&value[0][value_offset],
              n_values[recv_proc],
              MPI_DOUBLE,
              recv_proc,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &status);
     
     // Shift the received row_or_column_start by value_offset
     const unsigned long end = row_or_col_offset+n_rows_or_cols[recv_proc];
     for (unsigned long i=row_or_col_offset;
          i<end;
          i++)
      {
       row_or_column_start[0][i] += value_offset;
      }

     // Wait for sends to complete
     MPI_Waitall(4, request, MPI_STATUS_IGNORE);
  
    } // end of loop over communications with other processors
  } // End of matrix and residual assembly
}


//================================================================
/// Assembles a block of successive rows of the Jacobian matrix 
/// in distributed compressed row format (as required by
/// the distributed storage version of SuperLU_dist) or in compressed
/// column storage, plus associated distributed residuals.
/// This function assembles the rows [or columns] for each processor
/// by first calling partial_sparse_assemble and then communicates
/// between processors to assemble distributed sets of rows [or columns].
/// Arguments are the obvious ones, but note the following:
/// - Matrix is stored in CR [or CC] format, however only the
///   entries for a subset of rows are included, with
///   \c first_row_or_column being the first row [or column] stored here.
/// - The i-th entry in \c row_or_column_start corresponds
///   to the (i+first_row_or_column) -th row [or column] in the global matrix.
/// - The residual vector contains the subset of entries corresponding
///   to the same set of rows [or columns] of the matrix, so the
///   i-th entry in the residual vector returned corresponds
///   to the (i+first_row_or_column) -th entry in the global residual vector.
///   (This is the format that is required by SuperLU_dist).
/// - \c n_row_or_column is number of consecutive rows [columns] stored here.
/// - \c n_tot is total number of rows [columns] in the overall matrix.
/// - \c compressed_row_flag defaults to true.
//================================================================
void Problem::distributed_matrix_sparse_assemble(
 Vector<Vector<int> > &column_or_row_index,
 Vector<Vector<int> > &row_or_column_start,
 Vector<Vector<double> > &value,
 Vector<Vector<double>*> &residuals,
 unsigned long& first_row_or_column,
 unsigned long& n_row_or_column,
 unsigned long& n_tot,
 bool compressed_row_flag)
{
 //---------------------------------------------------------------
 // Note: Main comments refer to compressed row storage, comments
 //       in square brackets refer to compressed column storage
 //---------------------------------------------------------------

 // Set n_tot - total number of rows [columns]
 n_tot=ndof();
 
 // deal with single processor case
 if (MPI_Helpers::Nproc==1)
  {
   sparse_assemble_row_or_column_compressed(column_or_row_index,
                                            row_or_column_start,
                                            value,
                                            residuals,
                                            compressed_row_flag);
   // Set n_row_or_column and first_row_or_column
   n_row_or_column = n_tot;
   first_row_or_column = 0;
  }
 // for multiple processors form the distributed matrix
 else
  {
   // Clear everything to be safe
   value[0].clear();
   row_or_column_start[0].clear();
   column_or_row_index[0].clear();
   (*residuals[0]).clear();
  
   // Set n_tot - total number of rows [columns]
   n_tot=ndof();
  
   // Partition matrix - calculate first row [or column] and the number
   // of rows [columns] for all processors
  
   // Setup the first row and the number of rows for all processors
   Vector<unsigned long> first_rows_or_cols(MPI_Helpers::Nproc+1);
   Vector<unsigned long>  n_rows_or_cols(MPI_Helpers::Nproc);
   for (int i=0; i<MPI_Helpers::Nproc; i++)
    {
     first_rows_or_cols[i]=static_cast<unsigned long>
      (double(i*n_tot)/double(MPI_Helpers::Nproc));
    }
   first_rows_or_cols[MPI_Helpers::Nproc]=n_tot;
   for (int i=0; i<MPI_Helpers::Nproc; i++)
    {
     n_rows_or_cols[i]=first_rows_or_cols[i+1]-first_rows_or_cols[i];
    }

   // Set the number of rows [columns] this processor owns
   first_row_or_column=first_rows_or_cols[MPI_Helpers::My_rank];
   n_row_or_column=n_rows_or_cols[MPI_Helpers::My_rank];
   
   // Vectors to hold my data
   Vector<Vector<double> > my_value(1);
   Vector<Vector<int> > my_col_or_row_index(1);
   Vector<Vector<int> > my_row_or_col_start(1);
   Vector<double> my_residuals;
   Vector<Vector<double>*> my_residuals_vector(1);
   my_residuals_vector[0] = &my_residuals;

   
   // Call sparse assembly in compressed row [or column] mode
   sparse_assemble_row_or_column_compressed(my_col_or_row_index,
                                            my_row_or_col_start,
                                            my_value,
                                            my_residuals_vector,
                                            compressed_row_flag);
   
   
   // Vectors of maps that stores values and column indices
   // for each row
   Vector< std::map<int,double> > matrix_data(n_row_or_column);
  
   // Allocate storage for the residual vector and set values to zero
   // (this stores only the subset of entries this processor is in charge of)
   (*residuals[0]).resize(n_row_or_column, 0.0);
  
   // Add the contributions from partial_sparse_assemble to the rows [or
   // columns] in the portion of the Jacobian matrix to be assembled by
   // this processor, i.e. from first_row_or_column to
   // first_row_or_column+n_row_or_column (plus associated entries in
   // the residuals)
   unsigned long end = first_row_or_column+n_row_or_column;
   for(unsigned long i=first_row_or_column;
       i<end;
       i++)
    {
     // Loop through the row [column] entries
     for(int j=my_row_or_col_start[0][i];
         j<my_row_or_col_start[0][i+1];
         j++)
      {
       // copy values
       matrix_data[i-first_row_or_column][my_col_or_row_index[0][j]] = 
        my_value[0][j];
      }
  
     // Note: residual vector only stores a subset of entries with
     // numering starting at zero where as sparse assemble
     // returns the entries labeled by their global numbers:
     (*residuals[0])[i-first_row_or_column]=my_residuals[i];
    }
  
   // Vectors to hold data to send
   Vector<double> value_to_send;
   Vector<double> residuals_to_send;
   Vector<int> col_or_row_index_to_send;
   Vector<int> row_or_col_start_to_send;
  
   // Vectors to hold received data
   Vector<double> value_recvd;
   Vector<double> residuals_recvd;
   Vector<int> col_or_row_index_recvd;
   Vector<int> row_or_col_start_recvd;
  
   // loop over communications with other processors
   for (int comm=1; comm<MPI_Helpers::Nproc; comm++)
    {
     // Select processor to send data to
     int send_proc = MPI_Helpers::My_rank + comm;
     if (send_proc >= MPI_Helpers::Nproc)
      {
       send_proc -= MPI_Helpers::Nproc;
      }
    
     // Select processor to receive data from
     int recv_proc = MPI_Helpers::My_rank - comm;
     if (recv_proc < 0)
      {
       recv_proc += MPI_Helpers::Nproc;
      }
  
     // Set up data to send - note matrix and residual are indexed
     // such that i-th row [or column] in the matrix sent corresponds
     // to i+first_row_or_col_to_send-th row [or column] in the global
     // matrix and i-th entry i in the residual sent corresponds to
     // i+first_row_or_col_to_send-th entry in the global residual
     const unsigned long first_row_or_col_to_send = 
      first_rows_or_cols[send_proc];
     const unsigned long n_row_or_col_to_send = n_rows_or_cols[send_proc];
  
     // Initialise and resize send arrays
     value_to_send.clear();
     col_or_row_index_to_send.clear();
     row_or_col_start_to_send.resize(n_row_or_col_to_send+1);
     residuals_to_send.resize(n_row_or_col_to_send);
       
     // Count of number of entries inserted
     int entry_count=0;
  
     // Loop over rows [or columns] to be sent
     end = first_row_or_col_to_send+n_row_or_col_to_send;
     for(unsigned long i=first_row_or_col_to_send;
         i<end;
         i++)
      {
       // Set row [column] start
       row_or_col_start_to_send[i-first_row_or_col_to_send]=entry_count;
  
       // Loop over my entries in this row
       for(int j=my_row_or_col_start[0][i];
           j<my_row_or_col_start[0][i+1];
           j++)
        {
         // Add to values and index vectors
         value_to_send.push_back(my_value[0][j]);
         col_or_row_index_to_send.push_back(my_col_or_row_index[0][j]);
  
         // Bump up number of entries
         entry_count++;
        }
  
       // Copy residual (indexed from zero!)
       residuals_to_send[i-first_row_or_col_to_send]=my_residuals[i];
      }
     // Remember to set final entry in row start vector
     row_or_col_start_to_send[n_row_or_col_to_send]=entry_count;
     
     // Set number of values to send
     unsigned long n_value_to_send = value_to_send.size();
     
     // stuff for MPI communications
     MPI_Status status;
     MPI_Request send_request[5];
     MPI_Request recv_request[3];

     // Send n_value_to_send and receive corresponding value from recv_proc
     unsigned long n_value_recvd=0;
     
     // non-blocking send
     MPI_Isend(&n_value_to_send, 1,
               MPI_UNSIGNED_LONG,
               send_proc, 0,
               MPI_COMM_WORLD,
               &send_request[0]);
         
     // blocking receive
     MPI_Recv(&n_value_recvd, 1,
              MPI_UNSIGNED_LONG,
              recv_proc, 
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &status);
     
     // Note: if n_values_recvd is 0 then we can bypass the receiving the
     // matrix and if n_value_to_send is 0 then we can bypass sending
     // What about residuals?

     // resize Vectors for receiving data
     value_recvd.resize(n_value_recvd);
     col_or_row_index_recvd.resize(n_value_recvd);
     row_or_col_start_recvd.resize(n_row_or_column+1);
     residuals_recvd.resize(n_row_or_column);
     
     // Communicate residuals

     // non-blocking send
     MPI_Isend(&residuals_to_send[0],
               n_row_or_col_to_send,
               MPI_DOUBLE,
               send_proc, 0,
               MPI_COMM_WORLD,
               &send_request[1]);

     // blocking receive
     MPI_Recv(&residuals_recvd[0],
              n_row_or_column,
              MPI_DOUBLE,
              recv_proc,
              MPI_ANY_TAG,
              MPI_COMM_WORLD,
              &status);
     
     // non-blocking sends of Jacobian matrix
     if (n_value_to_send>0)
      {
       MPI_Isend(&col_or_row_index_to_send[0],
                 n_value_to_send,
                 MPI_INT,
                 send_proc, 0,
                 MPI_COMM_WORLD,
                 &send_request[2]);

       MPI_Isend(&row_or_col_start_to_send[0],
                 n_row_or_col_to_send+1,
                 MPI_INT,
                 send_proc, 0,
                 MPI_COMM_WORLD,
                 &send_request[3]);

       MPI_Isend(&value_to_send[0],
                 n_value_to_send, 
                 MPI_DOUBLE,
                 send_proc, 0,
                 MPI_COMM_WORLD,
                 &send_request[4]);
      }
     
     // non-blocking receives of Jacobian matrix
     if (n_value_recvd>0)
      {
       MPI_Irecv(&col_or_row_index_recvd[0],
                 n_value_recvd,
                 MPI_INT,
                 recv_proc,
                 MPI_ANY_TAG,
                 MPI_COMM_WORLD,
                 &recv_request[0]);

       MPI_Irecv(&row_or_col_start_recvd[0],
                 n_row_or_column+1,
                 MPI_INT,
                 recv_proc,
                 MPI_ANY_TAG,
                 MPI_COMM_WORLD,
                 &recv_request[1]);

       MPI_Irecv(&value_recvd[0],
                 n_value_recvd,
                 MPI_DOUBLE,
                 recv_proc,
                 MPI_ANY_TAG,
                 MPI_COMM_WORLD,
                 &recv_request[2]);
      }

  
     // Add new residuals to my global values
     for (unsigned long i=0;i<n_row_or_column;i++)
      {
       // Add contribution to residual vector
       (*residuals[0])[i]+=residuals_recvd[i];
      }

     // Add any new matrix values to my global values
     // after non-blocking receives have finished
     if (n_value_recvd>0)
      {
       MPI_Waitall(3, recv_request, MPI_STATUS_IGNORE);

       for (unsigned long i=0;i<n_row_or_column;i++)
        {
         // loop through the entries in the row [column]
         for (int j=row_or_col_start_recvd[i];
              j<row_or_col_start_recvd[i+1];
              j++)
          {
           // Add the data into the map
           matrix_data[i][col_or_row_index_recvd[j]] += value_recvd[j];
          }
        }
      }
      
     // Ensure all non-blocking sends have completed
     if (n_value_to_send>0)
      {
       MPI_Waitall(5, send_request, MPI_STATUS_IGNORE);
      }
     else
      {
       MPI_Waitall(2, send_request, MPI_STATUS_IGNORE);
      }

    } // end of loop over communications with other processors
  
  
   // Finally, copy from Vector of Maps to compressed
   // storage format in Vectors:
   
   // Provide storage for row [column] starts
   row_or_column_start[0].resize(n_row_or_column+1);
  
   // Counter for the number of entries so far in the storage scheme
   unsigned long entry_count=0;
  
   // Loop over the rows [or columns]
   for(unsigned long i=0; i<n_row_or_column; i++)
    {
     // Start index for the present row [column]
     row_or_column_start[0][i] = entry_count;
  
     // Loop over all the entries in the map corresponding to the given
     // row [or column] (it will be ordered)
     for(std::map<int,double>::iterator it = matrix_data[i].begin();
         it!=matrix_data[i].end();
         ++it)
      {
       // The first value is the column [row] index
       column_or_row_index[0].push_back(it->first);
  
       // The second value is the actual data value
       value[0].push_back(it->second);
  
       // Increase the value of the counter
       entry_count++;
      }
    }
   // Remember final entry in the row [column] start vector
   row_or_column_start[0][n_row_or_column] = entry_count;
  } // End of multiple processor case

}

#endif

//================================================================
/// \short Get the full Jacobian by finite differencing
//================================================================
void Problem::get_fd_jacobian(Vector<double> &residuals, 
                              DenseMatrix<double> &jacobian)
{

#ifdef OOMPH_HAS_MPI

 if (Problem_has_been_distributed)
  {
   OomphLibWarning("This is unlikely to work with a distributed problem",
                   " Problem::get_fd_jacobian()",
                   OOMPH_EXCEPTION_LOCATION);
  }
#endif


 //Find number of dofs
 const unsigned long n_dof = ndof();
 
// Advanced residuals
 Vector<double> residuals_pls(n_dof);

 // Get reference residuals
 get_residuals(residuals);

 const double FD_step=1.0e-8;
 //Loop over all dofs
 for(unsigned long jdof=0;jdof<n_dof;jdof++) 
  {
   double backup=*Dof_pt[jdof];
   *Dof_pt[jdof]+=FD_step;

   // We're checking if the new values for Dof_pt[] actually
   // solve the entire problem --> update as if problem had
   // been solved 
   actions_before_newton_solve();
   actions_before_newton_convergence_check();
   actions_after_newton_solve();

   // Get advanced residuals
   get_residuals(residuals_pls);

   for (unsigned long ieqn=0;ieqn<n_dof;ieqn++) 
    {
     jacobian(ieqn,jdof)=(residuals_pls[ieqn]-residuals[ieqn])/FD_step;
    }
   *Dof_pt[jdof]=backup;
  }

 // Reset problem to state it was in
 actions_before_newton_solve();
 actions_before_newton_convergence_check();
 actions_after_newton_solve();

}

//======================================================================
/// \short Get derivative of the residuals vector wrt a global parameter
/// This is required in continuation problems
//=======================================================================
void Problem::get_derivative_wrt_global_parameter(double* const &parameter_pt,
                                                  Vector<double> &result)
{

#ifdef OOMPH_HAS_MPI

 if (Problem_has_been_distributed)
  {
   OomphLibWarning("This is unlikely to work with a distributed problem",
                   "Problem::get_derivative_wrt_global_parameter()",
                   OOMPH_EXCEPTION_LOCATION);
  }
#endif


 //Initialise the result vector to zero 
 result.initialise(0.0);

 //Get the residuals and store in the result vector
 get_residuals(result);

 //Find the number of degrees of freedom in the problem
 const unsigned long n_dof = this->ndof();

 //Storage for the new residuals
 Vector<double> newres(n_dof);
  
 //Increase the global parameter
 const double FD_step = 1.0e-8;
 
 //Store the current value of the parameter
 double param_value = *parameter_pt;

 //Increase the parameter
 *parameter_pt += FD_step;

 //Do any possible updates
 actions_after_change_in_global_parameter();

 //Get the new residuals
 get_residuals(newres);

 //Do the finite differencing
 for(unsigned long n=0;n<n_dof;++n)
  {
   result[n] = (newres[n] - result[n])/FD_step;
  }

 //Reset the value of the parameter
 *parameter_pt = param_value;

 //Do any possible updates
 actions_after_change_in_global_parameter();
}

//================================================================
/// \short Get derivative of an element in the problem wrt a global
/// parameter, to be used in continuation problems
//================================================================
void Problem::get_derivative_wrt_global_parameter(
 double* const &parameter_pt,
 GeneralisedElement* const &elem_pt,
 Vector<double> &result)
{

#ifdef OOMPH_HAS_MPI

 if (Problem_has_been_distributed)
  {
   OomphLibWarning("This is unlikely to work with a distributed problem",
                   "Problem::get_derivative_wrt_global_parameter()",
                   OOMPH_EXCEPTION_LOCATION);
  }
#endif

 //Locally cache pointer to assembly handler
 AssemblyHandler* const assembly_handler_pt = Assembly_handler_pt;

 //Should definitely give this a more global scope
 double FD_Jstep = 1.0e-8;
 
 //Find the number of variables in the element, e
 unsigned nvar = assembly_handler_pt->ndof(elem_pt);
 //Create storage for residuals
 Vector<double> residuals(nvar), newres(nvar);

 //Get the "original" residuals
 assembly_handler_pt->get_residuals(elem_pt,residuals);

 //Save the old value of the global parameter
 double old_var = *parameter_pt;

 //Increment the value
 *parameter_pt += FD_Jstep;
 
 //Now do any possible updates
 actions_after_change_in_global_parameter();

 //Get the "new" residuals
 assembly_handler_pt->get_residuals(elem_pt,newres);

 //Do the finite differences
 for(unsigned m=0;m<nvar;m++)
  {
   result[m] = (newres[m] - residuals[m])/FD_Jstep;
  }
 
 //Reset value of the global parameter
 *parameter_pt = old_var;

 //Now do any possible updates
 actions_after_change_in_global_parameter();
}

//==================================================================
/// Solve the eigenproblem
//==================================================================
void Problem::solve_eigenproblem(const unsigned &n_eval,
                                 Vector<std::complex<double> > &eigenvalue,
                                 Vector<Vector<double> > &eigenvector)
{
 //Call the Eigenproblem for the eigensolver
 Eigen_solver_pt->solve_eigenproblem(this,n_eval,eigenvalue,eigenvector);
}

//===================================================================
/// Get the matrices required to solve an eigenproblem
//===================================================================
void Problem::get_eigenproblem_matrices(CRDoubleMatrix &mass_matrix,
                                        CRDoubleMatrix &main_matrix,
                                        const double &shift)
{
 //Store the old assembly handler
 AssemblyHandler* old_assembly_handler_pt = Assembly_handler_pt;
 //Now setup the eigenproblem handler, pass in the value of the shift
 Assembly_handler_pt = new EigenProblemHandler(shift);

 //Prepare the storage formats.
 Vector<Vector<int> > column_or_row_index(2);
 Vector<Vector<int> > row_or_column_start(2);
 Vector<Vector<double> > value(2); 
 //Allocate pointer to residuals, although not used in these problems
 Vector<Vector<double>*> residuals_vectors(0);

 bool compressed_row_flag=true;

 sparse_assemble_row_or_column_compressed(column_or_row_index,
                                          row_or_column_start,
                                          value,
                                          residuals_vectors,
                                          compressed_row_flag);

 //Get the number of dofs (size of matrices)
 unsigned long n_dof = ndof();

 //The main matrix is the first entry
 main_matrix.build(value[0],column_or_row_index[0],row_or_column_start[0],
                   n_dof,n_dof);
 //The mass matrix is the second entry
 mass_matrix.build(value[1],column_or_row_index[1],row_or_column_start[1],
                   n_dof,n_dof);   

 //Delete the eigenproblem handler
 delete Assembly_handler_pt;
 //Reset the assembly handler to the original handler
 Assembly_handler_pt = old_assembly_handler_pt;
}

//=======================================================================
/// Stored the current values of the dofs
//=======================================================================
void Problem::store_current_dof_values()
{
 //Find the number of dofs
 unsigned long n_dof = ndof();
 //If memory has not been allocated, then allocated memory for the saved
 //dofs
 if(Saved_dof_pt==0) {Saved_dof_pt = new Vector<double>;}
 //Resize the vector
 Saved_dof_pt->resize(n_dof);

 //Transfer the values over
 for(unsigned long n=0;n<n_dof;n++) {(*Saved_dof_pt)[n] = dof(n);}
}

//====================================================================
/// Restore the saved dofs
void Problem::restore_dof_values()
{
 //Check that we can do this
 if(Saved_dof_pt==0) 
  {
   throw OomphLibError(
    "There are no stored values, use store_current_dof_values()\n",
    "Problem::restore_dof_values()",
    OOMPH_EXCEPTION_LOCATION);
  }

 //Find the number of dofs
 unsigned long n_dof = ndof();

 if(Saved_dof_pt->size() != n_dof)
  {
    throw OomphLibError(
    "The number of stored values is not equal to the current number of dofs\n",
    "Problem::restore_dof_values()",
    OOMPH_EXCEPTION_LOCATION);
  }

 //Transfer the values over
 for(unsigned long n=0;n<n_dof;n++) {dof(n) = (*Saved_dof_pt)[n];}

 //Delete the memory
 delete Saved_dof_pt;
 Saved_dof_pt = 0;
}

//======================================================================
/// Assign the eigenvector passed to the function to the dofs
//======================================================================
void Problem::assign_eigenvector_to_dofs(Vector<double> &eigenvector) 
{
 unsigned long n_dof = ndof();
 //Check that the eigenvector has the correct size
 if(eigenvector.size() != n_dof)
  {
   std::ostringstream error_message;
   error_message << "Eigenvector has size " << eigenvector.size() 
                 << ", not equal to the number of dofs in the problem," 
                 << n_dof << std::endl;

   throw OomphLibError(error_message.str(),
                       "Problem::assign_eigenvector_to_dofs()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 //Loop over the dofs and assign the eigenvector
 for(unsigned long n=0;n<n_dof;n++)
  {
   dof(n) = eigenvector[n];
  }
}



//======================================================================
/// Add the eigenvector passed to the function to the dofs with 
/// magnitude epsilon
//======================================================================
void Problem::add_eigenvector_to_dofs(const double &epsilon,
                                      Vector<double> &eigenvector) 
{
 unsigned long n_dof = ndof();
 //Check that the eigenvector has the correct size
 if(eigenvector.size() != n_dof)
  {
   std::ostringstream error_message;
   error_message << "Eigenvector has size " << eigenvector.size() 
                 << ", not equal to the number of dofs in the problem," 
                 << n_dof << std::endl;

   throw OomphLibError(error_message.str(),
                       "Problem::assign_eigenvector_to_dofs()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 //Loop over the dofs and add the eigenvector
 for(unsigned long n=0;n<n_dof;n++)
  {
   dof(n) += epsilon*eigenvector[n];
  }
}



//================================================================
/// General Newton solver. Requires only a convergence tolerance. 
/// The linear solver takes a pointer to the problem (which defines
/// the Jacobian \b J and the residual Vector \b r) and returns 
/// the solution \b x of the system 
/// \f[ {\bf J} {\bf x} = - \bf{r} \f]. 
//================================================================
void Problem::newton_solve()
{
 // Initialise timers
 double total_linear_solver_time=0.0;
#ifdef OOMPH_HAS_MPI   
 double t_start = MPI_Wtime();
#else
 clock_t t_start = clock();
#endif

 //Find total number of dofs
 unsigned long n_dofs = ndof();

 //Set up the Vector to hold the solution
 Vector<double> dx(n_dofs,0.0);

 //Set the counter
 unsigned count=0;
 //Set the loop flag
 unsigned LOOP_FLAG=1;

 //Update anything that needs updating
 actions_before_newton_solve();

 //Now do the Newton loop
 do
  {
   count++;
   
   //Do any updates that are required 
   actions_before_newton_step();


   // No degrees of freedom? What are you solving for?
   if (n_dofs==0)
    {
     oomph_info << std::endl << std::endl << std::endl;
     oomph_info << "This is a bit bizarre: The problem has no dofs." 
               << std::endl;
     oomph_info 
      << "I'll just return from the Newton solver without doing anything." 
      << std::endl;

     // Do any updates that would have been performed
     actions_before_newton_convergence_check();
     actions_after_newton_step();
     actions_before_newton_convergence_check();
     actions_after_newton_solve();

     oomph_info << "I hope this is what you intended me to do..." << std::endl;
     oomph_info 
      << std::endl << "Note: All actions_...() functions were called" 
      << std::endl;
     oomph_info << std::endl << "      before returning." << std::endl;
     oomph_info << std::endl << std::endl << std::endl;
     return;
    }

   //Calculate initial residuals
   if(count==1)
    {
     // Is the problem nonlinear? If not ignore the pre-iteration 
     // convergence check.
     if (Problem_is_nonlinear)
      {
#ifdef OOMPH_HAS_MPI
       // Synchronise the solution on different processors
       synchronise_dofs();
#endif
       actions_before_newton_convergence_check();
       get_residuals(dx);
       //Get maximum residuals, using our own abscmp function
       double maxres = std::abs(*std::max_element(dx.begin(),dx.end(),
                                                  AbsCmp<double>()));
       if (!Shut_up_in_newton_solve) 
        {
         oomph_info << "Initial Maximum residuals " << maxres << std::endl;
        }
       if(maxres < Newton_solver_tolerance) {LOOP_FLAG=0; continue;}
      }
     else
      {
       if (!Shut_up_in_newton_solve) 
        {
         oomph_info 
          << "Linear problem -- convergence in one iteration assumed." 
          << std::endl;
        }
      }
    }

   // Initialise timer for linear solver
#ifdef OOMPH_HAS_MPI   
 double t_solver_start = MPI_Wtime();
#else
 clock_t t_solver_start = clock();
#endif
   
   //Now do the linear solve -- recycling Jacobian if requested
   if (Jacobian_reuse_is_enabled&&Jacobian_has_been_computed)
    {     
     if (!Shut_up_in_newton_solve) 
      {
       oomph_info << "Not recomputing Jacobian! " << std::endl;
      }
     
     // If we're doing the first iteration and the problem is nonlinear, 
     // the residuals have already been computed above during the
     // initial convergence check. Otherwise compute them here.
     if ((count!=1)||(!Problem_is_nonlinear)) get_residuals(dx);

     // Backup residuals
     Vector<double> resid(dx);
     
     // Resolve
     Linear_solver_pt->resolve(resid,dx);
    }
   else
    {
     if (Jacobian_reuse_is_enabled)
      {
       if (!Shut_up_in_newton_solve) 
        {
         oomph_info << "Enabling resolve" << std::endl;
        }
       Linear_solver_pt->enable_resolve();
      }
     Linear_solver_pt->solve(this,dx);
     Jacobian_has_been_computed=true;
    }

   // End of linear solver
#ifdef OOMPH_HAS_MPI   
   double t_solver_end = MPI_Wtime();
   total_linear_solver_time+=t_solver_end-t_solver_start;
#else
   clock_t t_solver_end = clock();
   total_linear_solver_time+=
    double(t_solver_end-t_solver_start)/CLOCKS_PER_SEC;
#endif
   if (!Shut_up_in_newton_solve) 
    {
     oomph_info << std::endl << "Time for linear solver (ndof="
                << n_dofs << ") [sec]: " 
#ifdef OOMPH_HAS_MPI 
                << t_solver_end-t_solver_start 
#else
                << double(t_solver_end-t_solver_start)/CLOCKS_PER_SEC 
#endif
                << std::endl << std::endl;
    }
   //Subtract the new values from the true dofs
   for(unsigned l=0;l<n_dofs;l++)
    { 
     // This is needed during parallel runs when dofs that are not
     // held on the current processor are nulled out. Can change
     // this once/if the Dof_pt vector is distributed too. 
     if (Dof_pt[l]!=0) *Dof_pt[l] -= dx[l];
    }

#ifdef OOMPH_HAS_MPI
   // Synchronise the solution on different processors
   synchronise_dofs();
#endif

   // Do any updates that are required 
   actions_after_newton_step();
   actions_before_newton_convergence_check();

   // Maximum residuals
   double maxres=0.0;
   // If the user has declared that the Problem is linear
   // we ignore the convergence check
   if (Problem_is_nonlinear)
    {
     //Calculate the new residuals
     get_residuals(dx);

     //Get the maximum residuals
     maxres = std::abs(*std::max_element(dx.begin(),dx.end(),
                                         AbsCmp<double>()));
     if (!Shut_up_in_newton_solve) 
      {
       oomph_info << "Newton Step " << count << ": Maximum residuals "
                  << maxres << std::endl;
      }
    }

   //If we have converged jump straight to the test at the end of the loop
   if(maxres < Newton_solver_tolerance) {LOOP_FLAG=0; continue;} 

   //This section will not be reached if we have converged already
   //If the maximum number of residuals is too high or the maximum number
   //of iterations has been reached
   if((maxres > Max_residuals) || (count == Max_newton_iterations))
    {
     // Print a warning -- regardless of what the throw does
     if (maxres > Max_residuals) 
      {
       oomph_info << "Max. residual (" << Max_residuals 
                  << ") has been exceeded in Newton solver." << std::endl;
      }
     if (count == Max_newton_iterations)
      {
       oomph_info << "Reached max. number of iterations (" 
                  << Max_newton_iterations
                  << ") in Newton solver." << std::endl;
      }
     // Now throw...
     throw NewtonSolverError(count,maxres);
    }

  }
 while(LOOP_FLAG);

 //Now update anything that needs updating
 actions_after_newton_solve();

 // Finalise/doc timings
 if (!Shut_up_in_newton_solve) 
  {
   oomph_info << std::endl;
   oomph_info << "Total time for linear solver (ndof="<< n_dofs << ") [sec]: " 
              << total_linear_solver_time << std::endl;
  }
#ifdef OOMPH_HAS_MPI   
 double t_end = MPI_Wtime();
 double total_time=t_end-t_start;
#else
 clock_t t_end = clock();
 double total_time=double(t_end-t_start)/CLOCKS_PER_SEC;
#endif
 if (!Shut_up_in_newton_solve) 
  {
   oomph_info << "Total time for Newton solver (ndof="<< n_dofs << ") [sec]: " 
              << total_time << std::endl;
  }
 if (total_time>0.0)
  {
   if (!Shut_up_in_newton_solve) 
    {
     oomph_info << "Time outside linear solver        : "
                << (total_time-total_linear_solver_time)/total_time*100.0
                << " %"
                << std::endl;
    }
  }
 else
  {
   if (!Shut_up_in_newton_solve) 
    {
     oomph_info << "Time outside linear solver        : "
                << (total_time-total_linear_solver_time)/total_time*100.0
                << "[too fast]"
                << std::endl;
    }
  }
 if (!Shut_up_in_newton_solve) oomph_info << std::endl;
}  
 



//========================================================================
/// Solve a steady problem, in the context of an overall unsteady problem.
/// This is achieved by setting the weights in the timesteppers to be zero
/// which has the effect of rendering them steady timesteppers
/// The optional argument max_adapt specifies the max. number of 
/// adaptations of all refineable submeshes are performed to 
/// achieve the the error targets specified in the refineable submeshes.
//========================================================================
void Problem::steady_newton_solve(unsigned const &max_adapt)
{
 //Find out how many timesteppers there are
 unsigned n_time_steppers = ntime_stepper();

 // Vector of bools to store the is_steady status of the various
 // timesteppers when we came in here
 std::vector<bool> was_steady(n_time_steppers);

 //Loop over them all and make them (temporarily) static
 for(unsigned i=0;i<n_time_steppers;i++)
  {
   was_steady[i]=time_stepper_pt(i)->is_steady();
   time_stepper_pt(i)->make_steady();
  }

 try
  {
   //Solve the non-linear problem with Newton's method
   if (max_adapt==0)
    {
     newton_solve();
    }
   else
    {
     newton_solve(max_adapt);
    }
  }
 //Catch any exceptions thrown in the Newton solver
 catch(NewtonSolverError &error)
  {
   oomph_info << std::endl << "USER-DEFINED ERROR IN NEWTON SOLVER " 
             << std::endl;
   //Check whether it's the linear solver
   if(error.linear_solver_error)
    {
     oomph_info << "ERROR IN THE LINEAR SOLVER" << std::endl;
    }
   //Check to see whether we have reached Max_iterations
   else if(error.iterations==Max_newton_iterations)
    {
     oomph_info << "MAXIMUM NUMBER OF ITERATIONS (" << error.iterations << 
      ") REACHED WITHOUT CONVERGENCE " << std::endl;
    }
   //If not, it must be that we have exceeded the maximum residuals
   else
    {
     oomph_info << "MAXIMUM RESIDUALS: " << error.maxres
                << " EXCEEDS PREDEFINED MAXIMUM " << Max_residuals
          << std::endl;
    }

   //Die horribly!!
   std::ostringstream error_stream;
   error_stream << "Error occured in Newton solver. "
                << std::endl;
   throw OomphLibError(error_stream.str(),
                       "Problem::steady_newton_solve()",
                       OOMPH_EXCEPTION_LOCATION);
  }


 // Reset the is_steady status of all timesteppers that
 // weren't already steady when we came in here and reset their
 // weights
 for(unsigned i=0;i<n_time_steppers;i++)
  {
   if (!was_steady[i])
    {
     time_stepper_pt(i)->undo_make_steady();
    }
  }

 // Since we performed a steady solve, the history values 
 // now have to be set as if we had performed an impulsive start from
 // the current solution. This ensures that the time-derivatives
 // evaluate to zero even now that the timesteppers have been
 // reactivated. 
 assign_initial_values_impulsive();

}

//===========================================================================
///Perform a basic continuation step using Newton's method. The governing
///parameter of the problem is passed as a pointer to the routine. The
///number of Newton steps taken is returned
//==========================================================================
unsigned Problem::
newton_solve_continuation(double* const &parameter_pt)
{
 //Set up memory for z 
 unsigned long n_dofs = ndof();
 Vector<double> z(n_dofs);
 //Call the solver
 return newton_solve_continuation(parameter_pt,z);
}


//===================================================================
/// This function performs a basic continuation step using the Newton method. 
/// The number of Newton steps taken is returned, to be used in any 
/// external step-size control routines. 
/// The governing parameter of the problem is passed as a pointer to the
/// routine, as is the sign of the Jacobian and a Vector in which 
/// to store the derivatives wrt the parameter, if required.
//==================================================================
unsigned Problem::
newton_solve_continuation(double* const &parameter_pt,
                          Vector<double> &z)
{
 //Find the total number of dofs
 unsigned long n_dofs = ndof();
 //Assign memory for solutions of the equations
 Vector<double> y(n_dofs);
 //Assign memory for the dot products of the uderivatives and y and z
 double uderiv_dot_y = 0.0, uderiv_dot_z = 0.0;
 //Set and initialise the counter
 unsigned count=0;
 //Set the loop flag
 unsigned LOOP_FLAG=1;
     
 //Update anything that needs updating
 actions_before_newton_solve();

 //Check the arc-length constraint
 double arc_length_constraint_residual=0.0;

 //Are we storing the matrix in the linear solve
 bool enable_resolve = Linear_solver_pt->resolve_is_enabled();
 
 //For this problem, we must store the residuals
 Linear_solver_pt->enable_resolve();
 
 //Now do the Newton loop
 do
  {
   count++;
       
   //Do any updates that are required 
   actions_before_newton_step();
   
   //Calculate initial residuals
   if(count==1)
    {
#ifdef OOMPH_HAS_MPI
     // Synchronise the solution on different processors
     synchronise_dofs();
#endif
     actions_before_newton_convergence_check();
     get_residuals(y);
     //Get maximum residuals, using our own abscmp function
     double maxres = std::abs(*std::max_element(y.begin(),y.end(),
                                                AbsCmp<double>()));

     //Assemble the residuals for the arc-length step
     arc_length_constraint_residual = 0.0; 
     //Add the variables
     for(unsigned long l=0;l<n_dofs;l++)
      {
       arc_length_constraint_residual +=
        Dof_derivatives[l]*(*Dof_pt[l] - Dofs_current[l]);
      }
     arc_length_constraint_residual *= Theta_squared;
     arc_length_constraint_residual += 
      Parameter_derivative*(*parameter_pt - Parameter_current) - Ds_current;

     //Is it the max
     if(std::abs(arc_length_constraint_residual) > maxres)
      {
       maxres = std::abs(arc_length_constraint_residual);
      }

     //Find the max
     oomph_info << "Initial Maximum residuals " << maxres << std::endl;
     //If we are below the Tolerance, then return immediately
     if(maxres < Newton_solver_tolerance) {LOOP_FLAG=0; count=0; continue;}
    }
   
   //If it's the block hopf solver we need to solve for both rhs's
   //simultaneously. This is because the block decomposition involves
   //solves with two different matrices and storing both at once to 
   //allow general resolves would be more expensive than necessary.
   if(dynamic_cast<BlockHopfLinearSolver*>(Linear_solver_pt))
    {
     //Get the vector dresiduals/dparameter
     get_derivative_wrt_global_parameter(parameter_pt,z);
     
     // Copy rhs vector into local storage so it doesn't get overwritten
     // if the linear solver decides to initialise the solution vector, say,
     // which it's quite entitled to do!
     Vector<double> input_z(z);

     //Solve the system for the two right-hand sides.
     dynamic_cast<BlockHopfLinearSolver*>(Linear_solver_pt)->
      solve_for_two_rhs(this,y,input_z,z);
    }
   //Otherwise
   else
    {
     //Solve the standard problem
     Linear_solver_pt->solve(this,y);

     //Get the vector dresiduals/dparameter
     get_derivative_wrt_global_parameter(parameter_pt,z);
     
     // Copy rhs vector into local storage so it doesn't get overwritten
     // if the linear solver decides to initialise the solution vector, say,
     // which it's quite entitled to do!
     Vector<double> input_z(z);
     
     //Now resolve the system with the new RHS
     Linear_solver_pt->resolve(input_z,z);
    }
  
   //Now we need to calculate dparam, for which we must calculate the 
   //dot product of the derivatives and y and z
   //Reset these values to zero
   uderiv_dot_y = 0.0; uderiv_dot_z=0.0;
   //Now calculate the dot products of the derivative and the solutions
   //of the linear system
   for(unsigned long l=0;l<n_dofs;l++) 
    {
     uderiv_dot_y += Dof_derivatives[l]*y[l];
     uderiv_dot_z += Dof_derivatives[l]*z[l];
    }
   uderiv_dot_y *= Theta_squared;
   uderiv_dot_z *= Theta_squared;

   //The set the change in the parameter, given by the pseudo-arclength
   //equation. Note that here we are assuming that the arc-length
   //equation is always exactly zero, 
   //which seems to work OK, and saves on some storage. 
   //In fact, it's more subtle than that. If we include this
   //proper residual then we will have to solve the eigenproblem.
   //This will make the solver more robust and *should* be done
   // ... at some point.
   double dparam = (arc_length_constraint_residual - uderiv_dot_y)
    /(Parameter_derivative - uderiv_dot_z);
   
   //Set the new value of the parameter
   *parameter_pt -= dparam;
       
   //Update the values of the other degrees of freedom
   for(unsigned long l=0;l<n_dofs;l++) {*Dof_pt[l] -= y[l] - dparam*z[l];}

   // Do any updates that are required 
   actions_after_newton_step();
   
   //Calculate the new residuals
#ifdef OOMPH_HAS_MPI
   // Synchronise the solution on different processors
   synchronise_dofs();
#endif
   actions_before_newton_convergence_check();
   get_residuals(y);
   
   //Get the maximum residuals
   double maxres = std::abs(*std::max_element(y.begin(),y.end(),
                                              AbsCmp<double>()));

   //Assemble the residuals for the arc-length step
   arc_length_constraint_residual = 0.0; 
   //Add the variables
   for(unsigned long l=0;l<n_dofs;l++)
    {
     arc_length_constraint_residual +=
      Dof_derivatives[l]*(*Dof_pt[l] - Dofs_current[l]);
    }
   arc_length_constraint_residual *= Theta_squared;
   arc_length_constraint_residual +=
    Parameter_derivative*(*parameter_pt - Parameter_current) 
    - Ds_current;

   //Is it the max
   if(std::abs(arc_length_constraint_residual) > maxres)
    {
     maxres = std::abs(arc_length_constraint_residual);
    }

   oomph_info << "Newton Step " << count 
              << ": Maximum residuals " << maxres << std::endl;
   
   //If we have converged jump straight to the test at the end of the loop
   if(maxres < Newton_solver_tolerance) {LOOP_FLAG=0; continue;} 
   
   //This section will not be reached if we have converged already
   //If the maximum number of residuals is too high or the maximum number
   //of iterations has been reached
   if((maxres > Max_residuals) || (count == Max_newton_iterations))
    {
     throw NewtonSolverError(count,maxres);
    }
   
  }
 while(LOOP_FLAG);
 
 //Now update anything that needs updating
 actions_after_newton_solve();

 //Reset the storage of the matrix on the linear solver to what it was
 //on entry to this routine
 if (enable_resolve)
  {
   Linear_solver_pt->enable_resolve();
  }
 else
  {
   Linear_solver_pt->disable_resolve();
  }
 
 //Return the number of Newton Steps taken
 return count;
}

//=========================================================================
/// A function to calculate the derivatives wrt the arc-length. This version
/// of the function actually does a linear solve so that the derivatives
/// are calculated "exactly" rather than using the values at the Newton
/// step just before convergence. This is only necessary in spatially adaptive
/// problems, in which the number of degrees of freedom changes and so
/// the appropriate derivatives must be calculated for the new variables.
//=========================================================================
void Problem::
calculate_continuation_derivatives(double* const &parameter_pt)
{
 //Find the number of degrees of freedom in the problem
 const unsigned long n_dofs = ndof();

 //Assign memory for solutions of the equations
 Vector<double> z(n_dofs);

 //If it's the block hopf solver need to solve for both RHS
 //at once, but this would all be alleviated if we have the solve
 //for the non-residuals RHS.
 if(dynamic_cast<BlockHopfLinearSolver*>(Linear_solver_pt))
  {
   //Get the vector dresiduals/dparameter
   get_derivative_wrt_global_parameter(parameter_pt,z);
   
   
   // Copy rhs vector into local storage so it doesn't get overwritten
   // if the linear solver decides to initialise the solution vector, say,
   // which it's quite entitled to do!
   Vector<double> dummy(n_dofs), input_z(z);

   //Solve for the two RHSs
   dynamic_cast<BlockHopfLinearSolver*>(Linear_solver_pt)->
    solve_for_two_rhs(this,dummy,input_z,z);
  }
 //Otherwise we can use the normal resolve
 else
  {
   //Save the status before entry to this routine
   bool enable_resolve = Linear_solver_pt->resolve_is_enabled();
   
   //We need to do resolves
   Linear_solver_pt->enable_resolve();

   //Solve the standard problem, we only want to make sure that
   //we factorise the matrix, if it has not been factorised. We shall
   //ignore the return value of z.
   Linear_solver_pt->solve(this,z);
   
   //Get the vector dresiduals/dparameter
   get_derivative_wrt_global_parameter(parameter_pt,z);
   
   
   // Copy rhs vector into local storage so it doesn't get overwritten
   // if the linear solver decides to initialise the solution vector, say,
   // which it's quite entitled to do!
   Vector<double> input_z(z);
   
   //Now resolve the system with the new RHS and overwrite the solution
   Linear_solver_pt->resolve(input_z,z);
   
   //Restore the storage status of the linear solver
   if (enable_resolve)
    {
     Linear_solver_pt->enable_resolve();
    }
   else 
    {
     Linear_solver_pt->disable_resolve();
    }
  }

 //Now, we can calculate the derivatives, etc
 calculate_continuation_derivatives(z);
}

//=======================================================================
/// A function to calculate the derivatives with respect to the arc-length
/// required for continuation. The arguments is the solution of the 
/// linear system,
/// Jz = dR/dparameter, that gives du/dparameter and the direction 
/// output from the newton_solve_continuation function. The derivatives
/// are stored in the ContinuationParameters namespace.
//===================================================================
void Problem::calculate_continuation_derivatives(const Vector<double> &z)
{
 //Calculate the continuation derivatives
 calculate_continuation_derivatives_helper(z);

 //Scale the value of theta if the control flag is set
 if(Scale_arc_length)
  {
   Theta_squared *= (Parameter_derivative*Parameter_derivative/
                     Desired_proportion_of_arc_length)*
    ((1.0 - Desired_proportion_of_arc_length)/
     (1.0 - Parameter_derivative*Parameter_derivative));
 
   //Recalculate the continuation derivatives with the new scaled values
   calculate_continuation_derivatives_helper(z);
  }
}

//=======================================================================
/// A private helper function to
/// calculate the derivatives with respect to the arc-length
/// required for continuation. The arguments is the solution of the 
/// linear system,
/// Jz = dR/dparameter, that gives du/dparameter and the direction 
/// output from the newton_solve_continuation function. The derivatives
/// are stored in the ContinuationParameters namespace.
//===================================================================
void Problem::calculate_continuation_derivatives_helper(
 const Vector<double> &z)
{
 //Find the number of degrees of freedom in the problem
 unsigned long n_dofs = ndof();
 
 //Work out the continuation direction
 Continuation_direction = Parameter_derivative;
 for(unsigned long l=0;l<n_dofs;l++) 
  {Continuation_direction -= Dof_derivatives[l]*z[l];}

 //Calculate the magnitude of the du/ds Vector
 //Note that actually, we are usually approximating by using the value at 
 //newton step just before convergence, which saves one additional 
 //Newton solve.
 
 //First calculate the magnitude of du/dparameter, chi
 double chi = 0.0; 
 for(unsigned long l=0;l<n_dofs;l++) {chi += z[l]*z[l];}
                                                   
 //Calculate the current derivative of the parameter wrt the arc-length
 Parameter_derivative = 1.0/sqrt(1.0 + Theta_squared*chi);
 
 //If the dot product of the current derivative wrt the Direction
 //is less than zero, switch the sign of the derivative to ensure 
 //smooth continuation
 if(Parameter_derivative*Continuation_direction < 0.0) 
  {Parameter_derivative*= -1.0;}

 //Resize the derivatives array, if necessary
 if(Dof_derivatives.size() != n_dofs) {Dof_derivatives.resize(n_dofs,0.0);}
 //Calculate the new derivatives wrt the arc-length
 for(unsigned long l=0;l<n_dofs;l++)
  {
   //This comes from the formulation J u_dot + dr/dlambda  lambda_dot = 0
   //on the curve and then it follows that.
   Dof_derivatives[l] = -Parameter_derivative*z[l];
  }
}

//============================================================
/// Activate the fold tracking system by changing the assembly
/// handler and initialising it using the parameter addressed 
/// by parameter_pt.
//============================================================
void Problem::activate_fold_tracking(double* const &parameter_pt,
                                     const bool &block_solve)
{
 //Reset the assembly handler to default
 reset_assembly_handler_to_default();
 //Set the new assembly handler. Note that the constructor actually
 //solves the original problem to get some initial conditions, but
 //this is OK because the RHS is always evaluated before assignment.
 Assembly_handler_pt = new FoldHandler(this,parameter_pt);

 //If we are using a block solver, we must set the linear solver pointer
 //to the block fold solver. The present linear solver is
 //used by the block solver and so must be passed as an argument.
 //The destructor of the Fold handler returns the linear
 //solver to the original non-block version.
 if(block_solve)
  {
   Linear_solver_pt = new BlockFoldLinearSolver(Linear_solver_pt);
  }
}


//==================================================================
/// Activate the pitchfork tracking system by changing the assembly
/// handler and initialising it using the parameter addressed 
/// by parameter_pt and a symmetry vector. The boolean flag is
/// used to specify whether a block solver is used, default is true.
//===================================================================
void Problem::activate_pitchfork_tracking(
 double* const &parameter_pt,
 const Vector<double> &symmetry_vector,const bool &block_solve)
{
 //Reset the assembly handler to default
 reset_assembly_handler_to_default();
 //Set the new assembly handler. Note that the constructor actually
 //solves the original problem to get some initial conditions, but
 //this is OK because the RHS is always evaluated before assignment.
 Assembly_handler_pt = new PitchForkHandler(this,parameter_pt,
                                            symmetry_vector);

 //If we are using a block solver, we must set the linear solver pointer
 //to the block pitchfork solver. The present linear solver is
 //used by the block solver and so must be passed as an argument.
 //The destructor of the PitchFork handler returns the linear
 //solver to the original non-block version.
 if(block_solve)
  {
   Linear_solver_pt = new BlockPitchForkLinearSolver(Linear_solver_pt);
  }
}



//============================================================
/// Activate the hopf tracking system by changing the assembly
/// handler and initialising it using the parameter addressed 
/// by parameter_pt.
//============================================================
void Problem::activate_hopf_tracking(
 double* const &parameter_pt, const bool &block_solve)
{
 //Reset the assembly handler to default
 reset_assembly_handler_to_default();
 //Set the new assembly handler. Note that the constructor actually
 //solves the original problem to get some initial conditions, but
 //this is OK because the RHS is always evaluated before assignment.
 Assembly_handler_pt = new HopfHandler(this,parameter_pt);

 //If we are using a block solver, we must set the linear solver pointer
 //to the block hopf solver. The present linear solver is
 //used by the block solver and so must be passed as an argument.
 //The destructor of the Hopf handler returns the linear
 //solver to the original non-block version.
 if(block_solve)
  {
   Linear_solver_pt = new BlockHopfLinearSolver(Linear_solver_pt);
  }
}


//============================================================
/// Activate the hopf tracking system by changing the assembly
/// handler and initialising it using the parameter addressed 
/// by parameter_pt and the frequency and null vectors
/// specified.
//============================================================
void Problem::activate_hopf_tracking(
 double* const &parameter_pt, const double &omega,
 const Vector<double> &null_real, const Vector<double> &null_imag,
 const bool &block_solve)
{
 //Reset the assembly handler to default
 reset_assembly_handler_to_default();
 //Set the new assembly handler. Note that the constructor actually
 //solves the original problem to get some initial conditions, but
 //this is OK because the RHS is always evaluated before assignment.
 Assembly_handler_pt = new HopfHandler(this,parameter_pt,omega,
                                       null_real,null_imag);

 //If we are using a block solver, we must set the linear solver pointer
 //to the block hopf solver. The present linear solver is
 //used by the block solver and so must be passed as an argument.
 //The destructor of the Hopf handler returns the linear
 //solver to the original non-block version.
 if(block_solve)
  {
   Linear_solver_pt = new BlockHopfLinearSolver(Linear_solver_pt);
  }
}

 
//===============================================================
///Reset the assembly handler to default
//===============================================================
void Problem::reset_assembly_handler_to_default()
{
 //If we have a non-default handler
 if(Assembly_handler_pt != Default_assembly_handler_pt) 
  {
   //Delete the current assembly handler 
   delete Assembly_handler_pt; 
   //Reset the assembly handler
   Assembly_handler_pt = Default_assembly_handler_pt;
  }
}

//===================================================================
/// This function takes one step of length ds in pseudo-arclength.The 
/// argument parameter_pt is a pointer to the parameter (global variable) 
/// that is being traded for arc-length. The function returns the next desired
/// arc-length according to criteria based upon the desired number of Newton 
/// Iterations per solve.
//=====================================================================
double Problem::arc_length_step_solve(double* const &parameter_pt,
                                      const double &ds)
{
 //----------------------MAKE THE PROBLEM STEADY-----------------------
 //Loop over the timesteppers and make them (temporarily) steady.
 //We can only do continuation for steady problems!
 unsigned n_time_steppers = ntime_stepper();
  for(unsigned i=0;i<n_time_steppers;i++) 
  {
   time_stepper_pt(i)->make_steady();
  }
 
 //----------SAVE THE INITIAL VALUES, IN CASE THE STEP FAILS-----------
 //Find total number of dofs
 unsigned long n_dofs = ndof();
 //Safety check, set up the array of Dof_derivatives, if necessary
 if(Dof_derivatives.size() != n_dofs) {Dof_derivatives.resize(n_dofs,0.0);}
 //Save the current value of the parameter
 Parameter_current = *parameter_pt;

 //Save the current values of the degrees of freedom 
 //Safety check, set up the array of Dof_derivatives, if necessary
 if(Dofs_current.size() != n_dofs) {Dofs_current.resize(n_dofs);}
 for(unsigned long l=0;l<n_dofs;l++) {Dofs_current[l] = *Dof_pt[l];}
 //Set the value of ds_current
 Ds_current = ds;

 //----SET UP MEMORY FOR QUANTITIES THAT ARE REQUIRED OUTSIDE THE LOOP----

 //Assign memory for solutions of the equations Jz = du/dparameter
 //This is needed here (outside the loop), so that we can save on 
 //one linear solve when calculating the derivatives wrt the arc-length
 Vector<double> z(n_dofs);
 //Store sign of the Jacobian, used for bifurcation detection
 //If this is the first time that we are calling the arc-length solver,
 //this should not be used.
 int previous_sign = Sign_of_jacobian;

 //Counter for the number of newton steps
 unsigned count=0;
 //Flag to indicate a successful step
 bool STEP_REJECTED=false;
 
 //Flag to indicate a sign change
 bool SIGN_CHANGE=false;

 //Loop around the step in arc-length
 do
  {
   //Check that the step has not fallen below the minimum tolerance
   if(std::abs(Ds_current) < Minimum_ds)
    {
     std::ostringstream error_message;
     error_message << "DESIRED ARC-LENGTH STEP " << Ds_current 
                   << " HAS FALLEN BELOW MINIMUM TOLERANCE, " 
                   << Minimum_ds << std::endl;
     
     throw OomphLibError(error_message.str(),
                         "Problem::arc_length_step_solve()",
                         OOMPH_EXCEPTION_LOCATION);
    }

   //Assume that we shall accept the step
   STEP_REJECTED=false;
   //Set initial value of the parameter
   *parameter_pt += Parameter_derivative*Ds_current;
   //Loop over the variables and set their initial values
   for(unsigned long l=0;l<n_dofs;l++) 
    {*Dof_pt[l] += Dof_derivatives[l]*Ds_current;}
   
   //Actually do the newton solve stage for the continuation problem
   try
    {
     count = newton_solve_continuation(parameter_pt,z);
    } 
   //Catch any exceptions thrown in the Newton solver
   catch(NewtonSolverError &error)
    {
     //Check whether it's the linear solver
     if(error.linear_solver_error)
      {
       std::ostringstream error_stream;
       error_stream << std::endl 
                    << "USER-DEFINED ERROR IN NEWTON SOLVER " << std::endl;
       oomph_info << "ERROR IN THE LINEAR SOLVER" << std::endl;
       throw OomphLibError(error_stream.str(),
                           "Problem::arc_length_step_solve()",
                           OOMPH_EXCEPTION_LOCATION);
      }
     //Otherwise mark the step as having failed
     else 
      {
       oomph_info << "STEP REJECTED --- TRYING AGAIN" << std::endl;
       STEP_REJECTED=true;
       //Let's take a smaller step
       Ds_current *= (2.0/3.0);
       //Reset the dofs and parameter
       *parameter_pt = Parameter_current;
       for(unsigned long l=0;l<n_dofs;l++) {*Dof_pt[l] = Dofs_current[l];}
      }
    }
  }
 while(STEP_REJECTED); //continue until a step is accepted
 
 //Only recalculate the derivatives if there has been a Newton solve
 //If not, the previous values should be close enough 
 if(count>0)
  {
   //--------------------CHECK FOR POTENTIAL BIFURCATIONS-------------
   //If the sign of the jacobian is zero issue a warning
   if(Sign_of_jacobian == 0) 
    {
     std::string error_message =
      "The sign of the jacobian is zero after a linear solve\n";
     error_message +=
      "Either the matrix is singular (unlikely),\n";
     error_message +=
      "or the linear solver cannot compute the determinant of the matrix;\n";
     error_message += "e.g. an iterative linear solver.\n";
     error_message +=
      "If the latter, bifurcation detection must be via an eigensolver\n";
     OomphLibWarning(error_message,
                     "Problem::arc_length_step_solve",
                     OOMPH_EXCEPTION_LOCATION);
    }
   //If this is the first step, we cannot rely on the previous value 
   //of the jacobian so set the previous sign to the present sign
   if(!Arc_length_step_taken) {previous_sign = Sign_of_jacobian;}
   //If we have detected a sign change in the last converged Jacobian,
   //it must be a turning point or bifurcation
   if(Sign_of_jacobian != previous_sign)
    {
     //There has been, at least, one sign change
     First_jacobian_sign_change = true;

     //The sign has changed this time
     SIGN_CHANGE=true;

     //Calculate the dot product of the approximate null vector
     //of the Jacobian matrix ((badly) approximated by z)
     //and the vectors of derivatives of the residuals wrt the global parameter
     //If this is small it is a bifurcation rather than a turning point.
     //Get the derivative wrt global parameter
     Vector<double> dparam(n_dofs);
     get_derivative_wrt_global_parameter(parameter_pt,dparam);
     //Calculate the dot product
     double dot=0.0;
     for(unsigned long n=0;n<n_dofs;++n) {dot += dparam[n]*z[n];}

     //Write the output message
     std::ostringstream message;
     message << "-----------------------------------------------------------";
     message << std::endl << "SIGN CHANGE IN DETERMINANT OF JACOBIAN: " 
             << std::endl;
     message << "BIFURCATION OR TURNING POINT DETECTED BETWEEN "
             << Parameter_current << " AND " << *parameter_pt << std::endl;
     message << "APPROXIMATE DOT PRODUCT : " << dot << "," << std::endl;
     message << "IF CLOSE TO ZERO WE HAVE A BIFURCATION; ";
     message << "OTHERWISE A TURNING POINT" << std::endl;
     message << "-----------------------------------------------------------"
             << std::endl;

     //Write the message to standard output
     oomph_info << message.str();

     //Open the information file for appending
     std::ofstream bifurcation_info("bifurcation_info",std::ios_base::app);
     //Write the message to the file
     bifurcation_info << message.str();
     bifurcation_info.close();
    }
   
   //Calculate the derivatives required for the next stage of continuation
   //In this we pass the last value of z (i.e. approximation)
   calculate_continuation_derivatives(z); 

   //If it's the first step then the value of the next step should
   //be the change in parameter divided by the parameter derivative
   //to obtain approximately the same parameter change
   if(!Arc_length_step_taken) 
    {
     Ds_current = (*parameter_pt - Parameter_current)/Parameter_derivative;
    }
   
   //We have taken our first step
   Arc_length_step_taken = true;
  }
 //Otherwise calculate the continuation derivatives by solving the linear
 //system. We must do this to ensure that the derivatives are in sync
 //It could lead to problems near turning points when we should really be
 //solving an eigenproblem, but seems OK so far!
 else
  {
   //Save the current sign of the jacobian
   int temp_sign=Sign_of_jacobian;
   //Calculate the continuation derivatives, which includes a solve
   //of the linear system
   calculate_continuation_derivatives(parameter_pt);
   //Reset the sign of the jacobian, just in case the sign has changed when
   //solving the continuation derivatives. The sign change will be picked
   //up on the next continuation step.
   Sign_of_jacobian = temp_sign;
  }

 /*{
  Vector<double> z(n_dofs);
  //Cheeky tester
  double length=0.0;
  for(unsigned long l=0;l<n_dofs;l++)
   {
    z[l] = (*Dof_pt[l] - Dofs_current[l])/Ds_current;
    length += Theta_squared*z[l]*z[l];
   }

  double Z = (*parameter_pt - Parameter_current)/Ds_current;
  length += Z*Z;

  length = sqrt(length);
  for(unsigned long l=0;l<n_dofs;l++)
   {
    Dof_derivatives[l] = z[l]/length;
   }
  
  Parameter_derivative = Z/length;
  } */

 //If we are trying to find a bifurcation and the first sign change
 //has occured, use bisection
 if((Bifurcation_detection) && (First_jacobian_sign_change))
  {
   //If there has been a sign change we need to half the step size
   //and reverse the direction
   if(SIGN_CHANGE) {Ds_current *= -0.5;}
   //Otherwise
   else
    {
     //The size of the bracketed interval is always
     //2ds - Ds_current (this will work even if the original step failed)
     //We want our new step size to be half this
     Ds_current = ds - 0.5*Ds_current;
    }
   //Return the desired value of the step
   return Ds_current;
  }

 //If fewer than the desired number of Newton Iterations, increase the step
 if(count < Desired_newton_iterations_ds) {return Ds_current*1.5;}
 //If more than the desired number of Newton Iterations, reduce the step
 if(count > Desired_newton_iterations_ds) {return Ds_current*(2.0/3.0);}
 //Otherwise return the step just taken
 return Ds_current;
}

//========================================================================
/// Do one timestep of size dt using Newton's method with the specified 
/// tolerance and linear solver defined as member data of the Problem class.
/// This will be the most commonly used version 
/// of  unsteady_newton_solve, in which the time values are always shifted
/// This does not include any kind of adaptativity. If the solution fails to 
/// converge the program will end.
//========================================================================
void Problem::unsteady_newton_solve(const double &dt)
{
 //We shift the values, so shift_values is true
 unsteady_newton_solve(dt,true);
}

//========================================================================
/// Do one timestep forward of size dt using Newton's method with the 
/// specified tolerance and linear solver defined via member data of the 
/// Problem class.
/// The boolean flag shift_values is used to control whether the time values
/// should be shifted or not. 
//========================================================================
void Problem::unsteady_newton_solve(const double &dt, const bool &shift_values)
{
 //Shift the time values and the dts, according to the control flag
 if(shift_values) {shift_time_values();}
 
 // Advance global time and set current value of dt 
 time_pt()->time()+=dt;
 time_pt()->dt()=dt;

 //Find out how many timesteppers there are
 unsigned n_time_steppers = ntime_stepper();

 //Loop over them all and set the weights
 for(unsigned i=0;i<n_time_steppers;i++)
  {
   time_stepper_pt(i)->set_weights();
  }

 //Now update anything that needs updating before the timestep
 //This could be time-dependent boundary conditions, for example.
 actions_before_implicit_timestep();

 try
  {
   //Solve the non-linear problem for this timestep with Newton's method
   newton_solve();
  }
 //Catch any exceptions thrown in the Newton solver
 catch(NewtonSolverError &error)
  {
   oomph_info << std::endl << "USER-DEFINED ERROR IN NEWTON SOLVER " 
              << std::endl;
   //Check whether it's the linear solver
   if(error.linear_solver_error)
    {
     oomph_info << "ERROR IN THE LINEAR SOLVER" << std::endl;
    }
   //Check to see whether we have reached Max_iterations
   else if(error.iterations==Max_newton_iterations)
    {
     oomph_info << "MAXIMUM NUMBER OF ITERATIONS (" << error.iterations 
                << ") REACHED WITHOUT CONVERGENCE " << std::endl;
    }
   //If not, it must be that we have exceeded the maximum residuals
   else
    {
     oomph_info << "MAXIMUM RESIDUALS: " << error.maxres
                << " EXCEEDS PREDEFINED MAXIMUM " << Max_residuals
                << std::endl;
    }
   //Die horribly!!
   std::ostringstream error_stream;
   error_stream << "Error occured in unsteady Newton solver. "
                << std::endl;
   throw OomphLibError(error_stream.str(),
                       "Problem::unsteady_newton_solve()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 //Now update anything that needs updating after the timestep
 actions_after_implicit_timestep();
}

//=======================================================================
/// Attempt to take one timestep forward using dt_desired. The error control
/// parameter, epsilon, is used to specify the desired approximate value of the
/// global error norm per timestep. The routine returns the value an estimate
/// of the next value of dt that should be taken. 
//=======================================================================
double Problem::
adaptive_unsteady_newton_solve(const double &dt_desired,
                               const double &epsilon)
{
 //We always want to shift the time values
 return adaptive_unsteady_newton_solve(dt_desired,epsilon,true);
}


//=======================================================================
/// Attempt to take  one timestep forward using the dt_desired.
/// This is the driver for a number of adaptive solvers. If the solution
/// fails to converge at a given timestep, the routine will automatically
/// halve the time step and try again, until the time step falls below the
/// specified minimum value. The routine returns the value an estimate
/// of the next value of dt that should be taken. 
//========================================================================
double Problem::
adaptive_unsteady_newton_solve(const double &dt_desired,
                               const double &epsilon, 
                               const bool &shift_values)
{
 //First, we need to backup the existing dofs, in case the timestep is 
 //rejected 
 //Find total number of dofs
 unsigned long n_dofs = ndof();
 //Now set up a Vector to hold current values
 Vector<double> dofs_current(n_dofs);
 //Load values into dofs_current
 for(unsigned i=0;i<n_dofs;i++) dofs_current[i] = *Dof_pt[i];
 //Store the time
 double time_current = time_pt()->time();

 //Flag to detect whether the timestep has been rejected or not
 unsigned REJECT_TIMESTEP=0;
 //Flag to detect whether any of the timesteppers are adaptive
 unsigned ADAPTIVE_FLAG=0;
 //The value of the actual timestep, by default the same as desired timestep
 double dt_actual=dt_desired;
 //Timestep rescaling factor, 1.0 by default
 double DTSF = 1.0;
 
 //Determine the number of timesteppers
 unsigned n_time_steppers = ntime_stepper();
 //Find out whether any of the timesteppers are adaptive
 for(unsigned i=0;i<n_time_steppers;i++)
  {
   if(time_stepper_pt(i)->adaptive_flag())
    {
     ADAPTIVE_FLAG=1; 
     break;
    }
  }

 //Shift the time_values according to the control flag
 if(shift_values) {shift_time_values();}

 //This loop surrounds the adaptive time-stepping critera
 do
  {
 //This loop surrounds the Newton solver and will not
 //be broken until a timestep is accepted
 do
  {
   //Initially the timestep is presumed to be accepted
   REJECT_TIMESTEP=0;

   //Set the new time and value of dt
   time_pt()->time() += dt_actual;
   time_pt()->dt() = dt_actual;

   //Loop over all timesteppers and set the weights and predictor weights
   for(unsigned i=0;i<n_time_steppers;i++)
    {
     //If the time_stepper is non-adaptive, this will be zero
     time_stepper_pt(i)->set_predictor_weights();
     time_stepper_pt(i)->set_weights();
    }

   //Now calculate the predicted values for the all data and all positions
   calculate_predictions();

   //Do any updates/boundary conditions changes here
   actions_before_implicit_timestep();
   
   //Attempt to solver the non-linear system
   try
    {
     //Solve the non-linear problem at this timestep
     newton_solve();
    }
   //Catch any exceptions thrown
   catch(NewtonSolverError &error)
    {
     //If it's a solver error then die
     if(error.linear_solver_error)
      {
       std::string error_message =
        "USER-DEFINED ERROR IN NEWTON SOLVER\n";
       error_message +=  "ERROR IN THE LINEAR SOLVER\n";

       //Die
       throw OomphLibError(error_message,
                           "Problem::adaptive_unsteady_newton_solve()",
                           OOMPH_EXCEPTION_LOCATION);
      }
     else
      {
       oomph_info << "TIMESTEP REJECTED --- HALVING TIMESTEP AND TRYING AGAIN" 
                  << std::endl;
       //Reject the timestep, if we have an exception
       REJECT_TIMESTEP=1;
       //Essentially all I do here is half the next timestep
       dt_actual *= 0.5;
       //Reset the time
       time_pt()->time() = time_current;
       //Reload the dofs
       for(unsigned i=0;i<n_dofs;i++) *Dof_pt[i] = dofs_current[i];

#ifdef OOMPH_HAS_MPI
       // Synchronise the solution on different processors
       synchronise_dofs();
#endif
       //Call all "after" actions, e.g. to handle mesh updates
       actions_after_newton_step();
       actions_before_newton_convergence_check();
       actions_after_newton_solve();
       actions_after_implicit_timestep();
       //Skip to the next iteration
       continue;
      }
    }

   //Break out of the loop if the timestep has become too small
   if(dt_actual < Minimum_dt)
    {
     std::ostringstream error_message;
     error_message 
      << "TIMESTEP (" << dt_actual 
      << ") HAS FALLEN BELOW SPECIFIED THRESHOLD: Problem::Minimum_dt=" 
      << Minimum_dt << std::endl;

     throw OomphLibError(error_message.str(),
                         "Problem::adaptive_unsteady_newton_solve()",
                         OOMPH_EXCEPTION_LOCATION);
    }

   //Update anything that needs updating after the timestep
   actions_after_implicit_timestep();
  }
 //Keep looping until we accept the timestep
 while(REJECT_TIMESTEP);
  
 //If we have an adapative timestepper
 if(ADAPTIVE_FLAG)
  {
   //Once timestep has been accepted can do fancy error processing
   //Set the error weights
   for(unsigned i=0;i<n_time_steppers;i++)
    {
     time_stepper_pt(i)->set_error_weights();
    }

   //Call a global error, at the moment I'm just going to use a square norm 
   double error = global_temporal_error_norm(); 

   //Calculate the scaling  factor
   DTSF = pow((epsilon/error),
              (1.0/(1.0+time_stepper_pt()->order())));

   oomph_info << "DTSF is  " << DTSF << std::endl;
   oomph_info << "Estimated timestepping error is " << error << std::endl;

   //Now decide what to do based upon DTSF
   //If it's small reject the timestep
   if(DTSF <= 0.8)
    {
     oomph_info << "TIMESTEP REJECTED" << std::endl;
     //Reject the timestep
     REJECT_TIMESTEP=1;
     //Modify the actual timestep
     dt_actual *= DTSF;
     //Reset the time
     time_pt()->time() = time_current;
     //Reload the dofs
     for(unsigned i=0;i<n_dofs;i++) *Dof_pt[i] = dofs_current[i];

#ifdef OOMPH_HAS_MPI
       // Synchronise the solution on different processors
       synchronise_dofs();
#endif

     //Call all "after" actions, e.g. to handle mesh updates
     actions_after_newton_step();
     actions_before_newton_convergence_check();
     actions_after_newton_solve();
     actions_after_implicit_timestep();
     continue;
    }
   //If it's large change the timestep
   if(DTSF >= 1.0)
    {
     //Restrict the increase
     if(DTSF > DTSF_max_increase)
      {
       DTSF = DTSF_max_increase;
       oomph_info << "DTSF LIMITED TO " << DTSF_max_increase << std::endl;
      }
    }

  } //End of if adaptive flag

  }
 //Keep this loop going, again until we accept the timestep
 while(REJECT_TIMESTEP);


 //Make sure timestep doesn't get too large
 if ((dt_actual*DTSF) > Maximum_dt)
  {
   oomph_info << "DTSF WOULD INCREASE TIMESTEP "
        << "ABOVE SPECIFIED THRESHOLD: Problem::Maximum_dt=" 
        <<  Maximum_dt << std::endl;
   DTSF =  Maximum_dt/dt_actual;
   oomph_info << "ADJUSTING DTSF TO " << DTSF << std::endl;
  }
 

 //Once the timestep has been accepted, return the actual timestep taken, 
 //suitably scaled, to be used the next time
 return (dt_actual*DTSF);
}



//=======================================================================
///  Unsteady "doubly" adaptive Newton solve: Does temporal
/// adaptation first, i.e. we try to do a timestep with an increment
/// of dt, and adjusting dt until the solution on the given mesh satisfies
/// the temporal error measure with tolerance epsilon. Following
/// this, we do up to max_adapt spatial adaptions (without 
/// re-examining the temporal error). If first==true, the initial conditions
/// are re-assigned after the mesh adaptations.
/// Shifting of time can be suppressed by overwriting the
/// default value of shift (true). [Shifting must be done
/// if first_timestep==true because we're constantly re-assigning
/// the initial conditions; if first_timestep==true and shift==false
/// shifting is performed anyway and a warning is issued.
//========================================================================
double Problem::doubly_adaptive_unsteady_newton_solve(const double &dt_desired,
                                                     const double &epsilon, 
                                                     const unsigned &max_adapt,
                                                     const bool &first,
                                                     const bool &shift_values)
{
 //Store the initial time
 double initial_time = time_pt()->time();

 // Take adaptive timestep, adjusting dt until tolerance is satisfied
 double new_dt=adaptive_unsteady_newton_solve(dt_desired, 
                                              epsilon,
                                              shift_values);
 double dt_taken=time_pt()->dt();
 oomph_info << "Accepted solution taken with timestep: " 
            << dt_taken << std::endl;

 // Adapt problem/mesh
 unsigned n_refined=0;
 unsigned n_unrefined=0;
 adapt(n_refined,n_unrefined); 
 
 // Re-solve the problem if the adaptation has changed anything
 if ((n_refined  !=0)||
     (n_unrefined!=0))
  {
   oomph_info << "Mesh was adapted --> we'll re-solve for current timestep." 
              << std::endl;

   // Reset time to what it was when we entered here
   // because it will be incremented again by dt_taken.
   time_pt()->time()=initial_time;

   // Shift the timesteps? No! They've been shifted already when we
   // called the solve with pure temporal adaptivity...
   bool shift=false;

   // Reset the inital condition on refined meshes
   if (first) 
    {
     //Reset the initial conditions
     oomph_info << "Re-assigning initial condition at time=" 
                << time_pt()->time()<< std::endl;
     set_initial_condition();

     // This is the first timestep so shifting
     // has to be done following the assignment of initial conditions.
     // In fact, unsteady_newton_solve(...) does that automatically.
     // We're changing the flag here to avoid warning messages.
     shift=true;
    }

   // Now take the step again on the refined mesh, using the same
   // timestep as used before.
   unsteady_newton_solve(dt_taken,max_adapt,first,shift);
  }
 else
  {
   oomph_info << "Mesh wasn't adapted --> we'll accept spatial refinement." 
              << std::endl;
  }

 return new_dt;

}




//========================================================================
/// \short Initialise the previous values of the variables for time stepping
/// corresponding to an impulsive start. Previous history for all data
/// is generated by the appropriate timesteppers. Previous nodal 
/// positions are simply copied backwards.
//========================================================================
void Problem::assign_initial_values_impulsive()
{
 //Assign the impulsive values in the "master" mesh
 Mesh_pt->assign_initial_values_impulsive();

 // Loop over global data
 unsigned Nglobal=Global_data_pt.size();
 for (unsigned iglobal=0;iglobal<Nglobal;iglobal++)
  {
   Global_data_pt[iglobal]->time_stepper_pt()->
    assign_initial_values_impulsive(Global_data_pt[iglobal]);
  }

}


//=======================================================================
///Assign the values for an impulsive start and also set the initial
///values of the previous dts to both be dt
//======================================================================
void Problem::assign_initial_values_impulsive(const double &dt)
{
 //First initialise the dts and set the weights
 initialise_dt(dt);
 //Now call assign_initial_values_impulsive
 assign_initial_values_impulsive();
}


//========================================================================
/// Shift all time-dependent data along for next timestep.
//========================================================================
void Problem::shift_time_values()
{
 //Move the values of dt in the Time object
 Time_pt->shift_dt(); 

 //Only shift time values in the "master" mesh, otherwise things will
 //get shifted twice in complex problems
 Mesh_pt->shift_time_values();

 // Shift global data with their own timesteppers
 unsigned Nglobal=Global_data_pt.size();
 for (unsigned iglobal=0;iglobal<Nglobal;iglobal++)
  {
   Global_data_pt[iglobal]->time_stepper_pt()->
    shift_time_values(Global_data_pt[iglobal]);
  }

}


//========================================================================
/// Calculate the predictions of all variables in problem
//========================================================================
void Problem::calculate_predictions()
{
 //Calculate all predictions in the "master" mesh
 Mesh_pt->calculate_predictions();

 //Calculate predictions for global data with their own timesteppers
 unsigned Nglobal=Global_data_pt.size();
 for (unsigned iglobal=0;iglobal<Nglobal;iglobal++)
  {
   Global_data_pt[iglobal]->time_stepper_pt()->
    calculate_predicted_values(Global_data_pt[iglobal]);
  }

}





//=========================================================================
/// Copy Data values, nodal positions etc from specified problem.
/// Note: This is not a copy constructor. We assume that the current
/// and the "original" problem have both been created by calling
/// the same problem constructor so that all Data objects,
/// time steppers etc. in the two problems are completely independent.
/// This function copies the nodal, internal and global values
/// and the time parameters from the original problem into "this"
/// one. This functionality is required, e.g. for
/// multigrid computations.
//=========================================================================
void Problem::copy(Problem* orig_problem_pt)
{
  
 // Copy time
 //----------

 // Flag to indicate that orig problem is unsteady problem
 bool unsteady_flag=(orig_problem_pt->time_pt()!=0);

 // Copy current time and previous time increments for proper unsteady run
 if (unsteady_flag)
  {
   oomph_info << "Copying an unsteady problem." << std::endl;
   // Current time
   this->time_pt()->time()=orig_problem_pt->time_pt()->time();
   // Timesteps
   unsigned n_dt=orig_problem_pt->time_pt()->ndt();
   time_pt()->resize(n_dt);
   for (unsigned i=0;i<n_dt;i++)
    { 
     time_pt()->dt(i)=orig_problem_pt->time_pt()->dt(i);
    }

   //Find out how many timesteppers there are
   unsigned n_time_steppers = ntime_stepper();
   
   //Loop over them all and set the weights
   for(unsigned i=0;i<n_time_steppers;i++)
    {
     time_stepper_pt(i)->set_weights();
    }


  }

 // Copy nodes
 //-----------

 // Loop over submeshes:
 unsigned nmesh=nsub_mesh();
 if (nmesh==0) nmesh=1;
 for (unsigned m=0;m<nmesh;m++)
  {
   // Find number of nodes in present mesh
   unsigned long n_node = mesh_pt(m)->nnode(); 
   
   // Check # of nodes: 
   unsigned long n_node_orig=orig_problem_pt->mesh_pt(m)->nnode();
   if (n_node!=n_node_orig)
    {
     std::ostringstream error_message;
     error_message << "Number of nodes in copy " << n_node 
                   << " not equal to the number in the original "
                   << n_node_orig << std::endl;

     throw OomphLibError(error_message.str(),
                         "Problem::copy()",
                         OOMPH_EXCEPTION_LOCATION);
    }
   
   //Loop over the nodes
   for(unsigned long i=0;i<n_node;i++)
    {     
     /// Try to cast to elastic node \todo there's got to be a better way
     /// but making Problem::mesh_pt() virtual doesn't do the right thing...
     SolidNode* el_node_pt=dynamic_cast<SolidNode*>(mesh_pt(m)->node_pt(i));
     if (el_node_pt!=0)
      {
       SolidNode* el_node_orig_pt=
        dynamic_cast<SolidNode*>(orig_problem_pt->mesh_pt(m)->node_pt(i));
       el_node_pt->copy(el_node_orig_pt);
      }
     else
      {
       mesh_pt(m)->node_pt(i)->copy(orig_problem_pt->mesh_pt(m)->node_pt(i));
      }
    }
  }


 // Copy global data:
 //------------------

 // Number of global data
 unsigned n_global=Global_data_pt.size();

 // Check # of nodes in orig problem
 unsigned long n_global_orig=orig_problem_pt->nglobal_data();
 if (n_global!=n_global_orig)
  {
   std::ostringstream error_message;
   error_message << "Number of global data in copy " << n_global
                 << " not equal to the number in the original "
                 << n_global_orig << std::endl;

   throw OomphLibError(error_message.str(),
                       "Problem::copy()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 for (unsigned iglobal=0;iglobal<n_global;iglobal++)
  {
   Global_data_pt[iglobal]->copy(orig_problem_pt->global_data_pt(iglobal));
  }


 // Copy internal data of elements:
 //--------------------------------

 // Loop over submeshes:
 for (unsigned m=0;m<nmesh;m++)
  {
   // Loop over elements and deal with internal data
   unsigned n_element=mesh_pt(m)->nelement();
   for (unsigned e=0;e<n_element;e++)
    {
     GeneralisedElement* el_pt=mesh_pt(m)->element_pt(e);
     unsigned n_internal=el_pt->ninternal_data();
     if (n_internal>0)
      {
       // Check # of internals :
       unsigned long n_internal_orig=orig_problem_pt->
        mesh_pt(m)->element_pt(e)->ninternal_data();
       if (n_internal!=n_internal_orig)
        {
         std::ostringstream error_message;
         error_message << "Number of internal data in copy " << n_internal
                       << " not equal to the number in the original "
                       << n_internal_orig << std::endl;
         
         throw OomphLibError(error_message.str(),
                             "Problem::copy()",
                             OOMPH_EXCEPTION_LOCATION);
        }
       for (unsigned i=0;i<n_internal;i++)
        {
         el_pt->internal_data_pt(i)->copy(orig_problem_pt->
                                          mesh_pt(m)->element_pt(e)->
                                          internal_data_pt(i));
        }
      }
    }
  }

}



//=========================================================================
/// Dump refinement pattern of all refineable meshes and all  generic
/// Problem data to file for restart. 
//=========================================================================
void Problem::dump(std::ofstream& dump_file)
{

 // Number of submeshes?
 unsigned n_mesh=nsub_mesh();
 
 // Single mesh:
 //------------
 if(n_mesh==0)
  {
   // Dump single mesh refinement pattern (if mesh is refineable)
   if(RefineableMeshBase* mmesh_pt = 
      dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
    { 
     mmesh_pt->dump_refinement(dump_file);
    }
  }
 
 //Multiple submeshes
 //------------------
 else
  {
   // Loop over submeshes
   for (unsigned imesh=0;imesh<n_mesh;imesh++)
    {
     // Dump single mesh refinement pattern (if mesh is refineable)
     if(RefineableMeshBase* mmesh_pt =
        dynamic_cast<RefineableMeshBase*>(mesh_pt(imesh)))
      {
       mmesh_pt->dump_refinement(dump_file);
      }
    } // End of loop over submeshes
  } 

 // Dump time
 // ---------

 // Flag to indicate unsteady run
 bool unsteady_flag=(time_pt()!=0);
 dump_file << unsteady_flag << " # bool flag for unsteady" << std::endl;

 // Current time and previous time increments for proper unsteady run
 if (unsteady_flag)
  {
   // Current time
   dump_file << time_pt()->time() << " # Time " << std::endl;
   // Timesteps
   unsigned n_dt=time_pt()->ndt();
   dump_file << n_dt << " # Number of timesteps " << std::endl;
   for (unsigned i=0;i<n_dt;i++)
    { 
     dump_file << time_pt()->dt(i) << " # dt " << std::endl;
    }
  }
 // Dummy time and previous time increments for steady run
 else
  {
   // Current time
   dump_file << "0.0 # Dummy time from steady run " << std::endl;
   // Timesteps
   dump_file << "0 # Dummy number of timesteps from steady run" << std::endl;
  }

 // Loop over submeshes and dump their data
 unsigned nmesh=nsub_mesh();
 if (nmesh==0) nmesh=1;
 for(unsigned m=0;m<nmesh;m++) {mesh_pt(m)->dump(dump_file);}

 // Dump global data

 // Loop over global data
 unsigned Nglobal=Global_data_pt.size();
 dump_file << Nglobal << " # number of global Data items " << std::endl;
 for (unsigned iglobal=0;iglobal<Nglobal;iglobal++)
  {
   Global_data_pt[iglobal]->dump(dump_file);
   dump_file << std::endl;
  }

}

//=========================================================================
/// Read refinement pattern of all refineable meshes and refine them
/// accordingly, then read all Data and nodal position info from 
/// file for restart. Return flag to indicate if the restart was from 
/// steady or unsteady solution.
//=========================================================================
void Problem::read(std::ifstream& restart_file, bool& unsteady_restart) 
{

 //Call the actions before adaptation
 actions_before_adapt();
 
 // Number of submeshes?
 unsigned n_mesh=nsub_mesh();
 
 // Single mesh:
 //------------
 if(n_mesh==0)
  {
   // Refine single mesh (if it's refineable)
   if(RefineableMeshBase* mmesh_pt =
      dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
    { 
     // When we get in here the problem has been constructed
     // by the constructor and the mesh is its original unrefined
     // form. 
     // RefineableMeshBase::refine(...) reads the refinement pattern from the
     // specified file and performs refinements until the mesh has
     // reached the same level of refinement as the mesh that existed
     // when the problem was dumped to disk.
     mmesh_pt->refine(restart_file);
    }
  }
 
 //Multiple submeshes
 //------------------
 else
  {
   // Loop over submeshes
    for (unsigned imesh=0;imesh<n_mesh;imesh++)
     {
      // Refine single mesh (if its refineable)
      if(RefineableMeshBase* mmesh_pt
         =dynamic_cast<RefineableMeshBase*>(mesh_pt(imesh)))
       {
        // When we get in here the problem has been constructed
        // by the constructor and the mesh is its original unrefined
        // form. 
        // RefineableMeshBase::refine(...) reads the refinement pattern from 
        // the specified file and performs refinements until the mesh has
        // reached the same level of refinement as the mesh that existed
        // when the problem was dumped to disk.
        mmesh_pt->refine(restart_file);
       }
     } // End of loop over submeshes

    // Rebuild the global mesh
    rebuild_global_mesh();
  } 

 //Any actions after adapt
 actions_after_adapt();
    
 // Setup equation numbering scheme
 oomph_info <<"\nNumber of equations: " << assign_eqn_numbers() 
      << std::endl<< std::endl; 

 std::string input_string;
 
 // Read time
 //----------

 // Read line up to termination sign
 getline(restart_file,input_string,'#');

 // Ignore rest of line
 restart_file.ignore(80,'\n');

 // Is the restart data from an unsteady run?
 unsteady_restart=atol(input_string.c_str());

 // Read line up to termination sign
 getline(restart_file,input_string,'#');

 // Ignore rest of line
 restart_file.ignore(80,'\n');

 // Read in initial time and set
 double time=atof(input_string.c_str());
 if (unsteady_restart) time_pt()->time()=time;
 

 // Read line up to termination sign
 getline(restart_file,input_string,'#');

 // Ignore rest of line
 restart_file.ignore(80,'\n');

 // Read & set number of timesteps
 unsigned n_dt=atoi(input_string.c_str());
 if (unsteady_restart) time_pt()->resize(n_dt);
 Vector<double> dt(n_dt);


 // Read in timesteps:
 for (unsigned i=0;i<n_dt;i++)
  {
   // Read line up to termination sign
   getline(restart_file,input_string,'#');
   
   // Ignore rest of line
   restart_file.ignore(80,'\n');
   
   // Read in initial time and set
   double prev_dt=atof(input_string.c_str());
   dt[i]=prev_dt;
  }
 
 // Initialise timestep -- also sets the weights for all timesteppers
 // in the problem.
 if (unsteady_restart) initialise_dt(dt);

 // Loop over submeshes:
 unsigned nmesh=nsub_mesh();
 if (nmesh==0) nmesh=1;
 for (unsigned m=0;m<nmesh;m++)
  {
   mesh_pt(m)->read(restart_file);
  }


 // Read global data:
 //------------------

 // Number of global data
 unsigned Nglobal=Global_data_pt.size();

 // Read line up to termination sign
 getline(restart_file,input_string,'#');

 // Ignore rest of line
 restart_file.ignore(80,'\n');

 // Check # of nodes:
 unsigned long check_nglobal=atoi(input_string.c_str());

 if (check_nglobal!=Nglobal)
  {
   std::ostringstream error_message;
   error_message << "The number of global data " << Nglobal 
                 << " is not equal to that specified in the input file " 
                 <<   check_nglobal << std::endl;

   throw OomphLibError(error_message.str(),
                       "Problem::read()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 for (unsigned iglobal=0;iglobal<Nglobal;iglobal++)
  {
   Global_data_pt[iglobal]->read(restart_file);
  }

}

//===================================================================
/// Set all timesteps to the same value, dt, and assign 
/// weights for all timesteppers in the problem.
//===================================================================
void Problem::initialise_dt(const double& dt)
{
 // Initialise the timesteps in the Problem's time object
 Time_pt->initialise_dt(dt);
 
 //Find out how many timesteppers there are
 unsigned n_time_steppers = ntime_stepper();
 
 //Loop over them all and set the weights
 for(unsigned i=0;i<n_time_steppers;i++)
    {
     time_stepper_pt(i)->set_weights();
    }
}

//=========================================================================
/// Set the value of the timesteps to be equal to the values passed in 
/// a vector and assign weights for all timesteppers in the problem
//========================================================================
void Problem::initialise_dt(const Vector<double>& dt)
{
 // Initialise the timesteps in the Problem's time object
 Time_pt->initialise_dt(dt);
 
 //Find out how many timesteppers there are
 unsigned n_time_steppers = ntime_stepper();
 
 //Loop over them all and set the weights
 for(unsigned i=0;i<n_time_steppers;i++)
  {
   time_stepper_pt(i)->set_weights();
  }
}

//========================================================
/// Self-test: Check meshes and global data. Return 0 for OK
//========================================================
unsigned Problem::self_test()
{ 
 // Initialise
 bool passed=true;

 // Are there any submeshes?
 unsigned Nmesh=nsub_mesh();

 // Just one mesh: Check it
 if (Nmesh==0)
  {
   if (mesh_pt()->self_test()!=0)
    {
     passed=false;
     oomph_info 
      << "\n ERROR: Failed Mesh::self_test() for single mesh in problem" 
      << std::endl;
    }
  }
 // Loop over all submeshes and check them
 else
  {
   for (unsigned imesh=0;imesh<Nmesh;imesh++)
    {
     if (mesh_pt(imesh)->self_test()!=0)
      {
       passed=false;
       oomph_info << "\n ERROR: Failed Mesh::self_test() for mesh imesh" 
                 << imesh  << std::endl;
      }
    }
  }
  

 // Check global data
 unsigned Nglobal=Global_data_pt.size();
 for (unsigned iglobal=0;iglobal<Nglobal;iglobal++)
  {
   if (Global_data_pt[iglobal]->self_test()!=0)
    {
     passed=false;
     oomph_info << "\n ERROR: Failed Data::self_test() for global data iglobal" 
          << iglobal << std::endl;
    }
  }


#ifdef OOMPH_HAS_MPI

 if (Problem_has_been_distributed)
  {
   // Note: This throws an error if it fails so no return is required.
   DocInfo tmp_doc_info;
   tmp_doc_info.doc_flag()=false;
   check_halo_schemes(tmp_doc_info);
  }

#endif

 // Return verdict
 if (passed) {return 0;}
 else {return 1;}

}

//========================================================================
/// Adapt problem:
/// Perform mesh adaptation for (all) refineable (sub)mesh(es),
/// based on their own error estimates and the target errors specified
/// in the mesh(es). Following mesh adaptation,
/// update global mesh, and re-assign equation numbers. 
/// Return # of refined/unrefined elements. On return from this
/// function, Problem can immediately be solved again.
//======================================================================
void Problem::adapt(unsigned &n_refined, unsigned &n_unrefined)
{
 oomph_info << std::endl << std::endl;
 oomph_info << "Adapting problem:" << std::endl;
 oomph_info << "=================" << std::endl;

 //Call the actions before adaptation
 actions_before_adapt();

 // Initialise counters
 n_refined=0;
 n_unrefined=0;
 
 // Number of submeshes?
 unsigned Nmesh=nsub_mesh();
 
 // Single mesh:
 //------------
 if(Nmesh==0)
  {
   // Refine single mesh uniformly if possible
   if(RefineableMeshBase* mmesh_pt = 
      dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
    { 
     if (mmesh_pt->adapt_flag())
      {
       // Get pointer to error estimator
       ErrorEstimator* error_estimator_pt=mmesh_pt->
        spatial_error_estimator_pt();
       
#ifdef PARANOID
       if (error_estimator_pt==0)
        {
         throw OomphLibError(
          "Error estimator hasn't been set yet",
          "Problem::adapt()",
          OOMPH_EXCEPTION_LOCATION);
        }
#endif

       // Get error for all elements
       Vector<double> elemental_error(mmesh_pt->nelement());
       
       if (mmesh_pt->doc_info_pt()==0)
        {
         error_estimator_pt->get_element_errors(mesh_pt(0),elemental_error);
        }
       else
        {
         error_estimator_pt->get_element_errors(mesh_pt(0),elemental_error,
                                                *mmesh_pt->doc_info_pt());
        }
       
        // Store max./min actual error
       mmesh_pt->max_error()=
        std::abs(*std::max_element(elemental_error.begin(),
                                   elemental_error.end(),AbsCmp<double>()));
       
       mmesh_pt->min_error()=
        std::abs(*std::min_element(elemental_error.begin(),
                                   elemental_error.end(),AbsCmp<double>()));

       oomph_info << "\n Max/min error: " 
            << mmesh_pt->max_error() << " "
            << mmesh_pt->min_error() << std::endl;
       
       // Adapt mesh
        mmesh_pt->adapt(elemental_error);
        
        // Add to counters
        n_refined+=mmesh_pt->nrefined();
        n_unrefined+=mmesh_pt->nunrefined();

      }
     else
      {
       oomph_info << "Info/Warning: Mesh adaptation is disabled." << std::endl;
      }
    }
   else
    {
     oomph_info << "Info/Warning: Mesh cannot be adapted" << std::endl;
    }

  }

 //Multiple submeshes
 //------------------
 else
  {
   // Loop over submeshes
    for (unsigned imesh=0;imesh<Nmesh;imesh++)
     {
      // Refine single mesh uniformly if possible
      if(RefineableMeshBase* mmesh_pt =
         dynamic_cast<RefineableMeshBase*>(mesh_pt(imesh)))
       {
        // Get pointer to error estimator
        ErrorEstimator* error_estimator_pt=mmesh_pt->
        spatial_error_estimator_pt();
        
#ifdef PARANOID
        if (error_estimator_pt==0)
         {
          throw OomphLibError(
           "Error estimator hasn't been set yet",
           "Problem::adapt()",
           OOMPH_EXCEPTION_LOCATION);
         }
#endif
        
        if (mmesh_pt->adapt_flag())
         {
          // Get error for all elements
         Vector<double> elemental_error(mmesh_pt->nelement());
         if (mmesh_pt->doc_info_pt()==0)
          {
           error_estimator_pt->get_element_errors(mesh_pt(imesh),
                                                   elemental_error);
          }
         else
          {
            error_estimator_pt->get_element_errors(mesh_pt(imesh),
                                                   elemental_error,
                                                   *mmesh_pt->doc_info_pt());
          }
        
         // Store max./min error
         mmesh_pt->max_error()=
          std::abs(*std::max_element(elemental_error.begin(),
                                     elemental_error.end(),AbsCmp<double>()));
          
         mmesh_pt->min_error()=
          std::abs(*std::min_element(elemental_error.begin(),
                                     elemental_error.end(),AbsCmp<double>()));

         oomph_info << "\n Max/min error: " 
              << mmesh_pt->max_error() << " "
              << mmesh_pt->min_error() << std::endl;

         // Adapt mesh
         mmesh_pt->adapt(elemental_error); 
  
         // Add to counters
         n_refined+=mmesh_pt->nrefined();
         n_unrefined+=mmesh_pt->nunrefined();

         }
        else
         {
          oomph_info << "Info/Warning: Mesh adaptation is disabled." 
                     << std::endl;
         }
       }
      else
       {
        oomph_info << "Info/Warning: Mesh cannot be adapted." << std::endl;
       }
      
     } // End of loop over submeshes

    // Rebuild the global mesh
    rebuild_global_mesh();

  } 

 //Any actions after adapt
 actions_after_adapt();

 //Attach the boundary conditions to the mesh
 oomph_info <<"\nNumber of equations: " << assign_eqn_numbers() 
      << std::endl<< std::endl; 

}

//========================================================================
/// \short Get max and min error for all elements in submeshes
//========================================================================
void Problem::doc_errors(DocInfo& doc_info)
{
 
 // Number of submeshes?
 unsigned Nmesh=nsub_mesh();
 
 // Single mesh:
 //------------
 if (Nmesh==0)
  {
   // Refine single mesh uniformly if possible
   if (RefineableMeshBase* mmesh_pt =
       dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
    { 
     
     // Get pointer to error estimator
     ErrorEstimator* error_estimator_pt=mmesh_pt->
      spatial_error_estimator_pt();

#ifdef PARANOID
     if (error_estimator_pt==0)
      {
       throw OomphLibError(
        "Error estimator hasn't been set yet",
        "Problem::doc_errors()",
        OOMPH_EXCEPTION_LOCATION);
      }
#endif

     // Get error for all elements
     Vector<double> elemental_error(mmesh_pt->nelement());
     if (!doc_info.doc_flag())
      {
       error_estimator_pt->get_element_errors(mesh_pt(0),
                                              elemental_error);
      }
     else
      {
       error_estimator_pt->get_element_errors(mesh_pt(0),
                                              elemental_error,
                                              doc_info);
      }

     // Store max./min actual error
     mmesh_pt->max_error()=
      std::abs(*std::max_element(elemental_error.begin(),
                                 elemental_error.end(),AbsCmp<double>()));
      
     mmesh_pt->min_error()=
      std::abs(*std::min_element(elemental_error.begin(),
                                 elemental_error.end(),AbsCmp<double>()));
      
     oomph_info << "\n Max/min error: " 
          << mmesh_pt->max_error() << " "
          << mmesh_pt->min_error() << std::endl;

    }
  }
  
 //Multiple submeshes
 //------------------
 else
  {
   // Loop over submeshes
   for (unsigned imesh=0;imesh<Nmesh;imesh++)
    {

     // Refine single mesh uniformly if possible
     if (RefineableMeshBase* mmesh_pt=
         dynamic_cast<RefineableMeshBase*>(mesh_pt(imesh)))
      { 

       // Get pointer to error estimator
       ErrorEstimator* error_estimator_pt=mmesh_pt->
        spatial_error_estimator_pt();

#ifdef PARANOID
       if (error_estimator_pt==0)
        {
         throw OomphLibError(
          "Error estimator hasn't been set yet",
          "Problem::doc_errors()",
          OOMPH_EXCEPTION_LOCATION);
        }
#endif

       // Get error for all elements
       Vector<double> elemental_error(mmesh_pt->nelement());
       if (mmesh_pt->doc_info_pt()==0)
        {
         error_estimator_pt->get_element_errors(mesh_pt(imesh),
                                                elemental_error);
        }
       else
        {
         error_estimator_pt->get_element_errors(mesh_pt(imesh),
                                                elemental_error,
                                                *mmesh_pt->doc_info_pt());
        }
        
       // Store max./min error
       mmesh_pt->max_error()=
        std::abs(*std::max_element(elemental_error.begin(),
                                   elemental_error.end(),AbsCmp<double>()));
        
       mmesh_pt->min_error()=
        std::abs(*std::min_element(elemental_error.begin(),
                                   elemental_error.end(),AbsCmp<double>()));
        
       oomph_info << "\n Max/min error: " 
            << mmesh_pt->max_error() << " "
            << mmesh_pt->min_error() << std::endl;
      }
      
    } // End of loop over submeshes

  } 

}

//========================================================================
/// Refine (one and only!) mesh by splitting the elements identified
/// by their numbers relative to the problems' only mesh, then rebuild 
/// the problem. [Can't see how/why one would want to do this for multiple
/// meshes -- if you need this functionality implement it yourself;
/// you'll probably need to pass a Vector of refinement Vectors...].
//========================================================================
void Problem::refine_selected_elements(const Vector<unsigned>& 
                                       elements_to_be_refined)
 {
  actions_before_adapt();
 
  // Number of submeshes?
  unsigned Nmesh=nsub_mesh();

  // Single mesh:
  if (Nmesh==0)
   {
    // Refine single mesh if possible
    if(RefineableMeshBase* mmesh_pt = 
       dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
     {
      mmesh_pt->refine_selected_elements(elements_to_be_refined);
     }
    else
     {
      oomph_info << "Info/Warning: Mesh cannot be refined " 
                 << std::endl;
     }
   }
  //Multiple submeshes
  else
   {
    std::string error_message = 
     "Problem::refine_selected_elements(...) only works for\n";
    error_message += "single-mesh problems at the moment.\n";
    
    throw OomphLibError(error_message,"Problem::refine_selected_elements()",
                        OOMPH_EXCEPTION_LOCATION);
   }

  //Any actions after the adapatation phase
  actions_after_adapt();

  //Attach the boundary conditions to the mesh
  oomph_info <<"Number of equations: " 
            << assign_eqn_numbers() << std::endl; 

 }




//========================================================================
/// Refine (one and only!) mesh by splitting the elements identified
/// by their pointers, then rebuild 
/// the problem. [Can't see how/why one would want to do this for multiple
/// meshes -- if you need this functionality implement it yourself;
/// you'll probably need to pass a Vector of refinement Vectors...].
//========================================================================
void Problem::refine_selected_elements(const Vector<RefineableElement*>& 
                                       elements_to_be_refined_pt)
 {
  actions_before_adapt();
 
  // Number of submeshes?
  unsigned Nmesh=nsub_mesh();

  // Single mesh:
  if (Nmesh==0)
   {
    // Refine single mesh if possible
    if(RefineableMeshBase* mmesh_pt = 
       dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
     {
      mmesh_pt->refine_selected_elements(elements_to_be_refined_pt);
     }
    else
     {
      oomph_info << "Info/Warning: Mesh cannot be refined " 
                 << std::endl;
     }
   }
  //Multiple submeshes
  else
   {
    std::string error_message = 
     "Problem::refine_selected_elements(...) only works for\n";
    error_message += "single-mesh problems at the moment.\n";
    
    throw OomphLibError(error_message,"Problem::refine_selected_elements()",
                        OOMPH_EXCEPTION_LOCATION);
   }

  //Any actions after the adapatation phase
  actions_after_adapt();

  //Do equation numbering
  oomph_info <<"Number of equations: " << assign_eqn_numbers()
            << std::endl; 

 }




//========================================================================
/// Refine specified submesh by splitting the elements identified
/// by their pointers, then rebuild the problem. 
//========================================================================
void Problem::refine_selected_elements(const unsigned& i_mesh,
                                       const Vector<RefineableElement*>& 
                                       elements_to_be_refined_pt)
 {
  actions_before_adapt();
 
  // Number of submeshes?
  unsigned n_mesh=nsub_mesh();

  if (i_mesh>=n_mesh)
   {
    std::ostringstream error_message;
    error_message <<
     "Problem only has " << n_mesh << " submeshes. Cannot refine submesh " 
                         << i_mesh << std::endl;
    throw OomphLibError(error_message.str(),
                        "Problem::refine_selected_elements()",
                        OOMPH_EXCEPTION_LOCATION);   
   }

  // Refine single mesh if possible
  if(RefineableMeshBase* mmesh_pt = 
     dynamic_cast<RefineableMeshBase*>(mesh_pt(i_mesh)))
   {
    mmesh_pt->refine_selected_elements(elements_to_be_refined_pt);
   }
  else
   {
    oomph_info << "Info/Warning: Mesh cannot be refined " 
               << std::endl;
   }

  if (n_mesh>1)
   {
    //Rebuild the global mesh
    rebuild_global_mesh();
   }

  //Any actions after the adapatation phase
  actions_after_adapt();

  //Do equation numbering
  oomph_info <<"Number of equations: " << assign_eqn_numbers()
            << std::endl; 

 }






//========================================================================
/// Refine (all) refineable (sub)mesh(es) uniformly and rebuild problem
/// and doc refinement process.
//========================================================================
void Problem::refine_uniformly(DocInfo& doc_info)
 {
  actions_before_adapt();

  // Number of submeshes?
  unsigned Nmesh=nsub_mesh();
  
  // Single mesh:
  if (Nmesh==0)
   {
    // Refine single mesh uniformly if possible
    if(RefineableMeshBase* mmesh_pt = 
       dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
     {
      mmesh_pt->refine_uniformly(doc_info);
     }
    else
     {
      oomph_info << "Info/Warning: Mesh cannot be refined uniformly " 
                 << std::endl;
     }
   }
  //Multiple submeshes
  else
   {
    // Loop over submeshes
    for (unsigned imesh=0;imesh<Nmesh;imesh++)
     {
      // Refine i-th submesh uniformly if possible
      if (RefineableMeshBase* mmesh_pt =
          dynamic_cast<RefineableMeshBase*>(mesh_pt(imesh)))
       {
        mmesh_pt->refine_uniformly(doc_info);
       }
      else
       {
        oomph_info << "Info/Warning: Cannot refine mesh " << imesh 
                   << std::endl;
       } 
     }
    //Rebuild the global mesh
    rebuild_global_mesh();
   }

  //Any actions after the adaptation phase
  actions_after_adapt();

  //Do equation numbering
  oomph_info <<"Number of equations: " 
            << assign_eqn_numbers() << std::endl; 

 }

//========================================================================
/// Refine submesh i_mesh uniformly and rebuild problem;
/// doc refinement process.
//========================================================================
void Problem::refine_uniformly(const unsigned& i_mesh, 
                               DocInfo& doc_info)
{
 actions_before_adapt();
 
#ifdef PARANOID
 // Number of submeshes?
  if (i_mesh>=nsub_mesh())
   {
    std::ostringstream error_message;
    error_message  << "imesh " << i_mesh 
                   << " is greater than the number of sub meshes " 
                   << nsub_mesh() << std::endl;
 
    throw OomphLibError(error_message.str(),
                        "Problem::refine_uniformly()",
                        OOMPH_EXCEPTION_LOCATION);
   }
#endif

  // Refine single mesh uniformly if possible
  if(RefineableMeshBase* mmesh_pt = 
     dynamic_cast<RefineableMeshBase*>(mesh_pt(i_mesh)))
   {
    mmesh_pt->refine_uniformly(doc_info);
   }
  else
   {
    oomph_info << "Info/Warning: Mesh cannot be refined uniformly " 
               << std::endl;
   }

  //Rebuild the global mesh
  rebuild_global_mesh();

  //Any actions after the adaptation phase
  actions_after_adapt();

  //Do equation numbering
  oomph_info <<"Number of equations: " 
            << assign_eqn_numbers() << std::endl; 

 }
 

//========================================================================
/// Unrefine (all) refineable (sub)mesh(es) uniformly and rebuild problem.
/// Return 0 for success,
/// 1 for failure (if unrefinement has reached the coarsest permitted
/// level)
//========================================================================
unsigned Problem::unrefine_uniformly()
 {
  actions_before_adapt();

  // Has unrefinement been successful?
  unsigned success_flag=0;

  // Number of submeshes?
  unsigned Nmesh=nsub_mesh();

  // Single mesh:
  if (Nmesh==0)
   {
    // Unrefine single mesh uniformly if possible
    if(RefineableMeshBase* mmesh_pt = 
       dynamic_cast<RefineableMeshBase*>(mesh_pt(0)))
     {
      success_flag+=mmesh_pt->unrefine_uniformly();
     }
    else
     {
      oomph_info << "Info/Warning: Mesh cannot be unrefined uniformly " 
                 << std::endl;
     }
   }
  //Multiple submeshes
  else
   {
    // Loop over submeshes
    for (unsigned imesh=0;imesh<Nmesh;imesh++)
     {
      // Unrefine i-th submesh uniformly if possible
      if (RefineableMeshBase* mmesh_pt=
          dynamic_cast<RefineableMeshBase*>(mesh_pt(imesh)))
       {
        success_flag+=mmesh_pt->unrefine_uniformly();
       }
      else
       {
        oomph_info << "Info/Warning: Cannot unrefine mesh " << imesh 
                   << std::endl;
       } 
     }
    //Rebuild the global mesh
    rebuild_global_mesh();
   }

  //Any actions after the adaptation phase
  actions_after_adapt();

  //Do equation numbering
  oomph_info <<"Number of equations: " 
            << assign_eqn_numbers() << std::endl; 

  // Judge success
  if (success_flag>0)
   {
    return 1;
   }
  else
   {
    return 0;
   }

 }

//========================================================================
/// Unrefine submesh i_mesh uniformly and rebuild problem.
/// Return 0 for success,
/// 1 for failure (if unrefinement has reached the coarsest permitted
/// level)
//========================================================================
unsigned Problem::unrefine_uniformly(const unsigned& i_mesh)
 {
  actions_before_adapt();

  // Has unrefinement been successful?
  unsigned success_flag=0;

#ifdef PARANOID
  // Number of submeshes?
  if (i_mesh>=nsub_mesh())
   {
    std::ostringstream error_message;
    error_message  << "imesh " << i_mesh 
                   << " is greater than the number of sub meshes " 
                   << nsub_mesh() << std::endl;
 
    throw OomphLibError(error_message.str(),
                        "Problem::unrefine_uniformly()",
                        OOMPH_EXCEPTION_LOCATION);
   }
#endif

  // Unrefine single mesh uniformly if possible
  if(RefineableMeshBase* mmesh_pt = 
     dynamic_cast<RefineableMeshBase*>(mesh_pt(i_mesh)))
   {
    success_flag+=mmesh_pt->unrefine_uniformly();
   }
  else
   {
    oomph_info << "Info/Warning: Mesh cannot be unrefined uniformly " 
               << std::endl;
   }

  //Rebuild the global mesh
  rebuild_global_mesh();

  //Any actions after the adaptation phase
  actions_after_adapt();

  //Do equation numbering
  oomph_info <<"Number of equations: " 
            << assign_eqn_numbers() << std::endl; 

  // Judge success
  if (success_flag>0)
   {
    return 1;
   }
  else
   {
    return 0;
   }

 }


//========================================================================
/// Do one timestep, dt, forward  using Newton's method with specified 
/// tolerance and linear solver specified via member data.
/// Keep adapting on all meshes to criteria specified in
/// these meshes (up to max_adapt adaptations are performed).
/// If first_timestep==true, re-set initial conditions after mesh adaptation.
/// Shifting of time can be suppressed by overwriting the
/// default value of shift (true). [Shifting must be done
/// if first_timestep==true because we're constantly re-assigning
/// the initial conditions; if first_timestep==true and shift==false
/// shifting is performed anyway and a warning is issued.
//========================================================================
void Problem::unsteady_newton_solve(const double &dt, 
                                    const unsigned &max_adapt, 
                                    const bool &first_timestep,
                                    const bool& shift)
{
 

 // Do shifting or not?
 bool shift_it=shift;

 // Warning:
 if (first_timestep&&(!shift))
  {
   shift_it=true;
   oomph_info 
    << "\n\n===========================================================\n";
   oomph_info << "                  ********  WARNING *********** \n";
   oomph_info 
    << "===========================================================\n";
   oomph_info << "Problem::unsteady_newton_solve() called with " << std::endl;
   oomph_info << "first_timestep: " << first_timestep << std::endl;
   oomph_info << "shift: " << shift << std::endl;
   oomph_info << "This doesn't make sense (shifting does have to be done" 
             << std::endl;
   oomph_info 
    << "since we're constantly re-assigning the initial conditions"
    << std::endl;
   oomph_info 
    << "\n===========================================================\n\n";
  }


 //Find the initial time
 double initial_time = time_pt()->time();

 // Max number of solves
 unsigned max_solve=max_adapt+1;
 
 // Adaptation loop
 //----------------
 for (unsigned isolve=0;isolve<max_solve;isolve++)
  {
   // Only adapt after the first solve has been done!
   if (isolve>0)
    {
     unsigned n_refined;
     unsigned n_unrefined;
     
     // Adapt problem 
     adapt(n_refined,n_unrefined);

#ifdef OOMPH_HAS_MPI
       // Adaptation only converges if ALL the processes have no
       // refinement or unrefinement to perform
       unsigned total_refined=0;
       unsigned total_unrefined=0;
       if (mesh_pt()->mesh_has_been_distributed())
        {
         MPI_Allreduce(&n_refined,&total_refined,1,MPI_INT,MPI_SUM,
                       MPI_COMM_WORLD);
         n_refined=total_refined;
         MPI_Allreduce(&n_unrefined,&total_unrefined,1,MPI_INT,MPI_SUM,
                       MPI_COMM_WORLD);
         n_unrefined=total_unrefined;
        }
#endif

       oomph_info << "---> " << n_refined << " elements to be refined, and " 
                  << n_unrefined << " to be unrefined, in total." << std::endl;
     
     // Check convergence of adaptation cycle
     if ((n_refined==0)&&(n_unrefined==0))
      {
       oomph_info << "\n \n Solution is fully converged in "
            << "Problem::unsteady_newton_solver() \n \n ";
       break;
      }       

     //Reset the time
     time_pt()->time() = initial_time;

     // Reset the inital condition on refined meshes
     if (first_timestep) 
      {
       oomph_info << "Re-setting initial condition " << std::endl;
       set_initial_condition();
      }
    }

   //Now do the actual unsteady timestep
   //If it's the first time around the loop, or the first timestep 
   //shift the timevalues, otherwise don't
   // Note: we need to shift if it's the first timestep because
   // we're constantly re-assigning the initial condition above!
   if((isolve==0) || (first_timestep))
    {
     Problem::unsteady_newton_solve(dt,shift_it);
    }
   // Subsequent solve: Have shifted already -- don't do it again.
   else
    {
     shift_it=false;
     Problem::unsteady_newton_solve(dt,shift_it);
    }

   if (isolve==max_solve-1)
    {
     oomph_info << std::endl 
          << "----------------------------------------------------------" 
          << std::endl
          << "Reached max. number of adaptations in \n"
          << "Problem::unsteady_newton_solver().\n"
          << "Accepting current soln with errors:" ;
     //Call actions_before_adapt to handle case of mixed elemental mesh
     actions_before_adapt();
     doc_errors();
     //Complete the build of the problem 
     //[given that it may have been broken by actions_before_adapt()]
     actions_after_adapt();
     assign_eqn_numbers();
     
     oomph_info << std::endl
          << "----------------------------------------------------------" 
          << std::endl
          << std::endl;
    }
   
  } // End of adaptation loop


}



//========================================================================
/// \short Adaptive Newton solver. 
/// The linear solver takes a pointer to the problem (which defines
/// the Jacobian \b J and the residual Vector \b r) and returns 
/// the solution \b x of the system 
/// \f[ {\bf J} {\bf x} = - \bf{r} \f]. 
/// Performs at most max_adapt adaptations on all meshes. 
//========================================================================
void Problem::newton_solve(const unsigned &max_adapt)
{
 // Max number of solves
 unsigned max_solve=max_adapt+1;

 // Adaptation loop
 //----------------
 for (unsigned isolve=0;isolve<max_solve;isolve++)
  {

     // Only adapt after the first solve has been done!
     if (isolve>0)
      {

       unsigned n_refined;
       unsigned n_unrefined;
       
       // Adapt problem 
       adapt(n_refined,n_unrefined);
       
#ifdef OOMPH_HAS_MPI
       // Adaptation only converges if ALL the processes have no
       // refinement or unrefinement to perform
       unsigned total_refined=0;
       unsigned total_unrefined=0;
       if (mesh_pt()->mesh_has_been_distributed())
        {
         MPI_Allreduce(&n_refined,&total_refined,1,MPI_INT,MPI_SUM,
                       MPI_COMM_WORLD);
         n_refined=total_refined;
         MPI_Allreduce(&n_unrefined,&total_unrefined,1,MPI_INT,MPI_SUM,
                       MPI_COMM_WORLD);
         n_unrefined=total_unrefined;
        }
#endif

       oomph_info << "---> " << n_refined << " elements to be refined, and " 
                  << n_unrefined << " to be unrefined, in total." << std::endl;

       // Check convergence of adaptation cycle
       if ((n_refined==0)&&(n_unrefined==0))
        {
         oomph_info << "\n \n Solution is fully converged in "
              << "Problem::newton_solver(). \n \n ";
         break;
        }       
      }


     // Do actual solve
     //----------------
     {
      
      //Now update anything that needs updating
      // NOT NEEDED -- IS CALLED IN newton_solve BELOW! #
      // actions_before_newton_solve();
      
      try
       {
        //Solve the non-linear problem for this timestep with Newton's method
        Problem::newton_solve();
       }
      //Catch any exceptions thrown in the Newton solver
      catch(NewtonSolverError &error)
       {
        oomph_info << std::endl
                  << "USER-DEFINED ERROR IN NEWTON SOLVER " << std::endl;
        //Check to see whether we have reached Max_iterations
        if(error.iterations==Max_newton_iterations)
         {
          oomph_info << "MAXIMUM NUMBER OF ITERATIONS (" 
                    << error.iterations 
                    << ") REACHED WITHOUT CONVERGENCE " << std::endl;
         }
        //If not, it must be that we have exceeded the maximum residuals
        else
         {
          oomph_info << "MAXIMUM RESIDUALS: " << error.maxres
                    <<"EXCEEDS PREDEFINED MAXIMUM " 
                    << Max_residuals
                    << std::endl;
         }

        //Die horribly!!
        std::ostringstream error_stream;
        error_stream << "Error occured in adaptive Newton solver. "
                     << std::endl;
        throw OomphLibError(error_stream.str(),
                            "Problem::newton_solve()",
                            OOMPH_EXCEPTION_LOCATION);
       }

      //Now update anything that needs updating
      // NOT NEEDED -- WAS CALLED IN newton_solve ABOVE
      // !actions_after_newton_solve();
      
     } //End of solve block


     if (isolve==max_solve-1)
      {
       oomph_info 
        << std::endl 
        << "----------------------------------------------------------" 
        << std::endl
        << "Reached max. number of adaptations in \n"
        << "Problem::newton_solver().\n"
        << "Accepting current soln with errors:" ;
       
       //Call actions_before_adapt to handle case of mixed elemental mesh
       actions_before_adapt();
       doc_errors(); 
       //Complete the build of the problem 
       //[given that it may have been broken by actions_before_adapt()]
       actions_after_adapt();
       assign_eqn_numbers();
       
       oomph_info 
        << std::endl
        << "----------------------------------------------------------" 
        << std::endl << std::endl;
      }

    } // End of adaptation loop


}


#ifdef OOMPH_HAS_MPI

//========================================================================
/// Check the halo/haloed/shared node/element schemes.
//========================================================================
void Problem::check_halo_schemes(DocInfo& doc_info)
{ 
 MPI_Status status;

 char filename[100];
 std::ofstream halo_file;
 std::ofstream haloed_file;
 std::ofstream shared_file;

 // Doc halo/haoloed element lookup schemes
 //-----------------------------------------
 if (doc_info.doc_flag())
  {
   // Loop over domains for halo elements
   for (int dd=0;dd<MPI_Helpers::Nproc;dd++)
    {
     sprintf(filename,"%s/halo_element_check%i_%i.dat",
             doc_info.directory().c_str(),MPI_Helpers::My_rank,dd);
     halo_file.open(filename);
     
     // Get vectors of halo/haloed elements by copy operation
     Vector<FiniteElement*> 
      halo_elem_pt(mesh_pt()->halo_element_pt(dd));
     
     unsigned nelem=halo_elem_pt.size();

     for (unsigned e=0;e<nelem;e++)
      {
       halo_file << "ZONE " << std::endl;
       unsigned nnod=halo_elem_pt[e]->nnode();
       for (unsigned j=0;j<nnod;j++)
        {
         Node* nod_pt=halo_elem_pt[e]->node_pt(j);
         unsigned ndim=nod_pt->ndim();
         for (unsigned i=0;i<ndim;i++)
          {
           halo_file << nod_pt->position(i) << " ";
          }
         halo_file << std::endl;
        }
      }
     halo_file.close(); 
    }
   
   
   
   // Loop over domains for halo elements
   for (int d=0;d<MPI_Helpers::Nproc;d++)
    {
     sprintf(filename,"%s/haloed_element_check%i_%i.dat",
             doc_info.directory().c_str(),d,MPI_Helpers::My_rank);
     haloed_file.open(filename);
     
     // Get vectors of halo/haloed elements by copy operation
     Vector<FiniteElement*> 
      haloed_elem_pt(mesh_pt()->haloed_element_pt(d));
     
     unsigned nelem2=haloed_elem_pt.size(); 
     for (unsigned e=0;e<nelem2;e++)
      {
       haloed_file << "ZONE " << std::endl;
       unsigned nnod2=haloed_elem_pt[e]->nnode();
       for (unsigned j=0;j<nnod2;j++)
        {
         Node* nod_pt=haloed_elem_pt[e]->node_pt(j);
         unsigned ndim=nod_pt->ndim();
         for (unsigned i=0;i<ndim;i++)
          {
           haloed_file << nod_pt->position(i) << " ";
          }
         haloed_file << std::endl;
        }
      }
     haloed_file.close(); 
    }
  }

 // Check halo/haloed element lookup schemes
 //-----------------------------------------
 double max_error=0.0;

 // Loop over domains for haloed elements
 for (int d=0;d<MPI_Helpers::Nproc;d++)
  {
   // Are my haloed elements being checked?
   if (d==MPI_Helpers::My_rank)
    {
     // Loop over domains for halo elements
     for (int dd=0;dd<MPI_Helpers::Nproc;dd++)
      {
       // Don't talk to yourself
       if (dd!=d)
        {
         // Get vectors of haloed elements by copy operation
         Vector<FiniteElement*> 
          haloed_elem_pt(mesh_pt()->haloed_element_pt(dd));
         
         // How many of my elements are haloed elements whose halo
         // counterpart is located on processor dd?
         int nelem_haloed=haloed_elem_pt.size();

         // Receive from processor dd how many of his elements are halo
         // nodes whose non-halo counterparts are located here
         int nelem_halo=0;
         MPI_Recv(&nelem_halo,1, MPI_INT,dd,
                  0,MPI_COMM_WORLD,&status);
         if (nelem_halo!=nelem_haloed)
          {
           std::ostringstream error_message;
           error_message 
            << "Clash in numbers of halo and haloed elements! " 
            << std::endl;           
           error_message 
            << "# of haloed elements whose halo counterpart lives on proc "
            << dd << ": " << nelem_haloed << std::endl;
           error_message
            << "# of halo elements whose non-halo counterpart lives on proc "
            << d << ": " << nelem_halo << std::endl;
           error_message 
            << "(Re-)run Problem::check_halo_schemes() with DocInfo object"
            << std::endl;
           error_message 
            << "to identify the problem" << std::endl;
           throw OomphLibError(error_message.str(),
                               "Problem::check_halo_schemes()",
                               OOMPH_EXCEPTION_LOCATION);
          }


         // Get strung-together elemental nodal positions from other processor
         unsigned nnod_per_el=mesh_pt()->
          finite_element_pt(0)->nnode();
         unsigned nod_dim=mesh_pt()->
          finite_element_pt(0)->node_pt(0)->ndim();
         Vector<double> other_nodal_positions(nod_dim*nnod_per_el*nelem_halo);
         Vector<int> other_nodal_hangings(nnod_per_el*nelem_halo);
         MPI_Recv(&other_nodal_positions[0],nod_dim*nnod_per_el*nelem_halo,
                  MPI_DOUBLE,dd,0,MPI_COMM_WORLD,&status);
         MPI_Recv(&other_nodal_hangings[0],nnod_per_el*nelem_halo,MPI_INT,dd,1,
                  MPI_COMM_WORLD,&status);

//         oomph_info << "Received from process " << dd 
//              << ", with size=" << nod_dim*nnod_per_el*nelem_halo << std::endl;
         
         sprintf(filename,"%s/error_haloed_check%i_%i.dat",
          doc_info.directory().c_str(),dd,MPI_Helpers::My_rank);
         haloed_file.open(filename);
         sprintf(filename,"%s/error_halo_check%i_%i.dat",
          doc_info.directory().c_str(),dd,MPI_Helpers::My_rank);
         halo_file.open(filename);
     
         unsigned count=0;         
         unsigned count_hanging=0;
         for (int e=0;e<nelem_haloed;e++)
          {   
           for (unsigned j=0;j<nnod_per_el;j++)
            {
             // andy, testing POSITIONS, not x location 
             // (cf hanging nodes, nodes.h)
             double x_haloed=haloed_elem_pt[e]->node_pt(j)->position(0);
             double y_haloed=haloed_elem_pt[e]->node_pt(j)->position(1);
             double z_haloed=0.0;
             if (nod_dim==3)
              {
               z_haloed=haloed_elem_pt[e]->node_pt(j)->position(2);
              }
             double x_halo=other_nodal_positions[count];
             count++;
             double y_halo=other_nodal_positions[count];
             count++;
             int other_hanging=other_nodal_hangings[count_hanging];
             count_hanging++;
             double z_halo=0.0;
             if (nod_dim==3)
              {
               z_halo=other_nodal_positions[count];
               count++;
              }
             double error=sqrt( pow(x_haloed-x_halo,2)+ 
                                pow(y_haloed-y_halo,2)+
                                pow(z_haloed-z_halo,2));
             if (fabs(error)>max_error) max_error=fabs(error);
             if (fabs(error)>0.0)
              {
               // Report error. NOTE: ERROR IS THROWN BELOW ONCE 
               // ALL THIS HAS BEEN PROCESSED.
               oomph_info
                << "Discrepancy between nodal coordinates of halo(ed) element."
                << " Error: " << error << std::endl;
               oomph_info
                << "Domain with non-halo (i.e. haloed) elem: " 
                << dd << std::endl;
               oomph_info
                << "Domain with    halo                elem: " << d
                << std::endl;
               oomph_info
                << "Current processor is " << MPI_Helpers::My_rank << std::endl
                << "Nodal positions: " << x_halo << " " << y_halo << std::endl
                << "and haloed: " << x_haloed << " " << y_haloed << std::endl
                << "Node pointer: " << haloed_elem_pt[e]->node_pt(j)
                << std::endl;
//               oomph_info << "Haloed: " << x_haloed << " " << y_haloed << " "
//                          << error << " " << MPI_Helpers::My_rank << " "
//                          << dd << std::endl;
//               oomph_info << "Halo: " << x_halo << " " << y_halo << " "
//                          << error << " " << MPI_Helpers::My_rank << " "
//                          << dd << std::endl;
               haloed_file << x_haloed << " " << y_haloed << " "
                           << error << " " << MPI_Helpers::My_rank << " " 
                           << dd << " "
                           << haloed_elem_pt[e]->node_pt(j)->is_hanging() 
                           << std::endl;
               halo_file << x_halo << " " << y_halo << " "
                         << error << " " << MPI_Helpers::My_rank << " " 
                         << dd << " " 
                         << other_hanging << std::endl; 
                          // (communicated is_hanging value)
              }
            } // j<nnod_per_el
          } // e<nelem_haloed
//         oomph_info << "Check count (receive)... " << count << std::endl;
         haloed_file.close();
         halo_file.close();  
        }
      }
    }
   // My haloed elements are not being checked: Send my halo elements
   // whose non-halo counterparts are located on processor d
   else
    {

     // Get vectors of halo elements by copy operation
     Vector<FiniteElement*> 
      halo_elem_pt(mesh_pt()->halo_element_pt(d));
     
     // How many of my elements are halo elements whose non-halo
     // counterpart is located on processor d?
     unsigned nelem_halo=halo_elem_pt.size();
          
     // Send it across to the processor whose haloed nodes are being checked
     MPI_Send(&nelem_halo,1,MPI_INT,d,0,
              MPI_COMM_WORLD);


     // Now string together the nodal positions of all halo nodes
     unsigned nnod_per_el=mesh_pt()->finite_element_pt(0)->nnode();
     unsigned nod_dim=mesh_pt()->finite_element_pt(0)->node_pt(0)->ndim();
     Vector<double> nodal_positions(nod_dim*nnod_per_el*nelem_halo); 
     Vector<int> nodal_hangings(nnod_per_el*nelem_halo);
     unsigned count=0;
     unsigned count_hanging=0;
     for (unsigned e=0;e<nelem_halo;e++)
      {
       FiniteElement* el_pt= halo_elem_pt[e];
       for (unsigned j=0;j<nnod_per_el;j++)
        {
         // andy: testing POSITIONS, not x location (cf hanging nodes, nodes.h)
         nodal_positions[count]=el_pt->node_pt(j)->position(0);
         count++;
         nodal_positions[count]=el_pt->node_pt(j)->position(1);
         count++;
         if (el_pt->node_pt(j)->is_hanging())
          {
           nodal_hangings[count_hanging]=1;
          }
         else
          {
           nodal_hangings[count_hanging]=0;
          }
         count_hanging++;
         if (nod_dim==3)
          {
           nodal_positions[count]=el_pt->node_pt(j)->position(2);
           count++;
          }
        }
      }
     // Send it across to the processor whose haloed elements are being 
     // checked

     MPI_Send(&nodal_positions[0],nod_dim*nnod_per_el*nelem_halo,
              MPI_DOUBLE,d,0,MPI_COMM_WORLD);
     MPI_Send(&nodal_hangings[0],nnod_per_el*nelem_halo,
              MPI_INT,d,1,MPI_COMM_WORLD);
    }
  }

 oomph_info << "Max. error for halo/haloed elements " << max_error
            << std::endl;

 if (max_error>Max_permitted_error_for_halo_check)
  {         
   std::ostringstream error_message;
   error_message
    << "This is bigger than the permitted threshold huiguy "
    << Problem::Max_permitted_error_for_halo_check << std::endl;
   error_message
    << "If you believe this to be acceptable for your problem\n"
    << "increase Problem::Max_permitted_error_for_halo_check and re-run \n";
   throw OomphLibError(error_message.str(),
                       "Problem::check_halo_schemes()",
                       OOMPH_EXCEPTION_LOCATION);
  }


 
 // Doc halo/haloed nodes lookup schemes
 //-------------------------------------
 if (doc_info.doc_flag())
  {
   // Loop over domains for halo nodes
   for (int dd=0;dd<MPI_Helpers::Nproc;dd++)
    {   
     sprintf(filename,"%s/halo_node_check%i_%i.dat",
             doc_info.directory().c_str(),MPI_Helpers::My_rank,dd);
     halo_file.open(filename);
     halo_file << "ZONE " << std::endl;
     
     unsigned nnod= mesh_pt()->nhalo_node(dd);
     for (unsigned j=0;j<nnod;j++)
      {
       Node* nod_pt=mesh_pt()->halo_node_pt(dd,j);
       unsigned ndim=nod_pt->ndim();
       for (unsigned i=0;i<ndim;i++)
        {
         halo_file << nod_pt->position(i) << " ";
        }
       halo_file << std::endl;
      }
     // Dummy output for processor that doesn't share halo nodes
     // (needed for tecplot)
     if (nnod==0)
      {
       unsigned ndim=mesh_pt()->finite_element_pt(0)->node_pt(0)->ndim();
       if (ndim==2)
        {
         halo_file   << " 1.0 1.1 " << std::endl;
        }
       else
        {
         halo_file   << " 1.0 1.1 1.1" << std::endl;
        }
      }
     halo_file.close(); 
    }
   
   
   // Loop over domains for haloed nodes
   for (int d=0;d<MPI_Helpers::Nproc;d++)
    {
     sprintf(filename,"%s/haloed_node_check%i_%i.dat",
             doc_info.directory().c_str(),d,MPI_Helpers::My_rank);
     haloed_file.open(filename);
     haloed_file << "ZONE " << std::endl;
     
     unsigned nnod= mesh_pt()->nhaloed_node(d);
     for (unsigned j=0;j<nnod;j++)
      {
       Node* nod_pt=mesh_pt()->haloed_node_pt(d,j);
       unsigned ndim=nod_pt->ndim();
       for (unsigned i=0;i<ndim;i++)
        {
         haloed_file << nod_pt->position(i) << " ";
        }
       haloed_file << std::endl;
      }
     // Dummy output for processor that doesn't share halo nodes
     // (needed for tecplot)
     if (nnod==0)
      {
       unsigned ndim=mesh_pt()->finite_element_pt(0)->node_pt(0)->ndim();
       if (ndim==2)
        {
         halo_file   << " 1.0 1.1 " << std::endl;
        }
       else
        {
         halo_file   << " 1.0 1.1 1.1" << std::endl;
        }
      }
     haloed_file.close(); 
    }
  }

 // Check halo/haloed nodes lookup schemes
 //---------------------------------------
 max_error=0.0;

 // Loop over domains for haloed nodes
 for (int d=0;d<MPI_Helpers::Nproc;d++)
  {
   // Are my haloed nodes being checked?
   if (d==MPI_Helpers::My_rank)
    {
     // Loop over domains for halo nodes
     for (int dd=0;dd<MPI_Helpers::Nproc;dd++)
      {
       // Don't talk to yourself
       if (dd!=d)
        {
         // How many of my nodes are haloed nodes whose halo
         // counterpart is located on processor dd?
         int nnod_haloed=mesh_pt()->nhaloed_node(dd);
         
         // Receive from processor dd how many of his nodes are halo
         // nodes whose non-halo counterparts are located here
         int nnod_halo=0;
         MPI_Recv(&nnod_halo,1, MPI_INT,dd,
                  0,MPI_COMM_WORLD,&status);
         
         if (nnod_haloed!=nnod_halo)
          {
           std::ostringstream error_message;
           
           error_message
            << "Clash in numbers of halo and haloed nodes! " 
            << std::endl;
           error_message 
            << "# of haloed nodes whose halo counterpart lives on proc "
            << dd << ": " << nnod_haloed << std::endl;
           error_message
            << "# of halo nodes whose non-halo counterpart lives on proc "
            << d << ": " << nnod_halo << std::endl;
           error_message 
            << "(Re-)run Problem::check_halo_schemes() with DocInfo object"
            << std::endl;
           error_message 
            << "to identify the problem" << std::endl;
           throw OomphLibError(error_message.str(),
                               "Problem::check_halo_schemes()",
                               OOMPH_EXCEPTION_LOCATION);
          }


         unsigned nod_dim=mesh_pt()->
          finite_element_pt(0)->node_pt(0)->ndim();
         
         // Get strung-together nodal positions from other processor
         Vector<double> other_nodal_positions(nod_dim*nnod_halo);
         MPI_Recv(&other_nodal_positions[0],nod_dim*nnod_halo,MPI_DOUBLE,dd,
                  0,MPI_COMM_WORLD,&status);

         // Check
         unsigned count=0;
         for (int j=0;j<nnod_halo;j++)
          {
           double x_haloed=
            mesh_pt()->haloed_node_pt(dd,j)->position(0);
           double y_haloed=
            mesh_pt()->haloed_node_pt(dd,j)->position(1);
           double z_haloed=0.0;
           if (nod_dim==3)
            {
             z_haloed=mesh_pt()->haloed_node_pt(dd,j)->position(2);
            }
           double x_halo=other_nodal_positions[count];
           count++;
           double y_halo=other_nodal_positions[count];
           count++;
           double z_halo=0.0;
           if (nod_dim==3)
            {
             z_halo=other_nodal_positions[count];
             count++;
            }
           double error=sqrt( pow(x_haloed-x_halo,2)+
                              pow(y_haloed-y_halo,2)+
                              pow(z_haloed-z_halo,2));
           if (fabs(error)>max_error)
            {
//              std::cout << "ZONE" << std::endl;
//              std::cout << x_halo << " " 
//                        << y_halo << " " 
//                        << y_halo << " " 
//                        << d << " " << dd 
//                        << std::endl;
//              std::cout << x_haloed << " " 
//                        << y_haloed << " " 
//                        << y_haloed << " "
//                        << d << " " << dd  
//                        << std::endl;
//              std::cout << std::endl;
             max_error=fabs(error);       
            }
          }
        }
      }
    }
   // My haloed nodes are not being checked: Send my halo nodes
   // whose non-halo counterparts are located on processor d
   else
    {
     int nnod_halo=mesh_pt()->nhalo_node(d);
     
     // Send it across to the processor whose haloed nodes are being checked
     MPI_Send(&nnod_halo,1,MPI_INT,d,0,
              MPI_COMM_WORLD);

     unsigned nod_dim=mesh_pt()->
      finite_element_pt(0)->node_pt(0)->ndim();
         
     // Now string together the nodal positions of all halo nodes
     Vector<double> nodal_positions(nod_dim*nnod_halo);
     unsigned count=0;
     for (int j=0;j<nnod_halo;j++)
      {
       nodal_positions[count]=mesh_pt()->halo_node_pt(d,j)->position(0);
       count++;
       nodal_positions[count]=mesh_pt()->halo_node_pt(d,j)->position(1);
       count++;
       if (nod_dim==3)
        {
         nodal_positions[count]=mesh_pt()->halo_node_pt(d,j)->position(2);
         count++;
        }
      }
     // Send it across to the processor whose haloed nodes are being checked
     MPI_Send(&nodal_positions[0],nod_dim*nnod_halo,MPI_DOUBLE,d,0,
              MPI_COMM_WORLD);     
    }
  }

 oomph_info << "Max. error for halo/haloed nodes " << max_error
            << std::endl;

 if (max_error>Max_permitted_error_for_halo_check)
  {         
   std::ostringstream error_message;
   error_message
    << "This is bigger than the permitted threshold "
    << Problem::Max_permitted_error_for_halo_check << std::endl;
   error_message
    << "If you believe this to be acceptable for your problem\n"
    << "increase Problem::Max_permitted_error_for_halo_check and re-run \n";
   throw OomphLibError(error_message.str(),
                       "Problem::check_halo_schemes()",
                       OOMPH_EXCEPTION_LOCATION);
  }

 // Doc shared nodes lookup schemes
 //-------------------------------------
 if (doc_info.doc_flag())
  {
   // Loop over domains for shared nodes
   for (int dd=0;dd<MPI_Helpers::Nproc;dd++)
    {   
     sprintf(filename,"%s/shared_node_check%i_%i.dat",
             doc_info.directory().c_str(),MPI_Helpers::My_rank,dd);
     shared_file.open(filename);
     shared_file << "ZONE " << std::endl;
     
     unsigned nnod= mesh_pt()->nshared_node(dd);
     for (unsigned j=0;j<nnod;j++)
      {
       Node* nod_pt=mesh_pt()->shared_node_pt(dd,j);
       unsigned ndim=nod_pt->ndim();
       for (unsigned i=0;i<ndim;i++)
        {
         shared_file << nod_pt->position(i) << " ";
        }
       shared_file << std::endl;
      }
     // Dummy output for processor that doesn't share nodes
     // (needed for tecplot)
     if (nnod==0)
      {
       unsigned ndim=mesh_pt()->finite_element_pt(0)->node_pt(0)->ndim();
       if (ndim==2)
        {
         shared_file   << " 1.0 1.1 " << std::endl;
        }
       else
        {
         shared_file   << " 1.0 1.1 1.1" << std::endl;
        }
      }
     shared_file.close(); 
    }

  }

 // Check shared nodes lookup schemes
 //---------------------------------------
 max_error=0.0;

 // Loop over domains for shared nodes
 for (int d=0;d<MPI_Helpers::Nproc;d++)
  {
   // Are my shared nodes being checked?
   if (d==MPI_Helpers::My_rank)
    {
     // Loop over domains for shared nodes
     for (int dd=0;dd<MPI_Helpers::Nproc;dd++)
      {
       // Don't talk to yourself
       if (dd!=d)
        {
         // How many of my nodes are shared nodes with processor dd?
         int nnod_shared=mesh_pt()->nshared_node(dd);
         
         // Receive from processor dd how many of his nodes are shared
         // with this processor
         int nnod_share=0;
         MPI_Recv(&nnod_share,1, MPI_INT,dd,
                  0,MPI_COMM_WORLD,&status);
         
         if (nnod_shared!=nnod_share)
          {
           std::ostringstream error_message;
           
           error_message
            << "Clash in numbers of shared nodes! " 
            << std::endl;
           error_message 
            << "# of shared nodes on proc "
            << dd << ": " << nnod_shared << std::endl;
           error_message
            << "# of shared nodes on proc "
            << d << ": " << nnod_share << std::endl;
           error_message 
            << "(Re-)run Problem::check_halo_schemes() with DocInfo object"
            << std::endl;
           error_message 
            << "to identify the problem" << std::endl;
           throw OomphLibError(error_message.str(),
                               "Problem::check_halo_schemes()",
                               OOMPH_EXCEPTION_LOCATION);
          }


         unsigned nod_dim=mesh_pt()->
          finite_element_pt(0)->node_pt(0)->ndim();
         
         // Get strung-together nodal positions from other processor
         Vector<double> other_nodal_positions(nod_dim*nnod_share);
         MPI_Recv(&other_nodal_positions[0],nod_dim*nnod_share,MPI_DOUBLE,dd,
                  0,MPI_COMM_WORLD,&status);

         // Check
         unsigned count=0;
         for (int j=0;j<nnod_share;j++)
          {
           double x_shared=
            mesh_pt()->shared_node_pt(dd,j)->position(0);
           double y_shared=
            mesh_pt()->shared_node_pt(dd,j)->position(1);
           double z_shared=0.0;
           if (nod_dim==3)
            {
             z_shared=mesh_pt()->shared_node_pt(dd,j)->position(2);
            }
           double x_share=other_nodal_positions[count];
           count++;
           double y_share=other_nodal_positions[count];
           count++;
           double z_share=0.0;
           if (nod_dim==3)
            {
             z_share=other_nodal_positions[count];
             count++;
            }
           double error=sqrt( pow(x_shared-x_share,2)+
                              pow(y_shared-y_share,2)+
                              pow(z_shared-z_share,2));
           if (fabs(error)>max_error)
            {
//              std::cout << "ZONE" << std::endl;
//              std::cout << x_halo << " " 
//                        << y_halo << " " 
//                        << y_halo << " " 
//                        << d << " " << dd 
//                        << std::endl;
//              std::cout << x_haloed << " " 
//                        << y_haloed << " " 
//                        << y_haloed << " "
//                        << d << " " << dd  
//                        << std::endl;
//              std::cout << std::endl;
             max_error=fabs(error);       
            }
          }
        }
      }
    }
   // My shared nodes are not being checked: Send my shared nodes
   // to the other processor
   else
    {
     int nnod_share=mesh_pt()->nshared_node(d);
     
     // Send it across to the processor whose shared nodes are being checked
     MPI_Send(&nnod_share,1,MPI_INT,d,0,
              MPI_COMM_WORLD);


     unsigned nod_dim=mesh_pt()->
      finite_element_pt(0)->node_pt(0)->ndim();
         
     // Now string together the nodal positions of all shared nodes
     Vector<double> nodal_positions(nod_dim*nnod_share);
     unsigned count=0;
     for (int j=0;j<nnod_share;j++)
      {
       nodal_positions[count]=mesh_pt()->shared_node_pt(d,j)->position(0);
       count++;
       nodal_positions[count]=mesh_pt()->shared_node_pt(d,j)->position(1);
       count++;
       if (nod_dim==3)
        {
         nodal_positions[count]=mesh_pt()->shared_node_pt(d,j)->position(2);
         count++;
        }
      }
     // Send it across to the processor whose shared nodes are being checked
     MPI_Send(&nodal_positions[0],nod_dim*nnod_share,MPI_DOUBLE,d,0,
              MPI_COMM_WORLD);     
    }
  }

 oomph_info << "Max. error for shared nodes " << max_error
            << std::endl;

 if (max_error>Max_permitted_error_for_halo_check)
  {         
   std::ostringstream error_message;
   error_message
    << "This is bigger than the permitted threshold "
    << Problem::Max_permitted_error_for_halo_check << std::endl;
   error_message
    << "If you believe this to be acceptable for your problem\n"
    << "increase Problem::Max_permitted_error_for_halo_check and re-run \n";
   throw OomphLibError(error_message.str(),
                       "Problem::check_halo_schemes()",
                       OOMPH_EXCEPTION_LOCATION);
  }
 
}




//========================================================================
///  Synchronise the degrees of freedom by overwriting
/// the haloed values with their non-halo counterparts held
/// on other processors
//========================================================================
void Problem::synchronise_dofs()
{ 
 MPI_Status status;

 // Loop over all processors whose eqn numbers are to be updated
 for (int rank=0;rank<MPI_Helpers::Nproc;rank++)
  {   
   // Prepare a vector of values
   Vector<double> values_on_other_proc;
   Vector<double> internal_values_on_other_proc;
   
   // If I'm not the processor whose halo values are updated,
   // some of my nodes may be haloed: Stick their
   // values into the vector
   if (rank!=MPI_Helpers::My_rank)
    {
     // How many of my nodes are haloed by the processor whose values
     // are updated?
     unsigned nnod=mesh_pt()->nhaloed_node(rank);
     unsigned count=0;
     for (unsigned j=0;j<nnod;j++)
      {
       // Generalised to variable number of values per node
       Node* haloed_nod_pt=mesh_pt()->haloed_node_pt(rank,j);
       unsigned nval=haloed_nod_pt->nvalue();

       for (unsigned ival=0;ival<nval;ival++)
        {
         values_on_other_proc.push_back(haloed_nod_pt->value(ival));
         count++;
        }

       // Is it a solid node?
       SolidNode* solid_nod_pt=dynamic_cast<SolidNode*>(haloed_nod_pt);

       if (solid_nod_pt!=0)
        {
         unsigned nval=solid_nod_pt->variable_position_pt()->nvalue();
         for (unsigned ival=0; ival<nval; ival++)
          {
           values_on_other_proc.push_back(solid_nod_pt
                    ->variable_position_pt()->value(ival));
           count++;
          }
        }

      }
          
     // Since nval may vary from node to node in the most general case,
     // the best way to send/receive here is to get the size of
     // the array before the array is sent, and send that first, so that
     // the receiver knows how much data to expect.  The order will be
     // preserved since the halo/haloed nodes are already ordered correctly
     MPI_Send(&count,1,MPI_INT,rank,0,MPI_COMM_WORLD);

     // Send it across
     MPI_Send(&values_on_other_proc[0],count,MPI_DOUBLE,rank,1,
              MPI_COMM_WORLD);
     
     // Now loop over haloed elements and prepare to send internal data
     Vector<FiniteElement*> haloed_elem_pt=mesh_pt()->haloed_element_pt(rank);
     unsigned nelem_haloed=haloed_elem_pt.size();
     unsigned count_intern=0;

     for (unsigned e=0; e<nelem_haloed; e++)
      {
       // how many internal data values for this element?
       unsigned nintern_data = haloed_elem_pt[e]->ninternal_data();
 
       for (unsigned iintern=0; iintern<nintern_data; iintern++)
        {
         // Cache internal_data local copy
         Data* int_data_pt=haloed_elem_pt[e]->internal_data_pt(iintern);
         unsigned nval=int_data_pt->nvalue();

         for (unsigned ival=0;ival<nval;ival++)
          {
           internal_values_on_other_proc.push_back(int_data_pt->value(ival));
           count_intern++;
          }
        }
      }
     // send the size of the vector of internal data values to the receiver
     MPI_Send(&count_intern,1,MPI_INT,rank,2,MPI_COMM_WORLD);

     // now send the vector itself
     MPI_Send(&internal_values_on_other_proc[0],count_intern,MPI_DOUBLE,rank,3,
              MPI_COMM_WORLD);
  
     // done
    }
   // Receive the vector of values
   else
    {
     // Loop over all other processors to receive their
     // values
     for (int send_rank=0;send_rank<MPI_Helpers::Nproc;send_rank++)
      {
       
       // Don't talk to yourself
       if (send_rank!=MPI_Helpers::My_rank)
        {
         // How many of my nodes are halos whose non-halo counter
         // parts live on processor send_rank?
         unsigned nnod=mesh_pt()->nhalo_node(send_rank);
         
         // Receive size of vector of values
         unsigned count;
         MPI_Recv(&count,1,MPI_INT,send_rank,0,MPI_COMM_WORLD,&status);

         // Prepare vector for receipt of values
         values_on_other_proc.resize(count);
      
         // Receive 
         MPI_Recv(&values_on_other_proc[0],count, MPI_DOUBLE, send_rank,
                  1, MPI_COMM_WORLD,&status);

         // Copy into the values of the halo nodes
         // on the present processors
         count=0; // reset array index counter to zero
         for (unsigned j=0;j<nnod;j++)
          {
           // Generalised to variable number of values per node
           Node* halo_nod_pt=mesh_pt()->halo_node_pt(send_rank,j);
           unsigned nval=halo_nod_pt->nvalue();
        
           for (unsigned ival=0;ival<nval;ival++)
            {
             halo_nod_pt->set_value(ival,values_on_other_proc[count]);
             count++; // increase array index
            }

           // Is this a solid node?
           SolidNode* solid_nod_pt=dynamic_cast<SolidNode*>(halo_nod_pt);

           if (solid_nod_pt!=0)
            {
             unsigned nval=solid_nod_pt->variable_position_pt()->nvalue();
             for (unsigned ival=0;ival<nval;ival++)
              {
               solid_nod_pt->variable_position_pt()->set_value(ival,
                values_on_other_proc[count]);
               count++;
              }
            }

          }

         // Get number of halo elements whose non-halo is on process send_rank
         Vector<FiniteElement*> halo_elem_pt=mesh_pt()->
                                  halo_element_pt(send_rank);
         unsigned nelem_halo=halo_elem_pt.size();
         
         // Receive size of vector of internal data values
         unsigned count_intern;
         MPI_Recv(&count_intern,1,MPI_INT,send_rank,2,MPI_COMM_WORLD,&status);

         // Prepare and receive vector
         internal_values_on_other_proc.resize(count_intern);
         MPI_Recv(&internal_values_on_other_proc[0],count_intern,
           MPI_DOUBLE,send_rank,3,MPI_COMM_WORLD,&status);

         // reset array counter index to zero
         count_intern=0;
         for (unsigned e=0;e<nelem_halo;e++)
          {
           unsigned nintern_data=halo_elem_pt[e]->ninternal_data();
           for (unsigned iintern=0;iintern<nintern_data;iintern++)
             {
              // Cache internal_data local copy
              Data* int_data_pt=halo_elem_pt[e]->internal_data_pt(iintern);
              unsigned nval=int_data_pt->nvalue();
              for (unsigned ival=0;ival<nval;ival++)
               {
                int_data_pt->set_value(ival,
                   internal_values_on_other_proc[count_intern]);
                count_intern++;
               }
             }
          }
        }
      }
    }
  }
}




//========================================================================
///  Synchronise equation numbers and return the total
/// number of degrees of freedom in the overall problem
//========================================================================
long Problem::synchronise_eqn_numbers()
{ 
 MPI_Status status;

 // Step 1: Bump up eqn numbers by the number of dofs on 
 // previous processor

 // Assemble vector that contains current number of dofs on the
 // various processors
 Vector<int> dofs_on_proc(MPI_Helpers::Nproc,-1);
 
 // Store own number of dofs
 int my_ndof=ndof();
 dofs_on_proc[MPI_Helpers::My_rank]=my_ndof;

 // Gather information on root processor: First argument group
 // specifies what is to be sent (one int from each procssor, indicating
 // the number of dofs on it), the second group indicates where
 // the results are to be gathered (in rank order) on root processor.
 MPI_Gather(&dofs_on_proc[MPI_Helpers::My_rank],1,MPI_INT,
            &dofs_on_proc[0],1, MPI_INT,
            0,MPI_COMM_WORLD);
 
 // Now broadcast the result back out: Nproc integers, starting
 // from the beginning of dofs_on_proc. 
 MPI_Bcast(&dofs_on_proc[0], MPI_Helpers::Nproc,MPI_INT,0,
           MPI_COMM_WORLD);

 // Get total number of dofs in problem
 unsigned new_ndof=0;
 for (int i=0;i<MPI_Helpers::Nproc;i++)
  { 
   new_ndof+=dofs_on_proc[i];
  }
 
  // Accumulate offset for eqn numbers
 unsigned bump=0;
 for (int i=0;i<MPI_Helpers::My_rank;i++)
  { 
   bump+= dofs_on_proc[i];
  }

 // Backup current dof_pt vector
 Vector<double*> old_dof_pt(Dof_pt);
 
 // Resize Dof_pt vector to total number of dofs in problem
 // and null out ALL of its entries, not just the newly created ones
 Dof_pt.resize(new_ndof,0);
 for (unsigned i=0;i<new_ndof;i++) Dof_pt[i]=0;

 // Loop over all internal data (on elements) and bump up their
 // equation numbers if they exist

 unsigned nelem=mesh_pt()->nelement();
 for (unsigned e=0;e<nelem;e++)
  {
   GeneralisedElement* el_pt=mesh_pt()->element_pt(e); 

   unsigned nintern_data=el_pt->ninternal_data();
   for (unsigned iintern=0;iintern<nintern_data;iintern++)
    {
     Data* int_data_pt=el_pt->internal_data_pt(iintern);
     unsigned nval=int_data_pt->nvalue();
     for (unsigned ival=0;ival<nval;ival++)
      {
       int old_eqn_number=int_data_pt->eqn_number(ival);
       if (old_eqn_number>=0) // i.e. it's being used
        {
         // Bump up eqn number
         int new_eqn_number=old_eqn_number+bump;
         int_data_pt->eqn_number(ival)=new_eqn_number;
         // Update entries in Dof_pt
         Dof_pt[new_eqn_number]=old_dof_pt[old_eqn_number];
        }
      }
    }
  }

 // Loop over all nodes on current processor and bump up their 
 // equation numbers if they're not pinned!
 unsigned nnod=mesh_pt()->nnode();
 for (unsigned j=0;j<nnod;j++)
  {
   Node* nod_pt=mesh_pt()->node_pt(j);

   // loop over ALL eqn numbers - variable number of values
   unsigned nval=nod_pt->nvalue(); 
   
   for (unsigned ival=0;ival<nval;ival++)
    {
     int old_eqn_number=nod_pt->eqn_number(ival); 
     // andy: include all eqn numbers
     if (old_eqn_number>=0)
      {
       // Bump up eqn number
       int new_eqn_number=old_eqn_number+bump;
       nod_pt->eqn_number(ival)=new_eqn_number;
       // Update entries in Dof_pt
       Dof_pt[new_eqn_number]=old_dof_pt[old_eqn_number];
      }
    }

   // Is this a solid node? If so, need to bump up its equation number(s)
   SolidNode* solid_nod_pt=dynamic_cast<SolidNode*>(nod_pt);

   if (solid_nod_pt!=0)
    {
     // Find equation numbers
     unsigned nval=solid_nod_pt->variable_position_pt()->nvalue();
     for (unsigned ival=0;ival<nval;ival++)
      {
       int old_eqn_number=solid_nod_pt->variable_position_pt()
                              ->eqn_number(ival);
       // include all eqn numbers

       if (old_eqn_number>=0)
        {
         // Bump up eqn number
         int new_eqn_number=old_eqn_number+bump;
         solid_nod_pt->variable_position_pt()->eqn_number(ival)=new_eqn_number;
         // Update entries in Dof_pt
         Dof_pt[new_eqn_number]=old_dof_pt[old_eqn_number];  
        }
      }
    }
  }


 // Now copy the haloed eqn numbers across
 // This has to include the internal data equation numbers as well
 // as the solid node equation numbers

 // Loop over all processors whose eqn numbers are to be updated
 for (int rank=0;rank<MPI_Helpers::Nproc;rank++)
  {
   // Prepare a vector of equation numbers 
   Vector<int> eqn_numbers_on_other_proc;
   Vector<int> internal_eqn_numbers_on_other_proc;

   // If I'm not the processor whose halo eqn numbers are updated,
   // some of my nodes may be haloed: Stick their
   // eqn numbers into the vector
   if (rank!=MPI_Helpers::My_rank)
    {
     // How many of my nodes are haloed by the processor whose eqn
     // numbers are updated?
     unsigned nnod=mesh_pt()->nhaloed_node(rank);
     unsigned count=0;     
     for (unsigned j=0;j<nnod;j++)
      {
       // Generalise to variable number of values per node
       Node* haloed_nod_pt=mesh_pt()->haloed_node_pt(rank,j);
       unsigned nval=haloed_nod_pt->nvalue();

       for (unsigned ival=0;ival<nval;ival++)
        {
         eqn_numbers_on_other_proc.push_back(haloed_nod_pt->eqn_number(ival));
         count++;
        }

       // Is it a solid node?
       SolidNode* solid_nod_pt=dynamic_cast<SolidNode*>(haloed_nod_pt);

       if (solid_nod_pt!=0)
        {
         unsigned nval=solid_nod_pt->variable_position_pt()->nvalue();
         for (unsigned ival=0; ival<nval; ival++)
          {
           eqn_numbers_on_other_proc.push_back(solid_nod_pt
                    ->variable_position_pt()->eqn_number(ival));
           count++;
          }
        }
      }

     // The receiving process needs to know how many values it's getting
     // since it only descends into the loop over the nodes after
     // receiving the vector
     MPI_Send(&count,1,MPI_INT,rank,0,MPI_COMM_WORLD);

     // Send it across
     MPI_Send(&eqn_numbers_on_other_proc[0],count,MPI_INT,rank,1,
              MPI_COMM_WORLD);

     // now loop over haloed elements and prepare to send 
     // equation numbers for internal data
     Vector<FiniteElement*> haloed_elem_pt=mesh_pt()->haloed_element_pt(rank);
     unsigned nelem_haloed=haloed_elem_pt.size();
     unsigned count_intern=0;

     for (unsigned e=0; e<nelem_haloed; e++)
      {
       // how many internal data values for this element?
       unsigned nintern_data = haloed_elem_pt[e]->ninternal_data();
 
       for (unsigned iintern=0; iintern<nintern_data; iintern++)
        {
         // Cache local copy of data
         Data* int_data_pt=haloed_elem_pt[e]->internal_data_pt(iintern); 
         unsigned nval=int_data_pt->nvalue();

         for (unsigned ival=0;ival<nval;ival++)
          {
           internal_eqn_numbers_on_other_proc.push_back
               (int_data_pt->eqn_number(ival));
           count_intern++;
          }
        }
      }
     // Does this cause a problem if count_intern is zero?
     // Perhaps the vector should only be sent if count_intern is non-zero?
     
     // send the size of the vector of internal data values to the receiver
     MPI_Send(&count_intern,1,MPI_INT,rank,2,MPI_COMM_WORLD);

     // now send the vector itself
     MPI_Send(&internal_eqn_numbers_on_other_proc[0],count_intern,
              MPI_INT,rank,3,MPI_COMM_WORLD);
     // done

    }
   // Receive the vector of eqn numbers
   else
    {
     // Loop over all other processors to receive their
     // eqn numbers
     for (int send_rank=0;send_rank<MPI_Helpers::Nproc;send_rank++)
      {
       // Don't talk to yourself
       if (send_rank!=MPI_Helpers::My_rank)
        {
         // How many of my nodes are halos whose non-halo counter
         // parts live on processor send_rank?
         nnod=mesh_pt()->nhalo_node(send_rank);
         
         // Receive the size of the vector of eqn numbers
         unsigned count;
         MPI_Recv(&count,1,MPI_INT,send_rank,0,MPI_COMM_WORLD,&status);
         
         // Prepare vector for receipt of eqn numbers
         eqn_numbers_on_other_proc.resize(count);
         
         // Receive it
         MPI_Recv(&eqn_numbers_on_other_proc[0],count,MPI_INT,send_rank,
                  1, MPI_COMM_WORLD,&status);
         
         // Copy into the equation numbers of the halo nodes
         // on the present processors
         count=0; // reset count for array index
         for (unsigned j=0;j<nnod;j++)
          {
           // Generalise to variable number of values per node
           Node* halo_nod_pt=mesh_pt()->halo_node_pt(send_rank,j);
           unsigned nval=halo_nod_pt->nvalue();

           for (unsigned ival=0;ival<nval;ival++)
            {
             halo_nod_pt->eqn_number(ival)=eqn_numbers_on_other_proc[count];
             count++;
            }

           // Is this a solid node?
           SolidNode* solid_nod_pt=dynamic_cast<SolidNode*>(halo_nod_pt);

           if (solid_nod_pt!=0)
            {
             unsigned nval=solid_nod_pt->variable_position_pt()->nvalue();
             for (unsigned ival=0;ival<nval;ival++)
              {
               solid_nod_pt->variable_position_pt()->eqn_number(ival)=
                eqn_numbers_on_other_proc[count];
               count++;
              }
            }

          }
         // Get number of halo elements whose non-halo is on process send_rank
         Vector<FiniteElement*> halo_elem_pt=mesh_pt()->
                                     halo_element_pt(send_rank);
         unsigned nelem_halo=halo_elem_pt.size();

         // Receive size of vector of internal data values
         unsigned count_intern;
         MPI_Recv(&count_intern,1,MPI_INT,send_rank,2,MPI_COMM_WORLD,&status);

         // Prepare and receive vector
         internal_eqn_numbers_on_other_proc.resize(count_intern);
         MPI_Recv(&internal_eqn_numbers_on_other_proc[0],
                 count_intern,MPI_INT,send_rank,3,MPI_COMM_WORLD,&status);

         // reset array counter index to zero
         count_intern=0;
         for (unsigned e=0;e<nelem_halo;e++)
          {
           unsigned nintern_data=halo_elem_pt[e]->ninternal_data();
           for (unsigned iintern=0;iintern<nintern_data;iintern++)
             {
              Data* int_data_pt=halo_elem_pt[e]->internal_data_pt(iintern); 
              // cache internal_data_pt copy
              unsigned nval=int_data_pt->nvalue();

              for (unsigned ival=0;ival<nval;ival++)
               {
                int_data_pt->eqn_number(ival)=
                   internal_eqn_numbers_on_other_proc[count_intern];
                count_intern++;
               }
             }
          }
         
        }
      }
    }

  }




 // Now the global equation numbers have been updated.
 //---------------------------------------------------
 // Setup the local equation numbers again.
 //----------------------------------------

 // Loop over the submeshes: Note we need to call the submeshes' own
 // assign_*_eqn_number() otherwise we miss additional functionality
 // that is implemented (e.g.) in SolidMeshes!
 unsigned n_sub_mesh=nsub_mesh();
 if (n_sub_mesh==0)
  {
   mesh_pt()->assign_local_eqn_numbers();
  }
 else
  {
   for (unsigned i=0;i<n_sub_mesh;i++)
    {
     mesh_pt(i)->assign_local_eqn_numbers();
    }
  }

 // Return new total number of equations
 return new_ndof;

}



#endif









}