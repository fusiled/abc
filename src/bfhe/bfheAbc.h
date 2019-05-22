/**CFile****************************************************************

  FileName    [ioAbc.h]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Command processing package.]

  Synopsis    [External declarations.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: ioAbc.h,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#ifndef ABC__bfhe__bfheAbc_h
#define ABC__bfhe__bfheAbc_h


////////////////////////////////////////////////////////////////////////
///                          INCLUDES                                ///
////////////////////////////////////////////////////////////////////////

#include "base/abc/abc.h"
#include "bfhe.h"
#include "tfhe/tfhe.h"

////////////////////////////////////////////////////////////////////////
///                         PARAMETERS                               ///
////////////////////////////////////////////////////////////////////////



ABC_NAMESPACE_HEADER_START


////////////////////////////////////////////////////////////////////////
///                         BASIC TYPES                              ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                      MACRO DEFINITIONS                           ///
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
///                    FUNCTION DECLARATIONS                         ///
////////////////////////////////////////////////////////////////////////

/*=== bfheCreateNet.c ===========================================================*/
extern void               Bfhe_CreateNet( Abc_Ntk_t * pNtk, BfheNet * net, int fWriteLatches, int fBb2Wb, int fSeq );
extern void               Bfhe_CreateNetLogic( Abc_Ntk_t * pNtk, BfheNet * net, int fWriteLatches );

extern void               Bfhe_EvalNet( BfheNet * net, char graphModeEnabled);
extern void               Bfhe_LoadInput( BfheNet * net, char * path);
extern void               Bfhe_StoreOutput( BfheNet * net, char * path);

ABC_NAMESPACE_HEADER_END



#endif

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////

