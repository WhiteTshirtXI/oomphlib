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
// Common base class for all Triangle Meshes
#ifndef OOMPH_TRIMESH_HEADER
#define OOMPH_TRIMESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

//oomph-lib includes
#include "Vector.h"
#include "nodes.h"
#include "matrices.h"
#include "mesh.h"

namespace oomph
{

#ifdef OOMPH_HAS_TRIANGLE_LIB

//=====================================================================
/// The Triangle data structure, modified from the triangle.h header
/// supplied with triangle 1.6. by J. R. Schewchuk. We need to define
/// this here separately because we can't include a c header directly
/// into C++ code!
//=====================================================================
struct TriangulateIO 
{
 ///Pointer to list of points x coordinate followed by y coordinate
 double *pointlist;

 ///Pointer to list of point attributes
 double *pointattributelist;

 ///Pointer to list of point markers
 int *pointmarkerlist;
 int numberofpoints;
 int numberofpointattributes;
 
 int *trianglelist;
 double *triangleattributelist;
 double *trianglearealist;
 int *neighborlist;
 int numberoftriangles;
 int numberofcorners;
 int numberoftriangleattributes;
 
 int *segmentlist;
 int *segmentmarkerlist;
 int numberofsegments;
 
 double *holelist;
 int numberofholes;
 
 double *regionlist;
 int numberofregions;
 
 int *edgelist;
 int *edgemarkerlist;  // <---- contains boundary ID (offset by one)
 double *normlist;
 int numberofedges;

};


#endif





///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


//==================================================================
/// Helper namespace for triangle meshes
//==================================================================
namespace TriangleHelper
{

#ifdef OOMPH_HAS_TRIANGLE_LIB

 /// Clear TriangulateIO structure
 extern void clear_triangulateio(TriangulateIO& triangulate_io,
                                 const bool& clear_hole_data=true);

 /// Initialise TriangulateIO structure
 extern void initialise_triangulateio(TriangulateIO& triangle_io);

 /// \short Make (partial) deep copy of TriangulateIO object. We only copy
 /// those items we need within oomph-lib's adaptation procedures.
 /// Warnings are issued if triangulate_io contains data that is not
 /// not copied, unless quiet=true;
 extern TriangulateIO deep_copy_of_triangulateio_representation(
  TriangulateIO& triangle_io, const bool& quiet=false);

 /// \short Write the triangulateio data to disk as a poly file,
 /// mainly used for debugging
 extern void write_triangulateio_to_polyfile(TriangulateIO &triangle_io,
                                             std::ostream &poly_file);


 /// \short Create a triangulateio data file from ele node and poly
 /// files.
 extern void create_triangulateio_from_polyfiles(
  const std::string& node_file_name,
  const std::string& element_file_name,
  const std::string& poly_file_name, TriangulateIO &triangle_io);
  

 /// \short Dump the triangulatio data into a dump file
 extern void dump_triangulateio(TriangulateIO &triangle_io,
                                std::ostream &dump_file);

 /// \short Read the triangulateio data from a dump file
 extern void read_triangulateio(std::istream &dump_fil,
                                TriangulateIO &triangulate_io);

#endif

}


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//=====================================================================
/// Base class definining a mesh boundary
//=====================================================================
class TriangleMeshOpenCurve
{

  public:

 /// Empty constructor
 TriangleMeshOpenCurve(){}


 /// Empty destructor
 virtual ~TriangleMeshOpenCurve(){}

 /// \short Number of segments that this part of the
 /// boundary is to be represented by. This corresponds
 /// to the number of straight-line segments in triangle
 /// representation.
 virtual unsigned nsegment() const=0;

 /// Boundary ID
 virtual unsigned boundary_id()const=0;

};


 
//=====================================================================
/// Class definining a curvilinear triangle mesh boundary in terms 
/// of a GeomObject. Curvlinear equivalent of PolyLine.
//=====================================================================
class TriangleMeshCurviLine : public virtual TriangleMeshOpenCurve
{
 
public:
 
