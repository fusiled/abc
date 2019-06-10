#include "BfheNet.h"
#include "BfheNode.h"

#include <cassert>


#include <chrono>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <iostream>

BfheNet::BfheNet(std::string _name):
    name(_name), cloudKey(nullptr){
    
    }


#include "BfheCommon.h"


BfheNode * dummyBfheNode(std::string name){
    return new BfheNode(name,{},NOP,DUMMY);
}

#define DEFAULT_NET_NAME "BFHE_NET"
extern "C" {
bfhe_code BfheNet_newNet(BfheNet ** netPtr){
    BfheNet * newNet = new BfheNet(std::string(DEFAULT_NET_NAME));
    *netPtr=newNet;
    return OK;
}

bfhe_code BfheNet_destroy(BfheNet * netPtr){
    //Nothing to do for now. Things are going to be destroyed
    //Thanks to smart pointers
    return OK;
}

bfhe_code BfheNet::setName(std::string netName){
    this->name = netName;
    return OK;
}

bfhe_code BfheNet::addInput(std::string inputName){
    auto foundElement =nodeMap.find( inputName );
    assert(foundElement==nodeMap.end());
    BfheNode * newInput =  new BfheNode(inputName,nullptr);
    nodeMap[inputName]= std::unique_ptr<BfheNode>(newInput);
    inputs.push_back(newInput);
    return OK;
}


bfhe_code BfheNet::addOutput(std::string outputName){
    auto foundElement =nodeMap.find( outputName  );
    assert(foundElement==nodeMap.end());
    auto newOutput =new BfheNode(outputName,nullptr,{}, NOP, OUTPUT);
    assert(newOutput!=nullptr);
    outputs.push_back(newOutput);
    nodeMap[outputName]=std::unique_ptr<BfheNode>(newOutput);
    return OK;
}

BfheNode * BfheNet::newNode(op_enum node_op, std::string nodeName){
    auto foundElement =nodeMap.find(nodeName );
    BfheNode * newNode;
    if(foundElement==nodeMap.end()){
        newNode = new BfheNode(nodeName,node_op);
        nodeMap[nodeName]=std::unique_ptr<BfheNode>(newNode);
    }else{
         std::string newNodeName = nodeName+"_";
         newNode = new BfheNode(newNodeName,node_op);
         BfheNode * outputNode = foundElement->second.get();
         assert(outputNode->tree_type == OUTPUT);
         assert(outputNode->inputs.size() ==0 );
         assert(newNode!=nullptr);
         //std::cout<<"Setting "<<newNodeName<<" as input of "<<outputNode->name<<std::endl;
         outputNode->inputs.push_back(newNode);
         nodeMap[newNodeName]=std::unique_ptr<BfheNode>(newNode);
    }
    //std::cout<<"BfheNet created new node "<< *newNode<< std::endl;
    return newNode;
}


bfhe_code BfheNet::newNopNode(std::string inputName, std::string output){
    auto inputElement =nodeMap.find( inputName );
    assert(inputElement!=nodeMap.end()); //Need an existing node
    BfheNode * input = inputElement->second.get();
    return newNopNode(input,output);
}

bfhe_code BfheNet::newNopNode(BfheNode * input, std::string output){
    assert(input!=nullptr);
    auto outputElement = nodeMap.find(output );
    BfheNode * outputNode; 
    if(outputElement==nodeMap.end()){
        //The node does not exists. Create it.
        outputNode = new BfheNode(output,input,NOP,GATE); 
        nodeMap[output]=std::unique_ptr<BfheNode>(outputNode);
    }else{
        //The node exists. It must be an output without a current assigned input. Just bind it to the input
        outputNode = outputElement->second.get();
        assert(outputNode->tree_type == OUTPUT);
        assert(outputNode->inputs.size() ==0 );
    }

    //std::cout<<"Setting "<<input->name<<" as input of "<<outputNode->name<<std::endl;
    outputNode->inputs.push_back(input);
    return OK;
}

bfhe_code BfheNet::addInputToNode(std::string nodeName, std::string inputName){
    auto nodeElement = nodeMap.find(nodeName );
    assert(nodeElement!=nodeMap.end());
    auto inputElement = nodeMap.find(inputName);
    assert(inputElement !=nodeMap.end());
    //std::cout<<"Setting "<<inputElement->second.get()->name <<" as input of "<<nodeElement->second.get()->name<<std::endl;
    nodeElement->second.get()->inputs.push_back( inputElement->second.get() );
    return OK;
}

bfhe_code BfheNet::graphEval(){

    std::cout<<"Evaluating net in parallel"<<std::endl;

    nChildrenRuntime = std::map<std::string,unsigned int>(nChildren);
    std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for
    for(int i=0; i<outputs.size(); i++ ){
    //for(auto outputIter = outputs.begin(); outputIter != outputs.end(); ++outputIter ){
	//(*outputIter)->graphEval(cloudKey,this);
        outputs[i]->graphEval(cloudKey,this);
    }
    std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t_end - t_start ).count();
    std::cout << "Graph computed in "<< name <<": "<<duration/1000.0<<" secs"<< std::endl;
    std::cout << "Processed "<< nodeMap.size()/(duration/1000.0) << " nodes/second "<< std::endl;
    return OK;
}

