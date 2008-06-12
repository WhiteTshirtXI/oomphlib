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
#ifdef OOMPH_HAS_MPI
#include "mpi.h"
//#include "../../mpi/mpi_src/mpi_generic/mpi_helpers.h"
#endif

#include "refineable_quad_element.h"
#include "error_estimator.h"
#include "shape.h"

namespace oomph
{

//====================================================================
/// Recovery shape functions as functions of the global, Eulerian
/// coordinate x of dimension dim.
/// The recovery shape functions are  complete polynomials of 
/// the order specified by Recovery_order.
//====================================================================
void Z2ErrorEstimator::shape_rec(const Vector<double>& x,
                                 const unsigned& dim,
                                 Vector<double>& psi_r)
{
 std::ostringstream error_stream;

  /// Which spatial dimension are we dealing with?
 switch(dim)
  {

  case 1:

   // 1D:
   //====

   throw OomphLibError(
    "Recovery shape functions haven't been implemented for 1D",
    "Z2ErrorEstimator::shape_rec()",
    OOMPH_EXCEPTION_LOCATION);
   break;

  case 2:

   // 2D:
   //====

   /// Find order of recovery shape functions
   switch(recovery_order())
   {
    case 1:

     // Complete linear polynomial in 2D:
     psi_r[0]=1.0;
     psi_r[1]=x[0];
     psi_r[2]=x[1];
     break;

    case 2:

     // Complete quadratic polynomial in 2D:
     psi_r[0]=1.0;
     psi_r[1]=x[0];
     psi_r[2]=x[1];
     psi_r[3]=x[0]*x[0];
     psi_r[4]=x[0]*x[1];
     psi_r[5]=x[1]*x[1];
     break;

    case 3:

     // Complete cubic polynomial in 2D:
     psi_r[0]=1.0;
     psi_r[1]=x[0];
     psi_r[2]=x[1];
     psi_r[3]=x[0]*x[0];
     psi_r[4]=x[0]*x[1];
     psi_r[5]=x[1]*x[1];
     psi_r[6]=x[0]*x[0]*x[0];
     psi_r[7]=x[0]*x[0]*x[1];
     psi_r[8]=x[0]*x[1]*x[1];
     psi_r[9]=x[1]*x[1]*x[1];
     break;

    default:

     error_stream 
      << "Recovery shape functions for recovery order " 
      << recovery_order() << " haven't yet been implemented for 2D" 
      << std::endl;

     throw OomphLibError(error_stream.str(),
                         "Z2ErrorEstimator::shape_rec()",
                         OOMPH_EXCEPTION_LOCATION);
   }
   break;

  case 3:

   // 3D:
   //====
   /// Find order of recovery shape functions
   switch(recovery_order())
    {
    case 1:

     // Complete linear polynomial in 3D:
     psi_r[0]=1.0;
     psi_r[1]=x[0];
     psi_r[2]=x[1];
     psi_r[3]=x[2];
     break;

    case 2:

     // Complete quadratic polynomial in 3D:
     psi_r[0]=1.0;
     psi_r[1]=x[0];
     psi_r[2]=x[1];
     psi_r[3]=x[2];
     psi_r[4]=x[0]*x[0];
     psi_r[5]=x[0]*x[1];
     psi_r[6]=x[0]*x[2];
     psi_r[7]=x[1]*x[1];
     psi_r[8]=x[1]*x[2];
     psi_r[9]=x[2]*x[2];
     break;

    case 3:

     // Complete cubic polynomial in 3D:
     psi_r[0]=1.0;
     psi_r[1]=x[0];
     psi_r[2]=x[1];
     psi_r[3]=x[2];    
     psi_r[4]=x[0]*x[0];
     psi_r[5]=x[0]*x[1];
     psi_r[6]=x[0]*x[2];
     psi_r[7]=x[1]*x[1];
     psi_r[8]=x[1]*x[2];
     psi_r[9]=x[2]*x[2];
     psi_r[10]=x[0]*x[0]*x[0];
     psi_r[11]=x[0]*x[0]*x[1];
     psi_r[12]=x[0]*x[0]*x[2];
     psi_r[13]=x[1]*x[1]*x[1];
     psi_r[14]=x[0]*x[1]*x[1];
     psi_r[15]=x[2]*x[1]*x[1];
     psi_r[16]=x[2]*x[2]*x[2];
     psi_r[17]=x[2]*x[2]*x[0];
     psi_r[18]=x[2]*x[2]*x[1];
     psi_r[19]=x[0]*x[1]*x[2];

     break;
  
    default:

     error_stream 
      << "Recovery shape functions for recovery order " 
      << recovery_order() << " haven't yet been implemented for 3D" 
      << std::endl;

     throw OomphLibError(error_stream.str(),
                         "Z2ErrorEstimator::shape_rec()",
                         OOMPH_EXCEPTION_LOCATION);
    }


   break;

  default:

   // Any other dimension?
   //=====================
   error_stream << "No recovery shape functions for dim " 
                << dim << std::endl;
   throw OomphLibError(error_stream.str(),
                       "Z2ErrorEstimator::shape_rec()",
                       OOMPH_EXCEPTION_LOCATION);
   break;
  }
}




//======================================================================
/// Setup patches: For each vertex node pointed to by nod_pt,
/// adjacent_elements_pt[nod_pt] contains the pointer to the vector that 
/// contains the pointers to the elements that the node is part of.
/// Also returns a Vector of vertex nodes for use in get_element_errors.
//======================================================================
void Z2ErrorEstimator::setup_patches(Mesh*& mesh_pt,
      std::map<Node*,Vector<ElementWithZ2ErrorEstimator*>*>& 
                                     adjacent_elements_pt,
                                     Vector<Node*>& vertex_node_pt)

