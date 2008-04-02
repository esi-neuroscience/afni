#include "SUMA_suma.h"

extern SUMA_CommonFields *SUMAg_CF; 

static char s_ver[100];

static char * SUMA_ver2date(int ver)
{
   int yy, mm, dd;
   yy = ver/10000;
   mm = (ver % 10000) / 100;
   dd = ver % 100;
   sprintf(s_ver,"%d_%d_%d", yy, mm, dd);
   return(s_ver);
} 

typedef struct {
   char *envhelp;
   char *envname;
   char *envval;  /* This is the default */
}ENV_SPEC;

static ENV_SPEC envlist[] = {
   {  "Incremental arrow rotation angle in degrees",
      "SUMA_ArrowRotAngle",
      "5" } ,
   {  "Color pattern (AFNI, EURO, DEFAULT)",
      "SUMA_ColorPattern",
      "EURO" },
   {  "Swap mouse buttons 1 and 3",
      "SUMA_SwapButtons_1_3",
      "NO" },
   {  "Background color r g b. No space between values",
      "SUMA_BackgroundColor",
      "0.0,0.0,0.0" },
   {  "ROI color map (bgyr64, roi64, roi128, roi256)",
      "SUMA_ROIColorMap",
      "roi256" },
   {  "Number of smoothing operations to run on convexity data",
      "SUMA_NumConvSmooth",
      "5" },
   {  "Colormap for convexity (gray02, gray_i02, ngray20, bgyr64, etc.)",
      "SUMA_ConvColorMap",
      "gray02" },
   {  "Brightness factor for convexity ",
      "SUMA_ConvBrightFactor",
      "0.5" },
   {  "Number of smoothing operations to run on mixed foregroung color plane\n"
      " before mixing with background",
      "SUMA_NumForeSmoothing",
      "0" },
   {  "Setup the color mixing mode (ORIG, MOD1) ",
      "SUMA_ColorMixingMode",
      "ORIG" },
   {  "Port for communicating with AFNI\n"
      " Listening ports are derived from SUMA_AFNI_TCP_PORT\n"
      " Listening port i\n"
      " SUMA_AFNI_TCP_PORT + i (i > 0)",
      "SUMA_AFNI_TCP_PORT",
      "53211" },
   {  "Warn before closing with the Escape key (YES/NO)",
      "SUMA_WarnBeforeClose",
      "YES" },
   {  "Mask node values\n"
      " 0 ? YES/NO",
      "SUMA_MaskZero",
      "YES" },
   {  "Threshold if Val < thr (NO) or | Val | < | Thr | (YES)",
      "SUMA_AbsThreshold",
      "YES" },
   {  "Threshold scale precision. 2 is the minimum allowed. \n"
      " This value might be overriden in SUMA.",
      "SUMA_ThresholdScalePower",
      "2" },
   {  "Center of Rotation is based on nodes used in the mesh and not \n"
      " on all the nodes in NodeList",
      "SUMA_CenterOnPatch",
      "NO" },
   {  "Use cross ticks on axis ?",
      "SUMA_UseCrossTicks",
      "NO" },
   {  "Warn if 1D file looks like it needs a transpose",
      "SUMA_1D_Transponse_Warn",
      "YES" },
   {  "Adjust roation and translation factor of mouse with changes \n"
      " in zoom levels ",
      "SUMA_AdjustMouseMotionWithZoom",
      "YES" },
   {  "Use orthographic projection ",
      "SUMA_ViewOrthographicProjection",
      "NO" },
   {  "Percent gain for zooming in and out with the 'z' and 'Z' keys. \n"
      " Typical range from 0 to 50",
      "SUMA_KeyZoomGain",
      "5" },
   {  "Original FOV. Set between 1.0 and 100.0 \n"
      " Default is 30.0, -1 == auto",
      "SUMA_FOV_Original",
      "-1" },
   {  "light0 color",
      "SUMA_Light0Color",
      "1.0,1.0,1.0" },
   {  "Ambient light ",
      "SUMA_AmbientLight",
      "1.0,1.0,1.0" },
   {  "Allow for replacement of pre-loaded dsets",
      "SUMA_AllowDsetReplacement",
      "NO" },
   {  "Allow for surfaces with same DomainGrandParentID to share overlays",
      "SUMA_ShareGrandChildrenOverlays",
      "NO" },
   {  "Increase the resolution of images recorded with 'r' button.\n"
      " Increase is done by taking multiple shots that once stitched  \n"
      " together form a high-resolution image.\n"
      " The maximum resolution is set by the GL_MAX_VIEWPORT_DIMS of your\n" 
      " graphics card. I have 4096 pixels.\n"
      " If you exceed this number, SUMA will make adjustments automatically.\n"
      " Assemble images with program imcat.",
      "SUMA_SnapshotOverSampling",
      "1" },
   {  "Ignore consecutive duplicate images in recorder",
      "SUMA_NoDuplicatesInRecorder",
      "YES" },
   {  "start NIML (can't do this for more than one suma at a time!)",
      "SUMA_START_NIML",
      "YES" },
   {  "Allow (YES) datasets with the same filename but differing ID \n"
      " to be considered the same.\n"
      " This is only useful with SUMA_AllowDsetReplacement",
      "SUMA_AllowFilenameDsetMatch",
      "NO" },
   {  "Freeze zoom across states",
      "SUMA_FreezeFOVAcrossStates",
      "NO" },
   {  "Dset color map",
      "SUMA_DsetColorMap",
      "Spectrum:red_to_blue" },
   {  "Show only selected dset in suma's surface controller.",
      "SUMA_ShowOneOnly",
      "YES" },
   {  "Update graphs, even SUMA_ShowOneOnly (or suma's '1 Only') is turned on.",
      "SUMA_GraphHidden",
      "YES" },
   {  "Fraction of colormap to rotate with up/down arrow keys.",
      "SUMA_ColorMapRotationFraction",
      "0.05"},
   {  "Size of surface controller font. \n"
      " Values are SMALL, BIG (old style).",
      "SUMA_SurfContFontSize",
      "SMALL"},
   {  NULL, NULL, NULL  }
};
      


char * SUMA_env_list_help(){
   static char FuncName[]={"SUMA_env_list_help"};
   int i=0;
   char *sli=NULL;
   SUMA_STRING *SS=NULL;
   char *s=NULL, *eee=NULL, *userval=NULL;
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend(NULL, NULL);
   
   while (envlist[i].envhelp) {
      /* find the user's setting */
      char *eee = getenv(envlist[i].envname);
      if (userval) 
         SUMA_free(userval); 
      userval=NULL;
      if (!eee) userval = SUMA_copy_string(envlist[i].envval);
      else userval = SUMA_copy_string(eee);
      sli = SUMA_ReplaceChars(envlist[i].envhelp,"\n","\n//      ");
      SS = SUMA_StringAppend_va(SS,
                     "// %03d-%s:\n"
                     "//     %s\n"
                     "//     default:   %s = %s\n"
                     "   %s = %s\n",
                     i, envlist[i].envname,
                     sli,
                     envlist[i].envname,
                     envlist[i].envval,
                     envlist[i].envname,
                     userval);
      SUMA_free(sli); sli = NULL;
      ++i;
   }
   SUMA_SS2S(SS,s);
   
   SUMA_RETURN(s);
}

/*!
   \brief Returns a string with the new additions and version information
   
   \param ver (float) v (v > 0) for info on version v alone 
                      0.0 just the latest version info
                      -1.0 for all versions
   \param StampOnly (SUMA_Boolean) Want version number and time stamp only ?
                      
   \return s (char *) the string, yall have to free it with SUMA_free
   \sa SUMA_New_Additions_perver
   
   - To add a new version, you must add a case statement in SUMA_New_Additions_perver
     AND add the version number in the beginning of SUMA_VERSION_VECTOR  in SUMA_DataSets.h
*/

static int verv[] = { SUMA_VERSION_VECTOR }; 

