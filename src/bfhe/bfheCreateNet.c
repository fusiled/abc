/**CFile****************************************************************

  FileName    [ioWriteBlif.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Command processing package.]

  Synopsis    [Procedures to write BLIF files.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: ioWriteBlif.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "bfheAbc.h"
#include "bfhe.h"

#include "base/main/main.h"
#include "map/mio/mio.h"
#include "bool/kit/kit.h"
#include "map/if/if.h"

ABC_NAMESPACE_IMPL_START


////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

static void Bfhe_NtkWrite( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches, int fBb2Wb, int fSeq );
static void Bfhe_NtkWriteOne( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches, int fBb2Wb, int fSeq );
static void Bfhe_NtkWritePis( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches );
static void Bfhe_NtkWritePos( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches );
static void Bfhe_NtkWriteSubckt( BfheNet * net, Abc_Obj_t * pNode );
static void Bfhe_NtkWriteAsserts( BfheNet * net, Abc_Ntk_t * pNtk );
static void Bfhe_NtkWriteNodeFanins( BfheNet * net, Abc_Obj_t * pNode );
static int  Bfhe_NtkWriteNode( BfheNet * net, Abc_Obj_t * pNode, int Length );
static void Bfhe_NtkWriteLatch( BfheNet * net, Abc_Obj_t * pLatch );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Write the network into a BLIF file with the given name.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_CreateNetLogic( Abc_Ntk_t * pNtk, BfheNet * net, int fWriteLatches )
{
    Abc_Ntk_t * pNtkTemp;
    // derive the netlist
    pNtkTemp = Abc_NtkToNetlist(pNtk);
    if ( pNtkTemp == NULL )
    {
        fprintf( stdout, "Create BfheNet  has failed.\n" );
        return;
    }
    Bfhe_CreateNet( pNtkTemp, net, fWriteLatches, 0, 0 );
    Abc_NtkDelete( pNtkTemp );
}

/**Function*************************************************************

  Synopsis    [Write the network into a BLIF file with the given name.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_CreateNet( Abc_Ntk_t * pNtkOrig, BfheNet * net, int fWriteLatches, int fBb2Wb, int fSeq )
{
    Abc_Ntk_t * pNtk;
    pNtk = Abc_NtkToNetlist( pNtkOrig );
    if ( !Abc_NtkHasSop(pNtk) && !Abc_NtkHasMapping(pNtk) ){
            Abc_NtkToSop( pNtk, -1, ABC_INFINITY );
    }
    assert( Abc_NtkIsNetlist(pNtk) );
    
    // write the master network
    Bfhe_NtkWrite( net, pNtk, fWriteLatches, fBb2Wb, fSeq );
    // make sure there is no logic hierarchy
    assert( Abc_NtkWhiteboxNum(pNtk) == 0 );
    /*
    Abc_Ntk_t * pNtkTemp;
    // write the hierarchy if present
    if ( Abc_NtkBlackboxNum(pNtk) > 0 || Abc_NtkWhiteboxNum(pNtk) > 0 )
    {
        Vec_PtrForEachEntry( Abc_Ntk_t *, pNtk->pDesign->vModules, pNtkTemp, i )
        {
            if ( pNtkTemp == pNtk )
                continue;
            fprintf( net, "\n\n" );
            Bfhe_NtkWrite( net, pNtkTemp, fWriteLatches, fBb2Wb, fSeq );
        }
    }
    */
    Abc_NtkDelete( pNtk );
}

