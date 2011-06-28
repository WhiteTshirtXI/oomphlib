#! /bin/sh


#Set the number of tests to be checked
NUM_TESTS=8


# Setup validation directory
#---------------------------
touch Validation
rm -r -f Validation
mkdir Validation

# Validation for adaptive scattering
#-----------------------------------
cd Validation

echo "Running adaptive scattering validation. Dirichlet-to-Neumann BC"
mkdir RESLT
../adaptive_scattering --case 0 > OUTPUT_adapt_0
echo "done"
echo " " >> validation.log
echo "Scattering validation (Dirichlet-to-Neumann BC)" >> validation.log
echo "-------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > adaptive_scattering_results0.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/adaptive_scattering_results0.dat.gz   \
    adaptive_scattering_results0.dat  >> validation.log
fi
mv RESLT RESLT_adapt_0




echo "Running adaptive scattering validation. first order abc"
mkdir RESLT
../adaptive_scattering --case 1 > OUTPUT_adapt_1
echo "done"
echo " " >> validation.log
echo "Scattering validation (first order ABC)" >> validation.log
echo "---------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > adaptive_scattering_results1.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/adaptive_scattering_results1.dat.gz   \
    adaptive_scattering_results1.dat  >> validation.log
fi
mv RESLT RESLT_adapt_1





echo "Running adaptive scattering validation. second order abc"
mkdir RESLT
../adaptive_scattering --case 2 > OUTPUT_adapt_2
echo "done"
echo " " >> validation.log
echo "Scattering validation (second order ABC)" >> validation.log
echo "---------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > adaptive_scattering_results2.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/adaptive_scattering_results2.dat.gz   \
    adaptive_scattering_results2.dat  >> validation.log
fi
mv RESLT RESLT_adapt_2




echo "Running adaptive scattering validation. third order abc"
mkdir RESLT
../adaptive_scattering --case 3 > OUTPUT_adapt_3
echo "done"
echo " " >> validation.log
echo "Scattering validation (third order ABC)" >> validation.log
echo "---------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > adaptive_scattering_results3.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/adaptive_scattering_results3.dat.gz   \
    adaptive_scattering_results3.dat  >> validation.log
fi
mv RESLT RESLT_adapt_3


# Validation for scattering
#--------------------------

echo "Running  scattering validation. Dirichlet-to-Neumann BC"
mkdir RESLT
../scattering --case 0 > OUTPUT_0
echo "done"
echo " " >> validation.log
echo "Scattering  (Dirichlet-to-Neumann BC)" >> validation.log
echo "-------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > scattering_results0.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/scattering_results0.dat.gz   \
    scattering_results0.dat  >> validation.log
fi
mv RESLT RESLT_0




echo "Running  scattering validation. first order abc"
mkdir RESLT
../scattering --case 1 > OUTPUT_1
echo "done"
echo " " >> validation.log
echo "Scattering validation (first order ABC)" >> validation.log
echo "---------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > scattering_results1.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/scattering_results1.dat.gz   \
    scattering_results1.dat  >> validation.log
fi
mv RESLT RESLT_1





echo "Running  scattering validation. second order abc"
mkdir RESLT
../scattering --case 2 > OUTPUT_2
echo "done"
echo " " >> validation.log
echo "Scattering validation (second order ABC)" >> validation.log
echo "---------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > scattering_results2.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/scattering_results2.dat.gz   \
    scattering_results2.dat  >> validation.log
fi
mv RESLT RESLT_2




echo "Running  scattering validation. third order abc"
mkdir RESLT
../scattering --case 3 > OUTPUT_3
echo "done"
echo " " >> validation.log
echo "Scattering validation (third order ABC)" >> validation.log
echo "---------------------------------------" >> validation.log
echo " " >> validation.log
echo "Validation directory: " >> validation.log
echo " " >> validation.log
echo "  " `pwd` >> validation.log
echo " " >> validation.log
cat RESLT/soln0.dat > scattering_results3.dat

if test "$1" = "no_fpdiff"; then
  echo "dummy [OK] -- Can't run fpdiff.py because we don't have python or validata" >> validation.log
else
../../../../bin/fpdiff.py ../validata/scattering_results3.dat.gz   \
    scattering_results3.dat  >> validation.log
fi
mv RESLT RESLT_3



# Append output to global validation log file
#--------------------------------------------
cat validation.log >> ../../../../validation.log


cd ..

#######################################################################


#Check that we get the correct number of OKs
OK_COUNT=`grep -c 'OK' Validation/validation.log`
if  [ $OK_COUNT -eq $NUM_TESTS ]; then
 echo " "
 echo "======================================================================"
 echo " " 
 echo "All tests in" 
 echo " " 
 echo "    `pwd`    "
 echo " "
 echo "passed successfully."
 echo " "
 echo "======================================================================"
 echo " " 
else
  if [ $OK_COUNT -lt $NUM_TESTS ]; then
   echo " "
   echo "======================================================================"
   echo " " 
   echo "Only $OK_COUNT of $NUM_TESTS test(s) passed; see"
   echo " " 
   echo "    `pwd`/Validation/validation.log"
   echo " " 
   echo "for details" 
   echo " " 
   echo "======================================================================"
   echo " "
  else 
   echo " "
   echo "======================================================================"
   echo " " 
   echo "More OKs than tests! Need to update NUM_TESTS in"
   echo " " 
   echo "    `pwd`/validate.sh"
   echo " "
   echo "======================================================================"
   echo " "
  fi
fi