char * SUMA_New_Additions (int ver, SUMA_Boolean StampOnly)
{
   static char FuncName[]={"SUMA_New_Additions"};
   char *s = NULL;
   int i;
   SUMA_STRING *SS = NULL;
   
   SUMA_ENTRY;

   SS = SUMA_StringAppend (NULL, NULL);
   
   if (ver == 0) { /* just the latest */
      s = SUMA_New_Additions_perver( verv[0], StampOnly);
      if (s) {
         SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;
      }
   } else if (ver < 0) {
      /* all history */
      SS = SUMA_StringAppend (SS, "All Version Info:\n"); 
      i = 0;
      while (verv[i] > 0) {
         s = SUMA_New_Additions_perver( verv[i], StampOnly);
         if (s) {
         SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;
         SS = SUMA_StringAppend (SS, "\n");
         }
         ++i;
      }
      
   } else {
      /* just for ver */
      s = SUMA_New_Additions_perver( ver, StampOnly);
      if (s) {
         SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;
      }
   }
   
   /* add the CVS tag */
   SS = SUMA_StringAppend_va (SS, "\nCVS tag:\n   %s\n", SUMA_VERSION_LABEL);
   
   /* add the compile date */
   SS = SUMA_StringAppend_va (SS, "\nCompile Date:\n   %s\n",__DATE__);
   
   /* clean SS */
   SS = SUMA_StringAppend (SS, NULL);
   /* copy s pointer and free SS */
   s = SS->s;
   SUMA_free(SS); 
   
   SUMA_RETURN(s);      
   
}

 
/*!
   \brief Returns a string with version information
   \param ver (float) Version number
   \param StampOnly (SUMA_Boolean) if YUP 
                     then return the time stamp of the version only)
   \return s (char *) the string, yall have to free it with SUMA_free
   \sa SUMA_New_Additions
   
   - To add a new version, you must add a case statement in SUMA_New_Additions_perver
     AND add the version number in the beginning of SUMA_VERSION_VECTOR  in SUMA_DataSets.h
*/
char * SUMA_New_Additions_perver (int ver, SUMA_Boolean StampOnly)
{
   static char FuncName[]={"SUMA_New_Additions_perver"};
   char *s = NULL;
   SUMA_STRING *SS = NULL;
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend (NULL, NULL);
   
   
   switch (ver) {
      /* Must modify SUMA_VERSION_VECTOR  in SUMA_DataSets.h when add a new case  */
      /*
      case XX:
         SS = SUMA_StringAppend_va(SS, 
            "++ SUMA version %s\n", SUMA_ver2date(ver)); if (StampOnly) break;
         SS = SUMA_StringAppend(SS, 
            "New Programs:\n"
            "  + \n"
            "Modifications:\n"
            "  + \n");
         break; 
      */
      case 20060703:
         SS = SUMA_StringAppend_va(SS, 
            "++ SUMA version %s\n", SUMA_ver2date(ver)); if (StampOnly) break;
         SS = SUMA_StringAppend(SS, 
         "New Programs:\n"
         "  + SurfDsetInfo: Program to display surface dataset information.\n"
         "  + AnalyzeTrace: Program to analyze the output of -trace option.\n"
         "  + DriveSuma: Program to control SUMA from the command line\n"
         "  + imcat: Program to catenate images.\n"
         "  + Surf2VolCoord: Surface-node to voxel correspondence.\n"
         "  + SurfDist: Program to calculate internodal distances.\n"
         "Modifications:\n"
         "  + SUMA:\n"
         "    o Addition of new Displayable Objects (DO)(ctrl+Alt+s)\n"
         "    o Allow replacement of pre-loaded DO and Dsets\n"
         "    o Support for .niml.dset as format for surface-based anlysis\n"
         "    o High resolution image saving with ctrl+r\n"
         "    o Bug fixes for support of niml dset format\n"
         "    o Use of '[i]' to select node index from surface dset\n"
         "    o Scroll lists for I T and B selectors in SUMA\n"
         "    o Graphing of dset content with 'g'\n"
         "  + ConvertDset:\n"
         "    o Output of full dsets if needed\n"
         "  + ROIgrow:\n"
         "    o Grows regions separately, depending on labels.\n"
         "  + ROI2dataset:\n"
         "    o outputs full datasets if needed.\n"
         "  + SurfSmooth:\n"
         "    o Improved HEAT_05 method.\n"
         "    o New 'blurring to' a FWHM with HEAT_07 method.\n"
         "  + SurfFWHM:\n"
         "    o Estimating FWHM on the surface.\n" 
         "  + MapIcosahedron:\n"
         "    o Better handling of surface centers. \n"
            );
         break; 
      case 20041229:
         SS = SUMA_StringAppend_va(SS, 
            "++ SUMA version %s\n", SUMA_ver2date(ver)); if (StampOnly) break;
         SS = SUMA_StringAppend(SS, 
            "New Programs:\n"
            "  + SurfClust: Program to find clusters of activation\n"
            "               on the surface.\n"
            "  + IsoSurface: Program to create isosurfaces from AFNI volumes.\n"
            "  + ConvexHull: Program to create the convex hull of a set of\n"
            "                points.\n"
            "  + 3dSkullStrip: Program to remove the skull from anatomical \n"
            "                  volumes.\n"
            "  + 3dCRUISEtoAFNI: Program to convert CRUISE volumes to AFNI\n"
            "  + 3dBRAIN_VOYAGERtoAFNI: Program to convert BrainVoyager .vmr\n"
            "                           volumes to AFNI\n"
            "  + SurfMesh: Program to increase or decrease a mesh's density.\n"
            "  + SurfMask: Program to find the volume enclosed by a surface.\n"
            "  + SurfToSurf: Program to interpolate between non-isotopic surfaces.\n"
            "  + ROIgrow: Program to expand an ROI on the surface.\n"
            "Modifications:\n"
            "  + SUMA:\n"
            "    o Slight modification to threshold scale.\n"
            "    o Added environment variable SUMA_ThresholdScalePower.\n"
            "    o Fixed a few kinks in the surface controller.\n"
            "    o Fixed ROI drawing trace on OSX.\n"
            "    o Added geodesic distance measurements in ROI drawing\n"
            "    controller.\n"
            "    o Suma can read surfaces specified on command line.\n"
            "    o Fixed bug reading AFNI generated niml files.\n"
            "    o Useful axis displayed with F2 key.\n"
            "    o Fixed bug with recursive function used to fill ROIs.\n"
            "    o Support for reading CRUISE surfaces in OpenDX format\n"
            "    o Support for reading BrainVoyager surfaces (.srf) format\n"
            "    o Mouse motion effect is modulated with Zoom level\n"
            "    o F8 toggles between orthographic and perspective viewing\n"
            "    o Fixed bug causing crash in SUMA_MixColorOverlays\n"
            "  + ConvertSurface:\n"
            "    o Option -make_consistent added to make the winding\n"
            "    of the mesh consistent.  \n"
            "    o Option to project surface nodes to sphere\n" 
            "  + SurfQual:\n"
            "    o Checks and warns about mesh's winding inconsistency.\n"
            "  + SurfSmooth:\n"
            "    o Added NN_geom, nearest neighbor interpolation option.\n"
            "    o Combined with -match_vol or -match_area, this geometry\n"
            "    smoothing mode can be used to inflate surfaces.\n"
            "    o New weighting for Taubin geometry filtering.\n"
            "    o Option for masking nodes to be filtered.\n"
            "    o New HEAT method for filtering data.\n"
            "  + SurfaceMetrics:\n"
            "    o Option -vol calculates the volume of the closed surface.\n"
            "  + SurfPatch:\n"
            "    o Option -vol to calculate the volume between two isotopic\n"
            "    surface patches.\n"
            "  + ROI2dataset:\n"
            "    o Option -pad_to_node and -pad_label to output datasets\n"
            "    containing full node listings.\n"
            "  + ConvertDset:\n"
            "    o Option -o_1dp was added to write 1D file data only,\n"
            "    without additional comments.\n"
            "  + SurfaceMetrics:\n"
            "    o Options for finding surface boundaries.\n"
            "  + 3dSkullStrip:\n"
            "    o Fixed bug on 64 bit machines.\n"
            );
         break; 
         
      case 20040610:   /* used to be 25000 */
         SS = SUMA_StringAppend_va(SS, 
            "++ SUMA version %s (used to be 2.500)\n", SUMA_ver2date(ver)); if (StampOnly) break;
         SS = SUMA_StringAppend(SS, 
            "Modifications:\n"
            "  + SUMA's surface controller 'ctrl+s' has been\n"
            "    vastly improved. \n"
            "    Of note are the following features:\n"
            "     - interactive color mapping\n"
            "     - thresholding controls \n"
            "     - brightness modulation\n"
            "     - choice of colormaps\n"
            "     - coordinate bias (tres tres cool)\n"
            "     - info on current cross hair location\n"
            "    Use Bhelp button in the controller for detailed help.\n"
            "  + 3dVol2Surf can output NIML formatted datasets.\n"
            "    Options -first_node and -last_node can be used\n"
            "    to restrict the mapping to a subset of the nodes.\n"
            "    That is useful if your output file size exceeds 2GB.\n"
            "Bug Fix:\n"
            "  + Fixed bug on Mac OS-X that cause all viewers to close\n"
            "    after pressing 'Yes' on the 'Close this viewer' prompt.\n"  
            );
         break;
         
      case 20040116:    /* used to be 24800 */
         SS = SUMA_StringAppend_va(SS, 
            "++ SUMA version %s (used to be 2.480)\n", SUMA_ver2date(ver)); if (StampOnly) break;
         SS = SUMA_StringAppend(SS, 
            "New Programs:\n"
            "  + FS_readannot: Program to read FreeSurfer's\n"
            "                  annotation files.\n"
            "  + SurfPatch: Program to create surface patches\n"
            "               from a set of nodes.\n"
            "  + SurfQual: Program to report defects in surfaces.\n"
            "              For the moment, works on spherical \n"
            "              surfaces only.\n"
            "Modifications:\n"
            "  + Added affine transforms to ConvertSurface.\n"
            "  + Added datasets into SUMA's code (no interface).\n"
            "  + Added saving/loading of viewer settings.\n"
            "  + Beginning of multiple group support in SUMA.\n"
            "  + Redisplays of Surface Viewers due to X events\n"
            "    are no longer passed to the image recorder.\n" );
         break; 
         
      case 20040106:    /* used to be 24500 */
         SS = SUMA_StringAppend_va(SS, 
            "++ SUMA version %s (used to be 2.450)\n", SUMA_ver2date(ver)); if (StampOnly) break;
         SS = SUMA_StringAppend(SS, 
            "New Programs:\n"
            "  + inspec: Shows the contents of a spec file\n"
            "  + quickspec: Creates a minimal spec file for one\n"
            "               or a bunch of surfaces.\n"
            "  + SurfSmooth: Smoothes surface data or geometry\n"
            "  + SurfMeasures: Outputs various surface attributes  \n"
            "                  and measurements such as:\n"
            "                  Thickness, Area, Volume, etc.\n"
            "Modifications:\n"
            "  + Foreground color smoothing option (SUMA keyb. 8)\n"
            "  + No more MappingRef field in Spec files.\n"
            "    The field is broken up into a set of other\n"
            "    fields for more flexibility.\n"
            "  + Surface input to command-line programs is \n"
            "    now done via -spec files too.\n"
            "  + One-way communication with SUMA via niml.\n"
            "    Only available with SurfSmooth for the moment.\n"
            "  + Began, in good faith, to update the new version \n"
            "    information.\n"); 
         break;
      
      default:
         SS = SUMA_StringAppend_va(SS, "++ %d? No such version, fool!\n", ver);
         break;
   }
   
   /* clean SS */
   SS = SUMA_StringAppend (SS, NULL);
   /* copy s pointer and free SS */
   s = SS->s;
   SUMA_free(SS); 
   
   SUMA_RETURN(s);
}

