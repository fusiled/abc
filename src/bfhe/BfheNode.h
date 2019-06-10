#ifndef __BFHE_NODE__
#define __BFHE_NODE__

#include "BfheCommon.h"

#ifdef __cplusplus


#include <string>
#include <memory>
#include <vector>
#include <ostream>

#include <omp.h>

class BfheNet;
struct LweSample;
struct TFheGateBootstrappingCloudKeySet;
//Negate with bootsNOT
enum op_enum {
    NAND   = 0,      // !(a & b)    bootsNAND
    AND    = 1,      // a & b       bootsAND
    ANDNY  = 2,      // !a & b      bootsANDNY
    ANDYN  = 3,      // a & !b      bootsANDYN
    NOR    = 4,      // !(a | b)    bootsNOR
    OR     = 5,      // a | b       bootsOR
    ORNY   = 6,      // !a | b      bootsORNY
    ORYN   = 7,      //a | !b       bootsORYN
    XOR    = 8,      // a ^ b       bootsXOR
    XNOR   = 9,      // !(a ^ b)    bootsXNOR
    NOT    = 10,     // !a          bootNOT
    NOP    = 11,     // a           nothing to do, constant node
    ZERO   = 12,     // return 0
    ONE    = 13,     // return 1
    N_OP_ENUM,
};

enum tree_enum {
    INPUT,
    OUTPUT,
    GATE,
    DUMMY
};



struct BfheNode{
    std::string name;
    LweSample * result;
    std::vector<BfheNode *> inputs;
    op_enum op;
    tree_enum tree_type;
    omp_lock_t isComputingLock;

    BfheNode(std::string, LweSample *, std::vector<BfheNode*>, op_enum, tree_enum _tree_type); //Generic constructor
    BfheNode(std::string, LweSample *); //Input constructor
    BfheNode(std::string, op_enum); //Gate with no input yet
    BfheNode(std::string, BfheNode *, op_enum, tree_enum _tree_type ); //Single Input constructor
    BfheNode(std::string, BfheNode *, BfheNode *, op_enum); //Binary input constructor
    ~BfheNode();
    void clean();
    LweSample* graphEval(const TFheGateBootstrappingCloudKeySet*,BfheNet *);
    LweSample* eval(const TFheGateBootstrappingCloudKeySet*,BfheNet *);
};
    
    std::ostream& operator<<(std::ostream &strm, const BfheNode &a);

struct BfheGraphNodeParams{
    BfheNode * node;
    const TFheGateBootstrappingCloudKeySet* bk;
    BfheGraphNodeParams(BfheNode * _node, const TFheGateBootstrappingCloudKeySet* _bk):
        node(_node),bk(_bk){}
};


extern "C" {

#else
    struct BfheNode;
    typedef struct BfheNode BfheNode;

#endif //__cplusplus

    bfhe_code BfheNode_getName(BfheNode * node, const char ** name);
#ifdef __cplusplus
} //end Extern "C"
#endif
#endif
