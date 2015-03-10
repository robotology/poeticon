// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Pedro Vicente <pvicente@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include "translator.h"

using namespace yarp::os;
using namespace std;

// Translator Module
double TranslatorModule::getPeriod() {
    return 0;
}
TranslatorModule::switchCase TranslatorModule::hashtable(string command){
	if(command=="name")  return name;
	if(command=="desc2d")  return desc;
	if(command=="tooldesc2d")  return tooldesc2d;
	if(command=="is_hand")  return is_h;
	if(command=="in_hand")  return in_h;
	if(command=="on_top_of")  return on_t;
	if(command=="is_free")  return free;
	if(command=="reachable_with")  return re_w;
	if(command=="pullable_with")  return pu_w;
	if(command=="is_touching") return touch;
	if(command=="pos") return pos;
}
bool TranslatorModule::interruptModule() {

    cout << "Interrupting your module, for port cleanup" << endl;
    readingThread->_runit=false; 
    readingThread->askToStop();   
	if(!readingThread->_runit) {
        rpc_port.interrupt();
    	port_broad.interrupt();
    }
    translatorPort.interrupt();
    return true;
}

bool   TranslatorModule::close() {

    /* optional, close port explicitly */
    cout << "Calling close function\n";

    delete readingThread;
    rpc_port.close();
    port_broad.close();
    translatorPort.close();

    return true;
}
bool   TranslatorModule::updateModule() {
    int ObjectID;
    Bottle *receive,dataBase,ids2,*idsp;
    if(readingThread->_runit) {
        receive = translatorPort.read(false);  //non-block       
        if(receive == NULL)
            return true;
        if(receive->get(0).asString() == "update") {
            cout << "Writing the world state to file..." << endl;
            readingThread->guard.lock();
            dataBase = readingThread->_data;
            ids2 = readingThread->_ids;
            readingThread->guard.unlock();

            if(dataBase.size()>0 && (dataBase.get(1).asString()!="empty")) {
	            //cout << "f1: " << objIDsFileName << "f2: " << stateFileName << endl;
                //myfile.open( objIDsFileName);
                //myfile2.open ( stateFileName);

                myfile.open( objIDsFileName.c_str());
                myfile2.open ( stateFileName.c_str());

	            //myfile.open ("Object names-IDs.dat");
                //myfile2.open ("state.dat");
                idsp = ids2.get(1).asList();
                idsp = idsp->get(1).asList();
                for(int i=1;i<dataBase.size();i++){ // for each object
                    Bottle *objecto = dataBase.get(i).asList();
                    for(int j=0;j<objecto->size();j++) { // for each properties
                        Bottle *propriedade = objecto->get(j).asList();
                        switchCase r = hashtable(propriedade->get(0).asString());
                        switch(r) {
                            case name:{
			    		        myfile <<"(" << idsp->get((i-1)).asInt() << "," << propriedade->get(1).asString().c_str() << ");";
                                break;
                            }
                            case desc: {
                                break;
                            }
                            case tooldesc2d: {
                                break;
                            }
                            case pos: {
                                break;
                            }
                            case on_t: {
                                Bottle *ontop = propriedade->get(1).asList();
                                for(int k=0; k < ontop->size(); k++){
                                    myfile2 << idsp->get((i-1)).asInt() <<"_on_" <<ontop->get(k).asInt() <<"() ";
                                }
                                break;
                            }
                            case re_w: {
                                Bottle *reachable = propriedade->get(1).asList();
                                for(int k=0; k < reachable->size(); k++){
                                    myfile2 << idsp->get((i-1)).asInt() <<"_isreachable_with_" <<reachable->get(k).asInt() <<"() ";
                                }
                                break;
                            }
                            case pu_w: {
                                Bottle *pullable = propriedade->get(1).asList();
                                for(int k=0; k < pullable->size(); k++){
                                    myfile2 << idsp->get((i-1)).asInt() <<"_ispullable_with_" <<pullable->get(k).asInt() <<"() ";
                                }
                                break;
                            }
                            case is_h: {
                                if(propriedade->get(1).asString() == "true") {
                                    myfile2 << idsp->get((i-1)).asInt() <<"_ishand" <<"() ";
                                }
                                break;
                            }
                            case free: {
                                if(propriedade->get(1).asString() == "true") {
                                    myfile2 << idsp->get((i-1)).asInt() <<"_clearhand" <<"() ";
                                    myfile2 << idsp->get((i-1)).asInt() <<"_istool" <<"() ";
                                }
                                break;
                            }
                            case in_h: {
                                if(propriedade->get(1).asString() == "right") {
                                    myfile2 << idsp->get((i-1)).asInt() <<"_inhand_" << "12" << "() "; //according to dbhands.ini - id 12 corresponds to the right hand
                                    myfile2 << idsp->get((i-1)).asInt() <<"_istool" <<"() ";
                                }
                                if(propriedade->get(1).asString() == "left") {
                                    myfile2 << idsp->get((i-1)).asInt() <<"_inhand_" << "11" << "() "; //according to dbhands.ini - id 12 corresponds to the right hand
                                    myfile2 << idsp->get((i-1)).asInt() <<"_istool" <<"() ";
                                }
                                break;
                            }

                            default: {
                                cout << "not known attribute..." << endl;
                                break;
                            }
                        } // end switch
                    } // end for property
                } // end for object
        
                myfile.close();
                myfile2.close();

            } // end if
            Bottle &send = translatorPort.prepare();
            send.clear();
            send.addString("ACK");
            cout << "DONE" << endl;
            translatorPort.write();
        } // end "update"
        else if(receive->get(0).asString() == "querydesc2d" && receive->get(1).isInt()) {
            /*cout << "query received" << endl;*/
            readingThread->guard.lock();
            dataBase = readingThread->_data;
            ids2 = readingThread->_ids;
            readingThread->guard.unlock();
            idsp = ids2.get(1).asList();
            idsp = idsp->get(1).asList();
            ObjectID = receive->get(1).asInt();
            for(int i=0;i<idsp->size();i++) {
                if(idsp->get(i).asInt() == ObjectID) {
                    ObjectID = i;
                    break;
                }
            }
            Bottle *objecto = dataBase.get(ObjectID+1).asList();
            /*cout << ObjectID << endl;*/
            for(int j=0;j<objecto->size();j++) { // for each properties
                Bottle *propriedade = objecto->get(j).asList();
                switchCase r = hashtable(propriedade->get(0).asString());
                /*cout << propriedade->get(0).asString().c_str() << endl;*/
                switch(r) {
                    case name:{
    		            /*myfile <<"(" << idsp->get((i-1)).asInt() << "," << propriedade->get(1).asString().c_str() << ");";*/
                        break;
                    }
                    case desc: {
                        Bottle &send = translatorPort.prepare();
                        send.clear();
                        send.addList()=*propriedade->get(1).asList();
//                        cout << send.toString().c_str() << endl;
                        translatorPort.write();
                        break;
                    }
                    case pos: {
                        break;
                    }
                    case on_t: {
                        /*Bottle *ontop = propriedade->get(1).asList();
                        for(int k=0; k < ontop->size(); k++){
                        myfile2 << idsp->get((i-1)).asInt() <<"_on_" <<ontop->get(k).asInt() <<"() ";
                        }*/
                        break;
                    }
                    case re_w: {
                        /*Bottle *reachable = propriedade->get(1).asList();
                        for(int k=0; k < reachable->size(); k++){
                        myfile2 << idsp->get((i-1)).asInt() <<"_isreachable_with_" <<reachable->get(k).asInt() <<"() ";
                        }*/
                        break;
                    }
                    case pu_w: {
                        /*Bottle *pullable = propriedade->get(1).asList();
                        for(int k=0; k < pullable->size(); k++){
                            myfile2 << idsp->get((i-1)).asInt() <<"_ispullable_with_" <<pullable->get(k).asInt() <<"() ";
                        }*/
                        break;
                    }
                    /* case touch: {
                        Bottle *touch = propriedade->get(1).asList();
                        for(int k=0; k < touch->size(); k++){
                            myfile2 << idsp->get((i-1)).asInt() <<"_touch_" <<touch->get(k).asInt() <<"() ";
                        }
                        break;
                    }*/
                    case is_h: {
                        /*if(propriedade->get(1).asString() == "true") {
                            myfile2 << idsp->get((i-1)).asInt() <<"_ishand" <<"() ";
                        }*/
                        break;
                    }
                    case free: {
                        /*if(propriedade->get(1).asString() == "true") {
                            myfile2 << idsp->get((i-1)).asInt() <<"_clearhand" <<"() ";
                            myfile2 << idsp->get((i-1)).asInt() <<"_istool" <<"() ";
                        }*/
                        break;
                    }
                    case in_h: {
                        /*if(propriedade->get(1).asString() == "right") {
                            myfile2 << idsp->get((i-1)).asInt() <<"_inhand_" << "12" << "() "; //according to dbhands.ini - id 12 corresponds to the right hand
                            myfile2 << idsp->get((i-1)).asInt() <<"_istool" <<"() ";
                        }
                        if(propriedade->get(1).asString() == "left") {
                            myfile2 << idsp->get((i-1)).asInt() <<"_inhand_" << "11" << "() "; //according to dbhands.ini - id 12 corresponds to the right hand
                            myfile2 << idsp->get((i-1)).asInt() <<"_istool" <<"() ";
                        }*/
                        break;
                    }

                    default: {
                        cout << "not known attribute..." << endl;
                        break;
                    }
                }
            }
        } // end "querydesc2d"
        else if(receive->get(0).asString() == "querytooldesc2d" && receive->get(1).isInt()) {

            /*cout << "query received" << endl;*/
            readingThread->guard.lock();
            dataBase = readingThread->_data;
            ids2 = readingThread->_ids;
            readingThread->guard.unlock();
            idsp = ids2.get(1).asList();
            idsp = idsp->get(1).asList();
            ObjectID = receive->get(1).asInt();
            for(int i=0;i<idsp->size();i++) {
                if(idsp->get(i).asInt() == ObjectID) {
                    ObjectID = i;
                    break;
                }
            }
            Bottle *objecto = dataBase.get(ObjectID+1).asList();
            /*cout << ObjectID << endl;*/
            for(int j=0;j<objecto->size();j++) { // for each properties
                Bottle *propriedade = objecto->get(j).asList();
                switchCase r = hashtable(propriedade->get(0).asString());
                /*cout << propriedade->get(0).asString().c_str() << endl;*/
                switch(r) {
                    case tooldesc2d: {
                        Bottle &send = translatorPort.prepare();
                        send.clear();
                        send.addList()=*propriedade->get(1).asList();
                        translatorPort.write();
                        break;
                    }
                }
            }

        } // end "querytooldesc2d"
        else {
            Bottle &send = translatorPort.prepare();
            send.clear();
            send.addString("NACK");
            translatorPort.write();
        }
    }
    return true;
}