/*!
   \brief function called when help window is open
*/
void SUMA_Help_open (void *p)
{
   static char FuncName[]={"SUMA_Help_open"};

   SUMA_ENTRY;
   /* nothing to do here */
   
   SUMA_RETURNe;
}

/*!
   \brief function called when help window is destroyed
*/
void SUMA_Help_destroyed (void *p)
{
   static char FuncName[]={"SUMA_Help_destroyed"};
   
   SUMA_ENTRY;

   SUMAg_CF->X->Help_TextShell = NULL;
   
   SUMA_RETURNe;
}
/*!
   \brief function called when help window is open
*/
void SUMA_Help_Cmap_open (void *p)
{
   static char FuncName[]={"SUMA_Help_Cmap_open"};

   SUMA_ENTRY;
   /* nothing to do here */
   
   SUMA_RETURNe;
}
void SUMA_Help_Plot_open (void *p)
{
   static char FuncName[]={"SUMA_Help_Plot_open"};

   SUMA_ENTRY;
   /* nothing to do here */
   
   SUMA_RETURNe;
}

/*!
   \brief function called when help window is destroyed
*/
void SUMA_Help_Cmap_destroyed (void *p)
{
   static char FuncName[]={"SUMA_Help_Cmap_destroyed"};
   
   SUMA_ENTRY;

   SUMAg_CF->X->Help_Cmap_TextShell = NULL;
   
   SUMA_RETURNe;
}
/*!
   \brief function called when help window is destroyed
*/
void SUMA_Help_Plot_destroyed (void *p)
{
   static char FuncName[]={"SUMA_Help_Plot_destroyed"};
   
   SUMA_ENTRY;

   SUMAg_CF->X->Help_Plot_TextShell = NULL;
   
   SUMA_RETURNe;
}

/*!
   \brief function called when Message window is open
*/
void SUMA_Message_open (void *p)
{
   static char FuncName[]={"SUMA_Message_open"};

   SUMA_ENTRY;
   /* nothing to do here */
   
   SUMA_RETURNe;
}

/*!
   \brief function called when Message window is destroyed
*/
void SUMA_Message_destroyed (void *p)
{
   static char FuncName[]={"SUMA_Message_destroyed"};
   
   SUMA_ENTRY;

   SUMAg_CF->X->Log_TextShell = NULL;
   
   SUMA_RETURNe;
}

