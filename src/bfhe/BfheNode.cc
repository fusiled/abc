#include "BfheNode.h"

#include "tfhe/tfhe.h"

#include <driver_types.h>
#include <cuda_runtime_api.h>

#include <cassert>
#include <memory>
#include <string>

#include <iostream>


    //Array of function pointers with the same interface
void (*op_bin_function_map[])(LweSample *, const LweSample *, const LweSample *,const TFheGateBootstrappingCloudKeySet*) = {
    bootsNAND,
    bootsAND,
    bootsANDNY,
    bootsANDYN,
    bootsNOR,
    bootsOR,
    bootsORNY,
    bootsORYN,
    bootsXOR,
    bootsXNOR
};


LweSample * computeResult(op_enum op, std::vector<BfheNode*> inputs, const TFheGateBootstrappingCloudKeySet* bk){
    //assert(op!=NOP);
    LweSample * result = new_gate_bootstrapping_ciphertext(bk->params);
    //Handle the NOT operation first
    switch(op){
        case ZERO: {
            assert(inputs.size()==0);
            bootsCONSTANT(result,0,bk);
            break;
        }
        case ONE: {
            assert(inputs.size()==0);
            bootsCONSTANT(result,1,bk);
            break;
        }
        case NOT: {
            assert(inputs.size()==1);
            bootsNOT(result,inputs[0]->eval(bk), bk);
            break;
        }
        case NOP: {
            result = inputs[0]->eval(bk);
        }
            break;
        default: 
            //Hanldle binary operations now
            assert(inputs.size()==2);
            auto op_function = op_bin_function_map[op];
            op_function(result, inputs[0]->eval(bk),inputs[1]->eval(bk),bk);
    }
    return result;
}


BfheNode::BfheNode(std::string _name, LweSample * _result, std::vector<BfheNode*> _inputs, op_enum _op, tree_enum _tree_type):
    name(_name),result(_result), inputs(_inputs), op(_op), tree_type(_tree_type) {
   if(inputs.size()!=0){
        std::cout<<"The followings have being set as inputs of "<<name<<": ";
        for(auto it=inputs.begin(); it!=inputs.end(); ++it){
            std::cout<<(*it)->name<<" ";
        }
        std::cout<<std::endl;
   }
    }

BfheNode::BfheNode(std::string _name, op_enum _op): 
    name(_name),result(), inputs(), op(_op), tree_type(GATE) { }

BfheNode::BfheNode(std::string _name, LweSample * _result):
    name(_name),result(_result), inputs(), op(NOP),tree_type(INPUT) {}

BfheNode::BfheNode(std::string _name, BfheNode * node1, op_enum _op, tree_enum _tree_type ):
    name(_name),result(), inputs({node1}), op(_op), tree_type(_tree_type) {
        assert(tree_type!=GATE);

   if(inputs.size()!=0){
        std::cout<<"The followings have being set as inputs of "<<name<<": ";
        for(auto it=inputs.begin(); it!=inputs.end(); ++it){
            std::cout<<(*it)->name<<" ";
        }
        std::cout<<std::endl;
   }

    }


BfheNode::BfheNode(std::string _name, BfheNode * node1, BfheNode * node2, op_enum _op):
    name(_name),result(nullptr), inputs({node1,node2}), op(_op), tree_type(GATE) {
   
   if(inputs.size()!=0){
        std::cout<<"The followings have being set as inputs of "<<name<<": ";
        for(auto it=inputs.begin(); it!=inputs.end(); ++it){
            std::cout<<(*it)->name<<" ";
        }
        std::cout<<std::endl;
   }
}

#ifdef CUDAGRAPH 

cudaGraphNode_t BfheNode::buildGraph(cudaGraph_t graph, const TFheGateBootstrappingCloudKeySet* bk){
        std::vector<cudaGraphNode_t> deps;
        for( auto input : inputs){
            deps.push_back(input->buildGraph(graph,bk)) ;
    }
    cudaHostNodeParams hostParams = {0};
    hostParams.fn = BfheNode_graph_eval;
    BfheGraphNodeParams * nodeParams = new BfheGraphNodeParams(this,bk);
    hostParams.userData = nodeParams;

    if(cudaGraphAddHostNode(&graphNode, graph, deps.data(), deps.size(), &hostParams)!=cudaSuccess){
        std::cerr<<"Cannot add cudaGraphNode for "<<name<<" to computation graph"<<std::endl;
    }
    return graphNode;
}
#endif

LweSample *
BfheNode::eval(const TFheGateBootstrappingCloudKeySet* bk){
    //std::cout << "Start Evaluation of "<<name<<std::endl;;
    if(tree_type == INPUT){
        assert(result!=nullptr);
        return result;
    }
    if(result == nullptr){
        //std::cout << "calling Evaluation of "<<name<<std::endl;;
        result = computeResult(op,inputs,bk);
    }
    //std::cout << "End Evaluation of "<<name<<std::endl;;
    return result;
}

BfheNode::~BfheNode(){
   if(result != nullptr || op!=NOP){
        delete_gate_bootstrapping_ciphertext(result); 
   }
}

std::ostream& operator<<(std::ostream &strm, const BfheNode &a) {
  return strm << "Node( name:" << a.name <<", op:" << a.op<<" )";
}

extern "C" {

bfhe_code BfheNode_getName(BfheNode * node, const char ** name){
    const char * nodeName = node->name.c_str();
    *name=nodeName;
    return OK;
}

#ifdef CUDAGRAPH

void BfheNode_graph_eval(void * args){
    BfheGraphNodeParams * params =(BfheGraphNodeParams *) args;
    BfheNode * node = params->node;
    const TFheGateBootstrappingCloudKeySet* bk = params->bk;
    node->eval(bk);
}

#endif

} //End extern "C"