 /// \short Constructor: Specify GeomObject, the start and end coordinates
 /// of the relevant boundary in terms of the GeomObject's intrinsic
 /// coordinate, the number of (initially straight-line) segments that 
 /// this GeomObject is to be split up into, and the boundary ID.
 /// This is the curvlinear equivalent of PolyLine.
 TriangleMeshCurviLine(GeomObject* geom_object_pt,
                                 const double& zeta_start,
                                 const double& zeta_end,
                                 const unsigned& nsegment,
                                 const unsigned& boundary_id) :
  Geom_object_pt(geom_object_pt), Zeta_start(zeta_start),
  Zeta_end(zeta_end), Nsegment(nsegment), Boundary_id(boundary_id)
  {}
 
 /// \short Empty Destuctor
 virtual ~TriangleMeshCurviLine() { }
 
 /// Pointer to GeomObject that represents this part of the boundary
 GeomObject* geom_object_pt(){return Geom_object_pt;}
 
 /// Start coordinate in terms of the GeomObject's intrinisic coordinate
 double zeta_start(){return Zeta_start;}
 
 /// End coordinate in terms of the GeomObject's intrinisic coordinate
 double zeta_end(){return Zeta_end;}

 /// \short Number of (initially straight-line) segments that this part of the
 /// boundary is to be represented by
 unsigned nsegment() const {return Nsegment;}

 /// Boundary ID
 unsigned boundary_id() const {return Boundary_id;}

 /// Output curvilinear boundary at n_sample (default: 50) points
 void output(std::ostream &outfile, const unsigned& n_sample=50)
  {
   outfile << "ZONE T=\"Boundary " << Boundary_id << "\"\n";
   Vector<double> zeta(1);
   Vector<double> r(2);
   for (unsigned i=0;i<n_sample;i++)
    {
     zeta[0]=Zeta_start+(Zeta_end-Zeta_start)*double(i)/double(n_sample-1);
     Geom_object_pt->position(zeta,r);
     outfile << r[0] << " " << r[1] << std::endl;
    }
  }

  private:

 /// Pointer to GeomObject that represents this part of the boundary
 GeomObject* Geom_object_pt;
 
 /// Start coordinate in terms of the GeomObject's intrinisic coordinate
 double Zeta_start;
 
 /// End coordinate in terms of the GeomObject's intrinisic coordinate
 double Zeta_end;

 /// Number of (initially straight-line) segments that this part of the
 /// boundary is to be represented by
 unsigned Nsegment;

 /// Boundary ID
 unsigned Boundary_id;

};



//=====================================================================
/// Class defining a polyline for use in Triangle Mesh generation
//=====================================================================
class TriangleMeshPolyLine : public virtual TriangleMeshOpenCurve
{
 
public:
 
 /// \short Constructor: Takes vectors of vertex coordinates in order
 /// Also allows the optional specification of a boundary ID -- useful
 /// in a mesh generation context. If not specified it defaults to zero.
 TriangleMeshPolyLine(const Vector<Vector<double> >& vertex_coordinate,
                      const unsigned &boundary_id=0) :
  Vertex_coordinate(vertex_coordinate), Boundary_id(boundary_id)
  {
#ifdef PARANOID
   unsigned nvert=Vertex_coordinate.size();
   for (unsigned i=0;i<nvert;i++)
    {
     if (Vertex_coordinate[i].size()!=2)
      {
       std::ostringstream error_stream;
       error_stream 
        << "TriangleMeshPolyLine should only be used in 2D!\n"
        << "Your Vector of coordinates, contains data for " 
        << Vertex_coordinate[i].size() 
        << "-dimensional coordinates." << std::endl;
       throw OomphLibError(error_stream.str(),
                           "TriangleMeshPolyLine::TriangleMeshPolyLine()",
                           OOMPH_EXCEPTION_LOCATION);
      }
    }
#endif   
  }
 

 /// Empty destructor
 ~TriangleMeshPolyLine() {}

 
 /// Number of vertices
 unsigned nvertex() const {return Vertex_coordinate.size();}
   