/**Function*************************************************************

  Synopsis    [Write the network into a BLIF file with the given name.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWrite( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches, int fBb2Wb, int fSeq )
{
    Abc_Ntk_t * pExdc;
    //assert( Abc_NtkIsNetlist(pNtk) );
    // write the model name
    BfheNet_setName(net, Abc_NtkName(pNtk) );
    // write the network
    Bfhe_NtkWriteOne( net, pNtk, fWriteLatches, fBb2Wb, fSeq );
    // write EXDC network if it exists
    pExdc = Abc_NtkExdc( pNtk );
    if ( pExdc )
    {
        printf(".exdc not supported!\n");
        assert(0);
        //Bfhe_NtkWriteOne( net, pExdc, fWriteLatches, fBb2Wb, fSeq );
    }
}

/**Function*************************************************************

  Synopsis    [Write one network.]

  Description []
               
  SideEffects []

  SeeAlso     [] 

***********************************************************************/
void Bfhe_NtkWriteConvertedBox( BfheNet * net, Abc_Ntk_t * pNtk, int fSeq )
{
    printf("Cannot make Bfhe_NtkWriteConvertedBox!\n");
    assert(0);
    /*
    Abc_Obj_t * pObj;
    int i, v;
    Abc_NtkForEachPo( pNtk, pObj, i )
    { 
        // write the .names line
        fprintf( net, ".names" );
        Bfhe_NtkWritePis( net, pNtk, 1 );
        if ( fSeq )
            fprintf( net, " %s_in\n", Abc_ObjName(Abc_ObjFanin0(pObj)) );
        else
            fprintf( net, " %s\n", Abc_ObjName(Abc_ObjFanin0(pObj)) );
        for ( v = 0; v < Abc_NtkPiNum(pNtk); v++ )
            fprintf( net, "1" );
        fprintf( net, " 1\n" );
        if ( fSeq )
            fprintf( net, ".latch %s_in %s 1\n", Abc_ObjName(Abc_ObjFanin0(pObj)), Abc_ObjName(Abc_ObjFanin0(pObj)) );
    }*/
}

/**Function*************************************************************

  Synopsis    [Write one network.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWriteOne( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches, int fBb2Wb, int fSeq )
{
    ProgressBar * pProgress;
    Abc_Obj_t * pNode;
    int i, Length;

    // write the PIs inputs
    Bfhe_NtkWritePis( net, pNtk, fWriteLatches );

    // write the POs outputs
    Bfhe_NtkWritePos( net, pNtk, fWriteLatches );

    // write the blackbox
    if ( Abc_NtkHasBlackbox( pNtk ) )
    {
        printf("BlackBoxes not supported yet\n");
        assert(0);
        /*
        if ( fBb2Wb )
            Bfhe_NtkWriteConvertedBox( net, pNtk, fSeq );
        else
            fprintf( net, ".blackbox\n" );
        return;
        */
    }


    // write the latches
    if ( fWriteLatches && !Abc_NtkIsComb(pNtk) )
    {
        printf("Latches not supported yet\n");
        assert(0);
        /*
        Abc_Obj_t * pLatch;
        Abc_NtkForEachLatch( pNtk, pLatch, i )
            Bfhe_NtkWriteLatch( net, pLatch );
        */
    }

    // write the subcircuits
    assert( Abc_NtkWhiteboxNum(pNtk) == 0 );
/*    if ( Abc_NtkBlackboxNum(pNtk) > 0 || Abc_NtkWhiteboxNum(pNtk) > 0 )
    {
        fprintf( net, "\n" );
        Abc_NtkForEachBlackbox( pNtk, pNode, i )
            Bfhe_NtkWriteSubckt( net, pNode );
        fprintf( net, "\n" );
        Abc_NtkForEachWhitebox( pNtk, pNode, i )
            Bfhe_NtkWriteSubckt( net, pNode );
        fprintf( net, "\n" );
    }
*/
    // write each internal node
    Length = Abc_NtkHasMapping(pNtk)? Mio_LibraryReadGateNameMax((Mio_Library_t *)pNtk->pManFunc) : 0;
    pProgress = Extra_ProgressBarStart( stdout, Abc_NtkObjNumMax(pNtk) );
    Abc_NtkForEachNode( pNtk, pNode, i )
    {
        Extra_ProgressBarUpdate( pProgress, i, NULL );
        if ( Bfhe_NtkWriteNode( net, pNode, Length ) ) // skip the next node
            i++;
    }
    Extra_ProgressBarStop( pProgress );
}


