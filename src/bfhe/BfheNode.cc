#include "BfheNode.h"
#include "BfheNet.h"

#include "tfhe/tfhe.h"


#include <cassert>
#include <memory>
#include <string>

#include <iostream>


#include <omp.h>

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

LweSample * computeResult(op_enum op, std::vector<BfheNode*> inputs, const TFheGateBootstrappingCloudKeySet* bk, BfheNet * net){
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
            bootsNOT(result,inputs[0]->eval(bk, net), bk);
            break;
        }
        case NOP: {
            result = inputs[0]->eval(bk,net);
        }
	    break;
	case MUX: {
	    //Handle multiplexer case
	    assert(inputs.size()==3);
	    LweSample * select = NULL, *optionOne = NULL, *optionTwo=NULL;
	    #pragma omp parallel sections
	    {
		#pragma omp section
		{
 	    	  select  = inputs[0]->eval(bk,net);
		}
		#pragma omp section
		{
	    	  optionOne = inputs[1]->eval(bk,net);
		}
		#pragma omp section
		{
	    	  optionTwo = inputs[2]->eval(bk,net);
		}
	    }
	    bootsMUX(result,select, optionOne, optionTwo, bk);
	}
            break;
        default: 
            //Hanldle binary operations now
            assert(inputs.size()==2);
            auto op_function = op_bin_function_map[op];
	    LweSample *res0 = NULL, *res1 =NULL;
	    #pragma omp parallel sections
	    {
		#pragma omp section
		{
		    res0 =inputs[0]->eval(bk,net);
		}
		#pragma omp section
		{
		    res1 =inputs[1]->eval(bk,net);
		}
	    }
	    assert(res0!=NULL);
	    assert(res1!=NULL);
            op_function(result, res0,res1, bk);
    }
    return result;
}


BfheNode::BfheNode(std::string _name, LweSample * _result, std::vector<BfheNode*> _inputs, op_enum _op, tree_enum _tree_type):
    name(_name),result(_result), inputs(_inputs), op(_op), tree_type(_tree_type){
   omp_init_lock(&isComputingLock);
   if(inputs.size()!=0){
        std::cout<<"The followings have being set as inputs of "<<name<<": ";
        for(auto it=inputs.begin(); it!=inputs.end(); ++it){
            std::cout<<(*it)->name<<" ";
        }
        std::cout<<std::endl;
   }
    }

BfheNode::BfheNode(std::string _name, op_enum _op): 
    name(_name),result(), inputs(), op(_op), tree_type(GATE){
   omp_init_lock(&isComputingLock);
    }

BfheNode::BfheNode(std::string _name, LweSample * _result):
    name(_name),result(_result), inputs(), op(NOP),tree_type(INPUT){
    omp_init_lock(&isComputingLock);
    }

BfheNode::BfheNode(std::string _name, BfheNode * node1, op_enum _op, tree_enum _tree_type ):
    name(_name),result(), inputs({node1}), op(_op), tree_type(_tree_type){
        assert(tree_type!=GATE);
	omp_init_lock(&isComputingLock);

   if(inputs.size()!=0){
        std::cout<<"The followings have being set as inputs of "<<name<<": ";
        for(auto it=inputs.begin(); it!=inputs.end(); ++it){
            std::cout<<(*it)->name<<" ";
        }
        std::cout<<std::endl;
   }

    }


BfheNode::BfheNode(std::string _name, BfheNode * node1, BfheNode * node2, op_enum _op):
    name(_name),result(nullptr), inputs({node1,node2}), op(_op), tree_type(GATE){
	    omp_init_lock(&isComputingLock);
   
   if(inputs.size()!=0){
        std::cout<<"The followings have being set as inputs of "<<name<<": ";
        for(auto it=inputs.begin(); it!=inputs.end(); ++it){
            std::cout<<(*it)->name<<" ";
        }
        std::cout<<std::endl;
   }
}


LweSample *
BfheNode::graphEval(const TFheGateBootstrappingCloudKeySet* bk, BfheNet * net){
    //std::cout << "Start Evaluation of "<<name<<std::endl;
    omp_set_lock(&isComputingLock);
    if(tree_type == INPUT){
        assert(result!=nullptr);
    }
    if(result == nullptr){
        //std::cout << "calling Evaluation of "<<name<<std::endl;;
        result = computeResult(op,inputs,bk,net);
    }
    omp_unset_lock(&isComputingLock);
    //std::cout << "End Evaluation of "<<name<<std::endl;;
    //net->notifyPerformed(this);
    return result;
}

LweSample *
BfheNode::eval(const TFheGateBootstrappingCloudKeySet* bk, BfheNet * net){
    //std::cout << "Start Evaluation of "<<name<<std::endl;
    if(tree_type == INPUT){
        assert(result!=nullptr);
        return result;
    }
    if(result == nullptr){
        //std::cout << "calling Evaluation of "<<name<<std::endl;
        result = computeResult(op,inputs,bk,net);
    }
    //std::cout << "End Evaluation of "<<name<<std::endl;;
    net->notifyPerformed(this);
    return result;
}

BfheNode::~BfheNode(){
	clean();
}

void BfheNode::clean(){
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

} //End extern "C"