char * SUMA_sources_Info(void)
{
   static char FuncName[]={"SUMA_sources_Info"};
   char *s = NULL;
   SUMA_STRING *SS = NULL;
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend (NULL, NULL);
   
   SS = SUMA_StringAppend(SS,
   "Parts of SUMA use functions from various sources.\n"
   "Library sources:\n"
   "----------------\n"
   "* MarchingCubes:\n"
   "  The code for the heart of the program IsoSurface is a translation of:\n"
   "  Thomas Lewiner's C++ implementation of the algorithm in:\n"
   "  Efficient Implementation of Marching Cubes� Cases with Topological Guarantees\n"
   "  by Thomas Lewiner, H�lio Lopes, Ant�nio Wilson Vieira and Geovan Tavares \n"
   "  in Journal of Graphics Tools. \n"
   "  http://www-sop.inria.fr/prisme/personnel/Thomas.Lewiner/JGT.pdf\n"
   "  Permission to use this translation in other programs must be obtained \n"
   "  from Mr. Lewiner.\n"
   "\n"
   "* 3d Edge Detection:\n"
   "  The code for 3dEdge detection is from the library 3DEdge \n"
   "  by Gregoire Malandain (gregoire.malandain@sophia.inria.fr)\n"
   "  References for the algorithms:\n"
   "  -  Optimal edge detection using recursive filtering\n"
   "     R. Deriche, International Journal of Computer Vision,\n"
   "     pp 167-187, 1987.\n"
   "  -  Recursive filtering and edge tracking: two primary tools\n"
   "     for 3-D edge detection, O. Monga, R. Deriche, G. Malandain\n"
   "     and J.-P. Cocquerez, Image and Vision Computing 4:9, \n"
   "     pp 203-214, August 1991.\n"
   "\n"
   "* QHull:\n"
   "                       Qhull, Copyright (c) 1993-2001 \n"
   "  The National Science and Technology Research Center for \n"
   "          Computation and Visualization of Geometric Structures \n"
   "                          (The Geometry Center) \n"
   "                         University of Minnesota \n"
   "                              400 Lind Hall \n"
   "                          207 Church Street S.E. \n"
   "                         Minneapolis, MN 55455  USA \n"
   "   \n"
   "                         email: qhull@geom.umn.edu \n"
   "   \n"
   "  This software includes Qhull from The Geometry Center.  Qhull is  \n"
   "  copyrighted as noted above.  Qhull is free software and may be obtained  \n"
   "  via anonymous ftp from geom.umn.edu.  It may be freely copied, modified,  \n"
   "  and redistributed under the following conditions: \n"
   "   \n"
   "  1. All copyright notices must remain intact in all files. \n"
   "   \n"
   "  2. A copy of this text file must be distributed along with any copies  \n"
   "     of Qhull that you redistribute; this includes copies that you have  \n"
   "     modified, or copies of programs or other software products that  \n"
   "     include Qhull. \n"
   "   \n"
   "  3. If you modify Qhull, you must include a notice giving the \n"
   "    name of the person performing the modification, the date of \n"
   "     modification, and the reason for such modification. \n"
   "   \n"
   "  4. When distributing modified versions of Qhull, or other software \n"
   "     products that include Qhull, you must provide notice that the original \n"
   "     source code may be obtained as noted above.\n"
   "  \n"
   "  5. There is no warranty or other guarantee of fitness for Qhull, it is \n"
   "     provided solely ""as is"".  Bug reports or fixes may be sent to \n"
   "     qhull_bug@geom.umn.edu; the authors may or may not act on them as \n"
   "    they desire.\n"
   "  \n"
   "\n"
   "* GLUT: Mark. J. Kilgard's code from the book 'Programming OpenGL for \n"
   "        the X Window System' (ISBN:0-201-48359-9) published by \n"
   "        Addison-Wesley.\n"
   "  NOTICE:  This source code distribution contains source code contained\n"
   "  in the book 'Programming OpenGL for the X Window System' (ISBN:\n"
   "  0-201-48359-9) published by Addison-Wesley.  The programs and\n"
   "  associated files contained in the distribution were developed by Mark\n"
   "  J. Kilgard and are Copyright 1994, 1995, 1996 by Mark J.  Kilgard\n"
   "  (unless otherwise noted).  The programs are not in the public domain,\n"
   "  but they are freely distributable without licensing fees.  These\n"
   "  programs are provided without guarantee or warrantee expressed or\n"
   "  implied.\n"
   "\n"
   "* PLY: A set of functions for reading/writing PLY polygon files by\n"
   "       Greg Turk.\n"
   "  Copyright (c) 1994 The Board of Trustees of The Leland Stanford\n"
   "  Junior University.  All rights reserved.   \n"
   "   \n"
   "  Permission to use, copy, modify and distribute this software and its  \n" 
   "  documentation for any purpose is hereby granted without fee, provided \n"  
   "  that the above copyright notice and this permission notice appear in  \n" 
   "  all copies of this software and that you do not sell the software.   \n"
   "    \n"
   "  THE SOFTWARE IS PROVIDED 'AS IS' AND WITHOUT WARRANTY OF ANY KIND,   \n"
   "  EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY   \n"
   "  WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  \n" 
   "  \n"
   "* SimpEnvelopes: A set of functions for simplifying triangular meshes by\n"
   "                 Jonathan Cohen (cohenj@cs.unc.edu) and Amitabh Varshney\n"
   "                 (varshney@cs.sunysb.edu).\n"
   "  Copyright 1995 The University of North Carolina at Chapel Hill.\n"
   "  All Rights Reserved.\n"
   "\n"
   "  Permission to use, copy, modify and distribute this software and its\n"
   "  documentation for educational, research and non-profit purposes,\n"
   "  without fee, and without a written agreement is hereby granted,\n"
   "  provided that the above copyright notice and the following three\n"
   "  paragraphs appear in all copies.\n"
   "\n"
   "  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL BE\n"
   "  LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR\n"
   "  CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING OUT OF THE\n"
   "  USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY\n"
   "  OF NORTH CAROLINA HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH\n"
   "  DAMAGES.\n"
   "\n"
   "\n"
   "  Permission to use, copy, modify and distribute this software and its\n"
   "  documentation for educational, research and non-profit purposes,\n"
   "  without fee, and without a written agreement is hereby granted,\n"
   "  provided that the above copyright notice and the following three\n"
   "  paragraphs appear in all copies.\n"
   "\n"
   "  THE UNIVERSITY OF NORTH CAROLINA SPECIFICALLY DISCLAIM ANY\n"
   "  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF\n"
   "  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE\n"
   "  PROVIDED HEREUNDER IS ON AN 'AS IS' BASIS, AND THE UNIVERSITY OF\n"
   "  NORTH CAROLINA HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT,\n"
   "  UPDATES, ENHANCEMENTS, OR MODIFICATIONS.\n"
   "\n"
   "  The authors may be contacted via:\n"
   "\n"
   "  US Mail:  Jonathan Cohen                      Amitabh Varshney\n"
   "            Department of Computer Science      Department of Computer Science \n"
   "            Sitterson Hall, CB #3175            State University of New York\n"
   "            University of N. Carolina           Stony Brook, NY 11794-4400, USA \n"
   "            Chapel Hill, NC 27599-3175\n"
   "	    \n"
   "  Phone:    (919)962-1749                       Phone: (516)632-8446 \n"
   "	    \n"
   "  EMail:    cohenj@cs.unc.edu                   varshney@cs.sunysb.edu\n"
   "\n"
   "Sample code sources:\n"
   "--------------------\n"
   "* 'Mastering Algorithms with C': a book by Kyle Loudon,  \n"
   "                                 published by O'Reilly & Associates. \n"
   "   This code is under copyright and cannot be included in any other book, publication,\n"
   "   or  educational product  without  permission  from  O'Reilly & Associates.  No\n"
   "   warranty is attached; we cannot take responsibility for errors or  fitness for\n"
   "   use.\n"
   "\n"
   "* 'C Language Algorithms for Digital Signal Processing': a book by \n"
   "                Bruce Kimball, Paul Embree and Bruce Kimble\n"
   "                published by Prentice Hall, 1991.\n"
   "\n"
   "* 'Motif Programming Manual': a book by Dan Heller and Paula Ferguson\n"  
   "                              published by O'Reilly & Associates, Inc. 1994\n"  
   "   * Written by Dan Heller and Paula Ferguson.\n"  
   "   * Copyright 1994, O'Reilly & Associates, Inc.\n" 
   "   * Permission to use, copy, and modify this program without\n" 
   "   * restriction is hereby granted, as long as this copyright\n" 
   "   * notice appears in each copy of the program source code.\n" 
   "   * This program is freely distributable without licensing fees and\n" 
   "   * is provided without guarantee or warrantee expressed or implied.\n" 
   "   * This program is -not- in the public domain.\n"
   "\n"
   "* 'http://astronomy.swin.edu.au/~pbourke': Paul Bourke's personal pages\n"
   "                                           on geometry and rendering .\n"
   "   Copyright notice on webpage: Copyright Paul Bourke or a third party \n"
   "   contributer where indicated. You may view this site and its contents \n"
   "   using your web browser. You may print or save an electronic copy of \n"
   "   parts of this web site for your own personal use. \n"
   "   Permission must be sought for any other use. \n"
   "\n" ); 

   SUMA_SS2S(SS,s);
   
   SUMA_RETURN (s);
}

char * SUMA_help_Cmap_message_Info(SUMA_COLOR_MAP * ColMap)
{
   static char FuncName[]={"SUMA_help_Cmap_message_Info"};
   char stmp[1000], *s = NULL;
   SUMA_STRING *SS = NULL;
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend (NULL, NULL);

   s = SUMA_New_Additions (0, 1);
   SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;
   
   SS = SUMA_StringAppend (SS, 
      "\nColormap Keyboard Controls:\n"
      "     f: flip color map\n"
      "        See also Up/Down keys.\n"
      "     r: record image of colormap.\n"
      "\n"
      "     Ctrl+h: this help message\n"
      "\n"
      "     Z: Zoom in.\n"
      "        Maximum zoom in shows 2 colors in the map\n"
      "     z: Zoom out.\n"
      "        Minimum zoom in shows all colors in the map\n"
      "\n"
      "     Up/Down arrows: rotate colormap up/down by fraction of\n"
      "                     number of colors in color map. Fraction\n"
      "                     a number between 0 and 0.5 and set via\n"
      "                     the environment variable\n"
      "                     SUMA_ColorMapRotationFraction.\n"
      "                     See suma -environment for complete list\n"
      "                     of variables.\n"
      "     Ctrl+Up/Down arrows: rotate colormap up/down by one color\n"
      "     Shift+Up/Down arrows: move colormap up/down\n"
      "\n"
      "     Home: Reset zoom, translation and rotation parameters\n"
      "\n");
      
   SS = SUMA_StringAppend (SS, 
      "\nCmap properties\n");
   s = SUMA_ColorMapVec_Info(&ColMap, 1, 1);
   SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;

   /* Add help for all controller options */
   s = SUMA_Help_AllSurfCont();
   SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;
   
   /* clean SS */
   SS = SUMA_StringAppend (SS, NULL);
   /* copy s pointer and free SS */
   s = SS->s;
   SUMA_free(SS); 
   
   SUMA_RETURN (s);

}
static char PlotCommonHelp[]={
      "        Open a graphing window for the dataset\n"
      "        currently selected. The graphing window\n"
      "        updates with each new node selection.\n"
      "        A graphing window can be opened for each\n"
      "        dataset, and all graphs will update unless\n"
      "        '1 Only' is set in Surface Controller.\n"
      "        Use 'ctrl+h' in graph window for more help.\n" };