/**Function*************************************************************

  Synopsis    [Writes the primary input list.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWritePis( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches )
{
    printf("Writing inputs!\n");
    Abc_Obj_t * pTerm, * pNet;
    int i;

        Abc_NtkForEachCi( pNtk, pTerm, i )
        {
            pNet = Abc_ObjFanout0(pTerm);
            printf("Creating node with name: %s\n",Abc_ObjName(pNet));
            BfheNet_addInput( net, Abc_ObjName(pNet) );
        }
}

/**Function*************************************************************

  Synopsis    [Writes the primary output list.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWritePos( BfheNet * net, Abc_Ntk_t * pNtk, int fWriteLatches )
{
    Abc_Obj_t * pTerm, * pNet;
    printf("Writing outputs!\n");
    int i;

        Abc_NtkForEachCo( pNtk, pTerm, i )
        {
            pNet = Abc_ObjFanin0(pTerm);
            BfheNet_addOutput( net, Abc_ObjName(pNet) );
        }
}

/**Function*************************************************************

  Synopsis    [Write the latch into a file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWriteSubckt( BfheNet * net, Abc_Obj_t * pNode )
{
    printf(".subckt not supported!\n");\
    assert(0);
   /* Abc_Ntk_t * pModel = (Abc_Ntk_t *)pNode->pData;
    Abc_Obj_t * pTerm;
    int i;
    // write the subcircuit
//    fprintf( net, ".subckt %s %s", Abc_NtkName(pModel), Abc_ObjName(pNode) );
    fprintf( net, ".subckt %s", Abc_NtkName(pModel) );
    // write pairs of the formal=actual names
    Abc_NtkForEachPi( pModel, pTerm, i )
    {
        fprintf( net, " %s", Abc_ObjName(Abc_ObjFanout0(pTerm)) );
        pTerm = Abc_ObjFanin( pNode, i );
        fprintf( net, "=%s", Abc_ObjName(Abc_ObjFanin0(pTerm)) );
    }
    Abc_NtkForEachPo( pModel, pTerm, i )
    {
        fprintf( net, " %s", Abc_ObjName(Abc_ObjFanin0(pTerm)) );
        pTerm = Abc_ObjFanout( pNode, i );
        fprintf( net, "=%s", Abc_ObjName(Abc_ObjFanout0(pTerm)) );
    }
    */
}

/**Function*************************************************************

  Synopsis    [Write the latch into a file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWriteLatch( BfheNet * net, Abc_Obj_t * pLatch )
{
    printf("latches not supported!\n");\
    assert(0);
/*    Abc_Obj_t * pNetLi, * pNetLo;
    int Reset;
    pNetLi = Abc_ObjFanin0( Abc_ObjFanin0(pLatch) );
    pNetLo = Abc_ObjFanout0( Abc_ObjFanout0(pLatch) );
    Reset  = (int)(ABC_PTRUINT_T)Abc_ObjData( pLatch );
    // write the latch line
    fprintf( net, ".latch" );
    fprintf( net, " %10s",    Abc_ObjName(pNetLi) );
    fprintf( net, " %10s",    Abc_ObjName(pNetLo) );
    fprintf( net, "  %d\n",   Reset-1 );
*/
}


/**Function*************************************************************

  Synopsis    [Writes the primary input list.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWriteNodeFanins( BfheNet * net, Abc_Obj_t * pNode )
{
    assert(".names is not supported!\n");
    assert(0);
    /*
    Abc_Obj_t * pNet;
    int LineLength;
    int AddedLength;
    int NameCounter;
    char * pName;
    int i;

    LineLength  = 6;
    NameCounter = 0;
    Abc_ObjForEachFanin( pNode, pNet, i )
    {
        // get the fanin name
        pName = Abc_ObjName(pNet);
        // get the line length after the fanin name is written
        AddedLength = strlen(pName) + 1;
        if ( NameCounter && LineLength + AddedLength + 3 > IO_WRITE_LINE_LENGTH )
        { // write the line extender
            fprintf( net, " \\\n" );
            // reset the line length
            LineLength  = 0;
            NameCounter = 0;
        }
        fprintf( net, " %s", pName );
        LineLength += AddedLength;
        NameCounter++;
    }

    // get the output name
    pName = Abc_ObjName(Abc_ObjFanout0(pNode));
    // get the line length after the output name is written
    AddedLength = strlen(pName) + 1;
    if ( NameCounter && LineLength + AddedLength > 75 )
    { // write the line extender
        fprintf( net, " \\\n" );
        // reset the line length
        LineLength  = 0;
        NameCounter = 0;
    }
    */
}

