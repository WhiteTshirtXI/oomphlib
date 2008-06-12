#!MC 1100 




##################################################
# INDEX OF OUTPUT FILE WITH COARSE/FINE PLOTS
##################################################
$!VARSET |COARSE|=2
$!VARSET |FINE|=3

##################################################
# PNG OR AVI OUTPUT?
##################################################
$!VARSET |PNG|=1

##################################################
# WHAT IT SAYS ...WHEN PLOTTING SLICES [0/1]
##################################################
$!VARSET |SHOW_3D_MESH|=0

##################################################
# PLOT Y SLICE TOO?
##################################################
$!VARSET |PLOT_Y_SLICE|=1

        
$!VARSET |PNG_COUNT|=0


#===================================================
# Loop over the coarse and fine plots
#===================================================
$!LOOP 2


$!IF |LOOP|==1
        $!VARSET |STEP|=|COARSE|
$!ELSE
        $!VARSET |STEP|=|FINE|
$!ENDIF


$!NEWLAYOUT

$!FRAMELAYOUT ISTRANSPARENT = NO
$!FRAMELAYOUT WIDTH = 5
$!PAPER ORIENTPORTRAIT = YES
$!PAPER PAPERSIZE = A4
$!DRAWGRAPHICS NO
$!WORKSPACEVIEW FITPAPER

$!FRAMELAYOUT WIDTH = 6
$!FRAMELAYOUT HEIGHT = 6

$!READDATASET  '"RESLT/soln|STEP|.dat" ' 
  READDATAOPTION = NEW
  RESETSTYLE = YES
  INCLUDETEXT = NO
  INCLUDEGEOM = NO
  INCLUDECUSTOMLABELS = NO
  VARLOADMODE = BYNAME
  ASSIGNSTRANDIDS = YES
  INITIALPLOTTYPE = CARTESIAN3D