bool   TranslatorModule::configure(yarp::os::ResourceFinder &rf) {

    /* module name */
    moduleName = rf.check("name", Value("opc2prada"),
                          "Module name (string)").asString();

    setName(moduleName.c_str());

    /* port names */
	translatorPortName  = "/" + moduleName + "/cmd:io";

    /* open ports */
    if (!translatorPort.open(
            translatorPortName.c_str()))
    {
        cout << getName() << ": unable to open port"
        << translatorPortName << endl;
        return false;
    }

    /* OPC comunication*/

    string broadname = "/" + moduleName + "/broadcast:i";
    /* open broad port */
    if (!port_broad.open(
            broadname.c_str()))
    {
        cout << getName() << ": unable to open port"
        << broadname << endl;
        return false;
    }
    string RpcClientname = "/" + moduleName + "/rpcClient";
    /* open rpc client port */
    if (!rpc_port.open(
            RpcClientname.c_str()))
    {
        cout << getName() << ": unable to open port"
        << RpcClientname << endl;
        return false;
    }

    /* Rate thread period */
    threadPeriod = rf.check("threadPeriod", Value(0.033),
        "Control rate thread period key value(double) in seconds ").asDouble();
    int dummy;
    string objIdsName ="/Object_names-IDs.dat";
    string stateName ="/state.dat";
    string path = rf.findPath("contexts/"+rf.getContext());
    if (path==""){
        cout << "path to contexts/"+rf.getContext() << " not found" << endl;
        return false;    
    }
    else {
        cout << "Context FOUND!" << endl;
        //objIdsName=path+objIdsName;
        objIDsFileName=path+objIdsName;//objIdsName.c_str();
        //stateName = path+stateName;
        stateFileName = path+stateName;//stateName.c_str();
        cout << objIDsFileName << " and " << stateFileName<< endl;
    }

/*    if(rf.findFileByName("Object_names-IDs.dat") ==""){ // create file
        cout << "Object_names-IDs.dat -> file not found" << endl;
        return false;
    }
    else{
        objIDsFileName = rf.findFileByName("Object_names-IDs.dat").c_str();
    }
    if(rf.findFileByName("state.dat") == "") { // create file
        cout << "state.dat -> file not found" << endl;
        return false;
    } 
    else{
        stateFileName = rf.findFileByName("state.dat").c_str();
    }
*/


    
    /* Create the reading rate thread */
    readingThread = new Thread_read(&port_broad,
                                &rpc_port,
                                threadPeriod);
    
    /* Starts the thread */
    if (!readingThread->start()) {
        delete readingThread;
        return false;
    }

    return true;
}