char * SUMA_help_Plot_message_Info(void)
{
   static char FuncName[]={"SUMA_help_Plot_message_Info"};
   char stmp[1000], *s = NULL;
   SUMA_STRING *SS = NULL;
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend (NULL, NULL);

   
   SS = SUMA_StringAppend_va (SS, 
      "What's in it for me?\n"
      "%s"
      "\nButtons:\n"
      "  Save:   Write graph image to file\n"
      "  Freeze: Detach graph from SUMA.\n"
      "        Further clicks will not update\n"
      "        graph.\n"
      "  Done: Close graph forever.\n"
      "\nKeyboard Controls\n"
      "     Ctrl+h: this help message\n"
      "\n"
      "     q/Q: Quit\n"
      "     w: Write time series to 1D file.\n"
      "\n"
      "\n", PlotCommonHelp);
     
   /* clean SS */
   SS = SUMA_StringAppend (SS, NULL);
   /* copy s pointer and free SS */
   s = SS->s;
   SUMA_free(SS); 
   
   SUMA_RETURN (s);

}

char * SUMA_help_message_Info(void)
{
   static char FuncName[]={"SUMA_help_message_Info"};
   char stmp[1000], *s = NULL;
   SUMA_STRING *SS = NULL;
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend (NULL, NULL);

   s = SUMA_New_Additions (0, 1);
   SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;
   
   SS = SUMA_StringAppend (SS, 
      "\nKeyboard Controls (substitute Alt with Apple key on MACs)\n");
   SS = SUMA_StringAppend (SS, 
      "     a: attenuation by background, toggle.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     B: Backface/Frontface/Noface culling, toggle.\n");
   SS = SUMA_StringAppend (SS, 
      "     b: background color, toggle.\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend_va (SS, 
      "     Ctrl+C: Set screen-coordinate-based clipping planes\n"
      "      Alt+C: Set object-coordinate-based clipping planes\n"
      "           o Planes are defined by a string of the format:\n"
      "             NAME: a, b, c, d\n"
      "             Where NAME is a user-given short name,\n"
      "             a, b, c, and d define the plane's equation\n"
      "             aX + bY + cZ + d = 0\n"
      "                Example: A: 0, 0, 1, 0\n"
      "           o To delete a plane, just enter its name followed\n"
      "             by the ':' (e.g. 'A:')\n"
      "           o If you enter only two parameters after the name, \n"
      "             they are assumed to be the c and d parameters,\n"
      "             a and b are set to 0.\n"
      "           o You are allowed a maximum of %d planes\n"
      "\n",
      SUMA_MAX_N_CLIP_PLANES);
   SS = SUMA_StringAppend (SS, 
      "     c: load a node color file.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+d: draw ROI controller.\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     d: Show all DO objects in DOv.\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     Ctrl+e: Look for OpenGL errors.\n\n"); 
   SS = SUMA_StringAppend (SS, 
      "     F: Flip light position between +z and -z.\n");
   SS = SUMA_StringAppend (SS, 
      "     f: functional overlay, toggle.\n\n");
   SS = SUMA_StringAppend_va (SS, 
      "     g: graph data.\n"
      "%s\n", PlotCommonHelp); 
   SS = SUMA_StringAppend (SS, 
      "     H: Highlight nodes inside a specified box.\n"
      "        Does not update other viewers\n"
      "        Paints into existing colors\n"
      "        Highlight is wiped out with new\n"
      "        colors.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     h: NO LONGER USED.\n"
      "        Please use Ctrl+h instead.\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+h: help message\n\n");
   SS = SUMA_StringAppend (SS, 
      "     J: Set the selected FaceSet on Surface Object\n"
      "        in Focus. Does not update in other viewers\n"
      "        or in AFNI.\n");
   SS = SUMA_StringAppend (SS, 
      "     j: Set the cross hair to a certain node on \n"
      "        SO in Focus.\n"
      "        Does update in other viewers\n"
      "        if linked by index"
      "        and AFNI if connected\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+j: Set the cross hair's XYZ location. \n"
      "        Does update in other viewers\n"
      "        if linked by XYZ"
      "        and AFNI if connected\n");
   SS = SUMA_StringAppend (SS, 
      "     Alt+j: Set the Focus node. \n"
      "        Cross hair's XYZ remain unchanged.\n"
      "        Does not update in other viewers\n"
      "        or in AFNI\n\n");
   SS = SUMA_StringAppend_va (SS, 
      "     L: Light's XYZ coordinates.\n"
      "        Default setting is 0.0 0.0 %.1f \n", 1.0 * SUMA_INTITIAL_LIGHT0_SWITCH);
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     Ctrl+L: Dim all lights and colors by a factor of 0.8\n" );
   SS = SUMA_StringAppend (SS, 
      "     l: look at point\n");
   SS = SUMA_StringAppend (SS, 
      "     Alt+l: look at cross hair\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+l: Switch locking mode for all viewers \n"
      "             between: No Lock, Index Lock and \n"
      "             XYZ Lock. The switching is order is \n"
      "             based on the lock of the first viewer.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     Alt+Ctrl+M: Dumps memory trace to file \n"
      "                 called malldump.NNN where NNN\n"
      "                 is the smallest number between\n"
      "                 001 and 999 that has not been used.\n");
   SS = SUMA_StringAppend (SS, 
      "     m: momentum, toggle\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     n: bring a node to direct view (does not work AT ALL)\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+n: Open a new surface viewer window.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     p: Viewer rendering mode  \n"
      "        (Fill, Line, Points, Hide), switch.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     P: Reset viewer and all surfaces to Fill  \n"
      "        rendering mode.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     r: record current image\n"
      "        in an a la AFNI image viewer.\n"
      "        Identical images are rejected.\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     Alt+r: Set new center of rotation.\n"
      "            Enter nothing to go back to default.\n"
      "\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+r: Increase the image oversampling factor.\n"
      "             By increasing this factor, you can create\n"
      "             images at a resolution higher than that \n"
      "             of the SUMA window. This is done by subdividing \n"
      "             the scene into NxN sections and rendering each\n"
      "             section separately. The NxN renderings are\n"
      "             saved in the image recorder. After you \n"
      "             save the images to disk, you can stitch them\n"
      "             using imcat (a la AFNI montage). \n"
      "        Note that each section is still rendered at\n"
      "             the resolution of the SUMA window. So the bigger\n"
      "             the window the more resolution per section.\n"
      "             However, you cannot exceed a certain limit\n"
      "             on the number of pixels in the final image.\n"
      "             This limitation is due to the graphics card\n"
      "             on your system. SUMA will take care not to exceed\n"
      "             this limit.\n");
   SS = SUMA_StringAppend (SS, 
      "     R: Toggle continuous recording \n"
      "        to an a la AFNI image viewer.\n"
      "        Identical images are rejected.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     s: NO LONGER IN USE. \n"
      "        View the surface's structure contents.\n"
      "        Use:View->Surface Controller->More.\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+s: Open controller for \n"
      "             surface in Focus.\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+Alt+s: Input filename containing displayable objects.\n"
      "                 Files are of 1D format with a necessary comment\n"
      "                 at the top to indicate the type of objects in \n"
      "                 the file.\n"
      "                 Note 1: Repeatedly loading files with the same \n"
      "                 name will replace currently loaded versions.\n"
      "                 Note 2: Node-based (Types 3 and 4) objects\n"
      "                 will follow a node when its coordinates change.\n"
      "          Type 1:Segments between (x0,y0,z0) and (x1,y1,z1) \n"
      "                 1st line must be '#segments' (without quotes),\n"
      "                 or '#oriented_segments' (slower to render).\n"
      "                 Remainder of file is N rows, each defining a \n"
      "                 segment (or a vector) between two points.\n"
      "                 Column content depends on the number of columns\n"
      "                 in the file:\n"
      "                 6  cols: x0 y0 z0 x1 y1 z1\n"
      "                 7  cols: x0 y0 z0 x1 y1 z1 th\n"
      "                          with th being line thickness\n"
      "                 10 cols: x0 y0 z0 x1 y1 z1 c0 c1 c2 c3\n"
      "                          with c0..3 being the RGBA values\n"
      "                          between 0 and 1.0\n"
      "                 11 cols: x0 y0 z0 x1 y1 z1 c0 c1 c2 c3 th\n"
      "          Type 2:Spheres centered at (ox, oy, oz) \n"
      "                 1st line must be '#spheres' (without quotes).\n"
      "                 Remainder of file is N rows, each defining a \n"
      "                 sphere.\n"
      "                 Column content depends on the number of columns\n"
      "                 in the file:\n"
      "                 3  cols: ox oy oz\n"
      "                 4  cols: ox oy oz rd\n"
      "                          with rd being the radius of the sphere\n"
      "                 5  cols: ox oy oz rd st\n"
      "                          with st being the style of the sphere's\n"
      "                          rendering. Choose from:\n"
      "                             0: points\n"
      "                             1: Lines\n"
      "                             2: Filled\n"
      "                 7  cols: ox oy oz c0 c1 c2 c3 \n"
      "                          with c0..3 being the RGBA values\n"
      "                          between 0 and 1.0\n"
      "                 8  cols: ox oy oz c0 c1 c2 c3 rd\n"
      "                 9  cols: ox oy oz c0 c1 c2 c3 rd st\n"
      "          Type 3:Vectors (vx, vy, vz) at surface nodes \n"
      "                 1st line must be '#node-based_vectors' (without quotes)\n"
      "                 or '#node-based_ball-vectors' (slower to render).\n"
      "                 Remainder of file is N rows, each defining a \n"
      "                 a vector at a particular node of the current surface.\n"
      "                 Column content depends on the number of columns\n"
      "                 in the file:\n"
      "                 3  cols: vx, vy, vz \n"
      "                          node index 'n' is implicit equal to row index.\n"
      "                          Vector 'v' is from coordinates of node 'n' to \n"
      "                          coordinates of node 'n' + 'v'\n"
      "                 4  cols: n, vx, vy, vz \n"
      "                          Here the node index 'n' is explicit. You can\n"
      "                          have multiple vectors per node, one on \n"
      "                          each row.\n"
      "                 5  cols: n, vx, vy, vz, gn\n"
      "                          with gn being a vector gain factor\n"
      "                 8  cols: n, vx, vy, vz, c0 c1 c2 c3\n"
      "                          with with c0..3 being the RGBA values\n"
      "                          between 0 and 1.0\n"
      "                 9  cols: n, vx, vy, vz, c0 c1 c2 c3 gn\n"   
      "          Type 4:Spheres centered at nodes n of the current surface\n"
      "                 1st line must be '#node-based_spheres' (without quotes).\n"
      "                 Remainder of file is N rows, each defining a \n"
      "                 sphere.\n"
      "                 Column content depends on the number of columns\n"
      "                 in the file, see Type 2 for more details:\n"
      "                 1  cols: n\n"
      "                 2  cols: n rd\n"
      "                 3  cols: n rd st\n"
      "                 5  cols: n c0 c1 c2 c3 \n"
      "                 6  cols: n c0 c1 c2 c3 rd\n"
      "                 7  cols: n c0 c1 c2 c3 rd st\n"
      "          Type 5:Planes defined with: ax + by + cz + d = 0.\n"
      "                 1st line must be '#planes' (without quotes).\n"
      "                 Remainder of file is N rows, each defining a \n"
      "                 plane.\n"
      "                 Column content depends on the number of columns\n"
      "                 in the file:\n"
      "                 7  cols: a b c d cx cy cz\n"
      "                          with the plane's equation being:\n"
      "                          ax + by + cz + d = 0\n"
      "                          cx,cy,cz is the center of the plane's\n"
      "                          representation. \n"
      "                          Yes, d is not of much use here.\n"
      "                 There are no node-based planes at the moment.\n"
      "                 They are a little inefficient to reproduce with\n"
      "                 each redraw. Complain if you need them.\n"
      );
   SS = SUMA_StringAppend (SS, 
      "     Alt+s: Switch mouse buttons 1 and 3.\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     S: Show all surface objects registered in DOv.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     t: talk to AFNI, toggle.\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+t: Force a resend of \n"
      "            surfaces to AFNI.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     T: Start listening for niml connections\n\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+u: Open SUMA controller.\n\n");   
   SS = SUMA_StringAppend (SS, 
      "     v: NO LONGER IN USE. \n"
      "        View the viewer's structure contents.\n"
      "        Use: View->Viewer Controller->More.\n"
      "\n");
   SS = SUMA_StringAppend (SS, 
      "     w: This option has been dropped.\n"
      "        Instead, use 'r' or 'R' recording options\n"
      "        or use a screen grab instead. \n"
      "        (like xv on unix systems, and grab on Macs.)\n");
   SS = SUMA_StringAppend (SS, 
      "     W: Write ascii files containing the NodeList,\n"
      "        the FaceSetList and the nodecolors of the \n"
      "        surface in focus.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     Z/z: Zoom in/out\n\n");
   SS = SUMA_StringAppend (SS, 
      "     [: Show/Hide left hemisphere.\n"
      "     ]: Show/Hide right hemisphere.\n"
      "        Window title shows which \n"
      "        hemispheres are shown :LR:\n"
      "        :-R: :L-: or :--:\n\n");
   SS = SUMA_StringAppend (SS, 
      "  8: Set the number of smoothing iterations\n"
      "     to be applied to the foreground colors.\n"
      "     This setting will be applied to all subsequent\n"
      "     color sets.\n");
   SS = SUMA_StringAppend (SS, 
      "  *: Smooth node colors by averaging with neighbors.\n"
      "     The smoothing is only applied to the current colors,\n"
      "     and will be not be applied to new color sets.\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     @: Compute curvatures along principal directions \n"
      "        on the surface, results written to disk.\n\n");
   if (SUMAg_CF->Dev) SS = SUMA_StringAppend (SS, 
      "     (: Compute convexity of surface, \n"
      "        results written to disk.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     ,/. (think </>): Switch to next/previous view state.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     SPACE: Toggle between Mapping Reference and\n"
      "            Current view state.\n\n");

   SS = SUMA_StringAppend (SS, 
      "     L-R arrows: rotate about screen's Y axis\n");
   SS = SUMA_StringAppend (SS, 
      "     U-D arrows: rotate about screen's X axis\n");
   SS = SUMA_StringAppend (SS, 
      "     Shift+L-R arrows: translate about screen's \n"
      "                       Y axis\n");
   SS = SUMA_StringAppend (SS, 
      "     Shift+U-D arrows: translate about screen's \n"
      "                       X axis\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+L-R arrows: LR cardinal views\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+U-D arrows: IS cardinal views\n");
   SS = SUMA_StringAppend (SS, 
      "     Ctrl+Shift+U-D arrows: AP cardinal views\n\n");

   SS = SUMA_StringAppend (SS, 
      "     F1: screen axis (X-Red, Y-Green), toggle. \n");
   SS = SUMA_StringAppend (SS, 
      "     F2: surface axis (X-Red, Y-Green, Z-Blue), \n"
      "         switch. \n");
   SS = SUMA_StringAppend (SS, 
      "     F3: cross hair, toggle. \n");
   SS = SUMA_StringAppend (SS, 
      "     F4: node selection highlight, toggle. \n");
   SS = SUMA_StringAppend (SS, 
      "     F5: FaceSet selection highlight, toggle.\n");
   SS = SUMA_StringAppend (SS, 
      "     F6: Viewer background color, toggle.\n");
   SS = SUMA_StringAppend (SS, 
      "     F7: Switch between color mixing modes.\n"
      "         ORIG: Col = ( 1 - opacity ) * OldCol + opacity * NewCol \n"
      "         MOD1: Col = ( 1 - opacity ) * OldCol +           NewCol \n");
   SS = SUMA_StringAppend (SS, 
      "     F8: Viewing mode (Perspective or Orthographic Projection), toggle.\n"
      );
   SS = SUMA_StringAppend (SS, 
      "     F12: Time 20 scene renderings.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     HOME: reset view to startup\n\n");
   SS = SUMA_StringAppend (SS, 
      "     ESCAPE: close the surface viewer window.\n");
   SS = SUMA_StringAppend (SS, 
      "     Shft+ESCAPE: close all surface viewer windows.\n\n");
   SS = SUMA_StringAppend (SS, 
      "     Mouse Controls:\n");
   SS = SUMA_StringAppend (SS, 
      "     Button 1-Motion: rotation as if you were using\n"
      "                      a trackball.\n");
   SS = SUMA_StringAppend (SS, 
      "       Pure vertical motion is equivalent to using \n"
      "       the up/down arrow keys.\n");
   SS = SUMA_StringAppend (SS, 
      "       Pure horizontal motion is equivalent to using \n"
      "       the left/right arrow keys.\n");
   SS = SUMA_StringAppend (SS, 
      "       Of course, the advantage to using the mouse is \n"
      "       a continuous range of rotation angles and \n");
   SS = SUMA_StringAppend (SS, 
      "       simultaneous rotations about the screen's \n"
      "       X & Y axis.\n");
   SS = SUMA_StringAppend (SS, 
      "       This mode of rotation is similar to SGI's \n"
      "       ivview interface.\n");
   SS = SUMA_StringAppend (SS, 
      "     Button 2-Motion: translation\n"); 
   SS = SUMA_StringAppend (SS, 
      "     Button 1+2-Motion OR \n"
      "      Shift+Button2-Motion: \n"
      "          Zoom in/out\n");
   SS = SUMA_StringAppend (SS, 
      "     Button 3-Press: picking \n");
   SS = SUMA_StringAppend (SS, 
      "     shft+Button 3-Press: ROI drawing \n"
      "                          (when in DrawROI mode)\n");
   SS = SUMA_StringAppend (SS, 
      "    \n");
   SS = SUMA_StringAppend (SS, 
      "    File Menu:\n"
      "    ->Save View: Save viewer's display settings.\n"
      "    ->Load View: Load and apply display settings.\n"
      "    ->Close: Close this viewer.\n"
      "             Exit SUMA if this is the only viewer.\n");
   SS = SUMA_StringAppend (SS, 
      "    View Menu:\n"
      "    ->SUMA Controller: Open SUMA controller interface.\n"
      "    ->Surface Controller: Open selected surface's \n"
      "                          controller interface.\n"
      "    ->Viewer Controller: Open viewer's controller interface.\n"
      "    --------\n"
      "    ->Cross Hair: Toggle cross hair display.\n"
      "    ->Node in Focus: Toggle highlight of selected node.\n"
      "    ->Selected Faceset: Toggle highlight of selected faceset.\n");
   SS = SUMA_StringAppend (SS, 
      "    Tools Menu:\n"
      "    ->Draw ROI: Open Draw ROI controller.\n");
   SS = SUMA_StringAppend (SS, 
      "    Help Menu:\n"
      "    ->Usage: Opens window with this message.\n"
      "    ->Message Log: Opens window that will \n"
      "                   contain errors and warnings\n"
      "                   typically output to screen.\n"
      "    -------\n"
      "    ->SUMA Global: Output debugging information\n"
      "                   about some of SUMA's global \n"
      "                   structure's variables.\n"
      "    ->Viewer Struct: Output debugging info on \n"
      "                     a viewer's structure.\n"
      "    ->Surface Struct: Output debugging info on\n"
      "                      the selected surface's struct.\n"
      "    -------\n"
      "    ->InOut Notify: Turn on/off function in/out tracing.\n"
      "    ->MemTrace: Turn on memory tracing.\n"
      "                Once turned on, this can't be turned off.\n"
      "\n");
   SS = SUMA_StringAppend_va( SS,
                              "SUMA's list of environment variables:\n");
   s = SUMA_env_list_help();
   SS = SUMA_StringAppend( SS, s); SUMA_free(s); s = NULL;
   SS = SUMA_StringAppend( SS, "\n");
   
   SS = SUMA_StringAppend (SS, 
      "    More help at \n"
      "    http://afni.nimh.nih.gov/pub/dist/doc/SUMA/SUMA_doc.htm\n");
   SS = SUMA_StringAppend (SS, 
      "\n");
   
   /* add latest additions */
   SS = SUMA_StringAppend (SS, "Current Version Info:\n");
   s = SUMA_New_Additions (0, 0);
   SS = SUMA_StringAppend (SS, s); SUMA_free(s); s = NULL;

   /* clean SS */
   SS = SUMA_StringAppend (SS, NULL);
   /* copy s pointer and free SS */
   s = SS->s;
   SUMA_free(SS); 
   
   SUMA_RETURN (s);

}

