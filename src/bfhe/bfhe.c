/**CFile****************************************************************

  FileName    [io.c]

  SystemName  [ABC: Logic synthesis and verification system.]

  PackageName [Command processing package.]

  Synopsis    [Command file.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - June 20, 2005.]

  Revision    [$Id: io.c,v 1.00 2005/06/20 00:00:00 alanmi Exp $]

***********************************************************************/

#include "bfheAbc.h"
#include "base/main/mainInt.h"
#include "aig/saig/saig.h"
#include "proof/abs/abs.h"
#include "sat/bmc/bmc.h"

ABC_NAMESPACE_IMPL_START

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

static int BfheCommandCreateNet ( Abc_Frame_t * pAbc, int argc, char **argv );
static int BfheCommandEvalNet ( Abc_Frame_t * pAbc, int argc, char **argv );
static int BfheCommandLoadInput ( Abc_Frame_t * pAbc, int argc, char **argv );
static int BfheCommandStoreOutput ( Abc_Frame_t * pAbc, int argc, char **argv );
static int BfheCommandSetCloudKey ( Abc_Frame_t * pAbc, int argc, char **argv );
////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFINITIONS                         ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_Init( Abc_Frame_t * pAbc )
{
    Cmd_CommandAdd( pAbc, "Bfhe", "bfhe_create_net",    BfheCommandCreateNet,    0 );
    Cmd_CommandAdd( pAbc, "Bfhe", "bfhe_eval",    BfheCommandEvalNet,    0 );
    Cmd_CommandAdd( pAbc, "Bfhe", "bfhe_load_input",    BfheCommandLoadInput,    0 );
    Cmd_CommandAdd( pAbc, "Bfhe", "bfhe_store_output",    BfheCommandStoreOutput,    0 );
    Cmd_CommandAdd( pAbc, "Bfhe", "bfhe_set_cloud_key",    BfheCommandSetCloudKey,    0 );
}

#include "BfheNet.h"
#include "tfhe/tfhe.h"

BfheNet * THE_NET = NULL;

/**Function*************************************************************

  Synopsis    []

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Bfhe_End( Abc_Frame_t * pAbc )
{
    if(THE_NET!=NULL){
        BfheNet_destroy(THE_NET);
    }
}



/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int BfheCommandCreateNet( Abc_Frame_t * pAbc, int argc, char **argv )
{
    char c;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "h" ) ) != EOF )
    {
        switch ( c )
        {
            case 'h':
                goto usage;
            default:
                goto usage;
        }
    }
    if ( pAbc->pNtkCur == NULL )
    {
        fprintf( pAbc->Out, "Empty network.\n" );
        return 0;
    }
    BfheNet_newNet(&THE_NET);
    if(THE_NET==NULL){
        fprintf( pAbc->Err, "Cannot allocate new BfheNet!\n" );
        return 2;
    }
    Bfhe_CreateNet( pAbc->pNtkCur, THE_NET, 0,0,0 );
    fprintf(pAbc->Out, "Created BfheNet of size: %d\n", BfheNet_size(THE_NET) );
    return 0;

usage:
    fprintf( pAbc->Err, "usage: create_net [-h] <file>\n" );
    fprintf( pAbc->Err, "\t-h     : print the help message\n" );
    return 1;
}

//bfhe_eval 
int BfheCommandEvalNet( Abc_Frame_t * pAbc, int argc, char **argv )
{
    printf("Evaluating the net!\n"); 
    if ( THE_NET  == NULL )
    {
        fprintf( pAbc->Out, "Empty Bfhe network. Run create_net first\n" );
        return 0;
    }
    if(!BfheNet_hasCloudKeySet(THE_NET)){
        fprintf( pAbc->Out, "Please load the cloud key with bfhe_set_cloud_key command\n" );
        return 0;
    }

    int c;
    
    char graphMode = 0;
    Extra_UtilGetoptReset();
    while ( ( c = Extra_UtilGetopt( argc, argv, "gh" ) ) != EOF )
    {
        switch ( c )
        {
            case 'g':
                graphMode = 1;
                break;
            case 'h':
                goto usage;
        }
    }

    //Eval the net 
    printf("Entering the eval with graph mode: %d\n",graphMode); 
    Bfhe_EvalNet(THE_NET,graphMode);
    return 0;
usage:
    fprintf( pAbc->Err, "usage: bfhe_eval [-gh] <file>\n" );
    fprintf( pAbc->Err, "\t         Compute Homomorphically the already specified network with the given key\n" );
    fprintf( pAbc->Err, "\t-g     : toggle cudaGraph acceleration [default = no]\n" );
    fprintf( pAbc->Err, "\t-h     : prints the command summary\n" );
    return 1;

}

//bfhe_load_input <path-to-output>
int BfheCommandLoadInput ( Abc_Frame_t * pAbc, int argc, char **argv ){
    if(argc<2){
        fprintf( pAbc->Out, "Please pass a file as source of the input\n" );
        goto usage;
    }
    if ( THE_NET  == NULL )
    {
        fprintf( pAbc->Err, "Empty Bfhe network. Run create_net first\n" );
        return 1;
    }
    
    Bfhe_LoadInput(THE_NET, argv[1]);
    return 0;

usage:
    fprintf( pAbc->Err, "usage: bfhe_load_input  <file>\n" );
    return 1;
}


//bfhe_store_output <path-to-output>
int BfheCommandStoreOutput ( Abc_Frame_t * pAbc, int argc, char **argv ){
    if(argc<2){
        fprintf( pAbc->Err, "Please pass a file as destination of the output\n" );
        goto usage;
    }
    if ( THE_NET  == NULL )
    {
        fprintf( pAbc->Err, "Empty Bfhe network. Run create_net first\n" );
        return 0;
    }
    Bfhe_StoreOutput(THE_NET, argv[1]);    
    return 0;

usage:
    fprintf( pAbc->Err, "usage: bfhe_store_output  <file>\n" );
    return 1;
}

//bfhe_set_cloudKey <path-to-key>
int BfheCommandSetCloudKey ( Abc_Frame_t * pAbc, int argc, char **argv ){
    if ( THE_NET  == NULL )
    {
        fprintf( pAbc->Err, "Empty Bfhe network. Run create_net first\n" );
        return 0;
    }
    //Read the key from the file
    FILE* cloud_key = fopen(argv[1],"rb");
    if(cloud_key==NULL){
        fprintf( pAbc->Err, "Cannot open cloud key file at path %s\n",argv[1] );
        return 0;
    }
    TFheGateBootstrappingCloudKeySet* bk = new_tfheGateBootstrappingCloudKeySet_fromFile(cloud_key);
    fclose(cloud_key);
    fprintf( pAbc->Out, "Read key at path %s\n",argv[1] );

    BfheNet_setCloudKey(THE_NET, bk);
    printf("Finished setting new cloudKey!\n");
    return 0;
}
