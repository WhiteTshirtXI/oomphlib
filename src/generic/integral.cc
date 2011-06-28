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
//Static data for the Gaussian integration rules

//oomph-lib header
#include "integral.h"

namespace oomph
{

//Need to define the static data here

//============================================================
// Define the positions and weights of the 1D Gauss points 
//============================================================

//------------------------------------------------------------
//N=2
//------------------------------------------------------------
const double Gauss<1,2>::Knot[2][1]=
{{-0.577350269189626},{0.577350269189626}};

const double Gauss<1,2>::Weight[2]={1.0,1.0}; 

//------------------------------------------------------------
//N=3
//------------------------------------------------------------
const double Gauss<1,3>::Knot[3][1]= 
{{-0.774596669241483},{0.0},{0.774596669241483}};

const double Gauss<1,3>::Weight[3]={(5.0/9.0),(8.0/9.0),(5.0/9.0)};

//------------------------------------------------------------
//N=4
//------------------------------------------------------------
const double Gauss<1,4>::Knot[4][1]= 
{{-0.861136311594053},{-0.339981043584856}, {0.339981043584856},
 {0.861136311594053}};

const double Gauss<1,4>::Weight[4]={0.347854845137448, 0.652145154862546, 0.652145154862546, 0.347854845137448};



//============================================================
// Define the positions and weights of the 2D Gauss points 
//============================================================

//------------------------------------------------------------
//N=2
//------------------------------------------------------------
const double Gauss<2,2>::Knot[4][2]={
 {-0.577350269189626,-0.577350269189626},
 {-0.577350269189626,0.577350269189626},
 {0.577350269189626,-0.577350269189626},
 {0.577350269189626,0.577350269189626}};
const double Gauss<2,2>::Weight[4] = {1.0,1.0,1.0,1.0};

//------------------------------------------------------------
//N=3
//------------------------------------------------------------
const double Gauss<2,3>::Knot[9][2]= {
  {-0.774596669241483,-0.774596669241483},
  {-0.774596669241483,0.0},
  {-0.774596669241483,0.774596662941483},
  {0.0,-0.774596669241483},
  {0.0,0.0},
  {0.0,0.774596662941483},
  {0.774596662941483,-0.774596669241483},
  {0.774596662941483,0.0},
  {0.774596662941483,0.774596662941483}};
const double Gauss<2,3>::Weight[9] = {(25.0/81.0),(40.0/81.0),(25.0/81.0),
  (40.0/81.0),(64.0/81.0),(40.0/81.0),(25.0/81.0),(40.0/81.0),(25.0/81.0)};

//------------------------------------------------------------
//N=4
//------------------------------------------------------------
const double Gauss<2,4>::Knot[16][2]= {
 {-0.861136311594053,-0.861136311594053},
 {-0.339981043584856,-0.861136311594053},
 { 0.339981043584856,-0.861136311594053},
 { 0.861136311594053,-0.861136311594053},

 {-0.861136311594053,-0.339981043584856},
 {-0.339981043584856,-0.339981043584856},
 { 0.339981043584856,-0.339981043584856},
 { 0.861136311594053,-0.339981043584856},

 {-0.861136311594053, 0.339981043584856},
 {-0.339981043584856, 0.339981043584856},
 { 0.339981043584856, 0.339981043584856},
 { 0.861136311594053, 0.339981043584856},

 {-0.861136311594053, 0.861136311594053},
 {-0.339981043584856, 0.861136311594053},
 { 0.339981043584856, 0.861136311594053},
 { 0.861136311594053, 0.861136311594053}};

// Quick sanity check: they sum to 4  :)
const double Gauss<2,4>::Weight[16] = 
{0.1210029932855979 , 0.2268518518518480 , 0.2268518518518480, 0.1210029932855979,  
 0.2268518518518480 , 0.4252933030106941 , 0.4252933030106941, 0.2268518518518480,
 0.2268518518518480 , 0.4252933030106941 , 0.4252933030106941, 0.2268518518518480,
 0.1210029932855979 , 0.2268518518518480 , 0.2268518518518480, 0.1210029932855979};



//============================================================
// Define the positions and weights of the 3D Gauss points 
// (produced with utilities/gauss_weights.cc)
//============================================================

//------------------------------------------------------------
//N=2
//------------------------------------------------------------
const double Gauss<3,2>::Knot[8][3]=
{{-0.57735026918963,-0.57735026918963,-0.57735026918963},{-0.57735026918963,-0.57735026918963,0.57735026918963},{-0.57735026918963,0.57735026918963,-0.57735026918963},{-0.57735026918963,0.57735026918963,0.57735026918963},{0.57735026918963,-0.57735026918963,-0.57735026918963},{0.57735026918963,-0.57735026918963,0.57735026918963},{0.57735026918963,0.57735026918963,-0.57735026918963},{0.57735026918963,0.57735026918963,0.57735026918963}};
const double Gauss<3,2>::Weight[8] = {1,1,1,1,1,1,1,1};

//------------------------------------------------------------
//N=3
//------------------------------------------------------------
const double Gauss<3,3>::Knot[27][3]= 
{{-0.77459666924148,-0.77459666924148,-0.77459666924148},{-0.77459666924148,-0.77459666924148,0},{-0.77459666924148,-0.77459666924148,0.77459666924148},{-0.77459666924148,0,-0.77459666924148},{-0.77459666924148,0,0},{-0.77459666924148,0,0.77459666924148},{-0.77459666924148,0.77459666924148,-0.77459666924148},{-0.77459666924148,0.77459666924148,0},{-0.77459666924148,0.77459666924148,0.77459666924148},{0,-0.77459666924148,-0.77459666924148},{0,-0.77459666924148,0},{0,-0.77459666924148,0.77459666924148},{0,0,-0.77459666924148},{0,0,0},{0,0,0.77459666924148},{0,0.77459666924148,-0.77459666924148},{0,0.77459666924148,0},{0,0.77459666924148,0.77459666924148},{0.77459666924148,-0.77459666924148,-0.77459666924148},{0.77459666924148,-0.77459666924148,0},{0.77459666924148,-0.77459666924148,0.77459666924148},{0.77459666924148,0,-0.77459666924148},{0.77459666924148,0,0},{0.77459666924148,0,0.77459666924148},{0.77459666924148,0.77459666924148,-0.77459666924148},{0.77459666924148,0.77459666924148,0},{0.77459666924148,0.77459666924148,0.77459666924148}};


const double Gauss<3,3>::Weight[27] = {0.17146776406035,0.27434842249657,0.17146776406035,0.27434842249657,0.43895747599451,0.27434842249657,0.17146776406035,0.27434842249657,0.17146776406035,0.27434842249657,0.43895747599451,0.27434842249657,0.43895747599451,0.70233196159122,0.43895747599451,0.27434842249657,0.43895747599451,0.27434842249657,0.17146776406035,0.27434842249657,0.17146776406035,0.27434842249657,0.43895747599451,0.27434842249657,0.17146776406035,0.27434842249657,0.17146776406035};



//------------------------------------------------------------
//N=4
//------------------------------------------------------------
const double Gauss<3,4>::Knot[64][3]= 
{{-0.86113631159405,-0.86113631159405,-0.86113631159405},{-0.86113631159405,-0.86113631159405,-0.33998104358486},{-0.86113631159405,-0.86113631159405,0.33998104358486},{-0.86113631159405,-0.86113631159405,0.86113631159405},{-0.86113631159405,-0.33998104358486,-0.86113631159405},{-0.86113631159405,-0.33998104358486,-0.33998104358486},{-0.86113631159405,-0.33998104358486,0.33998104358486},{-0.86113631159405,-0.33998104358486,0.86113631159405},{-0.86113631159405,0.33998104358486,-0.86113631159405},{-0.86113631159405,0.33998104358486,-0.33998104358486},{-0.86113631159405,0.33998104358486,0.33998104358486},{-0.86113631159405,0.33998104358486,0.86113631159405},{-0.86113631159405,0.86113631159405,-0.86113631159405},{-0.86113631159405,0.86113631159405,-0.33998104358486},{-0.86113631159405,0.86113631159405,0.33998104358486},{-0.86113631159405,0.86113631159405,0.86113631159405},{-0.33998104358486,-0.86113631159405,-0.86113631159405},{-0.33998104358486,-0.86113631159405,-0.33998104358486},{-0.33998104358486,-0.86113631159405,0.33998104358486},{-0.33998104358486,-0.86113631159405,0.86113631159405},{-0.33998104358486,-0.33998104358486,-0.86113631159405},{-0.33998104358486,-0.33998104358486,-0.33998104358486},{-0.33998104358486,-0.33998104358486,0.33998104358486},{-0.33998104358486,-0.33998104358486,0.86113631159405},{-0.33998104358486,0.33998104358486,-0.86113631159405},{-0.33998104358486,0.33998104358486,-0.33998104358486},{-0.33998104358486,0.33998104358486,0.33998104358486},{-0.33998104358486,0.33998104358486,0.86113631159405},{-0.33998104358486,0.86113631159405,-0.86113631159405},{-0.33998104358486,0.86113631159405,-0.33998104358486},{-0.33998104358486,0.86113631159405,0.33998104358486},{-0.33998104358486,0.86113631159405,0.86113631159405},{0.33998104358486,-0.86113631159405,-0.86113631159405},{0.33998104358486,-0.86113631159405,-0.33998104358486},{0.33998104358486,-0.86113631159405,0.33998104358486},{0.33998104358486,-0.86113631159405,0.86113631159405},{0.33998104358486,-0.33998104358486,-0.86113631159405},{0.33998104358486,-0.33998104358486,-0.33998104358486},{0.33998104358486,-0.33998104358486,0.33998104358486},{0.33998104358486,-0.33998104358486,0.86113631159405},{0.33998104358486,0.33998104358486,-0.86113631159405},{0.33998104358486,0.33998104358486,-0.33998104358486},{0.33998104358486,0.33998104358486,0.33998104358486},{0.33998104358486,0.33998104358486,0.86113631159405},{0.33998104358486,0.86113631159405,-0.86113631159405},{0.33998104358486,0.86113631159405,-0.33998104358486},{0.33998104358486,0.86113631159405,0.33998104358486},{0.33998104358486,0.86113631159405,0.86113631159405},{0.86113631159405,-0.86113631159405,-0.86113631159405},{0.86113631159405,-0.86113631159405,-0.33998104358486},{0.86113631159405,-0.86113631159405,0.33998104358486},{0.86113631159405,-0.86113631159405,0.86113631159405},{0.86113631159405,-0.33998104358486,-0.86113631159405},{0.86113631159405,-0.33998104358486,-0.33998104358486},{0.86113631159405,-0.33998104358486,0.33998104358486},{0.86113631159405,-0.33998104358486,0.86113631159405},{0.86113631159405,0.33998104358486,-0.86113631159405},{0.86113631159405,0.33998104358486,-0.33998104358486},{0.86113631159405,0.33998104358486,0.33998104358486},{0.86113631159405,0.33998104358486,0.86113631159405},{0.86113631159405,0.86113631159405,-0.86113631159405},{0.86113631159405,0.86113631159405,-0.33998104358486},{0.86113631159405,0.86113631159405,0.33998104358486},{0.86113631159405,0.86113631159405,0.86113631159405}};



const double Gauss<3,4>::Weight[64] = 
{0.042091477490529,0.078911515795068,0.078911515795068,0.042091477490529,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.042091477490529,0.078911515795068,0.078911515795068,0.042091477490529,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.14794033605678,0.27735296695391,0.27735296695391,0.14794033605678,0.14794033605678,0.27735296695391,0.27735296695391,0.14794033605678,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.14794033605678,0.27735296695391,0.27735296695391,0.14794033605678,0.14794033605678,0.27735296695391,0.27735296695391,0.14794033605678,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.042091477490529,0.078911515795068,0.078911515795068,0.042091477490529,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.078911515795068,0.14794033605678,0.14794033605678,0.078911515795068,0.042091477490529,0.078911515795068,0.078911515795068,0.042091477490529};

//1D Triangles, which are just the same as 1D Gauss elements, but scaled
//so their coordinate runs from 0 to 1, rather than -1 to 1

//------------------------------------------------------------
//N=2
//------------------------------------------------------------
const double TGauss<1,2>::Knot[2][1]=
{{0.5*(-0.577350269189626+1.0)},{0.5*(0.577350269189626+1.0)}};

const double TGauss<1,2>::Weight[2]={0.5,0.5}; 

//------------------------------------------------------------
//N=3
//------------------------------------------------------------
const double TGauss<1,3>::Knot[3][1]= 
{{0.5*(-0.774596669241483+1.0)},{0.5},{0.5*(0.774596669241483+1.0)}};

const double TGauss<1,3>::Weight[3]={(5.0/18.0),(8.0/18.0),(5.0/18.0)};

//------------------------------------------------------------
//N=4
//------------------------------------------------------------
const double TGauss<1,4>::Knot[4][1]= 
{{0.5*(-0.861136311594053+1.0)},{0.5*(-0.339981043584856+1.0)}, 
 {0.5*(0.339981043584856+1.0)}, {0.5*(0.861136311594053+1.0)}};

const double TGauss<1,4>::Weight[4]={0.5*0.347854845137448, 
0.5*0.652145154862546, 0.5*0.652145154862546, 0.5*0.347854845137448};


//// Define the positions and weights of the 2D Gauss points for triangles 
//
//------------------------------------------------------------
// "Full integration" weights for linear triangles
// accurate up to second order (Bathe p 467)
//------------------------------------------------------------
const double TGauss<2,2>::Knot[3][2]={
 {0.1666666666667,0.1666666666667},
 {0.6666666666667,0.1666666666667},
 {0.1666666666667,0.6666666666667}};

const double TGauss<2,2>::Weight[3] = {
0.1666666666667,
0.1666666666667,
0.1666666666667};

//------------------------------------------------------------
// "Full integration" weights for quadratic triangles
// accurate up to fifth order (Bathe p 467)
//------------------------------------------------------------
const double TGauss<2,3>::Knot[7][2]={
 {0.1012865073235,0.1012865073235},
 {0.7974269853531,0.1012865073235},
 {0.1012865073235,0.7974269853531},
 {0.4701420641051,0.0597158717898},
 {0.4701420641051,0.4701420641051},
 {0.0597158717898,0.4701420641051},
 {0.3333333333333,0.3333333333333}};

const double TGauss<2,3>::Weight[7] = {
0.5*0.1259391805448,
0.5*0.1259391805448,
0.5*0.1259391805448,
0.5*0.1323941527885,
0.5*0.1323941527885,
0.5*0.1323941527885,
0.5*0.225};



//------------------------------------------------------------
//"Full integration" weights for cubic triangles
// accurate up to seventh order (Bathe p 467)
//------------------------------------------------------------
const double TGauss<2,4>::Knot[13][2]={
 {0.0651301029022,0.0651301029022},
 {0.8697397941956,0.0651301029022},
 {0.0651301029022,0.8697397941956},
 {0.3128654960049,0.0486903154253},
 {0.6384441885698,0.3128654960049},
 {0.0486903154253,0.6384441885698},
 {0.6384441885698,0.0486903154253},
 {0.3128654960049,0.6384441885698},
 {0.0486903154253,0.3128654960049},
 {0.2603459660790,0.2603459660790},
 {0.4793080678419,0.2603459660790},
 {0.2603459660790,0.4793080678419},
 {0.3333333333333,0.3333333333333}};


const double TGauss<2,4>::Weight[13] = {
0.5*0.0533472356088,
0.5*0.0533472356088,
0.5*0.0533472356088,
0.5*0.0771137608903,
0.5*0.0771137608903,
0.5*0.0771137608903,
0.5*0.0771137608903,
0.5*0.0771137608903,
0.5*0.0771137608903,
0.5*0.1756152574332,
0.5*0.1756152574332,
0.5*0.1756152574332,
0.5*-0.1495700444677};


//// Define the positions and weights of the 3D Gauss points for tets

//------------------------------------------------------------
// "Full integration" weights for linear tets
// accurate up to second order (e.g. from German Zienkiwicz p 200)
//------------------------------------------------------------
const double TGauss<3,2>::Knot[4][3]={
 {0.138196601125011,0.138196601125011,0.585410196624969},
 {0.138196601125011,0.585410196624969,0.138196601125011},
 {0.585410196624969,0.138196601125011,0.138196601125011},
 {0.138196601125011,0.138196601125011,0.138196601125011}};


const double TGauss<3,2>::Weight[4] = {
0.0416666666667,
0.0416666666667,
0.0416666666667,
0.0416666666667};





//------------------------------------------------------------
//"Full integration" weights for quadratic tets
// accurate up to fifth order 
// The numbers are from Keast CMAME 55 pp339-348 (1986)
//------------------------------------------------------------
const double TGauss<3,3>::Knot[11][3]={
 {0.25,0.25,0.25},
 {0.785714285714286,0.071428571428571,0.071428571428571},
 {0.071428571428571,0.071428571428571,0.071428571428571},
 {0.071428571428571,0.785714285714286,0.071428571428571},
 {0.071428571428571,0.071428571428571,0.785714285714286},
 {0.399403576166799,0.399403576166799,0.100596423833201},
 {0.399403576166799,0.100596423833201,0.399403576166799},
 {0.100596423833201,0.399403576166799,0.399403576166799},
 {0.399403576166799,0.100596423833201,0.100596423833201},
 {0.100596423833201,0.399403576166799,0.100596423833201},
 {0.100596423833201,0.100596423833201,0.399403576166799}};


const double TGauss<3,3>::Weight[11] ={
-0.01315555555556,
 0.00762222222222,
 0.00762222222222,
 0.00762222222222,
 0.00762222222222,
 0.02488888888889,
 0.02488888888889,
 0.02488888888889,
 0.02488888888889,
 0.02488888888889,
 0.02488888888889
};

//------------------------------------------------------------
//"Full integration" weights for quartic tets
// accurate up to eighth order 
// The numbers are from Keast CMAME 55 pp339-348 (1986)
//------------------------------------------------------------
const double TGauss<3,5>::Knot[45][3]={
 {2.50000000000000000e-01, 2.50000000000000000e-01, 2.50000000000000000e-01},
 {1.27470936566639015e-01, 1.27470936566639015e-01, 1.27470936566639015e-01},
 {1.27470936566639015e-01, 1.27470936566639015e-01, 6.17587190300082967e-01},
 {1.27470936566639015e-01, 6.17587190300082967e-01, 1.27470936566639015e-01},
 {6.17587190300082967e-01, 1.27470936566639015e-01, 1.27470936566639015e-01},
 {3.20788303926322960e-02, 3.20788303926322960e-02, 3.20788303926322960e-02},
 {3.20788303926322960e-02, 3.20788303926322960e-02, 9.03763508822103123e-01},
 {3.20788303926322960e-02, 9.03763508822103123e-01, 3.20788303926322960e-02},
 {9.03763508822103123e-01, 3.20788303926322960e-02, 3.20788303926322960e-02},
 {4.97770956432810185e-02, 4.97770956432810185e-02, 4.50222904356718978e-01}, 
 {4.97770956432810185e-02, 4.50222904356718978e-01, 4.50222904356718978e-01},
 {4.50222904356718978e-01, 4.50222904356718978e-01, 4.97770956432810185e-02}, 
 {4.50222904356718978e-01, 4.97770956432810185e-02, 4.97770956432810185e-02},
 {4.97770956432810185e-02, 4.50222904356718978e-01, 4.97770956432810185e-02}, 
 {4.50222904356718978e-01, 4.97770956432810185e-02, 4.50222904356718978e-01},
 {1.83730447398549945e-01, 1.83730447398549945e-01, 3.16269552601450060e-01}, 
 {1.83730447398549945e-01, 3.16269552601450060e-01, 3.16269552601450060e-01},
 {3.16269552601450060e-01, 3.16269552601450060e-01, 1.83730447398549945e-01}, 
 {3.16269552601450060e-01, 1.83730447398549945e-01, 1.83730447398549945e-01},
 {1.83730447398549945e-01, 3.16269552601450060e-01, 1.83730447398549945e-01}, 
 {3.16269552601450060e-01, 1.83730447398549945e-01, 3.16269552601450060e-01},
 {2.31901089397150906e-01, 2.31901089397150906e-01, 2.29177878448171174e-02}, 
 {2.31901089397150906e-01, 2.29177878448171174e-02, 5.13280033360881072e-01}, 
 {2.29177878448171174e-02, 5.13280033360881072e-01, 2.31901089397150906e-01}, 
 {5.13280033360881072e-01, 2.31901089397150906e-01, 2.31901089397150906e-01},
 {2.31901089397150906e-01, 5.13280033360881072e-01, 2.31901089397150906e-01}, 
 {5.13280033360881072e-01, 2.31901089397150906e-01, 2.29177878448171174e-02},
 {2.31901089397150906e-01, 2.29177878448171174e-02, 2.31901089397150906e-01}, 
 {2.31901089397150906e-01, 5.13280033360881072e-01, 2.29177878448171174e-02}, 
 {2.29177878448171174e-02, 2.31901089397150906e-01, 5.13280033360881072e-01}, 
 {5.13280033360881072e-01, 2.29177878448171174e-02, 2.31901089397150906e-01}, 
 {2.29177878448171174e-02, 2.31901089397150906e-01, 2.31901089397150906e-01}, 
 {2.31901089397150906e-01, 2.31901089397150906e-01, 5.13280033360881072e-01},
 {3.79700484718286102e-02, 3.79700484718286102e-02, 7.30313427807538396e-01}, 
 {3.79700484718286102e-02, 7.30313427807538396e-01, 1.93746475248804382e-01}, 
 {7.30313427807538396e-01, 1.93746475248804382e-01, 3.79700484718286102e-02}, 
 {1.93746475248804382e-01, 3.79700484718286102e-02, 3.79700484718286102e-02},
 {3.79700484718286102e-02, 1.93746475248804382e-01, 3.79700484718286102e-02}, 
 {1.93746475248804382e-01, 3.79700484718286102e-02, 7.30313427807538396e-01},
 {3.79700484718286102e-02, 7.30313427807538396e-01, 3.79700484718286102e-02}, 
 {3.79700484718286102e-02, 1.93746475248804382e-01, 7.30313427807538396e-01},
 {7.30313427807538396e-01, 3.79700484718286102e-02, 1.93746475248804382e-01}, 
 {1.93746475248804382e-01, 7.30313427807538396e-01, 3.79700484718286102e-02},
 {7.30313427807538396e-01, 3.79700484718286102e-02, 3.79700484718286102e-02}, 
 {3.79700484718286102e-02,  3.79700484718286102e-02, 1.93746475248804382e-01} 
};

const double TGauss<3,5>::Weight[45] ={
-3.93270066412926145e-02,  4.08131605934270525e-03, 
 4.08131605934270525e-03,  4.08131605934270525e-03, 
 4.08131605934270525e-03,  6.58086773304341943e-04, 
 6.58086773304341943e-04,  6.58086773304341943e-04, 
 6.58086773304341943e-04,  4.38425882512284693e-03, 
 4.38425882512284693e-03,  4.38425882512284693e-03, 
 4.38425882512284693e-03,  4.38425882512284693e-03, 
 4.38425882512284693e-03,  1.38300638425098166e-02, 
 1.38300638425098166e-02,  1.38300638425098166e-02, 
 1.38300638425098166e-02,  1.38300638425098166e-02, 
 1.38300638425098166e-02,  4.24043742468372453e-03, 
 4.24043742468372453e-03,  4.24043742468372453e-03, 
 4.24043742468372453e-03,  4.24043742468372453e-03, 
 4.24043742468372453e-03,  4.24043742468372453e-03, 
 4.24043742468372453e-03,  4.24043742468372453e-03, 
 4.24043742468372453e-03,  4.24043742468372453e-03, 
 4.24043742468372453e-03,  2.23873973961420164e-03, 
 2.23873973961420164e-03,  2.23873973961420164e-03, 
 2.23873973961420164e-03,  2.23873973961420164e-03, 
 2.23873973961420164e-03,  2.23873973961420164e-03, 
 2.23873973961420164e-03,  2.23873973961420164e-03, 
 2.23873973961420164e-03,  2.23873973961420164e-03, 
 2.23873973961420164e-03}; 

}




