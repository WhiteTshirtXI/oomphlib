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
#ifndef OOMPH_NAVIER_STOKES_PRECONDITIONERS_HEADER
#define OOMPH_NAVIER_STOKES_PRECONDITIONERS_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif


// oomphlib headers
#include "../generic/matrices.h"
#include "../generic/block_preconditioner.h"
#include "../generic/preconditioner.h"
#include "../generic/SuperLU_preconditioner.h"

#include "navier_stokes_elements.h"
#include "refineable_navier_stokes_elements.h"


namespace oomph
{

//===========================================================================
/// \short The least-squares commutator (LSC; formerly BFBT) Navier Stokes 
/// preconditioner. It uses blocks corresponding to the velocity 
/// and pressure unknowns, i.e. there are a total of 2x2 blocks, 
/// and all velocity components are treated as a single block of unknowns.
/// \n\n
/// Here are the details: An "ideal" Navier-Stokes preconditioner
/// would solve the system
/// \f[
/// \left( 
/// \begin{array}{cc}
/// {\bf F} & {\bf G} \\ {\bf D} & {\bf 0} 
/// \end{array} 
/// \right)
/// \left( 
/// \begin{array}{c}
/// {\bf z}_u \\ {\bf z}_p
/// \end{array} 
/// \right) =
/// \left( 
/// \begin{array}{c}
/// {\bf r}_u \\ {\bf r}_p
/// \end{array} 
/// \right)
/// \f]
/// where \f$ {\bf F}\f$ is the momentum block,  \f$ {\bf G} \f$ the
/// discrete gradient operator, and \f$ {\bf D}\f$ the discrete
/// divergence operator. (For unstabilised elements, we have 
/// \f$ {\bf D} = {\bf G}^T \f$ and in much of the literature
/// the divergence matrix is denoted by \f$ {\bf B} \f$ .)
/// The use of this preconditioner would ensure the convergence
/// of any iterative linear solver in a single iteration but its
/// application is, of course, exactly as expensive as a direct solve.
/// The LSC/BFBT preconditioner replaces the exact Jacobian by 
/// a block-triangular approximation
/// \f[
/// \left( 
/// \begin{array}{cc}
/// {\bf F} & {\bf G} \\ {\bf 0} & -{\bf M}_s 
/// \end{array} 
/// \right) 
/// \left( 
/// \begin{array}{c}
/// {\bf z}_u \\ {\bf z}_p
/// \end{array} 
/// \right) =
/// \left( 
/// \begin{array}{c}
/// {\bf r}_u \\ {\bf r}_p
/// \end{array} 
/// \right),
/// \f]
/// where \f${\bf M}_s\f$ is an approximation to the pressure 
/// Schur-complement \f$ {\bf S} = {\bf D} {\bf F}^{-1}{\bf G}. \f$
/// This system can be solved in two steps:
/// -# Solve the second row for \f$ {\bf z}_p\f$ via
///    \f[ 
///    {\bf z}_p = - {\bf M}_s^{-1} {\bf r}_p
///    \f]
/// -# Given \f$ {\bf z}_p \f$ , solve the first row for \f$ {\bf z}_u\f$ via
///    \f[ 
///    {\bf z}_u = {\bf F}^{-1} \big( {\bf r}_u - {\bf G} {\bf z}_p \big)
///    \f]
/// .
/// In the LSC/BFBT preconditioner, the action of the inverse pressure
/// Schur complement 
/// \f[
/// {\bf z}_p = - {\bf M}_s^{-1} {\bf r}_p
/// \f]
/// is approximated by
/// \f[
/// {\bf z}_p = - 
/// \big({\bf D} \widehat{\bf Q}^{-1}{\bf G} \big)^{-1}
/// \big({\bf D} \widehat{\bf Q}^{-1}{\bf F} \widehat{\bf Q}^{-1}{\bf G}\big) 
/// \big({\bf D} \widehat{\bf Q}^{-1}{\bf G} \big)^{-1}
/// {\bf r}_p,
/// \f]
/// where  \f$ \widehat{\bf Q} \f$ is the diagonal of the velocity
/// mass matrix. The evaluation of this expression involves
/// two linear solves involving the matrix
/// \f[
/// {\bf P} = \big({\bf D} \widehat{\bf Q}^{-1}{\bf G} \big)
/// \f]
/// which has the character of a matrix arising from the discretisation 
/// of a Poisson problem on the pressure space. We also have
/// to evaluate matrix-vector products with the matrix 
/// \f[
/// {\bf E}={\bf D}\widehat{\bf Q}^{-1}{\bf F}\widehat{\bf Q}^{-1}{\bf G}
/// \f]
/// Details of the theory can be found in "Finite Elements and 
/// Fast Iterative Solvers with Applications in Incompressible Fluid 
/// Dynamics" by Howard C. Elman, David J. Silvester, and Andrew J. Wathen,
/// published by Oxford University Press, 2006.
/// \n\n
/// In our implementation of the preconditioner, the linear systems
/// can either be solved "exactly", using SuperLU (in its incarnation
/// as an exact preconditioner; this is the default) or by any 
/// other Preconditioner (inexact solver) specified via the access functions
/// \code
/// NavierStokesLSCPreconditioner::set_f_preconditioner(...)
/// \endcode
/// or 
/// \code
/// NavierStokesLSCPreconditioner::set_p_preconditioner(...)
/// \endcode
//===========================================================================
  class NavierStokesLSCPreconditioner :
  public BlockPreconditioner<CRDoubleMatrix>
  {
   
   public :
    
    /// Constructor - sets defaults for control flags
    NavierStokesLSCPreconditioner() : BlockPreconditioner<CRDoubleMatrix>()
    {
     // Flag to indicate that the preconditioner has been setup
     // previously -- if setup() is called again, data can
     // be wiped.
     Preconditioner_has_been_setup = false;

     // Default method used in matrix-matrix multiplication (this seems
     // fastest on our machines)
     Mult_method = 2; // BAD: Must get rid of this magic number.
     
     // resize the mesh pt
     Mesh_pt.resize(1);
     Mesh_pt[0] = 0;

     // Set default preconditioners (inexact solvers) -- they are 
     // members of this class!
     P_preconditioner_pt = &P_superlu_preconditioner;
     F_preconditioner_pt = &F_superlu_preconditioner;

     // Flag to determine if velocity mass matrix diagonal Q^{-1}
     // is used for scaling.
     P_matrix_using_scaling = true;

     // set P_matrix_pt to null
     P_matrix_pt = 0;

     // set Doc_time to false
     Doc_time = false;
    }


   /// Destructor
   ~NavierStokesLSCPreconditioner()
    {
     clean_up_memory();
    }


   /// Broken copy constructor
   NavierStokesLSCPreconditioner(const NavierStokesLSCPreconditioner&)
    {
     BrokenCopy::broken_copy("NavierStokesLSCPreconditioner");
    }


   /// Broken assignment operator
   void operator=(const NavierStokesLSCPreconditioner&)
    {
     BrokenCopy::broken_assign("NavierStokesLSCPreconditioner");
    }


   /// Setup the preconditioner
   void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt);

