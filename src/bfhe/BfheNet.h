#ifndef __BFHE_NET_HH__
#define __BFHE_NET_HH__


#include <driver_types.h>

#include "BfheCommon.h"
#include "BfheNode.h"

#include "tfhe/tfhe.h"

#ifdef __cplusplus
#include <map>
#include <string>
#include <memory>

class UniquePtrNodeComparator: public std::binary_function<std::unique_ptr<BfheNode>, std::unique_ptr<BfheNode>, bool>
{
 public:
    auto operator()(const std::unique_ptr<BfheNode> & a, const std::unique_ptr<BfheNode> & b) const -> bool
    {
       /* if(a.get()==nullptr){
            if(b.get()==nullptr){
                return false;
            }
            else{
                return true;
            }
        }else{
            if(b.get()==nullptr){
                return false;
            }
            else{*/
                return a->name < b->name;
   /*         }
        }*/
    }
};


struct BfheNet {

    std::string name;

    //This set uses the name of the node as the key! 
    //std::set<std::unique_ptr<BfheNode>,bool(*)(std::unique_ptr<BfheNode>, std::unique_ptr<BfheNode>)> nodeSet;
    std::map<std::string,std::unique_ptr<BfheNode> > nodeMap;

    std::vector<BfheNode *> inputs;
    std::vector<BfheNode *> outputs;
    TFheGateBootstrappingCloudKeySet* cloudKey; 

    BfheNet(std::string);
    ~BfheNet(){
        if(cloudKey!=nullptr){
            delete_gate_bootstrapping_cloud_keyset(cloudKey);
        }
    };


    BfheNode * newNode(op_enum node_op, std::string nodeName);
    
    bfhe_code setName(std::string netName);

    bfhe_code addInput( std::string  inputName);
    bfhe_code addOutput( std::string outputName);

    bfhe_code newNopNode(BfheNode *input, std::string output);
    bfhe_code newNopNode(std::string inputName, std::string output);

    bfhe_code addInputToNode(std::string nodeName, std::string inputName);
    
    bfhe_code eval();
    bfhe_code graphEval();

    bfhe_code loadInput(FILE* fp);
    bfhe_code storeOutput( FILE * fp);

    bool hasCloudKeySet();
    bfhe_code setCloudKey(TFheGateBootstrappingCloudKeySet* cloudKey  );
    int size();
};
extern "C" {
#else
    struct BfheNet;
    typedef struct BfheNet BfheNet;
#endif// __cplusplus
//C headers

bfhe_code BfheNet_newNet(BfheNet ** netPtr);
bfhe_code BfheNet_destroy(BfheNet * netPtr);
bfhe_code BfheNet_setName(BfheNet * net, char * netName);

bfhe_code BfheNet_addInput(BfheNet * net, char * inputName);
bfhe_code BfheNet_addOutput(BfheNet * net, char * outputName);

bfhe_code BfheNet_newNode(BfheNet * net, char * gateType, const char * nodeName, BfheNode ** nodePtr);
bfhe_code BfheNet_newNopNode(BfheNet * net, char * input, char * output);
bfhe_code BfheNet_newNopNodeWithNode(BfheNet * net, BfheNode * input, char * output);

bfhe_code BfheNet_addInputToNode(BfheNet * node, const char * nodeName, const char * inputName);

bfhe_code BfheNet_eval(BfheNet * net);
bfhe_code BfheNet_graphEval(BfheNet * net);

bfhe_code BfheNet_loadInput(BfheNet * net, FILE* fp);
bfhe_code BfheNet_storeOutput(BfheNet * net, FILE * fp);

int BfheNet_hasCloudKeySet(BfheNet * net);
bfhe_code BfheNet_setCloudKey(BfheNet * net, TFheGateBootstrappingCloudKeySet* cloudKey );

int BfheNet_size(BfheNet * net);

#ifdef  __cplusplus
} //end extern C
#endif// __cplusplus


#endif