//// Thread_read

//constructor thread_read

Thread_read::Thread_read(BufferedPort<Bottle> * broad_port,RpcClient * rpc,int r)
: RateThread(r) {
    _port_broad = broad_port;
    _rpc_port = rpc;
    _runit = false;
};

void Thread_read::threadRelease() {
        printf("I my God they kill kenny - the thread\n");
        //getchar();
}
bool Thread_read::threadInit(){
    _runit = false;
    bool test=true;
        bool test2=true;
        printf("Starting reading thread...Waiting for connection...\n");
        _runit = false;
        while( test && test2){
            test = _port_broad->getInputCount()==0 || _rpc_port->getOutputCount()==0 ;
            Time::delay(0.1);
            if(_port_broad->isClosed())
                return false;
           // cout << test2 << endl;
        }
        printf("Connection Done!");
        Bottle *received,rpc_cmd,rpc_response,aux;
        Bottle cmd,response;

        // Start syncronous broadcast to initialize _data and _ids
        cmd.clear();
        cmd.addVocab(Vocab::encode("sync"));
        cmd.addVocab(Vocab::encode("start"));
        cmd.add(0.1);
        _rpc_port->write(cmd,response);
        // read database and ask ids
        rpc_cmd.clear();
        rpc_cmd.addVocab(Vocab::encode("ask"));
        aux.addString("all");
        rpc_cmd.addList() = aux;
        received = _port_broad->read();
        // Stop synchronous broadcast
        cmd.clear();
        cmd.addVocab(Vocab::encode("sync"));
        cmd.addVocab(Vocab::encode("stop"));
        _rpc_port->write(cmd,response);
        Time::delay(1);
        // Start asynchronous
        _rpc_port->write(rpc_cmd,rpc_response);
        cmd.clear();
        cmd.addVocab(Vocab::encode("async"));
        cmd.addVocab(Vocab::encode("on"));
        _rpc_port->write(cmd,response);
        Time::delay(1);

        guard.lock();
        _data = *(received);
        _ids = rpc_response;
        guard.unlock();
        cout << "Initial Database - Check" << endl;
        _runit = true;
}

void Thread_read::afterStart(bool s){
    if (s) {
        printf("Reading Thread start\n");
    }
    else
        printf("Reading Thread did not start\n");
}

void Thread_read::run(){
    Bottle *received,rpc_cmd,rpc_response,aux;
    if(_runit) {
        
        rpc_cmd.addVocab(Vocab::encode("ask"));
        aux.addString("all");
        rpc_cmd.addList() = aux;
        received = _port_broad->read(false);
        //cout << received << endl;
        if(received != NULL) {
            _rpc_port->write(rpc_cmd,rpc_response);
            
            guard.lock();
            _data = *(received);
            
            _ids = rpc_response;
            guard.unlock();
            cout << "DATABASE UPDATED" << endl;
        }
    }
}