/*!
Controls help message
*/
void SUMA_help_message(FILE *Out)
{
	char *s=NULL;
   static char FuncName[]={"SUMA_help_message"};
   
   SUMA_ENTRY;

   if (Out == NULL) {
		Out = stdout;
	}
   
   s = SUMA_help_message_Info();
   if (!s) {
      fprintf (SUMA_STDERR, "Error %s: Failed in SUMA_help_message_Info.\n", FuncName);
   }else {
      fprintf (Out, "%s\n", s);
      SUMA_free(s);
   }
	
   SUMA_RETURNe;
}

char *SUMA_All_Programs(void )
{
   char *s=NULL;
   static char FuncName[]={"SUMA_All_Programs"};
   SUMA_STRING  *SS = NULL; 
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend (NULL, NULL);
   
   SS = SUMA_StringAppend ( SS,
         "+ List of programs in the SUMA package:\n"
         "  3dCRUISEtoAFNI\n"
         "  3dBRAIN_VOYAGERtoAFNI\n"
         "  3dSkullStrip\n"
         "  3dSurf2Vol\n"
         "  3dSurfMask\n"
         "  3dVol2Surf\n"
         "  AnalyzeTrace\n"
         "  CompareSurfaces\n"
         "  ConvertSurface\n"
         "  ConvexHull\n"
         "  CreateIcosahedron\n"
         "  DriveSuma\n"
         "  FSread_annot\n"
         "  inspec\n"
         "  IsoSurface\n"
         "  MakeColorMap\n"
         "  MapIcosahedron\n"
         "  quickspec\n"
         "  ROI2dataset\n"
         "  ROIgrow\n"
         "  ScaleToMap\n"
         "  SUMA_glxdino\n"
         "  SurfaceMetrics\n"
         "  SurfClust\n"
         "  SurfDsetInfo\n"
         "  SurfInfo\n"
         "  SurfMeasures\n"
         "  SurfMesh\n"
         "  SurfPatch\n"
         "  SurfQual\n"
         "  SurfSmooth\n"
         "  SurfToSurf\n"
     );
   
   /* clean SS */
   SS = SUMA_StringAppend (SS, NULL);
   /* copy s pointer and free SS */
   s = SS->s;
   SUMA_free(SS); 
   
   SUMA_RETURN(s);      
   
}