   /// Apply preconditioner to Vector r
   void preconditioner_solve(const Vector<double>&r, Vector<double> &z);

   /// \short The number of "blocks" that degrees of freedom
   /// are sub-divided into: Two -- velocity and pressure
   unsigned nblock_types(){return 2;}

   /// \short Access function to mesh containing the block-preconditionable
   /// Navier-Stokes elements.
   Mesh*& navier_stokes_mesh_pt()
    {
     return Mesh_pt[0];
    }

   /// \short Flag which is true if velocity mass matrix diagonal
   /// scaling is used in the Schur complement approximation
   bool& p_matrix_using_scaling() {return P_matrix_using_scaling;}

   /// Method used in the matrix-matrix multiplications
   unsigned& mult_method() {return Mult_method;}

   /// Function to set a new pressure matrix preconditioner (inexact solver)
   void set_p_preconditioner(Preconditioner& new_p_preconditioner)
   {
    // If the default preconditioner has been used
    // clean it up now...
    P_superlu_preconditioner.clean_up_memory();
    P_preconditioner_pt = &new_p_preconditioner;
   }

   /// \short Function to (re-)set pressure matrix preconditioner  (inexact 
   /// solver) to SuperLU
   void set_p_superlu_preconditioner()
   {
    P_preconditioner_pt = &P_superlu_preconditioner;
   }

   /// Function to set a new momentum matrix preconditioner (inexact solver)
   void set_f_preconditioner(Preconditioner& new_f_preconditioner)
   {
    // If the default preconditioner has been used
    // clean it up now...
    F_superlu_preconditioner.clean_up_memory();
    F_preconditioner_pt = &new_f_preconditioner;
   }

   ///\short Function to (re-)set momentum matrix preconditioner (inexact 
   /// solver) to SuperLU
   void set_f_superlu_preconditioner()
   {
    F_preconditioner_pt = &F_superlu_preconditioner;
   }