 /// Number of segments
 unsigned nsegment() const {return Vertex_coordinate.size()-1;}
  
 /// Boundary id
 unsigned boundary_id() const {return Boundary_id;}
  
 /// Coordinate vector of i-th vertex (const version)
 Vector<double> vertex_coordinate(const unsigned& i) const
  {
   return Vertex_coordinate[i];
  }   

 /// Coordinate vector of i-th vertex
 Vector<double>& vertex_coordinate(const unsigned& i)
  {
   return Vertex_coordinate[i];
  }
  
 /// Output the polyline -- close it if optional boolean flag is true
 void output(std::ostream &outfile, const bool& close_it=false)
  {
   outfile <<"ZONE T=\"TriangleMeshPolyLine with boundary ID" 
           << Boundary_id<<"\""<<std::endl;
   unsigned nvert=Vertex_coordinate.size();
   for(unsigned i=0;i<nvert;i++)
    {
     outfile << Vertex_coordinate[i][0] << " " 
             << Vertex_coordinate[i][1] << std::endl;
    }
   if (close_it)
    {
     outfile << Vertex_coordinate[0][0] << " " 
             << Vertex_coordinate[0][1] << std::endl;
    }
  }

private:
  
 /// Vector of Vector of vertex coordinates
 Vector<Vector<double> > Vertex_coordinate;
   
 /// Boundary ID
 unsigned Boundary_id;

};



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


//===================================================================
/// \short Namespace that allows the specification of a tolerance 
/// between vertices at the ends of polylines that are supposed
/// to be at the same position.
//===================================================================
namespace ToleranceForVertexMismatchInPolygons
{
 
 /// \short Acceptable discrepancy for mismatch in vertex coordinates.
 /// In paranoid mode, the code will die if the beginning/end of
 /// two adjacent polylines differ by more than that. If the
 /// discrepancy is smaller (but nonzero) one of the vertex coordinates
 /// get adjusted to match perfectly; without paranoia the vertex
 /// coordinates are taken as they come...
 extern double Tolerable_error;
}



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////



//=====================================================================
/// Base class defining a closed curve for the Triangle mesh generation
//=====================================================================
class TriangleMeshClosedCurve
{
  
public:

 /// Empty constructor
 TriangleMeshClosedCurve(){};


 /// Empty destructor
 virtual ~TriangleMeshClosedCurve(){};

};



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


//=====================================================================
/// Base class defining a internal closed curve in Triangle Mesh generation
//=====================================================================
class TriangleMeshInternalClosedCurve
{
 
  public:
 
 /// Constructor: Specify coordinate of internal point
 TriangleMeshInternalClosedCurve(const Vector<double>& internal_point) :
  Internal_point(internal_point)
  {};
 

 /// Empty destructor
 virtual ~TriangleMeshInternalClosedCurve(){};

 /// Output each sub-boundary at n_sample (default: 50) points
 /// and internal point
 virtual void output(std::ostream &outfile, const unsigned& n_sample=50)=0;
  
 /// Coordinate of the internal point
 Vector<double> internal_point() const
  {
   return Internal_point;
  }
   
 /// Coordinate of the internal point
 Vector<double> &internal_point() 
  {
   return Internal_point;
  }
  
  protected:
 
 /// Vector of vertex coordinates
 Vector<double> Internal_point;


};



///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////





//=====================================================================
/// Class defining a closed polygon for the Triangle mesh generation
//=====================================================================
class TriangleMeshPolygon : public virtual TriangleMeshClosedCurve
{
  
public:
  
 /// \short Constructor: Specify vector of pointers to TriangleMeshPolyLines
 /// that define the boundary of the segments of the polygon.
 /// Each TriangleMeshPolyLine has its own boundary ID and can contain
 /// multiple (straight-line) segments. For consistency across the
 /// various uses of this class, we insist that the closed boundary
 /// is represented by at least two separate TriangleMeshPolyLines
 /// whose joint vertices must be specified in both.
 /// (This is to allow the setup of unique boundary coordinate(s)
 /// around the polygon.) This may seem slightly annoying
 /// in cases where a polygon really only represents a single
 /// boundary, but...
 TriangleMeshPolygon(const Vector<TriangleMeshPolyLine*>& 
                     boundary_polyline_pt);
   