/*!
SUMA version 
*/

void SUMA_Version (FILE *Out)
{
   static char FuncName[]={"SUMA_Version"};
   char *s = NULL;
   
   if (Out == NULL) {
		Out = stdout;
	}
   s = SUMA_New_Additions (0, 0);
	if (s) {
      fprintf (Out, "\n   %s\n", s);
      SUMA_free(s);
   } else {
      fprintf (Out, "\n");
   }
   
	return;
}

/*!
Surface .. Volume relationships
*/
void SUMA_VolSurf_help (FILE *Out)
{
	if (Out == NULL) {
		Out = stdout;
	}
    fprintf (Out, "SUMA_VolSurf_help: This function is obsolete.\n");
	 return;
	 fprintf (Out, "\nVolume <--> Surface jeremiad:\n");
	 fprintf (Out, "-----------------------------\n");
	 fprintf (Out, "\tTo transform surface node coordinates to voxel coordinates a few parameters are required.\n");
	 fprintf (Out, "\tThose paramters vary depending on the type of surfaces used. Currently, SUMA supports \n");
	 fprintf (Out, "\tFreeSurfer and SureFit surfaces.\n");
	 fprintf (Out, "\nParent Volume (VolPar):\n");
	 fprintf (Out, "\tThe surface model is created from a high-resolution anatomical scan\n"); 
	 fprintf (Out, "\treferred to as Parent Volume (VolPar).\n"); 
	 fprintf (Out, "\tTo align the surface with data from a particular experiment, VolPar must\n"); 
	 fprintf (Out, "\tbe brought to alignment with the experiemt's data.\n"); 
	 fprintf (Out, "\tFor example, VolPar is aligned with data from experiment Day1 using:\n"); 
	 fprintf (Out, "\t3dvolreg -clipit -twopass -twodup -zpad 8 -rotcom -verbose \n"); 
	 fprintf (Out, "\t-base SPGR_Day1 -prefix VolPar_Day1 VolMast >>& VolParAlignLog\n");
	 fprintf (Out, "\twhere SPGR_Day1 is the high-resolution anatomical scan obtained in\n"); 
	 fprintf (Out, "\texperiment Day1 and VolPar_Day1 is VolPar aligned to SPGR_Day1.\n"); 
	 fprintf (Out, "\nSurface segmentation programs typically require the resolution of VolPar to\n"); 
	 fprintf (Out, "\tbe 1x1x1mm. Such volumes, especially for FreeSurfer are quite large and\n"); 
	 fprintf (Out, "\t3dvolreg might run out of memory. If that happens, you could resample \n"); 
	 fprintf (Out, "\tVolPar to a lower resolution such as 1.2x1.2x1.2mm, prior to registration. \n"); 
	 fprintf (Out, "\tNote that SPGR_Day1 must have the same resolution and number of slices as VolPar.\n"); 
	 fprintf (Out, "\n\t+FreeSurfer Parent Volume:\n"); 
	 fprintf (Out, "\tConstruct VolPar from the .COR images used to create the surface using:\n"); 
	 fprintf (Out, "\tto3d -prefix CW-cSurfParent-SPGR -xSLAB 127.5L-R -ySLAB 127.5S-I -zSLAB 127.5P-A COR-???\n");
	 fprintf (Out, "\tExample command line for a FreeSurfer suface with VolPar aligned to experiment ARzs:\n"); 
	 fprintf (Out, "\t./suma -vp CW-cSurfParent-SPGR_Reg2_ARzsspgrax_1mm_256pad_cor_RSP_down12+orig\\\n");
	 fprintf (Out, "\t -spec CW-FreeSurfer.SumaSpec\n");
	 fprintf (Out, "\n\t+SureFit Parent Volume:\n"); 
	 fprintf (Out, "\tVolPar is the anatomical 1x1x1 mm volume in the correct orientation (LPI) \n"); 
	 fprintf (Out, "\tthat is used by SureFit to create the surface. Typically, this volume has \n"); 
	 fprintf (Out, "\tthe .Orient string in its name unless it was in LPI orientation from the \n"); 
	 fprintf (Out, "\tstart. Because SureFit crops the volume before segmentation, it is also \n"); 
	 fprintf (Out, "\tnecessary to supply the .params file along with VolPar. The .params file is \n"); 
	 fprintf (Out, "\ttypically named something like: (anything here).L.full.sMRI.params for the \n"); 
	 fprintf (Out, "\tleft full hemisphere.  Example command line for a SureFit surface with VolPar:\n"); 
	 fprintf (Out, "\t./suma -vp colin_short_Orient+orig. colin_short+orig.L.full.sMRI.params\\\n");
	 fprintf (Out, "\t -s_s colin.fiducial.coord colin.topo");
	 fprintf (Out, "\nor:\n");
	 fprintf (Out, "\t./suma -vp CW-cSurfParent-SPGR-AX_LPI+orig. -spec CW-SureFit.SumaSpec\n");
	 fprintf (Out, "\t\n"); 
	 return;
}

