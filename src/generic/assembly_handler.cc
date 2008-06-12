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
//OOOMPH-LIB includes
#include "assembly_handler.h"
#include "elements.h"
#include "problem.h"
#include "mesh.h"

namespace oomph
{
 ///////////////////////////////////////////////////////////////////////
 // Non-inline functions for the AssemblyHandler class
 //////////////////////////////////////////////////////////////////////


 //===================================================================
 ///Get the number of elemental degrees of freedom. Direct call
 ///to the function in the element.
 //===================================================================
 unsigned AssemblyHandler::ndof(GeneralisedElement* const &elem_pt)
 {return elem_pt->ndof();}

 //==================================================================
 ///Get the global equation number of the local unknown. Direct call
 ///to the function in the element.
 //==================================================================
 unsigned long AssemblyHandler::eqn_number(GeneralisedElement* const &elem_pt,
                                           const unsigned &ieqn_local)
 {return elem_pt->eqn_number(ieqn_local);}
 
 //==================================================================
 ///Get the residuals by calling the underlying element's residuals
 ///directly.
 //=================================================================
 void AssemblyHandler::get_residuals(GeneralisedElement* const &elem_pt,
                                     Vector<double> &residuals)
 {elem_pt->get_residuals(residuals);}
 
 //=======================================================================
 /// Calculate the elemental Jacobian matrix "d equation / d variable" by
 /// calling the element's get_jacobian function.
 //======================================================================
 void AssemblyHandler::get_jacobian(GeneralisedElement* const &elem_pt,
                                    Vector<double> &residuals, 
                                    DenseMatrix<double> &jacobian)
 {elem_pt->get_jacobian(residuals,jacobian);}
 
 //=======================================================================
 /// Calculate all desired vectors and matrices that are required by
 /// the element by calling the get_jacobian function
 //=======================================================================
 void AssemblyHandler::get_all_vectors_and_matrices(
  GeneralisedElement* const &elem_pt,
  Vector<Vector<double> >&vec, Vector<DenseMatrix<double> > &matrix)
 {
  get_jacobian(elem_pt,vec[0],matrix[0]);
 }

 ///////////////////////////////////////////////////////////////////////
 // Non-inline functions for the EigenProblemHandler class
 //////////////////////////////////////////////////////////////////////


 //===================================================================
 ///Get the number of elemental degrees of freedom. Direct call
 ///to the function in the element.
 //===================================================================
 unsigned EigenProblemHandler::ndof(GeneralisedElement* const &elem_pt)
 {return elem_pt->ndof();}

 //==================================================================
 ///Get the global equation number of the local unknown. Direct call
 ///to the function in the element.
 //==================================================================
 unsigned long EigenProblemHandler::
 eqn_number(GeneralisedElement* const &elem_pt,const unsigned &ieqn_local)
 {return elem_pt->eqn_number(ieqn_local);}
 
 //==================================================================
 ///Cannot call get_residuals for an eigenproblem, so throw an error
 //=================================================================
 void EigenProblemHandler::get_residuals(GeneralisedElement* const &elem_pt,
                                     Vector<double> &residuals)
 {
  throw OomphLibError(
   "An eigenproblem does not have a get_residuals function",
   "EigenProblemHandler::get_residuals()",
   OOMPH_EXCEPTION_LOCATION);
 }
 
 //=======================================================================
 ///Cannot call get_jacobian for an eigenproblem, so throw an error
 //======================================================================
 void EigenProblemHandler::get_jacobian(GeneralisedElement* const &elem_pt,
                                        Vector<double> &residuals, 
                                        DenseMatrix<double> &jacobian)
 {
  throw OomphLibError(
   "An eigenproblem does not have a get_jacobian function",
   "EigenProblemHandler::get_jacobian()",
   OOMPH_EXCEPTION_LOCATION);
 }

 
 //=======================================================================
 /// Calculate all desired vectors and matrices that are required by
 /// the problem  by calling those of the underlying element.
 //=======================================================================
 void EigenProblemHandler::get_all_vectors_and_matrices(
  GeneralisedElement* const &elem_pt,
  Vector<Vector<double> >&vec, Vector<DenseMatrix<double> > &matrix)
 {
#ifdef PARANOID
  //Check dimension
  if(matrix.size() != 2) 
   {
    throw OomphLibError("EigenProblems should return two matrices",
                        "EigenProblemHandler::get_all_vectors_and_matrices()",
                        OOMPH_EXCEPTION_LOCATION);
   }
#endif
  //Find the number of variables
  unsigned n_var = elem_pt->ndof();
  //Assign a dummy residuals vector
  Vector<double> dummy(n_var);
  //Get the jacobian and mass matrices
  elem_pt->get_jacobian_and_mass_matrix(dummy,matrix[0],matrix[1]);
  
  //If we have a non-zero shift, then shift the A matrix
  if(Sigma_real != 0.0)
   {
    //Set the shifted matrix
    for(unsigned i=0;i<n_var;i++)
     {
      for(unsigned j=0;j<n_var;j++)
       {
        matrix[0](i,j) -= Sigma_real*matrix[1](i,j);
       }
     }
   }
 }


 //======================================================================
 /// Clean up the memory that may have been allocated by the solver
 //=====================================================================
 BlockFoldLinearSolver::~BlockFoldLinearSolver()
 {
  if(Alpha_pt!=0) {delete Alpha_pt;}
  if(E_pt!=0) {delete E_pt;}
 }

 //===================================================================
 /// Use a block factorisation to solve the augmented system
 /// associated with a PitchFork bifurcation.
 //===================================================================
 void BlockFoldLinearSolver:: solve(Problem* const &problem_pt, 
                                    Vector<double> &result)
 {
  //Locally cache the pointer to the handler.
  FoldHandler* handler_pt =
   static_cast<FoldHandler*>(problem_pt->assembly_handler_pt());

  //Switch the handler to "block solver" mode
  handler_pt->solve_block_system();

  //We need to find out the number of dofs in the problem
  unsigned n_dof = problem_pt->ndof();
  
  //Setup storage for temporary vectors
  Vector<double> a(n_dof), b(n_dof);

  //Allocate storage for Alpha which can be used in the resolve
  if(Alpha_pt!=0) {delete Alpha_pt;}
  Alpha_pt = new Vector<double>(n_dof);

  //We are going to do resolves using the underlying linear solver
  Linear_solver_pt->enable_resolve();

  //Solve the first system Aa = R
  Linear_solver_pt->solve(problem_pt,a);

  //The vector in the top right-hand block is the jacobian multiplied
  //by the null vector
  
  //Get the present null vector from the handler
  Vector<double> y(n_dof);
  for(unsigned n=0;n<(n_dof-1);++n)  {y[n] = handler_pt->Y[n];}
  //For simplicity later, add a zero at the end
  y[n_dof-1] = 0.0;

  //Loop over the elements and assemble the product
  //Local storage for the product terms
  Vector<double> Jy(n_dof,0.0);

  //Calculate the product of the jacobian matrices, etc
  unsigned long n_element = problem_pt->mesh_pt()->nelement();
  for(unsigned long e = 0;e<n_element;e++)
   {
    GeneralisedElement *elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the ndofs in each element
    unsigned n_var = elem_pt->ndof();
    //Get the jacobian matrix
    DenseMatrix<double> jac(n_var);
    //Get unperturbed raw jacobian
    elem_pt->get_jacobian(b,jac);
        
    //Multiply the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      for(unsigned m=0;m<n_var;m++)
       {
        unsigned unknown = elem_pt->eqn_number(m);
        Jy[eqn_number] += jac(n,m)*y[unknown];
       }
     }
   }
  //The final entry of the vector will be zero

  //Now resolve to find alpha
  Linear_solver_pt->resolve(Jy,*Alpha_pt);

  //The vector that multiplie the product matrix is actually y - alpha
  Vector<double> y_minus_alpha(n_dof);
  for(unsigned n=0;n<n_dof;n++)
   {
    y_minus_alpha[n] = y[n] - (*Alpha_pt)[n];
   }

  //We can now construct our multipliers
  //Prepare to scale
  double dof_length=0.0, a_length=0.0, alpha_length=0.0;
  for(unsigned n=0;n<n_dof;n++)
   {
    if(std::abs(problem_pt->dof(n)) > dof_length) 
     {dof_length = std::abs(problem_pt->dof(n));}
    if(std::abs(a[n]) > a_length) {a_length = std::abs(a[n]);}
    if(std::abs(y_minus_alpha[n]) > alpha_length) 
     {alpha_length = std::abs(y_minus_alpha[n]);}
   }

  double a_mult = dof_length/a_length;
  double alpha_mult = dof_length/alpha_length;
  const double FD_step = 1.0e-8;
  a_mult += FD_step; alpha_mult += FD_step;
  a_mult *= FD_step; alpha_mult *= FD_step;

  //Local storage for the product terms
  Vector<double> Jprod_a(n_dof,0.0), Jprod_alpha(n_dof,0.0);

  //Calculate the product of the jacobian matrices, etc
  for(unsigned long e = 0;e<n_element;e++)
   {
    GeneralisedElement *elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the ndofs in each element
    unsigned n_var = handler_pt->ndof(elem_pt);
    //Get the jacobian matrices
    DenseMatrix<double> jac(n_var), jac_a(n_var), jac_alpha(n_var);
    //Get unperturbed jacobian
    handler_pt->get_jacobian(elem_pt,b,jac);
    
    //Backup the dofs
    Vector<double> dof_bac(n_var);
    //Perturb the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      dof_bac[n] = problem_pt->dof(eqn_number);
      //Pertub by vector a
      problem_pt->dof(eqn_number) += a_mult*a[eqn_number]; 
     }
    
    problem_pt->actions_after_change_in_bifurcation_parameter();
    
    //Now get the new jacobian
    handler_pt->get_jacobian(elem_pt,b,jac_a);
    