 /// Empty virtual destructor
 virtual ~TriangleMeshPolygon() {}
 
 /// Number of constituent polylines
 unsigned npolyline() const {return Boundary_polyline_pt.size();}
  
 /// Coordinate of i-th constituent polyline
 TriangleMeshPolyLine* polyline_pt(const unsigned& i) const
  {
   return Boundary_polyline_pt[i];
  }
  
 /// Total number of segments
 unsigned nsegment()
  {
   unsigned npolyline=this->npolyline();
   unsigned nseg=0;
   for(unsigned j=0;j<npolyline;j++)
    {
     nseg += this->polyline_pt(j)->nsegment();      
    }  
   // If there's just one boundary poly line we have another segment
   // connecting the last vertex to the first one
   if(npolyline==1){nseg+=1;}

   return nseg;
  }
  
  
 /// Return vector of boundary ids of associated polylines
 Vector<unsigned> polygon_boundary_id()
  {     
   // Get the number of polylines
   unsigned nline=npolyline();
   Vector<unsigned> boundary_id(nline);
    
   // Loop over the polyline to get the id
   for(unsigned iline=0;iline<nline;iline++)
    {
     boundary_id[iline]=Boundary_polyline_pt[iline]->boundary_id();
    }
   return boundary_id;
  }
   


 /// Return max boundary id of associated polylines
 unsigned max_polygon_boundary_id()
  {     
   unsigned max=0;

   // Loop over the polyline to get the id
   unsigned nline=npolyline();
   for(unsigned iline=0;iline<nline;iline++)
    {
     unsigned boundary_id=Boundary_polyline_pt[iline]->boundary_id();
     if (boundary_id>max) max=boundary_id;
    }
   return max;
  }
   

 /// Number of vertices
 unsigned nvertex()
  {
   unsigned n_polyline=this->npolyline();
   unsigned nvertices=0;
   for(unsigned j=0;j<n_polyline;j++)
    {
     // Storing the number of the vertices
     nvertices += this->polyline_pt(j)->nvertex()-1;
    }
   // If there's just one boundary. All the vertices should be counted   
   if(n_polyline==1)
    {
     nvertices+=1;
    }
   return nvertices;
  } 
  
 /// Pointer to i-th constituent polyline
 TriangleMeshPolyLine* &polyline_pt(const unsigned& i)
  {
   return Boundary_polyline_pt[i];
  }

  
 /// Output the constituent polylines (nsample argument is ignored
 /// since we have straightline segments anyway)
 void output(std::ostream &outfile, const unsigned& n_sample=50)
 {
  unsigned nbound=Boundary_polyline_pt.size();
  bool close_it=false;
  if (nbound==1) close_it=true;
  for(unsigned j=0;j<nbound;j++)
   {
    Boundary_polyline_pt[j]->output(outfile,close_it);
   }
 }

protected:
   