/**Function*************************************************************

  Synopsis    [Writes the primary input list.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_NtkWriteSubcktFanins( BfheNet * net, Abc_Obj_t * pNode )
{
    printf("subckt not supported\n");
    assert(0);
    /*
    Abc_Obj_t * pNet;
    int LineLength;
    int AddedLength;
    int NameCounter;
    char * pName;
    int i;

    LineLength  = 6;
    NameCounter = 0;

    // get the output name
    pName = Abc_ObjName(Abc_ObjFanout0(pNode));
    // get the line length after the output name is written
    AddedLength = strlen(pName) + 1;
    fprintf( net, " m%d", Abc_ObjId(pNode) );

    // get the input names
    Abc_ObjForEachFanin( pNode, pNet, i )
    {
        // get the fanin name
        pName = Abc_ObjName(pNet);
        // get the line length after the fanin name is written
        AddedLength = strlen(pName) + 3;
        if ( NameCounter && LineLength + AddedLength + 3 > IO_WRITE_LINE_LENGTH )
        { // write the line extender
            fprintf( net, " \\\n" );
            // reset the line length
            LineLength  = 0;
            NameCounter = 0;
        }
        fprintf( net, " %c=%s", 'a'+i, pName );
        LineLength += AddedLength;
        NameCounter++;
    }

    // get the output name
    pName = Abc_ObjName(Abc_ObjFanout0(pNode));
    // get the line length after the output name is written
    AddedLength = strlen(pName) + 3;
    if ( NameCounter && LineLength + AddedLength > 75 )
    { // write the line extender
        fprintf( net, " \\\n" );
        // reset the line length
        LineLength  = 0;
        NameCounter = 0;
    }
    fprintf( net, " %c=%s", 'o', pName );
    */
}


/**Function*************************************************************

  Synopsis    [Writes the primary input list.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Bfhe_NtkWriteNodeGate( BfheNet * net, Abc_Obj_t * pNode, int Length )
{
    static int fReport = 0;
    Mio_Gate_t * pGate = (Mio_Gate_t *)pNode->pData;
    Mio_Pin_t * pGatePin;
    Abc_Obj_t * pNode2;
    int i;
    char * gateType = Mio_GateReadName(pGate);
    const char * gateName =Abc_ObjName( Abc_ObjFanout0(pNode) ); //The name of the gate is the name of the output
    //printf("Writing gate of type %s and name %s!\n",gateType,gateName);
    BfheNode * gateNode;
    BfheNet_newNode(net,gateType,gateName,&gateNode);
    //The name may be changed, update it
    BfheNode_getName(gateNode, &gateName);
    for ( pGatePin = Mio_GateReadPins(pGate), i = 0; pGatePin; pGatePin = Mio_PinReadNext(pGatePin), i++ ){
        char * inputName = Abc_ObjName( Abc_ObjFanin(pNode,i));
        BfheNet_addInputToNode(net,gateName,inputName);
    }
    assert ( i == Abc_ObjFaninNum(pNode) );
    if ( Mio_GateReadTwin(pGate) == NULL )
        return 0;
    pNode2 = Abc_NtkFetchTwinNode( pNode );
    if ( pNode2 == NULL )
    {
        if ( !fReport )
            fReport = 1, printf( "Warning: Missing second output of gate(s) \"%s\".\n", Mio_GateReadName(pGate) );
        return 0;
    }
    //We just need to make a bridge node with a different name
    char * gateName2 = Abc_ObjName( Abc_ObjFanout0(pNode2));
    BfheNet_newNopNodeWithNode(net,gateNode,gateName2);
    return 1;
}

/**Function*************************************************************

  Synopsis    [Write the node into a file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Bfhe_NtkWriteNode( BfheNet * net, Abc_Obj_t * pNode, int Length )
{
    int RetValue = 0;
    if ( Abc_NtkHasMapping(pNode->pNtk) )
    {
        // write the .gate line
        if ( Abc_ObjIsBarBuf(pNode) )
        {
            //This is .barbuf
            BfheNet_newNopNode(net, Abc_ObjName(Abc_ObjFanin0(pNode)), Abc_ObjName(Abc_ObjFanout0(pNode)));
        }
        else
        {
            RetValue = Bfhe_NtkWriteNodeGate( net, pNode, Length );
        }
    }
    else
    {
        printf(".names not supported!\n");
        assert(0);
        /*
        fprintf( net, ".names" );
        Bfhe_NtkWriteNodeFanins( net, pNode );
        // write the cubes
        fprintf( net, "%s", (char*)Abc_ObjData(pNode) );
        */
    }
    return RetValue;
}

/**Function*************************************************************

  Synopsis    [Write the node into a file.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Bfhe_NtkWriteNodeSubckt( BfheNet * net, Abc_Obj_t * pNode, int Length )
{
    printf(".subckt not supported!\n");
    assert(0);
    /*
    int RetValue = 0;
    Bfhe_NtkWriteSubcktFanins( net, pNode );
    return RetValue;*/
    return 0;
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