    //Perturb the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      problem_pt->dof(eqn_number) = dof_bac[n];
      //Pertub by vector a
      problem_pt->dof(eqn_number) += alpha_mult*y_minus_alpha[eqn_number]; 
     }
    
    problem_pt->actions_after_change_in_bifurcation_parameter();
    
    //Now get the new jacobian
    handler_pt->get_jacobian(elem_pt,b,jac_alpha);
    
    //Reset the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      problem_pt->dof(eqn_number) = dof_bac[n];
     }
    
    problem_pt->actions_after_change_in_bifurcation_parameter();
    
    //OK, now work out the products
    //Note the (n_var-1), we are only interested in the non-augmented
    //jacobian
    for(unsigned n=0;n<(n_var-1);n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      double prod_a=0.0, prod_alpha=0.0;
      for(unsigned m=0;m<(n_var-1);m++)
       {
        unsigned unknown = handler_pt->eqn_number(elem_pt,m);
        prod_a += (jac_a(n,m) - jac(n,m))*y[unknown];
        prod_alpha += (jac_alpha(n,m) - jac(n,m))*y[unknown];
       }
      Jprod_a[eqn_number] += prod_a/a_mult;
      Jprod_alpha[eqn_number] += prod_alpha/alpha_mult;
     }
   }

   Jprod_alpha[n_dof-1] = 0.0;
   Jprod_a[n_dof-1] = 0.0;

   //OK, now we can formulate the next vectors
   //The assumption here is that the result has been set to the
   //residuals.
   for(unsigned n=0;n<n_dof-1;n++)
    {
     b[n] = result[n_dof+n] - Jprod_a[n];
    }
   //The final residual is the entry corresponding to the original parameter
   b[n_dof-1] = result[n_dof-1];
   
   //Allocate storage for E which can be used in the resolve
   if(E_pt!=0) {delete E_pt;}
   E_pt = new Vector<double>(n_dof);

   Vector<double> f(n_dof);

   Linear_solver_pt->resolve(b,f);
   Linear_solver_pt->resolve(Jprod_alpha,*E_pt);

   //Calculate the final entry in the vector e
   const double e_final = (*E_pt)[n_dof-1];
   //Calculate the final entry in the vector d
   const double d_final = f[n_dof-1]/e_final;
   //Assemble the final corrections
   for(unsigned n=0;n<n_dof-1;n++)
    {
     result[n] = a[n] - (*Alpha_pt)[n]*d_final + d_final*y[n];
     result[n_dof+n] = f[n] - (*E_pt)[n]*d_final;
    }
   //The result corresponding to the parameter
   result[n_dof-1] = a[n_dof-1] - (*Alpha_pt)[n_dof-1]*d_final;

   //The sign of the jacobian is the sign of the final entry in e
   problem_pt->sign_of_jacobian() = 
    static_cast<int>(std::abs(e_final)/e_final);
   
   //Switch things to our block solver
   handler_pt->solve_full_system();

   //If we are not storing things, clear the results
   if(!Enable_resolve) 
    {
     //We no longer need to store the matrix
     Linear_solver_pt->disable_resolve();
     delete Alpha_pt; Alpha_pt=0;
     delete E_pt; E_pt=0;
    }
   //Otherwise, also store the problem pointer
   else
    {
     Problem_pt = problem_pt;
    }
 }


 //======================================================================
 //Hack the re-solve to use the block factorisation
 //======================================================================
 void BlockFoldLinearSolver::resolve(const Vector<double> & rhs,
                                     Vector<double> &result)
 {
  //Check that the factors have been stored
  if(Alpha_pt==0)
   {
    throw OomphLibError("The required vectors have not been stored",
                        "BlockFoldLinearSolver::resolve()",
                        OOMPH_EXCEPTION_LOCATION);
   }
  
  //Get the pointer to the problem
  Problem* const problem_pt = Problem_pt;

  FoldHandler* handler_pt =
   static_cast<FoldHandler*>(problem_pt->assembly_handler_pt());
  
  //Switch things to our block solver
  handler_pt->solve_block_system();
  //We need to find out the number of dofs
  unsigned n_dof = problem_pt->ndof();
  
  //Setup storage
  Vector<double> a(n_dof), b(n_dof);
  
  //Set the values of the a vector
  for(unsigned n=0;n<(n_dof-1);n++) {a[n] = rhs[n];}
  //The entry associated with the additional parameter is zero
  a[n_dof-1] = 0.0;
  
  Linear_solver_pt->enable_resolve();

  // Copy rhs vector into local storage so it doesn't get overwritten
  // if the linear solver decides to initialise the solution vector, say,
  // which it's quite entitled to do!
  Vector<double> input_a(a);
  
  Linear_solver_pt->resolve(input_a,a);
  
  //We can now construct our multipliers
  //Prepare to scale
  double dof_length=0.0, a_length=0.0;
  for(unsigned n=0;n<n_dof;n++)
   {
    if(std::abs(problem_pt->dof(n)) > dof_length) 
     {dof_length = std::abs(problem_pt->dof(n));}
    
    if(std::abs(a[n]) > a_length) {a_length = std::abs(a[n]);}
   }
  double a_mult = dof_length/a_length;
  const double FD_step = 1.0e-8;
  a_mult += FD_step; 
  a_mult *= FD_step;
  
  Vector<double> Jprod_a(n_dof,0.0);
  
  unsigned long n_element = problem_pt->mesh_pt()->nelement();
   for(unsigned long e = 0;e<n_element;e++)
    {
     GeneralisedElement *elem_pt = problem_pt->mesh_pt()->element_pt(e);
     //Loop over the ndofs in each element
     unsigned n_var = handler_pt->ndof(elem_pt);
     //Get some jacobian matrices
     DenseMatrix<double> jac(n_var), jac_a(n_var);
     //Get unperturbed jacobian
     handler_pt->get_jacobian(elem_pt,b,jac);
   
     //Backup the dofs
     Vector<double> dof_bac(n_var);
     //Perturb the dofs
     for(unsigned n=0;n<n_var;n++)
      {
       unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
       dof_bac[n] = problem_pt->dof(eqn_number);
       //Pertub by vector a
       problem_pt->dof(eqn_number) += a_mult*a[eqn_number]; 
      }

     problem_pt->actions_after_change_in_bifurcation_parameter();

     //Now get the new jacobian
     handler_pt->get_jacobian(elem_pt,b,jac_a);
     
     //Reset the dofs
     for(unsigned n=0;n<n_var;n++)
      {
       unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
       problem_pt->dof(eqn_number) = dof_bac[n];
      }

     problem_pt->actions_after_change_in_bifurcation_parameter();
 
     //OK, now work out the products
     for(unsigned n=0;n<(n_var-1);n++)
      {
       unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
       double prod_a=0.0;
       for(unsigned m=0;m<(n_var-1);m++)
        {
         unsigned unknown = handler_pt->eqn_number(elem_pt,m);
         prod_a += (jac_a(n,m) - jac(n,m))*handler_pt->Y[unknown];
        }
       Jprod_a[eqn_number] += prod_a/a_mult;
      }
    }

   Jprod_a[n_dof-1] = 0.0;
   
   //OK, now we can formulate the next vectors
   for(unsigned n=0;n<n_dof-1;n++)
    {
     b[n] = rhs[n_dof+n] - Jprod_a[n];
    }
   //The residuals for the final entry should be those associated
   //with the parameter
   b[n_dof-1] = rhs[n_dof-1];

   Vector<double> f(n_dof);

   Linear_solver_pt->resolve(b,f);

   //Calculate the final entry in the vector d
   const double d_final = f[n_dof-1]/(*E_pt)[n_dof-1];
   //Assemble the final corrections
   for(unsigned n=0;n<n_dof-1;n++)
    {
     result[n] = a[n] - (*Alpha_pt)[n]*d_final + d_final*handler_pt->Y[n];
     result[n_dof+n] = f[n] - (*E_pt)[n]*d_final;
    }
   //The result corresponding to the paramater
   result[n_dof-1] = a[n_dof-1] - (*Alpha_pt)[n_dof-1]*d_final;

   Linear_solver_pt->disable_resolve();

   //Switch things to our block solver
   handler_pt->solve_full_system();
  }


 ///////////////////////////////////////////////////////////////////////
 // Non-inline functions for the FoldHandler class
 //////////////////////////////////////////////////////////////////////


 //========================================================================
 /// Constructor: Initialise the fold handler, 
 /// by setting initial guesses for Y, Phi and calculating Count. 
 /// If the system changes, a new  handler must be constructed.
 //========================================================================
 FoldHandler::FoldHandler(Problem* const &problem_pt, 
                          double* const &parameter_pt) : 
  Solve_block_system(false)
 {
  //Set the problem pointer
  Problem_pt = problem_pt;
  //Set the number of degrees of freedom
  Ndof = problem_pt->ndof();

  //Resize the vectors of additional dofs and constants
  Phi.resize(Ndof);
  Y.resize(Ndof);
  Count.resize(Ndof,0);
  
  //Loop over all the elements in the problem
  unsigned n_element = problem_pt->mesh_pt()->nelement();
  for(unsigned e=0;e<n_element;e++)
   {
    GeneralisedElement* elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the local freedoms in the element
    unsigned n_var = elem_pt->ndof();
    for(unsigned n=0;n<n_var;n++)
     {
      //Increase the associated global equation number counter
      ++Count[elem_pt->eqn_number(n)];
     }
   }
  
  //Calculate the value Phi by
  //solving the system JPhi = dF/dlambda
  
  //Locally cache the linear solver
  LinearSolver* const linear_solver_pt = problem_pt->linear_solver_pt();

  //Save the status before entry to this routine
  bool enable_resolve = linear_solver_pt->resolve_is_enabled();

  //We need to do a resolve
  linear_solver_pt->enable_resolve();

  //Storage for the solution
  Vector<double> x(Ndof);

  //Solve the standard problem, we only want to make sure that
  //we factorise the matrix, if it has not been factorised. We shall
  //ignore the return value of x.
  linear_solver_pt->solve(problem_pt,x);

  //Get the vector dresiduals/dparameter
  problem_pt->get_derivative_wrt_global_parameter(parameter_pt,x);


  // Copy rhs vector into local storage so it doesn't get overwritten
  // if the linear solver decides to initialise the solution vector, say,
  // which it's quite entitled to do!
  Vector<double> input_x(x);

  //Now resolve the system with the new RHS and overwrite the solution
  linear_solver_pt->resolve(input_x,x);

  //Restore the storage status of the linear solver
  if (enable_resolve)
   {
    linear_solver_pt->enable_resolve();
   }
  else
   {
    linear_solver_pt->disable_resolve();
   }

  //Add the global parameter as an unknown to the problem
  problem_pt->Dof_pt.push_back(parameter_pt);
  
  //Normalise the initial guesses for phi
  double length = 0.0;
  for(unsigned n=0;n<Ndof;n++) {length += x[n]*x[n];}
  length = sqrt(length);
  
  //Now add the null space components to the problem unknowns
  //and initialise them and Phi to the same normalised values
  for(unsigned n=0;n<Ndof;n++)
   {
    problem_pt->Dof_pt.push_back(&Y[n]);
    Y[n] = Phi[n] = -x[n]/length;
   }
 }


 //=================================================================
 /// The number of unknowns is 2n+1
 //================================================================
 unsigned FoldHandler::ndof(GeneralisedElement* const &elem_pt)
 {
  unsigned raw_ndof = elem_pt->ndof();
  if(Solve_block_system) {return (raw_ndof+1);}
  else {return (2*raw_ndof + 1);}
 }
 
 //=====================================================================
 /// Return the global equation number associated with local equation
 /// number ieqn_local. We choose to number the unknowns according
 /// to the augmented system.
 //=======================================================================
 unsigned long FoldHandler::eqn_number(GeneralisedElement* const &elem_pt,
                                       const unsigned &ieqn_local)
 {
  //Find the number of non-augmented dofs in the element
  unsigned raw_ndof = elem_pt->ndof();
  //Storage for the global eqn number
  unsigned long global_eqn=0;
  //If we are a "standard" unknown, just return the standard equation number
  if(ieqn_local < raw_ndof) 
   {
    global_eqn = elem_pt->eqn_number(ieqn_local);
   }
  //Otherwise if we are at an unknown corresponding to the bifurcation 
  //parameter return the global equation number of the parameter
  else if(ieqn_local==raw_ndof)
   {
    global_eqn = Ndof;
   }
  //Otherwise we are in the unknown corresponding to a null vector
  //return the global unknown Ndof + 1 + global unknown of "standard" unknown.
  else
   {
    global_eqn = Ndof + 1 + elem_pt->eqn_number(ieqn_local - 1 - raw_ndof);
   }

  //Return the global equation number
  return global_eqn;
 }
 
 //====================================================================
 ///Formulate the augmented system
 //====================================================================
 void FoldHandler::get_residuals(GeneralisedElement* const &elem_pt,
                                 Vector<double> &residuals)
 {
  //Need to get raw residuals and jacobian
  unsigned raw_ndof = elem_pt->ndof();

  //If we are solving the block system
  if(Solve_block_system)
   {
    //Get the basic residuals
    elem_pt->get_residuals(residuals);
  
    //Zero the final residual
    residuals[raw_ndof] = 0.0;
   }
  else
   {
    DenseMatrix<double> jacobian(raw_ndof);
    //Get the basic residuals and jacobian initially
    elem_pt->get_jacobian(residuals,jacobian);
    
    //The normalisation equation must be initialised to -1.0/number of elements
    residuals[raw_ndof] = -1.0/Problem_pt->mesh_pt()->nelement();
    
    //Now assemble the equations Jy = 0
    for(unsigned i=0;i<raw_ndof;i++)
     {
      residuals[raw_ndof+1+i] = 0.0;
      for(unsigned j=0;j<raw_ndof;j++)
       {
        residuals[raw_ndof+1+i] += jacobian(i,j)*Y[elem_pt->eqn_number(j)];
       }
      //Add the contribution to phi.y=1
      //Need to divide by the number of elements that contribute to this
      //unknown so that we do get phi.y exactly.
      unsigned global_eqn = elem_pt->eqn_number(i);
      residuals[raw_ndof] += 
       (Phi[global_eqn]*Y[global_eqn])/Count[global_eqn];
     }
   }
 }
    
  
 //=================================================================
 /// Calculate the elemental Jacobian matrix "d equation 
 /// / d variable" for the augmented system
 //=================================================================
 void FoldHandler::get_jacobian(GeneralisedElement* const &elem_pt,
                                Vector<double> &residuals, 
                                DenseMatrix<double> &jacobian)
 {

  //Find the number of augmented dofs
  unsigned augmented_ndof = ndof(elem_pt);
  //Find the non-augmented dofs
  unsigned raw_ndof = elem_pt->ndof();
  
  //If solving the block system
  if(Solve_block_system)
   {
    //Get the full residuals, we need them
    get_residuals(elem_pt,residuals);
    
    //Need to get the raw jacobian (and raw residuals)
    Vector<double> newres(augmented_ndof);
    elem_pt->get_jacobian(newres,jacobian);
      
    //Now do finite differencing stuff
    const double FD_step = 1.0e-8;
    //Fill in the first lot of finite differences
    {
     //increase the global parameter
     double *unknown_pt = Problem_pt->Dof_pt[Ndof];
     double init = *unknown_pt;
     *unknown_pt += FD_step;
     
     //Not do any possible updates
     Problem_pt
      ->actions_after_change_in_bifurcation_parameter();
     
     //Get the new (modified) residuals
     get_residuals(elem_pt,newres);
     
     //The final column  is given by the difference
     //between the residuals
     for(unsigned n=0;n<raw_ndof;n++)
      {
       jacobian(n,augmented_ndof-1) = (newres[n] - residuals[n])/FD_step;
      }
     //Reset the global parameter
     *unknown_pt = init;
     
     //Now do any possible updates
     Problem_pt
      ->actions_after_change_in_bifurcation_parameter();
    }
      
    //Fill in the bottom row
    for(unsigned n=0;n<raw_ndof;n++)
     {
      unsigned local_eqn = elem_pt->eqn_number(n);
      jacobian(augmented_ndof-1,n) 
       = Phi[local_eqn]/Count[local_eqn];
     }
   }
  //Otherwise solving the full system
  else
   {
    //Get the residuals for the augmented system
    get_residuals(elem_pt,residuals);
    
    //Need to get the raw residuals and jacobian
    Vector<double> newres(raw_ndof);
    DenseMatrix<double> newjac(raw_ndof);
    elem_pt->get_jacobian(newres,jacobian);
    
    //Fill in the jacobian on the diagonal sub-block of 
    //the null-space equations
    for(unsigned n=0;n<raw_ndof;n++)
     {
      for(unsigned m=0;m<raw_ndof;m++)
       {
        jacobian(raw_ndof+1+n,raw_ndof+1+m) = jacobian(n,m);
       }
     }
    
    //Now finite difference wrt the global unknown
    const double FD_step = 1.0e-8;
    //Fill in the first lot of finite differences
    {
   //increase the global parameter
     double *unknown_pt = Problem_pt->Dof_pt[Ndof];
     double init = *unknown_pt;
     *unknown_pt += FD_step;
     //Need to update the function
     Problem_pt->actions_after_change_in_bifurcation_parameter();
     
     //Get the new raw residuals and jacobian
     elem_pt->get_jacobian(newres,newjac);
     
     //The end of the first row is given by the difference
     //between the residuals
     for(unsigned n=0;n<raw_ndof;n++)
      {
       jacobian(n,raw_ndof) = (newres[n] - residuals[n])/FD_step;
       //The end of the second row is given by the difference multiplied by
       //the product
       for(unsigned l=0;l<raw_ndof;l++)
        {
         jacobian(raw_ndof+1+n,raw_ndof)
          += (newjac(n,l) - jacobian(n,l))*Y[elem_pt->eqn_number(l)]/
          FD_step;
        }
      }
     //Reset the global parameter
     *unknown_pt = init;
     
     //Need to update the function
     Problem_pt->actions_after_change_in_bifurcation_parameter();
    }
    
    //Now fill in the first column of the second rows
    {
     for(unsigned n=0;n<raw_ndof;n++)
      {
       unsigned long global_eqn = eqn_number(elem_pt,n);
       //Increase the first lot
       double* unknown_pt = Problem_pt->Dof_pt[global_eqn];
       double init = *unknown_pt;
       *unknown_pt += FD_step;
       
       //Get the new jacobian
       elem_pt->get_jacobian(newres,newjac);
       
       //Work out the differences
       for(unsigned k=0;k<raw_ndof;k++)
        {
         //jacobian(raw_ndof+k,n) = 0.0;
         for(unsigned l=0;l<raw_ndof;l++)
          {
           jacobian(raw_ndof+1+k,n)
            += (newjac(k,l) - jacobian(k,l))*Y[elem_pt->eqn_number(l)]/
            FD_step;
          }
        }
       *unknown_pt = init;
      }
    }
    
    //Fill in the row corresponding to the parameter
    for(unsigned n=0;n<raw_ndof;n++)
     {
      unsigned global_eqn = elem_pt->eqn_number(n);
      jacobian(raw_ndof,raw_ndof+1+n) 
       = Phi[global_eqn]/Count[global_eqn];
     }

  //   //Loop over the dofs
//     for(unsigned n=0;n<augmented_ndof;n++)
//      {
//       unsigned long global_eqn = eqn_number(elem_pt,n);
//       double* unknown_pt = Problem_pt->Dof_pt[global_eqn];
//       double init = *unknown_pt;
//       *unknown_pt += FD_step;
             
//       //Get the new residuals
//       get_residuals(elem_pt,newres);

//       for(unsigned m=0;m<augmented_ndof;m++)
//        {
//         jacobian(m,n) = (newres[m] - residuals[m])/FD_step;
//        }
//       //Reset the unknown
//       *unknown_pt = init;
//      }
   }
 }
 
 //=======================================================================
 /// Destructor return the problem to its original (non-augmented) state
 //=======================================================================
 FoldHandler::~FoldHandler()
 {
  //If we are using the block solver reset the problem's linear solver 
  //to the original one
  BlockFoldLinearSolver *block_fold_solver_pt
   = dynamic_cast<BlockFoldLinearSolver*>(Problem_pt->linear_solver_pt());
  if(block_fold_solver_pt)
   {
    //Reset the problem's linear solver
    Problem_pt->linear_solver_pt() = block_fold_solver_pt->linear_solver_pt();
    //Delete the block solver
    delete block_fold_solver_pt;
   }

  //Resize the number of dofs
  Problem_pt->Dof_pt.resize(Ndof);
 }

 //====================================================================
 /// Set to solve the block system
 //===================================================================
 void FoldHandler::solve_block_system()
 {
  if(!Solve_block_system)
   {
    Solve_block_system = true;
    //Restrict the problem to the standard variables and
    //the bifurcation parameter only
    Problem_pt->Dof_pt.resize(Ndof+1);
   }
 }

 //=================================================================
 /// Set to Solve non-block system
 //=================================================================
 void FoldHandler::solve_full_system()
 {
  if(Solve_block_system)
   {
    Solve_block_system = false;
    
    //Add the additional unknowns back into the problem
    for(unsigned n=0;n<Ndof;n++)
     {
      Problem_pt->Dof_pt.push_back(&Y[n]);
     }
   }
 }

 //======================================================================
 /// Clean up the memory that may have been allocated by the solver
 //=====================================================================
 BlockPitchForkLinearSolver::~BlockPitchForkLinearSolver()
 {
  if(Alpha_pt!=0) {delete Alpha_pt;}
  if(E_pt!=0) {delete E_pt;}
 }

 //===================================================================
 /// Use a block factorisation to solve the augmented system
 /// associated with a PitchFork bifurcation.
 //===================================================================
 void BlockPitchForkLinearSolver:: solve(Problem* const &problem_pt, 
                                         Vector<double> &result)
 {
  //Locally cache the pointer to the handler.
  PitchForkHandler* handler_pt =
   static_cast<PitchForkHandler*>(problem_pt->assembly_handler_pt());

  //Switch the handler to "block solver" mode
  handler_pt->solve_block_system();

  //We need to find out the number of dofs in the problem
  unsigned n_dof = problem_pt->ndof();
  
  //Setup storage for temporary vectors
  Vector<double> a(n_dof), b(n_dof);

  //Allocate storage for Alpha which can be used in the resolve
  if(Alpha_pt!=0) {delete Alpha_pt;}
  Alpha_pt = new Vector<double>(n_dof);

  //We are going to do resolves using the underlying linear solver
  Linear_solver_pt->enable_resolve();
  //Solve the first system Aa = R
  Linear_solver_pt->solve(problem_pt,a);

  //Get the symmetry vector from the handler
  Vector<double> psi(n_dof);
  for(unsigned n=0;n<(n_dof-1);++n)  {psi[n] = handler_pt->Psi[n];}
  //Set the final entry to zero
  psi[n_dof-1] = 0.0;

  //Now resolve to find alpha
  Linear_solver_pt->resolve(psi,*Alpha_pt);

  //We can now construct our multipliers
  //Prepare to scale
  double dof_length=0.0, a_length=0.0, alpha_length=0.0;
  for(unsigned n=0;n<n_dof;n++)
   {
    if(std::abs(problem_pt->dof(n)) > dof_length) 
     {dof_length = std::abs(problem_pt->dof(n));}
    if(std::abs(a[n]) > a_length) {a_length = std::abs(a[n]);}
    if(std::abs((*Alpha_pt)[n]) > alpha_length) 
     {alpha_length = std::abs((*Alpha_pt)[n]);}
   }

  double a_mult = dof_length/a_length;
  double alpha_mult = dof_length/alpha_length;
  const double FD_step = 1.0e-8;
  a_mult += FD_step; alpha_mult += FD_step;
  a_mult *= FD_step; alpha_mult *= FD_step;

  //Local storage for the product terms
  Vector<double> Jprod_a(n_dof,0.0), Jprod_alpha(n_dof,0.0);

  //Calculate the product of the jacobian matrices, etc
  unsigned long n_element = problem_pt->mesh_pt()->nelement();
  for(unsigned long e = 0;e<n_element;e++)
   {
    GeneralisedElement *elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the ndofs in each element
    unsigned n_var = handler_pt->ndof(elem_pt);
    //Get the jacobian matrices
    DenseMatrix<double> jac(n_var), jac_a(n_var), jac_alpha(n_var);
    //Get unperturbed jacobian
    handler_pt->get_jacobian(elem_pt,b,jac);
    
    //Backup the dofs
    Vector<double> dof_bac(n_var);
    //Perturb the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      dof_bac[n] = problem_pt->dof(eqn_number);
      //Pertub by vector a
      problem_pt->dof(eqn_number) += a_mult*a[eqn_number]; 
     }
    
    problem_pt->actions_after_change_in_bifurcation_parameter();
    
    //Now get the new jacobian
    handler_pt->get_jacobian(elem_pt,b,jac_a);
    
    //Perturb the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      problem_pt->dof(eqn_number) = dof_bac[n];
      //Pertub by vector a
      problem_pt->dof(eqn_number) += alpha_mult*(*Alpha_pt)[eqn_number]; 
     }
    
    problem_pt->actions_after_change_in_bifurcation_parameter();
    
    //Now get the new jacobian
    handler_pt->get_jacobian(elem_pt,b,jac_alpha);
    
    //Reset the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      problem_pt->dof(eqn_number) = dof_bac[n];
     }
    
    problem_pt->actions_after_change_in_bifurcation_parameter();
    
    //OK, now work out the products
    for(unsigned n=0;n<(n_var-1);n++)
     {
      unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
      double prod_a=0.0, prod_alpha=0.0;
      for(unsigned m=0;m<(n_var-1);m++)
       {
        unsigned unknown = handler_pt->eqn_number(elem_pt,m);
        prod_a += (jac_a(n,m) - jac(n,m))*
         handler_pt->Y[unknown];
        prod_alpha += (jac_alpha(n,m) - jac(n,m))*
         handler_pt->Y[unknown];
       }
      Jprod_a[eqn_number] += prod_a/a_mult;
      Jprod_alpha[eqn_number] += prod_alpha/alpha_mult;
     }
   }

   Jprod_alpha[n_dof-1] = 0.0;
   Jprod_a[n_dof-1] = 0.0;

   //OK, now we can formulate the next vectors
   //The assumption here is that the result has been set to the
   //residuals.
   for(unsigned n=0;n<n_dof-1;n++)
    {
     b[n] = result[n_dof+n] - Jprod_a[n];
    }
   b[n_dof-1] = result[2*n_dof-1];

   
   //Allocate storage for E which can be used in the resolve
   if(E_pt!=0) {delete E_pt;}
   E_pt = new Vector<double>(n_dof);

   Vector<double> f(n_dof);

   Linear_solver_pt->resolve(b,f);
   Linear_solver_pt->resolve(Jprod_alpha,*E_pt);

   //Calculate the final entry in the vector e
   const double e_final = (*E_pt)[n_dof-1];
   //Calculate the final entry in the vector d
   const double d_final = -f[n_dof-1]/e_final;
   //Assemble the final corrections
   for(unsigned n=0;n<n_dof-1;n++)
    {
     result[n] = a[n] - (*Alpha_pt)[n]*d_final;
     result[n_dof+n] = f[n] + (*E_pt)[n]*d_final;
    }

   result[n_dof-1] = a[n_dof-1] - (*Alpha_pt)[n_dof-1]*d_final;
   result[2*n_dof-1] = d_final;

   //The sign of the jacobian is the sign of the final entry in e
   problem_pt->sign_of_jacobian() = 
    static_cast<int>(std::abs(e_final)/e_final);

   
   //Switch things to our block solver
   handler_pt->solve_full_system();

   //If we are not storing things, clear the results
   if(!Enable_resolve) 
    {
     //We no longer need to store the matrix
     Linear_solver_pt->disable_resolve();
     delete Alpha_pt; Alpha_pt=0;
     delete E_pt; E_pt=0;
    }
   //Otherwise also store the pointer to the problem
   else
    {
     Problem_pt = problem_pt;
    }
 }


 //==============================================================
 //Hack the re-solve to use the block factorisation
 //==============================================================
 void BlockPitchForkLinearSolver::resolve(const Vector<double> & rhs,
                                          Vector<double> &result)
 {
  //Check that the factors have been stored
  if(Alpha_pt==0)
   {
    throw OomphLibError("The required vectors have not been stored",
                        "BlockPitchForkLinearSolver::resolve()",
                        OOMPH_EXCEPTION_LOCATION);
   }
  
  //Get the pointer to the problem
  Problem* const problem_pt = Problem_pt;

  PitchForkHandler* handler_pt =
   static_cast<PitchForkHandler*>(problem_pt->assembly_handler_pt());
  
  //Switch things to our block solver
  handler_pt->solve_block_system();
  //We need to find out the number of dofs
  unsigned n_dof = problem_pt->ndof();
  
  //Setup storage
  Vector<double> a(n_dof), b(n_dof);
  
  //Set the values of the a vector
  for(unsigned n=0;n<n_dof;n++) {a[n] = rhs[n];}
  
  Linear_solver_pt->enable_resolve();

  // Copy rhs vector into local storage so it doesn't get overwritten
  // if the linear solver decides to initialise the solution vector, say,
  // which it's quite entitled to do!
  Vector<double> input_a(a);

  Linear_solver_pt->resolve(input_a,a);
  
  //We can now construct our multipliers
  //Prepare to scale
  double dof_length=0.0, a_length=0.0;
  for(unsigned n=0;n<n_dof;n++)
   {
    if(std::abs(problem_pt->dof(n)) > dof_length) 
     {dof_length = std::abs(problem_pt->dof(n));}
    
    if(std::abs(a[n]) > a_length) {a_length = std::abs(a[n]);}
   }
  double a_mult = dof_length/a_length;
  const double FD_step = 1.0e-8;
  a_mult += FD_step; 
  a_mult *= FD_step;
  
  Vector<double> Jprod_a(n_dof,0.0);
  
  unsigned long n_element = problem_pt->mesh_pt()->nelement();
   for(unsigned long e = 0;e<n_element;e++)
    {
     GeneralisedElement *elem_pt = problem_pt->mesh_pt()->element_pt(e);
     //Loop over the ndofs in each element
     unsigned n_var = handler_pt->ndof(elem_pt);
     //Get some jacobian matrices
     DenseMatrix<double> jac(n_var), jac_a(n_var);
     //Get unperturbed jacobian
     handler_pt->get_jacobian(elem_pt,b,jac);
   
     //Backup the dofs
     Vector<double> dof_bac(n_var);
     //Perturb the dofs
     for(unsigned n=0;n<n_var;n++)
      {
       unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
       dof_bac[n] = problem_pt->dof(eqn_number);
       //Pertub by vector a
       problem_pt->dof(eqn_number) += a_mult*a[eqn_number]; 
      }

     problem_pt->actions_after_change_in_bifurcation_parameter();

     //Now get the new jacobian
     handler_pt->get_jacobian(elem_pt,b,jac_a);
     
     //Reset the dofs
     for(unsigned n=0;n<n_var;n++)
      {
       unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
       problem_pt->dof(eqn_number) = dof_bac[n];
      }

     problem_pt->actions_after_change_in_bifurcation_parameter();
 
     //OK, now work out the products
     for(unsigned n=0;n<(n_var-1);n++)
      {
       unsigned eqn_number = handler_pt->eqn_number(elem_pt,n);
       double prod_a=0.0;
       for(unsigned m=0;m<(n_var-1);m++)
        {
         unsigned unknown = handler_pt->eqn_number(elem_pt,m);
         prod_a += (jac_a(n,m) - jac(n,m))*
          handler_pt->Y[unknown];
        }
       Jprod_a[eqn_number] += prod_a/a_mult;
      }
    }

   Jprod_a[n_dof-1] = 0.0;
   
   //OK, now we can formulate the next vectors
   for(unsigned n=0;n<n_dof-1;n++)
    {
     b[n] = rhs[n_dof+n] - Jprod_a[n];
    }
   b[n_dof-1] = rhs[2*n_dof-1];

   Vector<double> f(n_dof);

   Linear_solver_pt->resolve(b,f);

   //Calculate the final entry in the vector d
   const double d_final = -f[n_dof-1]/(*E_pt)[n_dof-1];
   //Assemble the final corrections
   for(unsigned n=0;n<n_dof-1;n++)
    {
     result[n] = a[n] - (*Alpha_pt)[n]*d_final;
     result[n_dof+n] = f[n] + (*E_pt)[n]*d_final;
    }

   result[n_dof-1] = a[n_dof-1] - (*Alpha_pt)[n_dof-1]*d_final;
   result[2*n_dof-1] = d_final;

   Linear_solver_pt->disable_resolve();

   //Switch things to our block solver
   handler_pt->solve_full_system();
  }

  ///////////////////////////////////////////////////////////////////////
 // Non-inline functions for the PitchForkHandler class
 //////////////////////////////////////////////////////////////////////

 //==================================================================
 ///Constructor: Initialise the PitchForkHandler by setting intial
 ///guesses for Sigma, Y, specifying C and Psi and calculating count.
 //==================================================================
 PitchForkHandler::PitchForkHandler(
  Problem* const &problem_pt, double* const &parameter_pt,
  const Vector<double> &symmetry_vector) : Solve_block_system(false), 
                                           Sigma(0.0)
 {
  //Set the problem pointer
  Problem_pt = problem_pt;
  //Set the number of degrees of freedom
  Ndof = Problem_pt->ndof();
  
  //Sort out Psi
  Psi.resize(Ndof);
  Y.resize(Ndof);
  C.resize(Ndof);
  Count.resize(Ndof,0);
  
  //Loop over all the elements in the problem
  unsigned n_element = Problem_pt->mesh_pt()->nelement();
  for(unsigned e=0;e<n_element;e++)
   {
    GeneralisedElement* elem_pt = Problem_pt->mesh_pt()->element_pt(e);
    unsigned n_var = elem_pt->ndof();
    for(unsigned n=0;n<n_var;n++)
     {
      ++Count[elem_pt->eqn_number(n)];
     }
   }
  
  //Add the parameter to the problem
  Problem_pt->Dof_pt.push_back(parameter_pt);
  
  //Find length of the symmetry vector
  double length = 0.0;
  for(unsigned n=0;n<Ndof;n++) 
   {length += symmetry_vector[n]*symmetry_vector[n];}
  length = sqrt(length);

  //Add the unknowns for the null vector to the problem
  //Normalise the symmetry vector and initialise the null vector to the
  //symmetry vector and set the constant vector, c, to the 
  //normalised symmetry vector.
  for(unsigned n=0;n<Ndof;n++)
   {
    Problem_pt->Dof_pt.push_back(&Y[n]);
    Psi[n] = Y[n] = C[n] = symmetry_vector[n]/length;
   }
  //Add the slack parameter to the problem
  Problem_pt->Dof_pt.push_back(&Sigma);
 }

 //=====================================================================
 /// Destructor return the problem to its original (non-augmented) state
 //=====================================================================
 PitchForkHandler::~PitchForkHandler()
 {
  //If we are using the block solver reset the problem's linear solver 
  //to the original one
  BlockPitchForkLinearSolver* block_pitchfork_solver_pt
   = dynamic_cast<BlockPitchForkLinearSolver*>(Problem_pt->linear_solver_pt());
  if(block_pitchfork_solver_pt)
   {
    //Reset the problem's linear solver
    Problem_pt->linear_solver_pt() = 
     block_pitchfork_solver_pt->linear_solver_pt();
    //Delete the block solver
    delete block_pitchfork_solver_pt;
   }
  //Now return the problem to its original size
  Problem_pt->Dof_pt.resize(Ndof);
 }
 
 //================================================================
 ///Get the number of elemental degrees of freedom
 //================================================================
 unsigned PitchForkHandler::ndof(GeneralisedElement* const &elem_pt)
 {
  unsigned raw_ndof = elem_pt->ndof();
  if(Solve_block_system) {return (raw_ndof + 1);}
  else {return (2*raw_ndof + 2);}
 }
 
 ///Get the global equation number of the local unknown
 unsigned long PitchForkHandler::eqn_number(GeneralisedElement* const &elem_pt,
                                            const unsigned &ieqn_local)
 {
  //Get the raw value
  unsigned raw_ndof = elem_pt->ndof();
  unsigned long global_eqn;

  //The usual equations
  if(ieqn_local < raw_ndof) 
   {
    global_eqn = elem_pt->eqn_number(ieqn_local);
   }
  //The bifurcation parameter equation
  else if(ieqn_local == raw_ndof)
   {
    global_eqn = Ndof;
   }
  //If we are assembling the full system we also have
  //The components of the null vector
  else if(ieqn_local < (2*raw_ndof + 1))
   {
    global_eqn = Ndof + 1 + elem_pt->eqn_number(ieqn_local - 1 - raw_ndof);
   }
  //The slack parameter
  else
   {
    global_eqn = 2*Ndof+1;
   }
  return global_eqn;
 }
 
 //==============================================================
 ///Get the residuals
 //==============================================================
 void PitchForkHandler::get_residuals(GeneralisedElement* const &elem_pt,
                                      Vector<double> &residuals)
 {
  //Need to get raw residuals and jacobian
  unsigned raw_ndof = elem_pt->ndof();

  //If we are solving the block system
  if(Solve_block_system)
   {
    //Get the basic residuals
    elem_pt->get_residuals(residuals);
  
    //Zero the final residual
    residuals[raw_ndof] = 0.0;
    //Now multiply to fill in the residuals for the final term
    for(unsigned i=0;i<raw_ndof;i++)
     {
      unsigned local_eqn = elem_pt->eqn_number(i);
      //Add the slack parameter to the final residuals
      residuals[i] += Sigma*Psi[local_eqn]/Count[local_eqn];
      //Final term that specifies the symmetry
      residuals[raw_ndof] += (*Problem_pt->Dof_pt[local_eqn]*Psi[local_eqn])/
       Count[local_eqn];
     }
   }
  else
   {
    DenseMatrix<double> jacobian(raw_ndof);
    
    //Get the basic residuals and jacobian
    elem_pt->get_jacobian(residuals,jacobian);
    
    //Initialise the final residuals
    residuals[raw_ndof] = 0.0;
    residuals[2*raw_ndof+1] = -1.0/Problem_pt->mesh_pt()->nelement();
    
    //Now multiply to fill in the residuals associated
    //with the null vector condition
    for(unsigned i=0;i<raw_ndof;i++)
     {
      unsigned local_eqn = elem_pt->eqn_number(i);
      residuals[raw_ndof+1+i] = 0.0;
      for(unsigned j=0;j<raw_ndof;j++)
       {
        unsigned local_unknown = elem_pt->eqn_number(j); 
        residuals[raw_ndof+1+i] += jacobian(i,j)*Y[local_unknown];
       }
      
      //Add the slack parameter to the governing equations
      residuals[i] += Sigma*Psi[local_eqn]/Count[local_eqn];
      
      //Specify the symmetry
      residuals[raw_ndof] += 
       (*Problem_pt->Dof_pt[local_eqn]*Psi[local_eqn])/Count[local_eqn];
      //Specify the normalisation
      residuals[2*raw_ndof+1] += (Y[local_eqn]*C[local_eqn])/
       Count[local_eqn];
     }
   }
 }
 
  //======================================================================
  /// \short Calculate the elemental Jacobian matrix "d equation 
  /// / d variable".
  //======================================================================
  void PitchForkHandler::get_jacobian(GeneralisedElement* const &elem_pt,
                    Vector<double> &residuals, 
                    DenseMatrix<double> &jacobian)
   {
    unsigned augmented_ndof = ndof(elem_pt);
    unsigned raw_ndof = elem_pt->ndof();
    
    //If solving the block system
    if(Solve_block_system)
     {
      //Get the full residuals, we need them
      get_residuals(elem_pt,residuals);
      
      //Need to get the raw jacobian (and raw residuals)
      Vector<double> newres(augmented_ndof);
      elem_pt->get_jacobian(newres,jacobian);
      
      //Now do finite differencing stuff
      const double FD_step = 1.0e-8;
      //Fill in the first lot of finite differences
      {
       //increase the global parameter
       double *unknown_pt = Problem_pt->Dof_pt[Ndof];
       double init = *unknown_pt;
       *unknown_pt += FD_step;
       
       //Not do any possible updates
       Problem_pt
        ->actions_after_change_in_bifurcation_parameter();
       
       //Get the new (modified) residuals
       get_residuals(elem_pt,newres);
       
       //The final column  is given by the difference
       //between the residuals
       for(unsigned n=0;n<raw_ndof;n++)
        {
         jacobian(n,augmented_ndof-1) = (newres[n] - residuals[n])/FD_step;
        }
       //Reset the global parameter
       *unknown_pt = init;
       
       //Now do any possible updates
       Problem_pt
        ->actions_after_change_in_bifurcation_parameter();
      }
      
      //Fill in the bottom row
      for(unsigned n=0;n<raw_ndof;n++)
       {
        unsigned local_eqn = elem_pt->eqn_number(n);
        jacobian(augmented_ndof-1,n) 
         = Psi[local_eqn]/Count[local_eqn];
       }
     }
    //Otherwise solving the full system
    else
     {
      //Get the basic jacobian and residuals
      elem_pt->get_jacobian(residuals,jacobian);
      //get the proper residuals
      get_residuals(elem_pt,residuals);
      
      //Now fill in the next diagonal jacobian entry
      for(unsigned n=0;n<raw_ndof;n++)
       {
        for(unsigned m=0;m<raw_ndof;m++)
         {
          jacobian(raw_ndof+1+n,raw_ndof+1+m) = jacobian(n,m);
         }
        unsigned local_eqn = elem_pt->eqn_number(n);
        //Add in the sigma contribution
        jacobian(n,2*raw_ndof+1) = Psi[local_eqn]/Count[local_eqn];
        //Symmetry constraint
        jacobian(raw_ndof,n) = Psi[local_eqn]/Count[local_eqn];
        //Non-zero constraint
        jacobian(2*raw_ndof+1,raw_ndof+1+n) = C[local_eqn]/Count[local_eqn];
       }
      
      //Finite difference the remaining blocks
      const double FD_step = 1.0e-8;
      
      Vector<double> newres_p(augmented_ndof);
      
      //Loop over the ndofs only
      for(unsigned n=0;n<raw_ndof;++n)
       {
        unsigned long global_eqn = eqn_number(elem_pt,n);
        double* unknown_pt = Problem_pt->Dof_pt[global_eqn];
        double init = *unknown_pt;
        *unknown_pt += FD_step;
        
        //Get the new residuals
        get_residuals(elem_pt,newres_p);
        //Fill in the entries in the block d(Jy)/dx
        for(unsigned m=0;m<raw_ndof;m++)
         {
          jacobian(raw_ndof+1+m,n) = 
           (newres_p[raw_ndof+1+m] - residuals[raw_ndof+1+m])/(FD_step);
         }
        
        //Reset the unknown
        *unknown_pt = init;
       }
      
      {
       //Now increase the global parameter
       double* unknown_pt = Problem_pt->Dof_pt[Ndof];
       double init = *unknown_pt;
       *unknown_pt += FD_step;
       
       Problem_pt->actions_after_change_in_bifurcation_parameter();
       //Get the new residuals
       get_residuals(elem_pt,newres_p);

       //Add in the first block d/dx
       for(unsigned m=0;m<raw_ndof;m++)
        {
         jacobian(m,raw_ndof) = 
            (newres_p[m] - residuals[m])/FD_step;
        }
       //Add in the second block d/dy
       for(unsigned m=raw_ndof+1;m<augmented_ndof-1;m++)
        {
         jacobian(m,raw_ndof) = 
          (newres_p[m] - residuals[m])/FD_step;
        }
       
       //Reset the unknown
       *unknown_pt = init;
       Problem_pt->actions_after_change_in_bifurcation_parameter();
      }
     }
   }
 
  //==============================================================
  /// Set to solve the block system
  //==============================================================
  void PitchForkHandler::solve_block_system()
   {
    if(!Solve_block_system)
     {
      Solve_block_system = true;
      //Restrict the problem to the standard variables and
      //the bifurcation parameter only
      Problem_pt->Dof_pt.resize(Ndof+1);
     }
   }

  //==============================================================
  /// \short Solve non-block system
  //==============================================================
 void PitchForkHandler::solve_full_system()
   {
    if(Solve_block_system)
     {
      Solve_block_system = false;
      
      //Add the additional unknowns back into the problem
      for(unsigned n=0;n<Ndof;n++)
       {
        Problem_pt->Dof_pt.push_back(&Y[n]);
       }
      Problem_pt->Dof_pt.push_back(&Sigma);
     }
   }


 ///////////////////////////////////////////////////////////////////////
 // Non-inline functions for the BlockHopfLinearSolver class
 //////////////////////////////////////////////////////////////////////


 //======================================================================
 /// Clean up the memory that may have been allocated by the solver
 //=====================================================================
 BlockHopfLinearSolver::~BlockHopfLinearSolver()
 {
  if(A_pt!=0) {delete A_pt;}
  if(E_pt!=0) {delete E_pt;}
  if(G_pt!=0) {delete G_pt;}
 }

 //===================================================================
 /// Use a block factorisation to solve the augmented system
 /// associated with a Hopf bifurcation.
 //===================================================================
 void BlockHopfLinearSolver::solve(Problem* const &problem_pt, 
                                    Vector<double> &result)
 {
  //Locally cache the pointer to the handler.
  HopfHandler* handler_pt =
   static_cast<HopfHandler*>(problem_pt->assembly_handler_pt());
  
  //Find the number of dofs in the augmented problem
  unsigned n_dof = problem_pt->ndof();

  //Firstly, let's calculate the derivative of the residuals wrt
  //the parameter
  Vector<double> dRdparam(n_dof);
  
  const double FD_step = 1.0e-8;
  {
   //Cache pointer to the parameter (second last entry in the vector
   double *param_pt = &problem_pt->dof(n_dof-2);
   //Backup the parameter
   double old_var = *param_pt;;
   //Increment the parameter
   *param_pt += FD_step;
   problem_pt->actions_after_change_in_bifurcation_parameter();
   
   //Now calculate the new residuals
   problem_pt->get_residuals(dRdparam);

   //Now calculate the difference assume original residuals in resul
   for(unsigned n=0;n<n_dof;n++)
    {
     dRdparam[n] = (dRdparam[n] - result[n])/FD_step;
    }
   
   //Reset the parameter
   *param_pt = old_var;
   problem_pt->actions_after_change_in_bifurcation_parameter();
  }

  //Switch the handler to "standard" mode
  handler_pt->solve_standard_system();

  //Find out the number of dofs in the non-standard problem
  n_dof = problem_pt->ndof();
  
  //Setup storage for temporary vector
  Vector<double> y1(n_dof), alpha(n_dof);

  //Allocate storage for A which can be used in the resolve
  if(A_pt!=0) {delete A_pt;}
  A_pt = new Vector<double>(n_dof);

  //We are going to do resolves using the underlying linear solver
  Linear_solver_pt->enable_resolve();

  //Solve the first system Jy1 = R
  Linear_solver_pt->solve(problem_pt,y1);

  //This should have set the sign of the jacobian, store it
  int sign_of_jacobian = problem_pt->sign_of_jacobian();

  //Now let's get the appropriate bit of alpha
  for(unsigned n=0;n<n_dof;n++) {alpha[n] = dRdparam[n];}
  //Resolve to find A
  Linear_solver_pt->resolve(alpha,*A_pt);

  //Now set to the complex system
  handler_pt->solve_complex_system();

  //Resize the stored vector G
  if(G_pt!=0) {delete G_pt;}
  G_pt = new Vector<double>(2*n_dof);

  //Solve the first Zg = (Mz -My)
  Linear_solver_pt->solve(problem_pt,*G_pt);
  //This should have set the sign of the complex matrix's determinant, multiply
  sign_of_jacobian *= problem_pt->sign_of_jacobian();

  //We can now construct our multipliers
  //Prepare to scale
  double dof_length=0.0, a_length=0.0, y1_length=0.0;
  //Loop over the standard number of dofs
  for(unsigned n=0;n<n_dof;n++)
   {
    if(std::abs(problem_pt->dof(n)) > dof_length) 
     {dof_length = std::abs(problem_pt->dof(n));}
    if(std::abs((*A_pt)[n]) > a_length) {a_length = std::abs((*A_pt)[n]);}
    if(std::abs(y1[n]) > y1_length) 
     {y1_length = std::abs(y1[n]);}
   }

  double a_mult = dof_length/a_length;
  double y1_mult = dof_length/y1_length;
  a_mult += FD_step; y1_mult += FD_step;
  a_mult *= FD_step; y1_mult *= FD_step;

  //Local storage for the product terms
  Vector<double> Jprod_a(2*n_dof,0.0), Jprod_y1(2*n_dof,0.0);

  //Temporary storage
  Vector<double> rhs(2*n_dof);

  //find the number of elements
  unsigned long n_element = problem_pt->mesh_pt()->nelement();

  //Calculate the product of the jacobian matrices, etc
  for(unsigned long e=0;e<n_element;e++)
   {
    GeneralisedElement *elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the ndofs in each element
    unsigned n_var = elem_pt->ndof();
    //Get the jacobian matrices
    DenseMatrix<double> jac(n_var), jac_a(n_var), jac_y1(n_var);
    DenseMatrix<double> M(n_var), M_a(n_var), M_y1(n_var);
    //Get unperturbed jacobian
    elem_pt->get_jacobian_and_mass_matrix(rhs,jac,M);
    
    //Backup the dofs
    Vector<double> dof_bac(n_var);
    //Perturb the original dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      dof_bac[n] = problem_pt->dof(eqn_number);
      //Pertub by vector A
      problem_pt->dof(eqn_number) += a_mult*(*A_pt)[eqn_number]; 
     }
    
    //Now get the new jacobian and mass matrix
    elem_pt->get_jacobian_and_mass_matrix(rhs,jac_a,M_a);
    
    //Perturb the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      problem_pt->dof(eqn_number) = dof_bac[n];
      //Pertub by vector y1
      problem_pt->dof(eqn_number) += y1_mult*y1[eqn_number]; 
     }
    
    //Now get the new jacobian and mass matrix
    elem_pt->get_jacobian_and_mass_matrix(rhs,jac_y1,M_y1);
    
    //Reset the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      problem_pt->dof(eqn_number) = dof_bac[n];
     }
    
    //OK, now work out the products
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      double prod_a1=0.0, prod_y11=0.0;
      double prod_a2=0.0, prod_y12=0.0;
      for(unsigned m=0;m<n_var;m++)
       {
        const unsigned unknown = elem_pt->eqn_number(m);
        const double y = handler_pt->Phi[unknown];
        const double z = handler_pt->Psi[unknown];
        const double omega = handler_pt->Omega;
        //Real part (first line)
        prod_a1 += (jac_a(n,m) - jac(n,m))*y
         + omega*(M_a(n,m) - M(n,m))*z;
        prod_y11 += (jac_y1(n,m) - jac(n,m))*y
         + omega*(M_y1(n,m) - M(n,m))*z;
        //Imag par (second line)
        prod_a2 += (jac_a(n,m) - jac(n,m))*z
         - omega*(M_a(n,m) - M(n,m))*y;
        prod_y12 += (jac_y1(n,m) - jac(n,m))*z
         - omega*(M_y1(n,m) - M(n,m))*y;
       }
      Jprod_a[eqn_number] += prod_a1/a_mult;
      Jprod_y1[eqn_number] += prod_y11/y1_mult;
      Jprod_a[n_dof + eqn_number] += prod_a2/a_mult;
      Jprod_y1[n_dof + eqn_number] += prod_y12/y1_mult;
     }
   }

   //The assumption here is still that the result has been set to the
   //residuals.
   for(unsigned n=0;n<2*n_dof;n++)
    {
     rhs[n] = result[n_dof+n] - Jprod_y1[n];
    }

   //Temporary storage
   Vector<double> y2(2*n_dof);
   //Solve it
   Linear_solver_pt->resolve(rhs,y2);

   //Assemble the next RHS
   for(unsigned n=0;n<2*n_dof;n++)
    {
     rhs[n] = dRdparam[n_dof+n] - Jprod_a[n];
    }

   //Resive the storage
   if(E_pt!=0) {delete E_pt;}
   E_pt = new Vector<double> (2*n_dof);
   //Solve for the next RHS
   Linear_solver_pt->resolve(rhs,*E_pt);

   //We can now calculate the final corrections
   //We need to work out a large number of dot products
   double dot_c = 0.0, dot_d = 0.0, dot_e = 0.0, dot_f = 0.0, dot_g=0.0;
   double dot_h = 0.0;

   for(unsigned n=0;n<n_dof;n++)
    {
     //Get the appopriate entry
     const double Cn = handler_pt->C[n];
     dot_c += Cn*y2[n];
     dot_d += Cn*y2[n_dof+n];
     dot_e += Cn*(*E_pt)[n];
     dot_f += Cn*(*E_pt)[n_dof+n];
     dot_g += Cn*(*G_pt)[n];
     dot_h += Cn*(*G_pt)[n_dof+n];
    }

   //Now we should be able to work out the corrections
   double denom = dot_e*dot_h - dot_g*dot_f;
   
   //Copy the previous residuals
   double R31 = result[3*n_dof], R32 = result[3*n_dof+1];
   //Delta parameter
   const double delta_param = 
    ((R32 - dot_d)*dot_g - (R31 - dot_c)*dot_h)/denom;
   //Delta frequency
   const double delta_w =  -((R32 - dot_d) + dot_f*delta_param)/(dot_h);

   //Load into the result vector
   result[3*n_dof] = delta_param;
   result[3*n_dof+1] = delta_w;

   //The corrections to the null vector
   for(unsigned n=0;n<2*n_dof;n++)
    {
     result[n_dof + n] = y2[n] - (*E_pt)[n]*delta_param - (*G_pt)[n]*delta_w;
    }

   //Finally add the corrections to the unknowns
   for(unsigned n=0;n<n_dof;n++)
    {
     result[n] = y1[n] - (*A_pt)[n]*delta_param;
    }

   //The sign of the jacobian is the previous signs multiplied by the 
   //sign of the denominator
   problem_pt->sign_of_jacobian() = sign_of_jacobian*
    static_cast<int>(std::abs(denom)/denom);
   
   //Switch things to our full solver
   handler_pt->solve_full_system();

   //If we are not storing things, clear the results
   if(!Enable_resolve) 
    {
     //We no longer need to store the matrix
     Linear_solver_pt->disable_resolve();
     delete A_pt; A_pt=0;
     delete E_pt; E_pt=0;
     delete G_pt; G_pt=0;
    }
   //Otherwise, also store the problem pointer
   else
    {
     Problem_pt = problem_pt;
    }
 }

 //=====================================================================
 /// Solve for two right hand sides, required for (efficient) continuation
 /// because otherwise we have to store the inverse of the jacobian
 /// and the complex equivalent ... nasty.
 //=====================================================================
 void BlockHopfLinearSolver::solve_for_two_rhs(Problem* const &problem_pt,
                                               Vector<double> &result,
                                               const Vector<double> &rhs2,
                                               Vector<double> &result2)
 {
  //Locally cache the pointer to the handler.
  HopfHandler* handler_pt =
   static_cast<HopfHandler*>(problem_pt->assembly_handler_pt());
  
  //Find the number of dofs in the augmented problem
  unsigned n_dof = problem_pt->ndof();

  //Firstly, let's calculate the derivative of the residuals wrt
  //the parameter
  Vector<double> dRdparam(n_dof);
  
  const double FD_step = 1.0e-8;
  {
   //Cache pointer to the parameter (second last entry in the vector
   double *param_pt = &problem_pt->dof(n_dof-2);
   //Backup the parameter
   double old_var = *param_pt;;
   //Increment the parameter
   *param_pt += FD_step;
   problem_pt->actions_after_change_in_bifurcation_parameter();
   
   //Now calculate the new residuals
   problem_pt->get_residuals(dRdparam);

   //Now calculate the difference assume original residuals in resul
   for(unsigned n=0;n<n_dof;n++)
    {
     dRdparam[n] = (dRdparam[n] - result[n])/FD_step;
    }
   
   //Reset the parameter
   *param_pt = old_var;
   problem_pt->actions_after_change_in_bifurcation_parameter();
  }

  //Switch the handler to "standard" mode
  handler_pt->solve_standard_system();

  //Find out the number of dofs in the non-standard problem
  n_dof = problem_pt->ndof();
  
  //Setup storage for temporary vector
  Vector<double> y1(n_dof), y1_resolve(n_dof), alpha(n_dof);

  //Allocate storage for A which can be used in the resolve
  if(A_pt!=0) {delete A_pt;}
  A_pt = new Vector<double>(n_dof);

  //We are going to do resolves using the underlying linear solver
  Linear_solver_pt->enable_resolve();

  //Solve the first system Jy1 = R
  Linear_solver_pt->solve(problem_pt,y1);

  //This should have set the sign of the jacobian, store it
  int sign_of_jacobian = problem_pt->sign_of_jacobian();

  //Now let's get the appropriate bit of alpha
  for(unsigned n=0;n<n_dof;n++) {alpha[n] = dRdparam[n];}
  //Resolve to find A
  Linear_solver_pt->resolve(alpha,*A_pt);

  //Get the solution for the second rhs
  for(unsigned n=0;n<n_dof;n++) {alpha[n] = rhs2[n];}
  //Resolve to find y1_resolve
  Linear_solver_pt->resolve(alpha,y1_resolve);

  //Now set to the complex system
  handler_pt->solve_complex_system();

  //Resize the stored vector G
  if(G_pt!=0) {delete G_pt;}
  G_pt = new Vector<double>(2*n_dof);

  //Solve the first Zg = (Mz -My)
  Linear_solver_pt->solve(problem_pt,*G_pt);
  //This should have set the sign of the complex matrix's determinant, multiply
  sign_of_jacobian *= problem_pt->sign_of_jacobian();

  //We can now construct our multipliers
  //Prepare to scale
  double dof_length=0.0, a_length=0.0, y1_length=0.0, y1_resolve_length=0.0;
  //Loop over the standard number of dofs
  for(unsigned n=0;n<n_dof;n++)
   {
    if(std::abs(problem_pt->dof(n)) > dof_length) 
     {dof_length = std::abs(problem_pt->dof(n));}
    if(std::abs((*A_pt)[n]) > a_length) {a_length = std::abs((*A_pt)[n]);}
    if(std::abs(y1[n]) > y1_length) 
     {y1_length = std::abs(y1[n]);}
    if(std::abs(y1_resolve[n]) > y1_resolve_length) 
     {y1_resolve_length = std::abs(y1[n]);}
   }


  double a_mult = dof_length/a_length;
  double y1_mult = dof_length/y1_length;
  double y1_resolve_mult = dof_length/y1_resolve_length;
  a_mult += FD_step; y1_mult += FD_step; y1_resolve_mult += FD_step;
  a_mult *= FD_step; y1_mult *= FD_step; y1_resolve_mult *= FD_step;

  //Local storage for the product terms
  Vector<double> Jprod_a(2*n_dof,0.0), Jprod_y1(2*n_dof,0.0);
  Vector<double> Jprod_y1_resolve(2*n_dof,0.0);

  //Temporary storage
  Vector<double> rhs(2*n_dof);

  //find the number of elements
  unsigned long n_element = problem_pt->mesh_pt()->nelement();

  //Calculate the product of the jacobian matrices, etc
  for(unsigned long e = 0;e<n_element;e++)
   {
    GeneralisedElement *elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the ndofs in each element
    unsigned n_var = elem_pt->ndof();
    //Get the jacobian matrices
    DenseMatrix<double> jac(n_var), jac_a(n_var), jac_y1(n_var), 
     jac_y1_resolve(n_var);
    DenseMatrix<double> M(n_var), M_a(n_var), M_y1(n_var), M_y1_resolve(n_var);
    //Get unperturbed jacobian
    elem_pt->get_jacobian_and_mass_matrix(rhs,jac,M);
    
    //Backup the dofs
    Vector<double> dof_bac(n_var);
    //Perturb the original dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      dof_bac[n] = problem_pt->dof(eqn_number);
      //Pertub by vector A
      problem_pt->dof(eqn_number) += a_mult*(*A_pt)[eqn_number]; 
     }
    
    //Now get the new jacobian and mass matrix
    elem_pt->get_jacobian_and_mass_matrix(rhs,jac_a,M_a);
    
    //Perturb the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      problem_pt->dof(eqn_number) = dof_bac[n];
      //Pertub by vector y1
      problem_pt->dof(eqn_number) += y1_mult*y1[eqn_number]; 
     }
    
    //Now get the new jacobian and mass matrix
    elem_pt->get_jacobian_and_mass_matrix(rhs,jac_y1,M_y1);

     //Perturb the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      problem_pt->dof(eqn_number) = dof_bac[n];
      //Pertub by vector y1
      problem_pt->dof(eqn_number) += y1_resolve_mult*y1_resolve[eqn_number]; 
     }

    //Now get the new jacobian and mass matrix
    elem_pt->get_jacobian_and_mass_matrix(rhs,jac_y1_resolve,M_y1_resolve);

    //Reset the dofs
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      problem_pt->dof(eqn_number) = dof_bac[n];
     }
    
    //OK, now work out the products
    for(unsigned n=0;n<n_var;n++)
     {
      unsigned eqn_number = elem_pt->eqn_number(n);
      double prod_a1=0.0, prod_y11=0.0, prod_y1_resolve1 = 0.0;
      double prod_a2=0.0, prod_y12=0.0, prod_y1_resolve2 = 0.0;
      for(unsigned m=0;m<n_var;m++)
       {
        const unsigned unknown = elem_pt->eqn_number(m);
        const double y = handler_pt->Phi[unknown];
        const double z = handler_pt->Psi[unknown];
        const double omega = handler_pt->Omega;
        //Real part (first line)
        prod_a1 += (jac_a(n,m) - jac(n,m))*y
         + omega*(M_a(n,m) - M(n,m))*z;
        prod_y11 += (jac_y1(n,m) - jac(n,m))*y
         + omega*(M_y1(n,m) - M(n,m))*z;
        prod_y1_resolve1 += (jac_y1_resolve(n,m) - jac(n,m))*y
         + omega*(M_y1_resolve(n,m) - M(n,m))*z;
        //Imag par (second line)
        prod_a2 += (jac_a(n,m) - jac(n,m))*z
         - omega*(M_a(n,m) - M(n,m))*y;
        prod_y12 += (jac_y1(n,m) - jac(n,m))*z
         - omega*(M_y1(n,m) - M(n,m))*y;
        prod_y1_resolve2 += (jac_y1_resolve(n,m) - jac(n,m))*z
         - omega*(M_y1_resolve(n,m) - M(n,m))*y;
       }
      Jprod_a[eqn_number] += prod_a1/a_mult;
      Jprod_y1[eqn_number] += prod_y11/y1_mult;
      Jprod_y1_resolve[eqn_number] += prod_y1_resolve1/y1_resolve_mult;
      Jprod_a[n_dof + eqn_number] += prod_a2/a_mult;
      Jprod_y1[n_dof + eqn_number] += prod_y12/y1_mult;
      Jprod_y1_resolve[n_dof + eqn_number] += prod_y1_resolve2/y1_resolve_mult;
     }
   }

   //The assumption here is still that the result has been set to the
   //residuals.
   for(unsigned n=0;n<2*n_dof;n++)
    {
     rhs[n] = result[n_dof+n] - Jprod_y1[n];
    }

   //Temporary storage
   Vector<double> y2(2*n_dof);
   //Solve it
   Linear_solver_pt->resolve(rhs,y2);

   //Assemble the next RHS
   for(unsigned n=0;n<2*n_dof;n++)
    {
     rhs[n] = dRdparam[n_dof+n] - Jprod_a[n];
    }

   //Resive the storage
   if(E_pt!=0) {delete E_pt;}
   E_pt = new Vector<double> (2*n_dof);
   //Solve for the next RHS
   Linear_solver_pt->resolve(rhs,*E_pt);

   //Assemble the next RHS
   for(unsigned n=0;n<2*n_dof;n++)
    {
     rhs[n] = rhs2[n_dof+n] - Jprod_y1_resolve[n];
    }

   Vector<double> y2_resolve(2*n_dof);
   Linear_solver_pt->resolve(rhs,y2_resolve);


   //We can now calculate the final corrections
   //We need to work out a large number of dot products
   double dot_c = 0.0, dot_d = 0.0, dot_e = 0.0, dot_f = 0.0, dot_g=0.0;
   double dot_h = 0.0;

   double dot_c_resolve = 0.0, dot_d_resolve = 0.0;

   for(unsigned n=0;n<n_dof;n++)
    {
     //Get the appopriate entry
     const double Cn = handler_pt->C[n];
     dot_c += Cn*y2[n];
     dot_d += Cn*y2[n_dof+n];
     dot_c_resolve += Cn*y2_resolve[n];
     dot_d_resolve += Cn*y2_resolve[n_dof+n];
     dot_e += Cn*(*E_pt)[n];
     dot_f += Cn*(*E_pt)[n_dof+n];
     dot_g += Cn*(*G_pt)[n];
     dot_h += Cn*(*G_pt)[n_dof+n];
    }

   //Now we should be able to work out the corrections
   double denom = dot_e*dot_h - dot_g*dot_f;
   
   //Copy the previous residuals
   double R31 = result[3*n_dof], R32 = result[3*n_dof+1];
   //Delta parameter
   const double delta_param = 
    ((R32 - dot_d)*dot_g - (R31 - dot_c)*dot_h)/denom;
   //Delta frequency
   const double delta_w =  -((R32 - dot_d) + dot_f*delta_param)/(dot_h);

   //Corrections
   double R31_resolve = rhs2[3*n_dof], R32_resolve = rhs2[3*n_dof+1];
   //Delta parameter
   const double delta_param_resolve = 
    ((R32_resolve - dot_d_resolve)*dot_g - 
     (R31_resolve - dot_c_resolve)*dot_h)/denom;
   //Delta frequency
   const double delta_w_resolve =  
    -((R32_resolve - dot_d_resolve) + dot_f*delta_param_resolve)/(dot_h);


   //Load into the result vector
   result[3*n_dof] = delta_param;
   result[3*n_dof+1] = delta_w;

   //The corrections to the null vector
   for(unsigned n=0;n<2*n_dof;n++)
    {
     result[n_dof + n] = y2[n] - (*E_pt)[n]*delta_param - (*G_pt)[n]*delta_w;
    }

   //Finally add the corrections to the unknowns
   for(unsigned n=0;n<n_dof;n++)
    {
     result[n] = y1[n] - (*A_pt)[n]*delta_param;
    }

    //Load into the result vector
   result2[3*n_dof] = delta_param_resolve;
   result2[3*n_dof+1] = delta_w_resolve;

   //The corrections to the null vector
   for(unsigned n=0;n<2*n_dof;n++)
    {
     result2[n_dof + n] = y2_resolve[n] - (*E_pt)[n]*delta_param_resolve 
      - (*G_pt)[n]*delta_w_resolve;
    }

   //Finally add the corrections to the unknowns
   for(unsigned n=0;n<n_dof;n++)
    {
     result2[n] = y1_resolve[n] - (*A_pt)[n]*delta_param_resolve;
    }


   //The sign of the jacobian is the previous signs multiplied by the 
   //sign of the denominator
   problem_pt->sign_of_jacobian() = sign_of_jacobian*
    static_cast<int>(std::abs(denom)/denom);
   
   //Switch things to our full solver
   handler_pt->solve_full_system();

   //If we are not storing things, clear the results
   if(!Enable_resolve) 
    {
     //We no longer need to store the matrix
     Linear_solver_pt->disable_resolve();
     delete A_pt; A_pt=0;
     delete E_pt; E_pt=0;
     delete G_pt; G_pt=0;
    }
   //Otherwise, also store the problem pointer
   else
    {
     Problem_pt = problem_pt;
    }
 }


 //======================================================================
 //Hack the re-solve to use the block factorisation
 //======================================================================
 void BlockHopfLinearSolver::resolve(const Vector<double> & rhs,
                                     Vector<double> &result)
 {
  throw OomphLibError("resolve() is not implemented for this solver",
                      "BlockHopfLinearSolver::resolve()",
                      OOMPH_EXCEPTION_LOCATION);

 }


 ///////////////////////////////////////////////////////////////////////
 // Non-inline functions for the HopfHandler class
 //////////////////////////////////////////////////////////////////////
 
 //====================================================================
 /// Constructor: Initialise the hopf handler, 
 /// by setting initial guesses for Phi, Psi  and calculating Count. 
 /// If the system changes, a new  handler must be constructed.
 //===================================================================
 HopfHandler::HopfHandler(Problem* const &problem_pt, 
                          double* const &parameter_pt) : 
  Solve_which_system(0), Parameter_pt(parameter_pt), Omega(0.0)
 {
  //Set the problem pointer
  Problem_pt = problem_pt;
  //Set the number of non-augmented degrees of freedom
  Ndof = problem_pt->ndof();

  //Resize the vectors of additional dofs
  Phi.resize(Ndof);
  Psi.resize(Ndof);
  C.resize(Ndof);
  Count.resize(Ndof,0);

  //Loop over all the elements in the problem
  unsigned n_element = problem_pt->mesh_pt()->nelement();
  for(unsigned e=0;e<n_element;e++)
   {
    GeneralisedElement* elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the local freedoms in an element
    unsigned n_var = elem_pt->ndof();
    for(unsigned n=0;n<n_var;n++)
     {
      //Increase the associated global equation number counter
      ++Count[elem_pt->eqn_number(n)];
     }
   }
  

  //Calculate the value Phi by
  //solving the system JPhi = dF/dlambda
  
  //Locally cache the linear solver
  LinearSolver* const linear_solver_pt = problem_pt->linear_solver_pt();

  //Save the status before entry to this routine
  bool enable_resolve = linear_solver_pt->resolve_is_enabled();

  //We need to do a resolve
  linear_solver_pt->enable_resolve();

  //Storage for the solution
  Vector<double> x(Ndof);

  //Solve the standard problem, we only want to make sure that
  //we factorise the matrix, if it has not been factorised. We shall
  //ignore the return value of x.
  linear_solver_pt->solve(problem_pt,x);

  //Get the vector dresiduals/dparameter
  problem_pt->get_derivative_wrt_global_parameter(parameter_pt,x);

  // Copy rhs vector into local storage so it doesn't get overwritten
  // if the linear solver decides to initialise the solution vector, say,
  // which it's quite entitled to do!
  Vector<double> input_x(x);

  //Now resolve the system with the new RHS and overwrite the solution
  linear_solver_pt->resolve(input_x,x);

  //Restore the storage status of the linear solver
  if (enable_resolve)
   {
    linear_solver_pt->enable_resolve();
   }
  else
   {
    linear_solver_pt->disable_resolve();
   }

  //Normalise the solution x
  double length = 0.0;
  for(unsigned n=0;n<Ndof;n++) {length += x[n]*x[n];}
  length = sqrt(length);
  
  //Now add the real part of the null space components to the problem 
  //unknowns and initialise it all
  //This is dumb at the moment ... fix with eigensolver?
  for(unsigned n=0;n<Ndof;n++)
   {
    problem_pt->Dof_pt.push_back(&Phi[n]);
    C[n] = Phi[n] = -x[n]/length;
   }

  //Set the imaginary part so that the appropriate residual is 
  //zero initially (eigensolvers)
  for(unsigned n=0;n<Ndof;n+=2)
   {
    //Make sure that we are not at the end of an array of odd length
    if(n!=Ndof-1)
     {
      Psi[n] = C[n+1];
      Psi[n+1] = -C[n];
     }
    //If it's odd set the final entry to zero
    else
     {
      Psi[n] = 0.0;
     }
   }

  //Next add the imaginary parts of the null space components to the problem
  for(unsigned n=0;n<Ndof;n++)
   {
    problem_pt->Dof_pt.push_back(&Psi[n]);
   }
  //Now add the parameter 
  problem_pt->Dof_pt.push_back(parameter_pt);
  //Finally add the frequency
  problem_pt->Dof_pt.push_back(&Omega);
 }

 //====================================================================
 /// Constructor: Initialise the hopf handler, 
 /// by setting initial guesses for Phi, Psi, Omega  and calculating Count. 
 /// If the system changes, a new  handler must be constructed.
 //===================================================================
 HopfHandler::HopfHandler(Problem* const &problem_pt, 
                          double* const &parameter_pt,
                          const double &omega,
                          const Vector<double> &phi,
                          const Vector<double> &psi) : 
  Solve_which_system(0), Parameter_pt(parameter_pt), Omega(omega)
 {
  //Set the problem pointer
  Problem_pt = problem_pt;
  //Set the number of non-augmented degrees of freedom
  Ndof = problem_pt->ndof();

  //Resize the vectors of additional dofs
  Phi.resize(Ndof);
  Psi.resize(Ndof);
  C.resize(Ndof);
  Count.resize(Ndof,0);

  //Loop over all the elements in the problem
  unsigned n_element = problem_pt->mesh_pt()->nelement();
  for(unsigned e=0;e<n_element;e++)
   {
    GeneralisedElement* elem_pt = problem_pt->mesh_pt()->element_pt(e);
    //Loop over the local freedoms in an element
    unsigned n_var = elem_pt->ndof();
    for(unsigned n=0;n<n_var;n++)
     {
      //Increase the associated global equation number counter
      ++Count[elem_pt->eqn_number(n)];
     }
   }

  //Normalise the guess for phi
  double length = 0.0;
  for(unsigned n=0;n<Ndof;n++) {length += phi[n]*phi[n];}
  length = sqrt(length);
  
  //Now add the real part of the null space components to the problem 
  //unknowns and initialise it all
  for(unsigned n=0;n<Ndof;n++)
   {
    problem_pt->Dof_pt.push_back(&Phi[n]);
    C[n] = Phi[n] = phi[n]/length;
    Psi[n] = psi[n]/length;
   }

  //Next add the imaginary parts of the null space components to the problem
  for(unsigned n=0;n<Ndof;n++)
   {
    problem_pt->Dof_pt.push_back(&Psi[n]);
   }

  //Now add the parameter 
  problem_pt->Dof_pt.push_back(parameter_pt);
  //Finally add the frequency
  problem_pt->Dof_pt.push_back(&Omega);
 }



 //=======================================================================
 /// Destructor return the problem to its original (non-augmented) state
 //=======================================================================
 HopfHandler::~HopfHandler()
 {
  //If we are using the block solver reset the problem's linear solver 
  //to the original one
  BlockHopfLinearSolver* block_hopf_solver_pt = 
   dynamic_cast<BlockHopfLinearSolver*>(Problem_pt->linear_solver_pt());
  if(block_hopf_solver_pt)
   {
    //Reset the problem's linear solver
    Problem_pt->linear_solver_pt() = block_hopf_solver_pt->linear_solver_pt();
    //Delete the block solver
    delete block_hopf_solver_pt;
   }
  //Now return the problem to its original size
  Problem_pt->Dof_pt.resize(Ndof);
 }


 //=============================================================
 ///Get the number of elemental degrees of freedom
 //=============================================================
 unsigned HopfHandler::ndof(GeneralisedElement* const &elem_pt)
 {
  unsigned raw_ndof = elem_pt->ndof();
  switch(Solve_which_system)
   {
    //Full augmented system
   case 0:
    return (3*raw_ndof + 2);
    break;
    //Standard non-augmented system
   case 1:
    return raw_ndof;
    break;
    //Complex system
   case 2:
    return (2*raw_ndof);
    break;

   default:
    throw OomphLibError("Solve_which_system can only be 0,1 or 2",
                        "HopfHander::ndof()",
                        OOMPH_EXCEPTION_LOCATION);
   }
 }

 //=============================================================
 ///Get the global equation number of the local unknown
 //============================================================
 unsigned long HopfHandler::eqn_number(GeneralisedElement* const &elem_pt,
                                       const unsigned &ieqn_local)
 {
  //Get the raw value
  unsigned raw_ndof = elem_pt->ndof();
  unsigned long global_eqn;
  if(ieqn_local < raw_ndof) 
   {
    global_eqn = elem_pt->eqn_number(ieqn_local);
   }
  else if(ieqn_local < 2*raw_ndof)
   {
    global_eqn = Ndof + elem_pt->eqn_number(ieqn_local - raw_ndof);
   }
  else if(ieqn_local < 3*raw_ndof)
   {
    global_eqn = 2*Ndof + elem_pt->eqn_number(ieqn_local - 2*raw_ndof);
   }
  else if(ieqn_local == 3*raw_ndof)
   {
    global_eqn = 3*Ndof;
   }
  else
   {
    global_eqn = 3*Ndof+1;
   }
  return global_eqn;
 }
 
 //==================================================================
 ///Get the residuals
 //=================================================================
 void HopfHandler::get_residuals(GeneralisedElement* const &elem_pt,
                                 Vector<double> &residuals)
 {
  //Should only call get residuals for the full system
  if(Solve_which_system==0)
   {
    //Need to get raw residuals and jacobian
    unsigned raw_ndof = elem_pt->ndof();

    DenseMatrix<double> jacobian(raw_ndof), M(raw_ndof);
    //Get the basic residuals, jacobian and mass matrix
    elem_pt->get_jacobian_and_mass_matrix(residuals,jacobian,M);
    
    //Initialise the pen-ultimate residual
    residuals[3*raw_ndof] = -1.0/
     (double)(Problem_pt->mesh_pt()->nelement());
    
    //Now multiply to fill in the residuals
    for(unsigned i=0;i<raw_ndof;i++)
     {
      residuals[raw_ndof+i] = 0.0;
      residuals[2*raw_ndof+i] = 0.0;
      for(unsigned j=0;j<raw_ndof;j++)
       {
        unsigned global_unknown = elem_pt->eqn_number(j); 
        //Real part
        residuals[raw_ndof+i] += 
         jacobian(i,j)*Phi[global_unknown] + Omega*M(i,j)*Psi[global_unknown];
        //Imaginary part
        residuals[2*raw_ndof+i] += 
         jacobian(i,j)*Psi[global_unknown] - Omega*M(i,j)*Phi[global_unknown]; 
       }
      //Get the global equation number
      unsigned global_eqn = elem_pt->eqn_number(i);
      
      //Real part
      residuals[3*raw_ndof] += (Phi[global_eqn]*C[global_eqn])/
       Count[global_eqn];
      //Imaginary part
      residuals[3*raw_ndof+1] += (Psi[global_eqn]*C[global_eqn])/
       Count[global_eqn];
     }
   }
  else
   {
    throw OomphLibError("Solve_which_system can only be 0",
                        "HopfHander::get_residuals()",
                        OOMPH_EXCEPTION_LOCATION);
    
   }
 }
    
  
 //===============================================================
 /// \short Calculate the elemental Jacobian matrix "d equation 
 /// / d variable".
