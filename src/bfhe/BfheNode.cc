#include "BfheNode.h"
#include "BfheNet.h"

#include "tfhe/tfhe.h"


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
        default: 
            //Hanldle binary operations now
            assert(inputs.size()==2);
            auto op_function = op_bin_function_map[op];
            op_function(result, inputs[0]->eval(bk,net),inputs[1]->eval(bk,net),bk);
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


//Tasktipe.precede(anotherTask)

tf::Task BfheNode::buildGraph(tf::Taskflow & tf, const TFheGateBootstrappingCloudKeySet* bk, BfheNet * net){
    tf::Task thisTask = tf.emplace([this,bk,net]{ this->eval(bk, net); }); //create the task in some way
    for( auto input : inputs){
        input->buildGraph(tf,bk, net).precede(thisTask) ;
    }
    return thisTask;
}

LweSample *
BfheNode::eval(const TFheGateBootstrappingCloudKeySet* bk, BfheNet * net){
    //std::cout << "Start Evaluation of "<<name<<std::endl;;
    if(tree_type == INPUT){
        assert(result!=nullptr);
        return result;
    }
    if(result == nullptr){
        //std::cout << "calling Evaluation of "<<name<<std::endl;;
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