 /// Vector of pointers to constituent polylines
 Vector<TriangleMeshPolyLine*> Boundary_polyline_pt;
   
};




//=====================================================================
/// Class upgrading a TriangleMeshPolygon to a internal closed curve 
/// for use during 
/// triangle mesh generation. For mesh generation purposes, the main (and only)
/// addition to the base class is the provision of the coordinates
/// of an internal point inside the polygon. 
//=====================================================================
class TriangleMeshInternalPolygon : public virtual TriangleMeshPolygon, 
 public virtual TriangleMeshInternalClosedCurve
{
 
public:
 
 /// \short Constructor: Specify coordinates of a point inside the 
 /// closed curve and a vector of pointers to TriangleMeshPolyLines
 /// that define the boundary segments of the polygon.
 /// Each TriangleMeshPolyLine has its own boundary ID and can contain
 /// multiple (straight-line) segments. For consistency across the
 /// various uses of this class, we insist that the closed boundary
 /// is represented by at least two separate TriangleMeshPolyLines
 /// whose joint vertices must be specified in both.
 /// (This is to allow the setup of unique boundary coordinate(s)
 /// around the polygon.) This may seem slightly annoying
 /// in cases where a polygon really only represents a single
 /// boundary, but...
 TriangleMeshInternalPolygon(const Vector<double>& internal_point,
                             const Vector<TriangleMeshPolyLine*>& 
                             boundary_polyline_pt) :
  TriangleMeshPolygon(boundary_polyline_pt), 
  TriangleMeshInternalClosedCurve(internal_point)
  {
#ifdef PARANOID

   // Check if internal point  is actually located in bounding polygon
   // Reference: http://paulbourke.net/geometry/insidepoly/
   
   // Vertex coordinates
   Vector<Vector<double> > polygon_vertex;
   
   // Total number of vertices
   unsigned nvertex=0;
   
   // Storage for first/last point on polyline for contiguousness check
   Vector<double> last_vertex(2);
   Vector<double> first_vertex(2);
   
   // Get vertices
   unsigned npolyline=boundary_polyline_pt.size();
   for (unsigned i=0;i<npolyline;i++)
    {
     // Number of vertices
     unsigned nvert=boundary_polyline_pt[i]->nvertex();
     for (unsigned j=0;j<nvert;j++)
      {
       // Check contiguousness
       if ((i>1)&&(j=0))
        {
         first_vertex=boundary_polyline_pt[i]->vertex_coordinate(j);
         double dist=sqrt(pow(first_vertex[0]-last_vertex[0],2)+
                          pow(first_vertex[1]-last_vertex[1],2));
         if (dist>ToleranceForVertexMismatchInPolygons::Tolerable_error)
          {
           std::ostringstream error_stream;
           error_stream
            << "The start and end points of polylines " << i 
            << " and " <<i+1<< " don't match when judged\n"
            << "with the tolerance of "
            << ToleranceForVertexMismatchInPolygons::Tolerable_error
            << " which is specified in the namespace \nvariable "
            << "ToleranceForVertexMismatchInPolygons::Tolerable_error.\n\n"
            << "Feel free to adjust this or to recompile the code without\n"
            << "paranoia if you think this is OK...\n"
            << std::endl;
           throw OomphLibError(
            error_stream.str(),
            "TriangleMeshInternalPolygon:: constructor ",
            OOMPH_EXCEPTION_LOCATION);
          }
        }
       // Get vertex (ignore end point)
       if (j<nvert-1)
        {
         polygon_vertex.push_back(
          boundary_polyline_pt[i]->vertex_coordinate(j));
        }
       // Prepare for check of contiguousness
       else
        {
         last_vertex=boundary_polyline_pt[i]->vertex_coordinate(j);
        }
      }
    }

   // Total number of vertices
   nvertex=polygon_vertex.size();

   // Counter for number of intersections
   unsigned intersect_counter=0;

   //Get first vertex
   Vector<double> p1=polygon_vertex[0];
   for (unsigned i=1;i<=nvertex;i++) 
    {
     // Get second vertex by wrap-around
     Vector<double> p2 = polygon_vertex[i%nvertex];

     if (Internal_point[1] > std::min(p1[1],p2[1])) 
      {
       if (Internal_point[1] <= std::max(p1[1],p2[1])) 
        {
         if (Internal_point[0] <= std::max(p1[0],p2[0]))
          {
           if (p1[1] != p2[1])
            {
             double xintersect = (Internal_point[1]-p1[1])*(p2[0]-p1[0])/
              (p2[1]-p1[1])+p1[0];
             if ( (p1[0] == p2[0]) || (Internal_point[0] <= xintersect) )
              {
               intersect_counter++;
              }
            }
          }
        }
      }
     p1 = p2;
    }
   

/*    oomph_info << Internal_point[0]<< " "  */
/*               << Internal_point[1]<< " "; */

   // Even number of intersections: outside
   if (intersect_counter%2==0)
    {
//     oomph_info << 1 << std::endl;
     
     std::ostringstream error_stream;
     error_stream
      << "The internal point at "
      << Internal_point[0]<< " "
      << Internal_point[1]
      << " isn't in the polygon that describes the internal closed curve!\n"
      << "Polygon vertices are at: \n";
     for (unsigned i=0;i<nvertex;i++)
      {
       error_stream << polygon_vertex[i][0] << " "
                    << polygon_vertex[i][1] << "\n";
      }
     error_stream 
      << "This may be because the internal point is defined by a\n"
      << "GeomObject that has deformed so much that it's \n"
      << "swept over the (initial) internal point.\n"
      << "If so, you should update the position of the internal point. \n"
      << "This could be done automatically by generating \n"
      << "an internal mesh inside the polygon and using one\n"
      << "of its internal nodes as the internal point. Actually not \n"
      << "why triangle doesn't do that automatically....\n";
     throw OomphLibError(
      error_stream.str(),
      "TriangleMeshInternalPolygon:: constructor ",
      OOMPH_EXCEPTION_LOCATION);
    }

#endif 
  }
  
 /// \short Empty Destuctor
 virtual ~TriangleMeshInternalPolygon() { }
 
  /// Output each sub-boundary at n_sample (default: 50) points
  /// and internal point
  void output(std::ostream &outfile, const unsigned& n_sample=50)
  {
   TriangleMeshPolygon::output(outfile,n_sample);
   outfile << "ZONE T=\"Internal point\"\n";
   outfile << Internal_point[0] << " " 
           << Internal_point[1] << "\n"; 
  }

};



/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////



//=====================================================================
/// Class definining a curvilinear internal closed curve for
/// triangle mesh.
//=====================================================================
class TriangleMeshCurvilinearClosedCurve :
public virtual TriangleMeshClosedCurve
{
 
 
  public:
 
 /// \short Constructor: Specify TriangleMeshCurviLine objects
 /// (at least two) that make up the closed curve
 TriangleMeshCurvilinearClosedCurve(Vector<TriangleMeshCurviLine*>
                                    curvilinear_boundary_pt) :
  Curvilinear_boundary_pt(curvilinear_boundary_pt)
  {
#ifdef PARANOID
   
   // Check that boundary bits match together
   unsigned nb=Curvilinear_boundary_pt.size();
   Vector<double> zeta(1);
   
   // Check last point of each boundary bit coincides with first point
   // on next one
   for (unsigned i=0;i<nb-1;i++)
    {
     TriangleMeshCurviLine* first_pt=Curvilinear_boundary_pt[i];
     GeomObject* geom_obj_pt=first_pt->geom_object_pt();
     zeta[0]=first_pt->zeta_end();
     Vector<double> end(2);
     geom_obj_pt->position(zeta,end);
     
     
     TriangleMeshCurviLine* second_pt=Curvilinear_boundary_pt[i+1];
     geom_obj_pt=second_pt->geom_object_pt();
     zeta[0]=second_pt->zeta_start();
     Vector<double> start(2);
     geom_obj_pt->position(zeta,start);
     
     double dist=sqrt(pow(start[0]-end[0],2)+pow(start[1]-end[1],2));
     if (dist>ToleranceForVertexMismatchInPolygons::Tolerable_error)
      {
       std::ostringstream error_stream;
       error_stream
        << "The start and end points of curvilinear boundary parts " << i 
        << " and " <<i+1<< " don't match when judged \nwith the tolerance of "
        << ToleranceForVertexMismatchInPolygons::Tolerable_error
        << " which is specified in the namespace \nvariable "
        << "ToleranceForVertexMismatchInPolygons::Tolerable_error.\n\n"
        << "Feel free to adjust this or to recompile the code without\n"
        << "paranoia if you think this is OK...\n"
        << std::endl;
       throw OomphLibError(
        error_stream.str(),
        "TriangleMeshCurvilinearClosedCurve:: constructor ",
        OOMPH_EXCEPTION_LOCATION);
      }
    }
   
   
   // Check wrap around
   {
    TriangleMeshCurviLine* first_pt=Curvilinear_boundary_pt[0];
    GeomObject* geom_obj_pt=first_pt->geom_object_pt();
    zeta[0]=first_pt->zeta_start();
    Vector<double> start(2);
    geom_obj_pt->position(zeta,start);
    
    TriangleMeshCurviLine* second_pt=Curvilinear_boundary_pt[nb-1];
    geom_obj_pt=second_pt->geom_object_pt();
    zeta[0]=second_pt->zeta_end();
    Vector<double> end(2);
    geom_obj_pt->position(zeta,end);
    
    double dist=sqrt(pow(start[0]-end[0],2)+pow(start[1]-end[1],2));
    if (dist>ToleranceForVertexMismatchInPolygons::Tolerable_error)
     {
      std::ostringstream error_stream;
      error_stream
       << "The start and end points of 1st and last curvilinear\n"
       << "boundary parts don't match when judged \nwith the tolerance of "
       << ToleranceForVertexMismatchInPolygons::Tolerable_error
       << " which is specified in the namespace \nvariable "
       << "ToleranceForVertexMismatchInPolygons::Tolerable_error.\n\n"
       << "Feel free to adjust this or to recompile the code without\n"
       << "paranoia if you think this is OK...\n"
       << std::endl;
      throw OomphLibError(
       error_stream.str(),
       "TriangleMeshCurvilinearClosedCurve:: constructor ",
       OOMPH_EXCEPTION_LOCATION);
     }
   }
   
#endif
  }
  
  /// \short Empty Destuctor
  virtual ~TriangleMeshCurvilinearClosedCurve() {}
  
  
 
  /// \short Number of TriangleMeshCurvilinearBoundaries that make up this
  /// internal closed curve
  unsigned ncurvilinear_boundary()
  {
   return Curvilinear_boundary_pt.size();
  }
  
 /// \short Access to the i-th TriangleMeshCurviLine 
 // that makes up this internal closed curve
 TriangleMeshCurviLine* curvilinear_boundary_pt(const unsigned& i)
  {
   return Curvilinear_boundary_pt[i];
  }
 

 /// Output each sub-boundary at n_sample (default: 50) points
 void output(std::ostream &outfile, const unsigned& n_sample=50)
  {
   unsigned nb=Curvilinear_boundary_pt.size();
   for (unsigned i=0;i<nb;i++)
    {
     Curvilinear_boundary_pt[i]->output(outfile,n_sample);
    }
  }

  private:
  
  /// Vector of curvlinear boundaries
  Vector<TriangleMeshCurviLine*> Curvilinear_boundary_pt;
  
  
};




//=====================================================================
/// Class definining an internal closed curve to define internal 
/// boundaries in a triangle mesh.
//=====================================================================
class TriangleMeshInternalCurvilinearClosedCurve : public virtual 
 TriangleMeshCurvilinearClosedCurve, 
 public virtual TriangleMeshInternalClosedCurve
{
 
  public:
 
 /// \short Constructor: Specify TriangleMeshCurviLine objects
 /// (at least two) that make up the boundary of the internal closed curve, and
 /// coordinates of a single point inside the closed curve
 TriangleMeshInternalCurvilinearClosedCurve(Vector<TriangleMeshCurviLine*>
                             curvilinear_boundary_pt,
                             const Vector<double>& internal_point) :
  TriangleMeshCurvilinearClosedCurve(curvilinear_boundary_pt), 
  TriangleMeshInternalClosedCurve(internal_point)
   {}
  
  
  /// \short Empty Destuctor
  virtual ~TriangleMeshInternalCurvilinearClosedCurve() {}
    
  /// Output each sub-boundary at n_sample (default: 50) points
  /// and internal point
  void output(std::ostream &outfile, const unsigned& n_sample=50)
  {
   TriangleMeshCurvilinearClosedCurve::output(outfile,n_sample);
   outfile << "ZONE T=\"Internal point\"\n";
   outfile << Internal_point[0] << " " 
           << Internal_point[1] << "\n"; 
  }
  
  
  private:
  
  /// Vector of curvlinear boundaries
  Vector<TriangleMeshCurviLine*> Curvilinear_boundary_pt;
    
};


/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////


//================================================================
/// Base class for triangle meshes (meshes made of 2D triangle elements).
//================================================================
class TriangleMeshBase : public virtual Mesh
{

public:

 /// Constructor 
 TriangleMeshBase() 
  {
#ifdef OOMPH_HAS_TRIANGLE_LIB

   //Initialise the TriangulateIO Data structure
   TriangleHelper::initialise_triangulateio(Triangulateio);

#endif
  }

 /// Broken copy constructor
 TriangleMeshBase(const TriangleMeshBase& node) 
  { 
   BrokenCopy::broken_copy("TriangleMeshBase");
  } 
 
 /// Broken assignment operator
 void operator=(const TriangleMeshBase&) 
  {
   BrokenCopy::broken_assign("TriangleMeshBase");
  }
 
 /// Destructor (empty)
 virtual ~TriangleMeshBase()
  {
#ifdef OOMPH_HAS_TRIANGLE_LIB  
   //Clear the triangulate data structure 
   TriangleHelper::clear_triangulateio(Triangulateio);
#endif
  }

 /// Setup lookup schemes which establish whic elements are located
 /// next to mesh's boundaries (wrapper to suppress doc).
 void setup_boundary_element_info()
  {
   std::ofstream outfile;
   setup_boundary_element_info(outfile);
  }

 /// \short Setup lookup schemes which establish which elements are located
 /// next to mesh's boundaries. Doc in outfile (if it's open).
 void setup_boundary_element_info(std::ostream &outfile);

#ifdef OOMPH_HAS_TRIANGLE_LIB

