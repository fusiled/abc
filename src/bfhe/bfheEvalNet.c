
#include "bfheAbc.h"
#include "bfhe.h"

#include "base/main/main.h"
#include "map/mio/mio.h"
#include "bool/kit/kit.h"
#include "map/if/if.h"

ABC_NAMESPACE_IMPL_START




void Bfhe_EvalNet(BfheNet * net, char graphModeEnabled){
    if(graphModeEnabled){
        BfheNet_graphEval(net);
    } else {
        BfheNet_eval(net);
    }
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


ABC_NAMESPACE_IMPL_END