bfhe_code BfheNet::eval(){
    if(cloudKey==nullptr){
        printf("Error! cloudKey not set in the Net! Aborting eval...\n");
        return ERROR;
    }
    std::cout<<"Evaluating net sequentially"<<std::endl;
    nChildrenRuntime = std::map<std::string,unsigned int>(nChildren);
    //Compute the result
    std::chrono::high_resolution_clock::time_point t_start = std::chrono::high_resolution_clock::now();
    for(auto outputIter = outputs.begin(); outputIter != outputs.end(); ++outputIter ){
        (*outputIter)->eval(cloudKey,this);
    }
    std::chrono::high_resolution_clock::time_point t_end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>( t_end - t_start ).count();
    std::cout << "Evaluation on net"<< name <<": "<<duration/1000.0<<" secs"<< std::endl;
    return OK;
}


bfhe_code BfheNet::loadInput( FILE * fp){
     for(auto inputIter =inputs.begin(); inputIter != inputs.end(); ++inputIter){
        LweSample * bitResult =  new_gate_bootstrapping_ciphertext(cloudKey->params); 
        import_gate_bootstrapping_ciphertext_fromFile(fp, bitResult, this->cloudKey->params);
        if(bitResult==nullptr){
            printf("Error! reading input failed!\n");
            return ERROR;
        }
        (*inputIter)->result = bitResult;
    }
    return OK;
   
}
bfhe_code BfheNet::storeOutput( FILE * fp){
    for(auto outputIter =outputs.begin(); outputIter != outputs.end(); ++outputIter){
        LweSample * bitResult = (*outputIter)->result;
        if(bitResult==nullptr){
            printf("Error! all the outputs have not being computed!\n");
            return ERROR;
        }
        export_gate_bootstrapping_ciphertext_toFile(fp, (*outputIter)->result, this->cloudKey->params);
    }
    return OK;
}


bool BfheNet::hasCloudKeySet(){
    return cloudKey!=nullptr;
}
bfhe_code BfheNet::setCloudKey(TFheGateBootstrappingCloudKeySet* _cloudKey  ){
    this->cloudKey = _cloudKey;
    printf("New cloudKey set to %p\n", cloudKey);
    return OK;
}

int BfheNet::size(){
    return nodeMap.size(); 
}

void BfheNet::notifyPerformed(BfheNode * node){
	//Reduce of 1 all the children of node
	//if nChildrenRuntime of a child is zero, then we can free it
	//This reduces the memory demand of the system
	for( auto child: node->inputs){
		#pragma omp critical
		{
		nChildrenRuntime[child->name]--;
		if(nChildrenRuntime[child->name]==0){
			child->clean();
		}
		} //end critical
	}
}


// C wrappers

bfhe_code BfheNet_setName(BfheNet * net, char * netName){
    return net->setName(std::string(netName) );
}

bfhe_code BfheNet_addInput(BfheNet * net, char * inputName){
    return net->addInput(std::string(inputName));
}

bfhe_code BfheNet_addOutput(BfheNet * net, char * outputName){
    return net->addOutput(std::string(outputName));
}


std::map<std::string,op_enum> str_op_map = {
    {"NAND",  NAND},  
    {"AND",   AND},  
    {"ANDNY", ANDNY},  
    {"ANDYN", ANDYN},  
    {"NOR",   NOR},  
    {"OR",    OR},  
    {"ORNY",  ORNY},  
    {"ORYN",  ORYN},  
    {"XOR",   XOR},  
    {"XNOR",  XNOR},  
    {"NOT",   NOT},  
    {"NOP",   NOP},  
    {"ZERO",  ZERO},
    {"ONE",   ONE}
};

bfhe_code BfheNet_newNode(BfheNet * net, char * gateType, const char * nodeName, BfheNode ** nodePtr){
    auto opMatch = str_op_map.find(std::string(gateType));
    assert(opMatch != str_op_map.end());
    op_enum op = opMatch->second;
    BfheNode * newNode = net->newNode(op,std::string(nodeName));
    *nodePtr = newNode;
    return OK;
}
bfhe_code BfheNet_newNopNode(BfheNet * net, char * input, char * output){
    return net->newNopNode(std::string(input), std::string(output) );
}
bfhe_code BfheNet_newNopNodeWithNode(BfheNet * net, BfheNode * input, char * output){
    return net->newNopNode(input,std::string(output) );
}

bfhe_code BfheNet_addInputToNode(BfheNet * net, const char * nodeName, const char * inputName){
    return net->addInputToNode(std::string(nodeName),std::string(inputName) );
}

bfhe_code BfheNet_eval(BfheNet * net){
    return net->eval();
}


bfhe_code BfheNet_graphEval(BfheNet * net){
    return net->graphEval();
}

bfhe_code BfheNet_loadInput(BfheNet * net, FILE * fp){
    return net->loadInput(fp);
}
bfhe_code BfheNet_storeOutput(BfheNet * net, FILE * fp){
    return net->storeOutput(fp);
}

int BfheNet_size(BfheNet * net){
    return net->size();
}

int BfheNet_inputSize(BfheNet * net){
    return net->inputs.size();
}

int BfheNet_outputSize(BfheNet * net){
    return net->outputs.size();
}

int BfheNet_hasCloudKeySet(BfheNet * net){
    return net->hasCloudKeySet();
}

bfhe_code BfheNet_setCloudKey(BfheNet * net, TFheGateBootstrappingCloudKeySet* cloudKey ){
    return net->setCloudKey(cloudKey);
}

} //End extern "C"