 /// Access to the triangulateio representation of the mesh
 TriangulateIO& triangulateio_representation() {return Triangulateio;}
 
 /// \short Helper function. Write a TriangulateIO object file with all the 
 /// triangulateio fields. String s is add to assign a different value for
 /// the input and/or output structure
 void write_triangulateio(TriangulateIO& triangulate_io, std::string& s);
 
 /// \short Helper function. Clean up the memory associated with the
 /// TriangulateIO object. This should really only be used to save
 /// memory in extremely tight situations.
 void clear_triangulateio()
  {TriangleHelper::clear_triangulateio(Triangulateio);}

 /// \short Dump the triangulateio structure to a dump file
 void dump_triangulateio(std::ostream &dump_file)
  {TriangleHelper::dump_triangulateio(Triangulateio,dump_file);}

 /// \short Regenerate the mesh from a dumped triangulateio file
 void remesh_from_triangulateio(std::istream &restart_file)
  {
   //Clear the existing triangulate io
   TriangleHelper::clear_triangulateio(Triangulateio);
   //Read the data into the file
   TriangleHelper::read_triangulateio(restart_file,Triangulateio);
   //Now remesh from the new data structure
   this->remesh_from_internal_triangulateio();
  }

 ///Virtual function that is used for specific remeshing from the triangulateio
 virtual void remesh_from_internal_triangulateio() 
  {
   std::ostringstream error_stream;
   error_stream << "Empty default remesh function called.\n";
   error_stream << "This should be overloaded in a specific TriangleMesh\n";
   throw OomphLibError(error_stream.str(),
                       "TriangleBaseMesh::remesh_from_triangulateio()",
                       OOMPH_EXCEPTION_LOCATION);
  }

#endif

  protected:

#ifdef OOMPH_HAS_TRIANGLE_LIB

 ///\short TriangulateIO representation of the mesh
 TriangulateIO Triangulateio;

#endif 
 
};




}

#endif