char * SUMA_Help_AllSurfCont ()
{
   static char FuncName[]={"SUMA_Help_AllSurfCont"};
   char *s = NULL;
   SUMA_STRING *SS = NULL;
   
   SUMA_ENTRY;
   
   SS = SUMA_StringAppend (NULL, NULL);
   
   SS = SUMA_StringAppend(SS, 
         "\n"
         "\n"
         "----------------------------\n"
         "Help for Surface Controller:\n"
         "----------------------------\n"
         "The surface controller is for \n"
         "controlling properties pertinent\n"
         "to the surface selected (in focus).\n"
         "The Surface Controller is launched\n"
         "with 'ctrl+s' or \n"
         "      View-->Surface Controller .\n"
         "\n"
         );
   SS = SUMA_StringAppend_va(SS, 
         "+ Surface Properties Block:\n"
         "\n"
         "++ more:\n%s\n"
         "\n"
         "++ RenderMode:\n%s\n"
         "\n"
         "++ Dsets:\n%s\n"
         "\n", 
         SUMA_SurfContHelp_more, SUMA_SurfContHelp_RenderMode, SUMA_SurfContHelp_Dsets);
   SS = SUMA_StringAppend_va(SS, 
         "+ Xhair Info Block:\n"
         "\n"
         "++ Xhr:\n%s\n"
         "\n"
         "++ Node:\n%s\n"
         "\n"
         "++ Tri:\n%s\n"
         "\n"
         "++ Node Values Table: %s\n"
         "+++ Col. Intens\n%s\n"
         "+++ Col. Thresh\n%s\n"
         "+++ Col. Bright:\n%s\n"
         "+++ Row  Val:\n%s\n"
         "\n"
         "++ Node Label Table:\n"
         "+++ Row  Lbl:\n%s\n"
         "\n",
         SUMA_SurfContHelp_Xhr, SUMA_SurfContHelp_Node, SUMA_SurfContHelp_Tri, 
         SUMA_SurfContHelp_NodeValTblr0, SUMA_SurfContHelp_NodeValTblc1,  
         SUMA_SurfContHelp_NodeValTblc2, SUMA_SurfContHelp_NodeValTblc3, 
         SUMA_SurfContHelp_NodeValTblr0,
         SUMA_SurfContHelp_NodeLabelTblr0);
   
   SS = SUMA_StringAppend_va(SS, 
         "+ Dset Controls Block:\n"
         "\n"
         "++ Dset Info Table: \n"
         "+++ Row  Lbl:\n%s\n"
         "+++ Row  Par:\n%s\n"
         "\n"
         "++ Ord:\n%s\n"
         "\n"
         "++ Opa:\n%s\n"
         "\n"
         "++ Dim:\n%s\n"
         "\n"
         "++ view:\n%s\n"
         "\n"
         "++ Switch Dset:\n%s\n"
         "\n", 
         SUMA_SurfContHelp_DsetLblTblr0, SUMA_SurfContHelp_DsetLblTblr1, 
         SUMA_SurfContHelp_DsetOrd, SUMA_SurfContHelp_DsetOpa, SUMA_SurfContHelp_DsetDim,
         SUMA_SurfContHelp_DsetView, SUMA_SurfContHelp_DsetSwitch);
   
   SS = SUMA_StringAppend_va(SS,       
         "++ Load Dset:\n%s\n"
         "\n"
         "++ Load Col:\n%s\n"
         "\n", 
         SUMA_SurfContHelp_DsetLoad,
         SUMA_SurfContHelp_DsetLoadCol);
         
   SS = SUMA_StringAppend_va(SS, 
         "+ Dset Mapping Block:\n"
         "\n"
         "++ Mapping Data: \n"
         "\n"
         "+++ I\n%s\n"
         "++++ v:\n%s\n"
         "+++ T\n%s\n"
         "++++ v\n%s\n"
         "+++ B\n%s\n"
         "++++ v\n%s\n"
         "\n", 
         SUMA_SurfContHelp_SelInt, SUMA_SurfContHelp_SelIntTgl,
         SUMA_SurfContHelp_SelThr, SUMA_SurfContHelp_SelThrTgl, 
         SUMA_SurfContHelp_SelBrt, SUMA_SurfContHelp_SelBrtTgl );
         
   SS = SUMA_StringAppend_va(SS, 
         "++ Mapping Parameters Table:\n%s\n"
         "+++ Col. Min\n%s\n"      
         "+++ Col. Max\n%s\n"
         "+++ Row  I\n%s\n"
         "+++ Row  B1\n%s\n"
         "+++ Row  B2\n%s\n"
         "+++ Row  C\n%s\n"
         "\n", 
         SUMA_SurfContHelp_SetRngTbl_r0,
         SUMA_SurfContHelp_SetRngTbl_c1, SUMA_SurfContHelp_SetRngTbl_c2,
         SUMA_SurfContHelp_SetRngTbl_r1, SUMA_SurfContHelp_SetRngTbl_r2,
         SUMA_SurfContHelp_SetRngTbl_r3, SUMA_SurfContHelp_SetRngTbl_r4);
   
   SS = SUMA_StringAppend_va(SS, 
         "++ Col\n%s\n"
         "\n"
         "++ Bias\n%s\n"
         "\n"
         "Cmp\n%s\n"
         "\n"
         "New\n%s\n"
         "\n"
         "|T|\n%s\n"
         "\n"
         "sym I\n%s\n"
         "\n"      
         "shw 0\n%s\n"
         "\n",      
         SUMA_SurfContHelp_Col, SUMA_SurfContHelp_Bias, SUMA_SurfContHelp_Cmp,
         SUMA_SurfContHelp_CmpNew, SUMA_SurfContHelp_AbsThr, SUMA_SurfContHelp_Isym,
         SUMA_SurfContHelp_Shw0);

   SS = SUMA_StringAppend_va(SS, 
         "++ Data Range Table:\n%s\n"
         "\n"
         "+++ Col Min\n%s\n"
         "+++ Col Node\n%s\n"
         "+++ Col Max\n%s\n"
         "+++ Col Node\n%s\n"
         "+++ Row I\n%s\n"
         "+++ Row T\n%s\n"
         "+++ Row B\n%s\n", 
         SUMA_SurfContHelp_RangeTbl_c0,
         SUMA_SurfContHelp_RangeTbl_c1, SUMA_SurfContHelp_RangeTbl_c2,
         SUMA_SurfContHelp_RangeTbl_c3, SUMA_SurfContHelp_RangeTbl_c4,
         SUMA_SurfContHelp_RangeTbl_r1, SUMA_SurfContHelp_RangeTbl_r2, 
         SUMA_SurfContHelp_RangeTbl_r3);
               
            
   SUMA_SS2S(SS, s);
   
   SUMA_RETURN(s);
}