 {
  // (see also note at the end of get_element_errors below) 
  // NOTE FOR FUTURE REFERENCE - revisit in case of adaptivity problems in
  //  parallel jobs at the boundaries between processes
  //
  // The current method for distributed problems neglects recovered flux
  // contributions from patches that cannot be assembled from (vertex) nodes
  // on the current process, but would be assembled if the problem was not 
  // distributed.  The only nodes for which this is the case are nodes which 
  // lie on the exact boundary between processes.  
  //
  // The suggested method for fixing this requires the current process (A) to
  // receive information from the process (B) on which the patch is 
  // assembled.  These patches are precisely the patches on process B which
  // contain no halo elements.  The contribution from these patches needs to
  // be sent to the nodes (on process A) that are on the boundary between 
  // A and B.  Therefore a map is required to denote such nodes; a node is on
  // the boundary of a process if it is a member of at least one halo element
  // and one non-halo element for that process.  Create a vector of bools which
  // is the size of the number of processes and make the entry true if the node
  // (through a map(nod_pt,Vector<bools> node_bnd)) is on the boundary for a
  // process and false otherwise.  This should be done here in setup_patches.
  //
  // When it comes to the error calculation in get_element_errors (see later)
  // the communication needs to take place after rec_flux_map(nod_pt,i) has
  // been assembled for all other patches.  A separate vector for the patches
  // to be sent needs to be assembled: the recovered flux contribution at
  // a node on process B is sent to the equivalent node on process A if the
  // patch contains ONLY halo elements AND the node is on the boundary between
  // A and B (i.e. both the entries in the mapped vector are set to true).
  //
  // If anyone else read this and has any questions, I have a fuller
  // explanation written out (with some relevant diagrams in the 2D case).
  //
  // Andrew.Gait@manchester.ac.uk

  // Auxiliary map that contains element-adjacency for ALL nodes
  std::map<Node*,Vector<ElementWithZ2ErrorEstimator*>*> 
   aux_adjacent_elements_pt;
  
#ifdef PARANOID
  // Check if all elements request the same recovery order
  unsigned ndisagree=0;
#endif

  // Loop over all elements to setup adjacency for all nodes.
  // Need to do this because midside nodes can be corner nodes for 
  // adjacent smaller elements! Admittedly, the inclusion of interior 
  // nodes is wasteful...
  unsigned nelem=mesh_pt->nelement();
  for (unsigned e=0;e<nelem;e++)
   {
    ElementWithZ2ErrorEstimator* el_pt=
     dynamic_cast<ElementWithZ2ErrorEstimator*>(mesh_pt->element_pt(e));

#ifdef PARANOID
    // Check if all elements request the same recovery order
    if (el_pt->nrecovery_order()!=Recovery_order){ndisagree++;}
#endif

    // Loop all nodes in element
    unsigned nnod=el_pt->nnode();
    for (unsigned n=0;n<nnod;n++)
     {
      Node* nod_pt=el_pt->node_pt(n);

        // Has this node been considered before?
        if (aux_adjacent_elements_pt[nod_pt]==0)
         {
        // Create Vector of pointers to its adjacent elements
          aux_adjacent_elements_pt[nod_pt]=new 
           Vector<ElementWithZ2ErrorEstimator*>;
         }

        // Add pointer to adjacent element
        (*aux_adjacent_elements_pt[nod_pt]).push_back(el_pt);
//       }

     }
   } // end element loop
   
#ifdef PARANOID
  // Check if all elements request the same recovery order
  if (ndisagree!=0)
   {
    oomph_info 
     << "\n\n========================================================\n";
    oomph_info << "WARNING: " << std::endl;
    oomph_info << ndisagree << " out of " << mesh_pt->nelement() 
              << " elements\n";
    oomph_info << "have different preferences for the order of the recovery\n";
    oomph_info << "shape functions. We are using: Recovery_order=" 
         << Recovery_order << std::endl;
    oomph_info 
     << "========================================================\n\n";
   }
#endif

  //Loop over all elements, extract adjacency for corner nodes only
  nelem=mesh_pt->nelement();
  for (unsigned e=0;e<nelem;e++)
   {
    ElementWithZ2ErrorEstimator* el_pt=
     dynamic_cast<ElementWithZ2ErrorEstimator*>(mesh_pt->element_pt(e));

    // Loop over corner nodes
    unsigned n_node=el_pt->nvertex_node();
    for (unsigned n=0;n<n_node;n++)
     {
      Node* nod_pt=el_pt->vertex_node_pt(n);
      
      // Has this node been considered before?
      if (adjacent_elements_pt[nod_pt]==0)
       {
        // Add the node pointer to the vertex node container
        vertex_node_pt.push_back(nod_pt);

        // Create Vector of pointers to its adjacent elements
        adjacent_elements_pt[nod_pt]=new 
         Vector<ElementWithZ2ErrorEstimator*>;
        
        // Copy across:
        unsigned nel=(*aux_adjacent_elements_pt[nod_pt]).size();
        for (unsigned e=0;e<nel;e++)
         {
          (*adjacent_elements_pt[nod_pt]).push_back(
           (*aux_adjacent_elements_pt[nod_pt])[e]);
         }
       }
     }

   } // end of loop over elements

  // Cleanup // hierher
  typedef std::map<Node*,Vector<ElementWithZ2ErrorEstimator*>*>::iterator ITT;
  for (ITT it=aux_adjacent_elements_pt.begin();
       it!=aux_adjacent_elements_pt.end();it++)
   {   
    delete it->second;
   }

 }



//======================================================================
/// Given the vector of elements that make up a patch,
/// the number of recovery and flux terms, and the
/// spatial dimension of the problem, compute
/// the matrix of recovered flux coefficients and return
/// a pointer to it.
//======================================================================
void Z2ErrorEstimator::get_recovered_flux_in_patch(
 const Vector<ElementWithZ2ErrorEstimator*>& patch_el_pt,
 const unsigned& num_recovery_terms,
 const unsigned& num_flux_terms,
 const unsigned& dim,
 DenseMatrix<double>*& recovered_flux_coefficient_pt)
{
 // Create/initialise matrix for linear system
 DenseDoubleMatrix recovery_mat(num_recovery_terms,num_recovery_terms,0.0);
 
 // Ceate/initialise vector of RHSs
 Vector< Vector<double> > rhs(num_flux_terms);
 for (unsigned irhs=0;irhs<num_flux_terms;irhs++)
  {
   rhs[irhs].resize(num_recovery_terms);
   for (unsigned j=0;j<num_recovery_terms;j++)
    {
     rhs[irhs][j]=0.0;
    }
  }
  
 //Loop over all elements in patch to assemble linear system
 unsigned nelem=patch_el_pt.size();
 for (unsigned e=0;e<nelem;e++)
  {
   // Get pointer to element
   ElementWithZ2ErrorEstimator* const el_pt=patch_el_pt[e]; // andy
  
   // Create storage for the recovery shape function values 
   Vector<double> psi_r(num_recovery_terms);
   
   //Create vector to hold local coordinates
   Vector<double> s(dim); 

   // multiple dereferencing occuring to el_pt->integral_pt() (andy)
   Integral* const integ_pt = el_pt->integral_pt();

   //Loop over the integration points
   //unsigned Nintpt = el_pt->integral_pt()->nweight();

   // testing
   unsigned Nintpt = integ_pt->nweight(); // changed

   for(unsigned ipt=0;ipt<Nintpt;ipt++)
    {
     //Assign values of s
     for(unsigned i=0;i<dim;i++)
      {
       s[i] = integ_pt->knot(ipt,i); // changed
      }
     
     //Get the integral weight
     double w = integ_pt->weight(ipt); // changed
     
     //Jaocbian of mapping
     double J = el_pt->J_eulerian(s);
     
     //Premultiply the weights and the Jacobian
     double W = w*J;
     
     // Global (Eulerian) coordinate
     Vector<double> x(dim);
     el_pt->interpolated_x(s,x);
     
     // Recovery shape functions at global (Eulerian) coordinate
     shape_rec(x,dim,psi_r);
     
     // Get FE estimates for Z2 flux: 
     Vector<double> fe_flux(num_flux_terms); 
     el_pt->get_Z2_flux(s,fe_flux);
     
     // Add elemental RHSs and recovery matrix to global versions
     //----------------------------------------------------------
     
     // RHS for different flux components
     for (unsigned i=0;i<num_flux_terms;i++)
      {
       // Loop over the nodes for the test functions 
       for(unsigned l=0;l<num_recovery_terms;l++)
        {
         rhs[i][l] += fe_flux[i]*psi_r[l]*W;
        }
      }

     // Loop over the nodes for the test functions 
     for(unsigned l=0;l<num_recovery_terms;l++)
      {
       //Loop over the nodes for the variables
       for(unsigned l2=0;l2<num_recovery_terms;l2++)
        { 
         //Add contribution to recovery matrix
         recovery_mat(l,l2)+=psi_r[l]*psi_r[l2]*W;
        }
      }
    }
    
  } // End of loop over elements that make up patch. 

 
 // Linear system is now assembled: Solve recovery system

 // LU decompose the recovery matrix
 recovery_mat.ludecompose();
 
 // Back-substitute (and overwrite for all rhs
 for (unsigned irhs=0;irhs<num_flux_terms;irhs++)
  {
   recovery_mat.lubksub(rhs[irhs]);
  }
 
 // Now create a matrix to store the flux recovery coefficients.
 // Pointer to this matrix will be returned.
 recovered_flux_coefficient_pt =
  new DenseMatrix<double>(num_recovery_terms,num_flux_terms);

 // Copy coefficients
 for (unsigned icoeff=0;icoeff<num_recovery_terms;icoeff++)
  {
   for (unsigned irhs=0;irhs<num_flux_terms;irhs++)
    {
     (*recovered_flux_coefficient_pt)(icoeff,irhs)=rhs[irhs][icoeff]; 
    }
  }

}


//==================================================================
/// Number of coefficients for expansion of recovered fluxes
/// for given spatial dimension of elements.
/// Use complete polynomial of given order for recovery
//==================================================================
unsigned Z2ErrorEstimator::nrecovery_terms(const unsigned& dim)
{

 unsigned num_recovery_terms;

#ifdef PARANOID
 if ((dim!=2)&&(dim!=3))
  {
   std::string error_message =
    "THIS HASN'T BEEN USED/VALIDATED YET -- CHECK NUMBER OF RECOVERY TERMS!\n";
   error_message += "Then remove this break and continue\n";

   throw OomphLibError(error_message,"Z2ErrorEstimator::nrecovery_terms()",
                       OOMPH_EXCEPTION_LOCATION);
  }
#endif
 
 switch(Recovery_order)
  {
  case 1:
   
   // Linear recovery shape functions
   //--------------------------------
   
   switch(dim)
    {
    case 1:
     // 1D:
     num_recovery_terms=2; // 1, x
     break;
     
    case 2:
     // 2D:
     num_recovery_terms=3; // 1, x, y
     break;
     
    case 3:
     // 3D:
     num_recovery_terms=4; // 1, x, y, z
     break;
     
    default:
     throw OomphLibError("Dim must be 1, 2 or 3",
                         "Z2ErrorEstimator::nrecovery_terms()",
                         OOMPH_EXCEPTION_LOCATION);
    }
   break;
   
  case 2:
   
   // Quadratic recovery shape functions
   //-----------------------------------
   
   switch(dim)
    {
    case 1:
     // 1D:
     num_recovery_terms=3; // 1, x, x^2
     break;
     
    case 2:
     // 2D:
     num_recovery_terms=6; // 1, x, y, x^2, xy, y^2
     break;
     
    case 3:
     // 3D:
     num_recovery_terms=10; // 1, x, y, z, x^2, y^2, z^2, xy, xz, yz
     break;
     
    default:
     throw OomphLibError("Dim must be 1, 2 or 3",
                         "Z2ErrorEstimator::nrecovery_terms()",
                         OOMPH_EXCEPTION_LOCATION);
    }
   break;
   
   
  case 3:
   
   // Cubic recovery shape functions
   //--------------------------------
   
   switch(dim)
    {
    case 1:
     // 1D:
     num_recovery_terms=4; // 1, x, x^2, x^3
     break;
     
    case 2:
     // 2D:  
     num_recovery_terms=10; // 1, x, y, x^2, xy, y^2, x^3, y^3, x^2 y, x y^2
     break;

    case 3:
     // 3D:
     num_recovery_terms=20; // 1, x, y, z, x^2, y^2, z^2, xy, xz, yz,
     // x^3, y^3, z^3,
     // x^2 y, x^2 z, 
     // y^2 x, y^2 z, 
     // z^2 x, z^2 y
     // xyz
     break;

    default:
     throw OomphLibError("Dim must be 1, 2 or 3",
                         "Z2ErrorEstimator::nrecovery_terms()",
                         OOMPH_EXCEPTION_LOCATION);
    }
   break;
    

  default:

   // Any other recovery order?
   //--------------------------
   std::ostringstream error_stream;
   error_stream 
    << "Wrong Recovery_order " << Recovery_order << std::endl;
   
   throw OomphLibError(error_stream.str(),
                       "Z2ErrorEstimator::nrecovery_terms()",
                       OOMPH_EXCEPTION_LOCATION);
  }
   
 return num_recovery_terms;
}




//======================================================================
/// Get Vector of Z2-based error estimates for all elements in mesh.
/// If doc_info.doc_flag()=true, doc FE and recovered fluxes in 
/// - flux_fe*.dat
/// - flux_rec*.dat 
//======================================================================
 void Z2ErrorEstimator::get_element_errors(Mesh*& mesh_pt,
                                           Vector<double>& elemental_error, 
                                           DocInfo& doc_info)
 {
#ifdef OOMPH_HAS_MPI
  MPI_Status status;
#endif

  // Extract a few vital parameters from first element in mesh:
  ElementWithZ2ErrorEstimator* el_pt=
   dynamic_cast<ElementWithZ2ErrorEstimator*>(mesh_pt->element_pt(0)) ;
#ifdef PARANOID
  if (el_pt==0)
   {
    throw OomphLibError(
     "Element needs to inherit from ElementWithZ2ErrorEstimator!",
     "Z2ErrorEstimator::get_element_errors()",
     OOMPH_EXCEPTION_LOCATION);
   }
#endif

  // Number of 'flux'-like terms to be recovered
  unsigned num_flux_terms=el_pt->num_Z2_flux_terms();
  
  // Determine spatial dimension of all elements from first element
  unsigned dim=el_pt->dim();

  // Do we need to determine the recovery order from first element?
  if (Recovery_order_from_first_element)
   {
    Recovery_order=el_pt->nrecovery_order();
   }

  // Determine number of coefficients for expansion of recovered fluxes
  // Use complete polynomial of given order for recovery
  unsigned num_recovery_terms=nrecovery_terms(dim);


  // Setup patches (also returns Vector of vertex nodes)
  //====================================================
  std::map<Node*,Vector<ElementWithZ2ErrorEstimator*>*> adjacent_elements_pt;
  Vector<Node*> vertex_node_pt;
  setup_patches(mesh_pt,adjacent_elements_pt,vertex_node_pt);

  // Loop over all patches to get recovered flux value coefficients
  //===============================================================

  // Map to store sets of pointers to the recovered flux coefficient matrices
  // for each node. 
  std::map<Node*,std::set<DenseMatrix<double>*> > flux_coeff_pt;

  // We store the pointers to the recovered flux coefficient matrices for 
  // various patches in a vector so we can delete them later
  Vector<DenseMatrix<double>*> vector_of_recovered_flux_coefficient_pt;

  typedef std::map<Node*,Vector<ElementWithZ2ErrorEstimator*>*>::iterator IT;

  // Need to translate ElementWithZ2ErrorEstimator pointer to element number
  // in order to give each processor elements to work on if the problem
  // has not yet been distributed.  In order to reduce the use of #ifdef this
  // is also done for the serial problem and the code is amended accordingly.

  std::map<ElementWithZ2ErrorEstimator*,int> elem_num;
  unsigned nelem=mesh_pt->nelement();
  for (unsigned e=0;e<nelem;e++)
   { 
     elem_num[dynamic_cast<ElementWithZ2ErrorEstimator*>(
               mesh_pt->element_pt(e))]=e;
   }

  // This isn't a global variable
  int n_patch=adjacent_elements_pt.size(); // also needed by serial version
  int itbegin,itend,range;

#ifdef OOMPH_HAS_MPI

  if (!(mesh_pt->mesh_has_been_distributed())) // see mesh.h
   {
    // setup the loop variables
    range = n_patch/MPI_Helpers::Nproc; // number of patches on each proc
 
    itbegin = MPI_Helpers::My_rank*range;
    itend = (MPI_Helpers::My_rank+1)*range;

    if (MPI_Helpers::My_rank==((MPI_Helpers::Nproc)-1))
       {
        itend=n_patch;
       }

   }
  else
   {
#endif
    itbegin=0; // mesh has already been distributed, or serial problem
    range=n_patch;
    itend=n_patch;
#ifdef OOMPH_HAS_MPI
   }
#endif

  // set up matrices and vectors which will be sent later
  // - full matrix of all recovered coefficients
  Vector<DenseMatrix<double>*> vector_of_recovered_flux_coefficient_pt_to_send;
  // - vectors containing element numbers in each patch
  Vector<Vector<int> > vector_of_elements_in_patch_to_send;

  // go to the first patch
//  IT it=adjacent_elements_pt.begin(); // hierher: this appears to contain
                                      // entries in different order 
                                      // on different processors.

//  for (int i=0;i<itbegin;i++) 
//   { it++; } // move through to the patch on current process

  // now we can loop over the patches on the current process
  for (int i=itbegin;i<itend;i++)
   {
    // Which vertex node are we at?
    Node* nod_pt=vertex_node_pt[i];

    // Pointer to vector of pointers to elements that make up
    // the patch. 
    Vector<ElementWithZ2ErrorEstimator*>* el_vec_pt=
     adjacent_elements_pt[nod_pt];

    // Is the corner node that is central to the patch surrounded by 
    // at least two elements? 
    unsigned nelem=(*el_vec_pt).size();

    if (nelem>=2) 
     {
      // store the number of elements in the patch
      Vector<int> elements_in_this_patch;
      for (unsigned e=0;e<nelem;e++)
       {
        elements_in_this_patch.push_back(elem_num[(*el_vec_pt)[e]]);
       }

      // put them into storage vector ready to send
      vector_of_elements_in_patch_to_send.push_back(elements_in_this_patch);
      
      // Given the vector of elements that make up the patch,
      // the number of recovery and flux terms, and the spatial
      // dimension of the problem,  compute
      // the matrix of recovered flux coefficients and return
      // a pointer to it.
      DenseMatrix<double>* recovered_flux_coefficient_pt=0;
      get_recovered_flux_in_patch(*el_vec_pt,num_recovery_terms,num_flux_terms,
                                  dim,recovered_flux_coefficient_pt);

      // Store pointer to recovered flux coefficients for 
      // current patch in vector so we can send and then delete it later
      vector_of_recovered_flux_coefficient_pt_to_send.
       push_back(recovered_flux_coefficient_pt);

      } // end of if(nelem>=2)
    //   it++; // move to next patch map
   }

// Now broadcast the result from each process to every other process
// if the mesh has not yet been distributed and MPI is initialised

#ifdef OOMPH_HAS_MPI

  if (!mesh_pt->mesh_has_been_distributed() 
       && MPI_Helpers::MPI_has_been_initialised)
  {
  // All local recovered fluxes have been calculated, so now share result
  for (int iproc=0;iproc<MPI_Helpers::Nproc;iproc++)
   {
     // broadcast number of patches processed
    int n_patches=vector_of_recovered_flux_coefficient_pt_to_send.size();

//    oomph_info << "Number of patches: " << n_patches << std::endl;

    MPI_Bcast(&n_patches,1,MPI_INT,iproc,MPI_COMM_WORLD);

//    oomph_info << "Number of patches after bcast: " << n_patches << std::endl;

    // loop over these patches, broadcast recovered flux coefficients
    for (int ipatch=0;ipatch<n_patches;ipatch++)
     {
      // number of elements in this patch
      Vector<int> elements(0);
      unsigned nelements; // needs to be int for elem_num call later??
      if (MPI_Helpers::My_rank==iproc) 
       {  
        elements=vector_of_elements_in_patch_to_send[ipatch]; 
        nelements=elements.size();
       }

//    broadcast elements
      MPI_Helpers::broadcast_vector(elements,iproc,MPI_COMM_WORLD);

      // now get recovered flux coefficients
      DenseMatrix<double>* recovered_flux_coefficient_pt;

      if (MPI_Helpers::My_rank==iproc)
       {
        recovered_flux_coefficient_pt=
         vector_of_recovered_flux_coefficient_pt_to_send[ipatch];
       }
      else
       {
        recovered_flux_coefficient_pt=new DenseMatrix<double>;
       }

      // broadcast this matrix from the loop processor
      DenseMatrix<double> mattosend=*recovered_flux_coefficient_pt;

      MPI_Helpers::broadcast_matrix(mattosend,iproc,MPI_COMM_WORLD);

      *recovered_flux_coefficient_pt=mattosend;

      // end of parallel broadcasting
      vector_of_recovered_flux_coefficient_pt.
       push_back(recovered_flux_coefficient_pt);

      // Loop over elements in patch (work out nelements again after bcast)
      nelements=elements.size();

      for (unsigned e=0;e<nelements;e++)
       {
        // Get pointer to element
        ElementWithZ2ErrorEstimator* el_pt=
         dynamic_cast<ElementWithZ2ErrorEstimator*>(
          mesh_pt->element_pt(elements[e]));

        // Loop over all nodes in element 
        unsigned num_nod=el_pt->nnode();
        for (unsigned n=0;n<num_nod;n++)
         {
          // get the node
          Node* nod_pt=el_pt->node_pt(n);
          // Add the pointer to the current flux coefficient matrix 
          // to the set for the node
          // Mesh not distributed here so nod_pt cannot be halo
          flux_coeff_pt[nod_pt].
           insert(recovered_flux_coefficient_pt);
         }
       }
 
     } // loop over patches on current processor

   } // loop over processors
   
  }
 else // mesh_has_been_distributed=true
  {

#endif // end ifdef OOMPH_HAS_MPI for parallel job without mesh distribution

   // do the same for a distributed mesh as for a serial job
   // up to the point where the elemental error is calculated
   // and then communicate that (see below)
   int n_patches=vector_of_recovered_flux_coefficient_pt_to_send.size();

   // loop over these patches
   for (int ipatch=0;ipatch<n_patches;ipatch++)
    {
     // number of elements in this patch
     Vector<int> elements;
     int nelements; // needs to be int for elem_num call later
     elements=vector_of_elements_in_patch_to_send[ipatch]; 
     nelements=elements.size();

     // now get recovered flux coefficients
     DenseMatrix<double>* recovered_flux_coefficient_pt;
     recovered_flux_coefficient_pt=
      vector_of_recovered_flux_coefficient_pt_to_send[ipatch];

     vector_of_recovered_flux_coefficient_pt.
       push_back(recovered_flux_coefficient_pt);

     for (int e=0;e<nelements;e++)
       {
        // Get pointer to element
        ElementWithZ2ErrorEstimator* el_pt=
         dynamic_cast<ElementWithZ2ErrorEstimator*>(
          mesh_pt->element_pt(elements[e]));

        // Loop over all nodes in element 
        unsigned num_nod=el_pt->nnode();
        for (unsigned n=0;n<num_nod;n++)
         {
          // Get the node
          Node* nod_pt=el_pt->node_pt(n);
          // Add the pointer to the current flux coefficient matrix 
          // to the set for this node
          flux_coeff_pt[nod_pt].
             insert(recovered_flux_coefficient_pt);
         }
       }
 
    } // loop over patches on current processor


#ifdef OOMPH_HAS_MPI
  } // end if(mesh_has_been_distributed)
#endif

  // Cleanup patch storage scheme // hierher
  for (IT it=adjacent_elements_pt.begin();
       it!=adjacent_elements_pt.end();it++)
   {   
    delete it->second;
   }
  adjacent_elements_pt.clear();

  // Loop over all nodes, take average of recovered flux values
  //-----------------------------------------------------------
  // and evaluate recovered flux at nodes
  //-------------------------------------

  // Map of (averaged) recoverd flux values at nodes
  MapMatrixMixed<Node*,int,double> rec_flux_map;

  // Loop over all nodes
  unsigned n_node=mesh_pt->nnode();
  for (unsigned n=0;n<n_node;n++)
   {
    Node* nod_pt=mesh_pt->node_pt(n);

    // How many patches is this node a member of?
    unsigned npatches=flux_coeff_pt[nod_pt].size();

    // Matrix of averaged coefficients for this node
    DenseMatrix<double> averaged_flux_coeff
     (num_recovery_terms,num_flux_terms,0.0);

    // Loop over matrices for different patches and add contributions
    // hierher
    typedef std::set<DenseMatrix<double>*>::iterator IT;
    for (IT it=flux_coeff_pt[nod_pt].begin();
           it!=flux_coeff_pt[nod_pt].end();it++)
     {
      for (unsigned i=0;i<num_recovery_terms;i++)
       {
        for (unsigned j=0;j<num_flux_terms;j++)

         {
          // ...just add it -- we'll divide by the number of patches later
          averaged_flux_coeff(i,j)+=(*(*it))(i,j);
         }
       }
     }
    
    // Now evaluate the recovered flux (based on the averaged coefficients)
    //---------------------------------------------------------------------
    // at the nodal position itself.
    //------------------------------
    
    // Get global (Eulerian) nodal position
    Vector<double> x(dim);
    for (unsigned i=0;i<dim;i++)
     {
      x[i]=nod_pt->x(i);
     }
    
    // Evaluate global recovery functions at node
    Vector<double> psi_r(num_recovery_terms);
    shape_rec(x,dim,psi_r);
    
    // Initialise nodal fluxes
    for (unsigned i=0;i<num_flux_terms;i++)
     {
      rec_flux_map(nod_pt,i)=0.0;
     }
    
    // Loop over coefficients for flux recovery 
    for (unsigned i=0;i<num_flux_terms;i++)
     {
      for (unsigned icoeff=0;icoeff<num_recovery_terms;icoeff++)
       {
        rec_flux_map(nod_pt,i)+=
         averaged_flux_coeff(icoeff,i)*psi_r[icoeff];
       }
      // Now take averaging into account
      rec_flux_map(nod_pt,i)/=double(npatches);
      
     }

   } // end loop over nodes

  // We're done with the recovered flux coefficient matrices for 
  // the various patches and can delete them 
  unsigned npatch=vector_of_recovered_flux_coefficient_pt.size();
  for (unsigned p=0;p<npatch;p++)
   {
    delete vector_of_recovered_flux_coefficient_pt[p];
   }

  // NOTE FOR FUTURE REFERENCE - revisit in case of adaptivity problems in 
  // parallel jobs
  //
  // The current method for distributed problems neglects recovered flux 
  // contributions from patches that cannot be assembled on the current 
  // processor, but would be assembled if the problem was not distributed.
  // The only nodes for which this is the case are nodes which lie on the 
  // exact boundary between processors.
  //
  // See the note at the start of setup_patches (above) for more details.  

  // Get error estimates for all elements
  //======================================
  double flux_norm=0.0;
  unsigned test_count=0;

  //Loop over all (non-halo) elements
  nelem=mesh_pt->nelement();
  for (unsigned e=0;e<nelem;e++)
   {
    ElementWithZ2ErrorEstimator* el_pt=
     dynamic_cast<ElementWithZ2ErrorEstimator*>(mesh_pt->element_pt(e));

#ifdef OOMPH_HAS_MPI
    if (!el_pt->is_halo()) // Should be fine for serial code with OOMPH_HAS_MPI
                           // as it will always return true since no haloes
     {
#endif

    Vector<double> s(dim);

    // Initialise elemental error
    double error=0.0;

    Integral* integ_pt = el_pt->integral_pt();

    //Set the value of Nintpt
    unsigned Nintpt = integ_pt->nweight();

    //Loop over the integration points
    for(unsigned ipt=0;ipt<Nintpt;ipt++)
     {

      //Assign values of s
      for(unsigned i=0;i<dim;i++)
       {
        s[i] = integ_pt->knot(ipt,i);
       }

      //Get the integral weight
      double w = integ_pt->weight(ipt);

      //Jacobian of mapping
      double J = el_pt->J_eulerian(s);
       
      //Premultiply the weights and the Jacobian
      double W = w*J; 

      // Number of FE nodes
      unsigned n_node=el_pt->nnode();

      // FE shape function
      Shape psi(n_node);

      //Get values of FE shape function
      el_pt->shape(s,psi);

      // Initialise recovered flux Vector
      Vector<double> rec_flux(num_flux_terms,0.0);

      // Loop over all nodes (incl. halo nodes) to assemble contribution
      for (unsigned n=0;n<n_node;n++)
       {
        Node* nod_pt=el_pt->node_pt(n);

          // Loop over components
          for (unsigned i=0;i<num_flux_terms;i++)
           {
            rec_flux[i]+=rec_flux_map(nod_pt,i)*psi[n];
           }
       }

      // FE flux 
      Vector<double> fe_flux(num_flux_terms);
      el_pt->get_Z2_flux(s,fe_flux);

      // Add to RMS error:
      double sum=0.0;
      double sum2=0.0;
      for (unsigned i=0;i<num_flux_terms;i++)
       {
        sum+=(rec_flux[i]-fe_flux[i])*(rec_flux[i]-fe_flux[i]);
        sum2+=rec_flux[i]*rec_flux[i];
       }
      error+=sum*W;

      // Add to flux norm
      flux_norm+=sum2*W;

     }
    // Unscaled elemental RMS error:
    test_count++; // counting elements visited
    elemental_error[e]=sqrt(error);

#ifdef OOMPH_HAS_MPI
     } // end if (!el_pt->is_halo())
#endif
   } // end of loop over elements

  // Communicate the error for haloed elements to halo elements:
  // - loop over processors
  // - if current process, receive to halo element error
  // - if not current process, send haloed element error
  // How do we know which part of elemental_error to send?
  // Loop over haloed elements and find element number using elem_num map;
  // send this - order preservation of halo/haloed elements
  // guarantees that they get through in the correct order
#ifdef OOMPH_HAS_MPI

  if (mesh_pt->mesh_has_been_distributed())
   {

  for (int iproc=0; iproc<MPI_Helpers::Nproc; iproc++)
   {
    if (iproc!=MPI_Helpers::My_rank) // not current process, so send
     {
      Vector<FiniteElement*> haloed_elem_pt=mesh_pt->haloed_element_pt(iproc);
      int nelem_haloed=haloed_elem_pt.size();
      int element_num;
      Vector<double> haloed_elem_error(nelem_haloed);
      for (int e=0; e<nelem_haloed; e++)
       {
        // find element number
        element_num=elem_num[dynamic_cast<ElementWithZ2ErrorEstimator*>
                             (haloed_elem_pt[e])];
        // Put the error in a vector to send
        haloed_elem_error[e]=elemental_error[element_num];
       }
      // Send the haloed_elem_error vector
      MPI_Send(&haloed_elem_error[0],nelem_haloed,MPI_DOUBLE,iproc,0,
               MPI_COMM_WORLD);
     }
    else // iproc=MPI_Helpers::My_rank, so receive errors from others
     {
      for (int send_rank=0; send_rank<MPI_Helpers::Nproc; send_rank++)
       {
        if (iproc!=send_rank) // iproc=MPI_Helpers::My_rank already!
         {
          Vector<FiniteElement*> halo_elem_pt=mesh_pt->
                                              halo_element_pt(send_rank);
          int nelem_halo=halo_elem_pt.size();
          int element_num;
          Vector<double> halo_elem_error(nelem_halo);

          // Receive the elem_error vector from process send_rank
          MPI_Recv(&halo_elem_error[0],nelem_halo,MPI_DOUBLE,send_rank,0,
                   MPI_COMM_WORLD,&status);

          for (int e=0; e<nelem_halo; e++)
           {
            // Find element number
            element_num=elem_num[dynamic_cast<ElementWithZ2ErrorEstimator*>
                                 (halo_elem_pt[e])];
            // Put the error in this location
            elemental_error[element_num]=halo_elem_error[e];
           }
         }
       }
     }
   }

   }

#endif

  // NOTE FOR FUTURE REFERENCE - revisit in case of adaptivity problems in 
  // parallel jobs
  //
  // The current method for distributed problems neglects recovered flux 
  // contributions from patches that cannot be assembled on the current
  // processor, but would be assembled if the problem was not distributed.
  // The only nodes for which this is the case are nodes which lie on the 
  // exact boundary between processors.
  //
  // See the note at the start of setup_patches (above) for more details.

  // Use computed flux norm or externally imposed reference value?
  if (Reference_flux_norm!=0.0)
   {
    flux_norm=Reference_flux_norm;
   }
  else
   {
    // In parallel, perform reduction operation to get global value
#ifdef OOMPH_HAS_MPI
    if (mesh_pt->mesh_has_been_distributed())
     {
      double total_flux_norm;
      // every process needs to know the sum
      MPI_Allreduce(&flux_norm,&total_flux_norm,1,
                    MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
      // take sqrt
      flux_norm=sqrt(total_flux_norm);
     }
    else // mesh has not been distirbuted, so flux_norm already global
     {
      flux_norm=sqrt(flux_norm);
     }
#else // serial problem, so flux_norm already global
    flux_norm=sqrt(flux_norm);
#endif
   }

  // Now loop over (all!) elements again and 
  // normalise errors by global flux norm
  
  nelem=mesh_pt->nelement();
  for (unsigned e=0;e<nelem;e++)
   {
    elemental_error[e]/=flux_norm;
   }

  // Doc global fluxes?
  if (doc_info.doc_flag())
   {
    doc_flux(mesh_pt,num_flux_terms,rec_flux_map,elemental_error,doc_info);
   }

 }


//==================================================================
/// Doc FE and recovered flux
//==================================================================
void Z2ErrorEstimator::doc_flux(Mesh* mesh_pt,
                                const unsigned& num_flux_terms,
                                MapMatrixMixed<Node*,int,double>& rec_flux_map,
                                const Vector<double>& elemental_error,
                                DocInfo& doc_info)
{
 // Setup output files
 std::ofstream some_file,feflux_file;
 char filename[100];
 sprintf(filename,"%s/flux_rec%i_on_proc_%i.dat",doc_info.directory().c_str(),
         doc_info.number(),MPI_Helpers::My_rank);
 some_file.open(filename);
 sprintf(filename,"%s/flux_fe%i_on_proc_%i.dat",doc_info.directory().c_str(),
         doc_info.number(),MPI_Helpers::My_rank);
 feflux_file.open(filename);

 // Extract first element to determine spatial dimension
 FiniteElement* el_pt=mesh_pt->finite_element_pt(0);
 unsigned dim=el_pt->dim();
 Vector<double> s(dim);   

 // Decide on the number of plot points
 unsigned nplot=5;

 //Loop over all elements
 unsigned nel=mesh_pt->nelement();
 for (unsigned e=0;e<nel;e++)
  {
   ElementWithZ2ErrorEstimator* el_pt=
    dynamic_cast<ElementWithZ2ErrorEstimator*>(mesh_pt->element_pt(e));
   
   // Write tecplot header
   feflux_file << el_pt->tecplot_zone_string(nplot);
   some_file << el_pt->tecplot_zone_string(nplot);

   unsigned num_plot_points=el_pt->nplot_points(nplot);
   for (unsigned iplot=0;iplot<num_plot_points;iplot++)
    {     
     // Get local coordinates of plot point
     el_pt->get_s_plot(iplot,nplot,s);
     
     //Coordinate
     Vector<double> x(dim);
     el_pt->interpolated_x(s,x);
     
     // Number of FE nodes
     unsigned n_node=el_pt->nnode();
     
     // FE shape function
     Shape psi(n_node);
     
     //Get values of FE shape function
     el_pt->shape(s,psi);
     
     // Initialise recovered flux Vector
     Vector<double> rec_flux(num_flux_terms,0.0);
     
     // Loop over nodes to assemble contribution
     for (unsigned n=0;n<n_node;n++)
      {
       
       Node* nod_pt=el_pt->node_pt(n);
       
       // Loop over components
       for (unsigned i=0;i<num_flux_terms;i++)
        {
         rec_flux[i]+=rec_flux_map(nod_pt,i)*psi[n];
        }
      }
     
     // FE flux
     Vector<double> fe_flux(num_flux_terms);
     el_pt->get_Z2_flux(s,fe_flux);
     
     for (unsigned i=0;i<dim;i++)
      {
       some_file << x[i] << " ";
      }
     for (unsigned i=0;i<num_flux_terms;i++)
      {
       some_file << rec_flux[i] << " ";
      }
     some_file << elemental_error[e]  << " " 
               << std::endl;
     
     
     for (unsigned i=0;i<dim;i++)
      {
       feflux_file << x[i] << " ";
      }
     for (unsigned i=0;i<num_flux_terms;i++)
      {
       feflux_file << fe_flux[i] << " ";
      }
     feflux_file << elemental_error[e]  << " " 
                 << std::endl;
    }
  }


 // Write tecplot footer (e.g. FE connectivity lists)
 // using the first element's output info.
 FiniteElement* first_el_pt=mesh_pt->finite_element_pt(0);
 first_el_pt->write_tecplot_zone_footer(some_file,nplot);
 first_el_pt->write_tecplot_zone_footer(feflux_file,nplot);

 some_file.close();
 feflux_file.close();
 
}

}