$!THREEDAXIS XDETAIL{SHOWAXIS = YES}
$!THREEDAXIS YDETAIL{SHOWAXIS = YES}
$!THREEDAXIS ZDETAIL{SHOWAXIS = YES}
$!THREEDAXIS GRIDAREA{ISFILLED = NO}
$!FIELDMAP [1-|NUMZONES|]  MESH{COLOR = BLACK}
$!FIELDLAYERS SHOWMESH = YES
$!FIELDMAP [1-|NUMZONES|]  MESH{LINETHICKNESS = 0.02
$!FIELDLAYERS SHOWEDGE = NO
$!GLOBALCONTOUR 1  VAR = 4
$!CONTOURLEVELS RESETTONICE
  CONTOURGROUP = 1
  APPROXNUMVALUES = 15
$!FIELDLAYERS SHOWCONTOUR = YES
$!THREEDVIEW 
  PSIANGLE = 60.98
  THETAANGLE = -114.881
  ALPHAANGLE = -1.19287
  VIEWERPOSITION
    {
    X = 8.70930832044
    Y = 4.30731673267
    Z = 5.52020035129
    }
$!VIEW FIT

$!SLICELAYERS SHOW = YES
$!SLICEATTRIBUTES 1  PRIMARYPOSITION{X = 0.48}
$!SLICEATTRIBUTES 1  MESH{SHOW = NO}

$!IF |PLOT_Y_SLICE|==1
        $!SLICEATTRIBUTES 2  SHOWGROUP = YES
        $!SLICEATTRIBUTES 2  PRIMARYPOSITION{Y = 0.48}
        $!SLICEATTRIBUTES 2  MESH{SHOW = NO}
$!ENDIF


$!PICK ADDALL
  SELECTFRAMES = YES
$!PICK COPY
$!PICK PASTE
$!FRAMELAYOUT XYPOS{Y = 6.1256}
$!READDATASET  '"RESLT/exact_soln|STEP|.dat" ' 
  READDATAOPTION = NEW
  RESETSTYLE = NO
  INCLUDETEXT = NO
  INCLUDEGEOM = NO
  INCLUDECUSTOMLABELS = NO
  VARLOADMODE = BYNAME
  ASSIGNSTRANDIDS = YES
  INITIALPLOTTYPE = CARTESIAN3D
$!LINKING BETWEENFRAMES{LINKFRAMESIZEANDPOSITION = YES}
$!LINKING BETWEENFRAMES{LINK3DVIEW = YES}
$!LINKING BETWEENFRAMES{LINKFRAMESIZEANDPOSITION = NO}
$!LINKING BETWEENFRAMES{LINKSLICEPOSITIONS = YES}
$!LINKING BETWEENFRAMES{LINKCONTOURLEVELS = YES}
$!PROPAGATELINKING 
  LINKTYPE = BETWEENFRAMES
  FRAMECOLLECTION = ALL



$!ATTACHTEXT 
  ANCHORPOS
    {
    X = 7.71922020951
    Y = 92.8758882093
    }
  TEXT = 'Exact solution' 

$!FRAMECONTROL PUSHTOP



$!ATTACHTEXT 
  ANCHORPOS
    {
    X = 7.71922020951
    Y = 92.8758882093
    }
  TEXT = 'FE solution' 

$!DRAWGRAPHICS YES
$!REDRAWALL


$!IF |STEP|==|COARSE|

$!IF |PNG|==0
        $!EXPORTSETUP
          EXPORTREGION = ALLFRAMES
          EXPORTFORMAT = AVI
          EXPORTFNAME = "compare.avi"
        $!EXPORTSETUP IMAGEWIDTH = 400
        $!EXPORTSETUP USESUPERSAMPLEANTIALIASING = YES
        $!EXPORTSTART
$!ELSE
        $!EXPORTSETUP EXPORTFORMAT = PNG
        $!EXPORTSETUP IMAGEWIDTH = 400
        $!EXPORTSETUP USESUPERSAMPLEANTIALIASING = YES
        $!EXPORTSETUP EXPORTFNAME = 'ccompare|PNG_COUNT|.png'
        $!EXPORT
          EXPORTREGION = ALLFRAMES
        $!VARSET |PNG_COUNT|+=1
$!ENDIF

$!ENDIF



$!FIELDMAP [1-|NUMZONES|]  CONTOUR{SHOW=NO}
$!IF |SHOW_3D_MESH|==0
        $!FIELDMAP [1-|NUMZONES|]  MESH{SHOW=NO}
$!ENDIF
$!FRAMECONTROL PUSHTOP
$!FIELDMAP [1-|NUMZONES|]  CONTOUR{SHOW=NO}
$!IF |SHOW_3D_MESH|==0
        $!FIELDMAP [1-|NUMZONES|]  MESH{SHOW=NO}
$!ENDIF

$!VARSET |NSTEP|=10
$!LOOP |NSTEP|
        $!VARSET |X_SLICE|=(|MINX|+(|MAXX|-|MINX|)*(|LOOP|-1)/(|NSTEP|-1))
        $!SLICEATTRIBUTES 1  PRIMARYPOSITION{X = |X_SLICE|}
        $!IF |PLOT_Y_SLICE|==1
          $!VARSET |Y_SLICE|=(|MINY|+(|MAXY|-|MINY|)*(|LOOP|-1)/(|NSTEP|-1))
          $!SLICEATTRIBUTES 2  PRIMARYPOSITION{Y = |Y_SLICE|}
        $!ENDIF
        $!REDRAWALL
        $!IF |STEP|==|FINE|
        $!IF |PNG|==0
                $!EXPORTNEXTFRAME
        $!ELSE
                $!EXPORTSETUP EXPORTFORMAT = PNG
                $!EXPORTSETUP IMAGEWIDTH = 400
                $!EXPORTSETUP USESUPERSAMPLEANTIALIASING = YES
                $!EXPORTSETUP EXPORTFNAME = 'compare|PNG_COUNT|.png'
                $!EXPORT
                  EXPORTREGION = ALLFRAMES
                $!VARSET |PNG_COUNT|+=1
        $!ENDIF
        $!ENDIF

$!ENDLOOP

$!ENDLOOP

$!IF |PNG|==0
        $!EXPORTFINISH
$!ENDIF