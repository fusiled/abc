
#include "bfheAbc.h"
#include "bfhe.h"

ABC_NAMESPACE_IMPL_START

void Bfhe_StoreOutput(BfheNet * net, char * path){
    FILE * fp = fopen(path,"wb");
    if(fp==NULL){
        printf("Cannot open network output file %s",path);
        return;
    }
    BfheNet_storeOutput(net, fp);
    fclose(fp);
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