//==================================================================
 void HopfHandler::get_jacobian(GeneralisedElement* const &elem_pt,
                                Vector<double> &residuals, 
                                DenseMatrix<double> &jacobian)
 {
  //The standard case
  if(Solve_which_system==0)
   {
    unsigned augmented_ndof = ndof(elem_pt);
    unsigned raw_ndof = elem_pt->ndof();
    
    //Get the basic residuals and jacobian
    DenseMatrix<double> M(raw_ndof);
    elem_pt->get_jacobian_and_mass_matrix(residuals,jacobian,M);
    //Now fill in the actual residuals
    get_residuals(elem_pt,residuals);
    
    //Now the jacobian appears in other entries
    for(unsigned n=0;n<raw_ndof;++n)
     {
      for(unsigned m=0;m<raw_ndof;++m)
       {
        jacobian(raw_ndof+n,raw_ndof+m) = jacobian(n,m);
        jacobian(raw_ndof+n,2*raw_ndof+m) = Omega*M(n,m);
        jacobian(2*raw_ndof+n,2*raw_ndof+m) = jacobian(n,m);
        jacobian(2*raw_ndof+n,raw_ndof+m) = -Omega*M(n,m);
        unsigned global_eqn = elem_pt->eqn_number(m);
        jacobian(raw_ndof+n,3*raw_ndof+1) += M(n,m)*Psi[global_eqn];
        jacobian(2*raw_ndof+n,3*raw_ndof+1) -= M(n,m)*Phi[global_eqn];
       }
      
      unsigned local_eqn = elem_pt->eqn_number(n);
      jacobian(3*raw_ndof,raw_ndof+n) = C[local_eqn]/Count[local_eqn];
      jacobian(3*raw_ndof+1,2*raw_ndof+n) = C[local_eqn]/Count[local_eqn];
     }
    
    const double FD_step = 1.0e-8;
    
    Vector<double> newres_p(augmented_ndof), newres_m(augmented_ndof);
    
    //Loop over the dofs
    for(unsigned n=0;n<raw_ndof;n++)
     {
      //Just do the x's
      unsigned long global_eqn = eqn_number(elem_pt,n);
      double* unknown_pt = Problem_pt->Dof_pt[global_eqn];
      double init = *unknown_pt;
      *unknown_pt += FD_step;
      
      //Get the new residuals
      get_residuals(elem_pt,newres_p);
      
      //Reset
      *unknown_pt = init;
      
      //Subtract
      *unknown_pt -= FD_step;
      get_residuals(elem_pt,newres_m);
      
      for(unsigned m=0;m<raw_ndof;m++)
       {
        jacobian(raw_ndof+m,n) = 
         (newres_p[raw_ndof+m] - residuals[raw_ndof+m])/(FD_step);
        jacobian(2*raw_ndof+m,n) = 
         (newres_p[2*raw_ndof+m] - residuals[2*raw_ndof+m])/(FD_step);
       }
      //Reset the unknown
      *unknown_pt = init;
     }
  
    {
     //Now do the global parameter
     double* unknown_pt = Problem_pt->Dof_pt[3*Ndof];
     double init = *unknown_pt;
     *unknown_pt += FD_step;
     
     Problem_pt->actions_after_change_in_bifurcation_parameter();
     //Get the new residuals
     get_residuals(elem_pt,newres_p);
     
     //Reset
     *unknown_pt = init;
     
     //Subtract
     *unknown_pt -= FD_step;
     get_residuals(elem_pt,newres_m);
     
     for(unsigned m=0;m<augmented_ndof-2;m++)
      {
       jacobian(m,3*raw_ndof) = 
        (newres_p[m] - residuals[m])/FD_step;
      }
     //Reset the unknown
     *unknown_pt = init;
     Problem_pt->actions_after_change_in_bifurcation_parameter();
    }
   } //End of standard case
  //Normal case
  else if(Solve_which_system==1)
   {
    //Just get the normal jacobian and residuals
    elem_pt->get_jacobian(residuals,jacobian);
   }
  //Otherwise the augmented complex case
  else if(Solve_which_system==2)
   {
    unsigned raw_ndof = elem_pt->ndof();
    
    //Get the basic residuals and jacobian
    DenseMatrix<double> M(raw_ndof);
    elem_pt->get_jacobian_and_mass_matrix(residuals,jacobian,M);

    //We now need to fill in the other blocks
    for(unsigned n=0;n<raw_ndof;n++)
     {
      for(unsigned m=0;m<raw_ndof;m++)
       {
        jacobian(n,raw_ndof+m) = Omega*M(n,m);
        jacobian(raw_ndof+n,m) = -Omega*M(n,m);
        jacobian(raw_ndof+n,raw_ndof+m) = jacobian(n,m);
       }
     }

    //Now overwrite to fill in the residuals
    //The decision take is to solve for the mass matrix multiplied
    //terms in the residuals because they require no additional
    //information to assemble.
    for(unsigned n=0;n<raw_ndof;n++)
     {
      residuals[n] = 0.0;
      residuals[raw_ndof+n] = 0.0;
      for(unsigned m=0;m<raw_ndof;m++)
       {
        unsigned global_unknown = elem_pt->eqn_number(m); 
        //Real part
        residuals[n] += M(n,m)*Psi[global_unknown];
        //Imaginary part
        residuals[raw_ndof+n] -= M(n,m)*Phi[global_unknown]; 
       }
     }
   } //End of complex augmented case
  else
   {
    throw OomphLibError("Solve_which_system can only be 0,1 or 2",
                        "HopfHander::get_residuals()",
                        OOMPH_EXCEPTION_LOCATION);
   }
 }
 
 //====================================================================
 /// Set to solve the standard (underlying jacobian)  system
 //===================================================================
 void HopfHandler::solve_standard_system()
 {
  if(Solve_which_system!=1)
   {
    Solve_which_system = 1;
    //Restrict the problem to the standard variables only
    Problem_pt->Dof_pt.resize(Ndof);
   }
 }

 //====================================================================
 /// Set to solve the complex (jacobian and mass matrix)  system
 //===================================================================
 void HopfHandler::solve_complex_system()
 {
  //If we were not solving the complex system resize the unknowns
  //accordingly
  if(Solve_which_system!=2)
   {
    Solve_which_system=2;
    //Resize to the first Ndofs (will work whichever system we were
    //solving before)
    Problem_pt->Dof_pt.resize(Ndof);
    //Add the first (real) part of the eigenfunction back into the problem
    for(unsigned n=0;n<Ndof;n++)
     {
      Problem_pt->Dof_pt.push_back(&Phi[n]);
     }
   }
 }


 //=================================================================
 /// Set to Solve full system system
 //=================================================================
 void HopfHandler::solve_full_system()
 {
  //If we are starting from another system
  if(Solve_which_system)
   {
    Solve_which_system = 0;
    
    //Resize to the first Ndofs (will work whichever system we were
    //solving before)
    Problem_pt->Dof_pt.resize(Ndof);
    //Add the additional unknowns back into the problem
    for(unsigned n=0;n<Ndof;n++)
     {
      Problem_pt->Dof_pt.push_back(&Phi[n]);
     }
    for(unsigned n=0;n<Ndof;n++)
     {
      Problem_pt->Dof_pt.push_back(&Psi[n]);
     }
    //Now add the parameter 
    Problem_pt->Dof_pt.push_back(Parameter_pt);
    //Finally add the frequency
    Problem_pt->Dof_pt.push_back(&Omega);
   }
 }
 
}