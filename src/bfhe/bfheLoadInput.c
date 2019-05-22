
#include "bfheAbc.h"
#include "bfhe.h"

ABC_NAMESPACE_IMPL_START

void Bfhe_LoadInput(BfheNet * net, char * path){
    FILE * fp = fopen(path,"rb");
    if(fp==NULL){
        printf("Cannot open network input file %s",path);
        return;
    }
    BfheNet_loadInput(net, fp);
    fclose(fp);
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