   /// Access function for Doc_time
   bool& doc_time() {return Doc_time;}

   protected:


   // oomph-lib objects
   // -----------------

   /// Matrix containing pointers to the block matrices
   DenseMatrix<CRDoubleMatrix*> Block_matrix_pt;

   /// \short Matrix for multiplication in Schur complement approximation
   /// \c E_matrix \f$ =
   /// {\bf D}\widehat{\bf Q}^{-1}{\bf F}\widehat{\bf Q}^{-1}{\bf G} \f$
   /// (stored as a matrix rather than a pointer to a matrix,
   /// because it comes out of a matrix-matrix product in that form)
   CRDoubleMatrix E_matrix;

   /// \short Matrix for solve in Schur complement approximation:
   /// \c P_matrix_pt \f$ =  {\bf D}  \widehat{\bf Q}^{-1}  {\bf G} \f$
   /// (stored via a pointer because that's how we use it).
   CRDoubleMatrix* P_matrix_pt;

   // Pointers to preconditioner (=inexact solver) objects
   // -----------------------------------------------------
   /// Pointer to the 'preconditioner' for the pressure matrix
   Preconditioner* P_preconditioner_pt;

   /// Pointer to the 'preconditioner' for the F matrix
   Preconditioner* F_preconditioner_pt;

    private:

   /// \short Control flag is true if the preconditioner has been setup
   /// (used so we can wipe the data when the preconditioner is
   /// called again)
   bool Preconditioner_has_been_setup;

   /// Method to use in matrix-matrix multiplication
   unsigned Mult_method;

   /// \short Control flag is true if velocity mass matrix diagonal scaling
   /// is used in the Schur complement approximation
   bool P_matrix_using_scaling;

   /// \short Helper function to multiply a CRDoubleMatrix by a
   /// diagonal matrix held in diag_matrix. The input and output
   /// matrices are held in matrix.
   static void mat_diag_multiply(const Vector<double>& diag_matrix,
                                 CRDoubleMatrix& matrix);

   /// \short Helper function to assemble the diagonal of the velocity
   /// mass matrix from the elemental contributions defined in
   /// NavierStokesEquations<DIM>::get_velocity_mass_matrix_diagonal(...).
   void assemble_velocity_mass_matrix_diagonal(Vector<double> &vmm_diagonal);

   /// \short Helper function to delete preconditioner data.
   void clean_up_memory();

   /// \short Identify the required blocks: Here we only need
   /// the momentum, gradient and divergence blocks of the
   /// 2x2 block-structured matrix -- this function can be overloaded in
   /// derived preconditioners.
   virtual void identify_required_blocks(DenseMatrix<bool>& required_blocks);
   
   /// Default preconditioner (inexact solver) for P matrix
   SuperLU_Preconditioner P_superlu_preconditioner;
   
   /// Default preconditioner (inexact solver) for F matrix
   SuperLU_Preconditioner F_superlu_preconditioner;

   /// Set Doc_time to true for outputting results of timings
   bool Doc_time;

  };



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////



//============================================================================
/// \short The exact Navier Stokes preconditioner. This extracts 2x2 blocks
/// (corresponding to the velocity and pressure unknowns) and uses these to
/// build a single preconditioner matrix for testing purposes.
/// Iterative solvers should converge in a single step if this is used.
/// If it doesn't something is wrong in the setup of the block matrices.
//=============================================================================
 template<typename MATRIX>
  class NavierStokesExactPreconditioner : public BlockPreconditioner<MATRIX>
  {

   public :
     
    /// Constructor - do nothing
    NavierStokesExactPreconditioner() : BlockPreconditioner<MATRIX>(){}
   
   
   /// Destructor - do nothing
   ~NavierStokesExactPreconditioner(){}

   
   /// Broken copy constructor
   NavierStokesExactPreconditioner(const NavierStokesExactPreconditioner&)
    {
     BrokenCopy::broken_copy("NavierStokesExactPreconditioner");
    }
   

   /// Broken assignment operator
   void operator=(const NavierStokesExactPreconditioner&)
    {
     BrokenCopy::broken_assign("NavierStokesExactPreconditioner");
    }
   
   
   /// Setup the preconditioner
   void setup(Problem* problem_pt, DoubleMatrixBase* matrix_pt);
   
   /// Apply preconditioner to r
   void preconditioner_solve(const Vector<double>&r,
                             Vector<double> &z);

   protected :
    
    /// Preconditioner matrix
    MATRIX P_matrix;

  };
 
}
#endif
