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
    return 0.0;
}
TranslatorModule::switchCase TranslatorModule::hashtable(string command){
    if(command=="name")  return name;
    if(command=="is_hand")  return is_h;

    if(command=="pos2d") return pos2d;
    if(command=="desc2d")  return desc;
    if(command=="tooldesc2d")  return tooldesc2d;
    if(command=="in_hand")  return in_h;
    if(command=="on_top_of")  return on_t;
    if(command=="reachable_with")  return re_w;
    if(command=="pullable_with")  return pull_w;
    if(command=="pushable_with")  return push_w;

    if(command=="is_free")  return free;
    return notfound;
}
/*IDL Functions*/
bool TranslatorModule::quit() {
    cout << "Received Quit command in RPC" << endl;
    return true;
}
Bottle TranslatorModule::loadObjects(){
    Bottle *receive,dataBase,ids2,*idsp;
    Bottle response;
    cout << "Received loadObjects command in RPC" << endl;
    readingThread->guard.lock();
    dataBase = readingThread->_data;
    ids2 = readingThread->_ids;
    readingThread->guard.unlock();
    if(dataBase.size()>0 && (dataBase.get(1).asString()!="empty")) {
        idsp = ids2.get(1).asList();
        idsp = idsp->get(1).asList();
        for(int i=1;i<dataBase.size();i++){ // for each object
            Bottle *objecto = dataBase.get(i).asList();
            for(int j=0;j<objecto->size();j++) { // for each properties
                Bottle *propriedade = objecto->get(j).asList();
                if(propriedade->get(0).asString() == "name"){
                    Bottle aux;
                    aux.addInt(idsp->get((i-1)).asInt());
                    aux.addString(propriedade->get(1).asString());
                    response.addList() = aux;
                }
            }
        }
    }
    return response;
}
bool TranslatorModule::update(){

    Bottle *receive,dataBase,ids2,*idsp;
    cout << "Received update command in RPC" << endl;
    cout << "Writing the world state to file..." << endl;
    readingThread->guard.lock();
    dataBase = readingThread->_data;
    ids2 = readingThread->_ids;
    readingThread->guard.unlock();
    //cout << "dataBase = " << dataBase.toString().c_str() << endl;

    if(dataBase.size()>0 && (dataBase.get(1).asString()!="empty")) {
        stateFile.open ( stateFileName.c_str());

        idsp = ids2.get(1).asList();
        idsp = idsp->get(1).asList();
        for(int i=1;i<dataBase.size();i++){ // for each object
            Bottle *objecto = dataBase.get(i).asList();
            for(int j=0;j<objecto->size();j++) { // for each properties
                Bottle *propriedade = objecto->get(j).asList();
                switchCase r = hashtable(propriedade->get(0).asString());
                switch(r) {
                    case name:{
                        break;
                    }
                    case desc: {
                        break;

                    }
                    case tooldesc2d: {
                        break;
                    }
                    case pos2d: {
                        break;
                    }
                    case on_t: {
                        Bottle *ontop = propriedade->get(1).asList();
                        cout << "Obj_"<< idsp->get((i-1)).asInt()  << ": On_top Bottle: " << ontop->toString().c_str() << endl << "size" << ontop->size() << endl;

                        for(int k=0; k < ontop->size(); k++){
                            stateFile << idsp->get((i-1)).asInt() <<"_on_" <<ontop->get(k).asInt() <<"() ";
                        }
                        break;

                    }
                    case re_w: {
                        Bottle *reachable = propriedade->get(1).asList();
                        for(int k=0; k < reachable->size(); k++){
                            stateFile << idsp->get((i-1)).asInt() <<"_isreachable_with_" <<reachable->get(k).asInt() <<"() ";
                        }
                        break;
                    }
                    case pull_w: {
                        Bottle *pullable = propriedade->get(1).asList();
                        for(int k=0; k < pullable->size(); k++){
                            stateFile << idsp->get((i-1)).asInt() <<"_ispullable_with_" <<pullable->get(k).asInt() <<"() ";
                        }
                        break;
                    }
                    case push_w: {
                        Bottle *pushable = propriedade->get(1).asList();
                        for(int k=0; k < pushable->size(); k++){
                            stateFile << idsp->get((i-1)).asInt() <<"_ispushable_with_" <<pushable->get(k).asInt() <<"() ";
                        }
                        break;
                    }

                    case is_h: {
                        if(propriedade->get(1).asString() == "true") {
                            stateFile << idsp->get((i-1)).asInt() <<"_ishand" <<"() ";
                        }
                        break;
                    }

                    case free: {
                        if(propriedade->get(1).asString() == "true") {
                            stateFile << idsp->get((i-1)).asInt() <<"_clearhand" <<"() ";
                        }
                        break;
                    }
                    case in_h: {
                        if(propriedade->get(1).asString() == "right") {
                            stateFile << idsp->get((i-1)).asInt() <<"_inhand_" << "12" << "() "; //according to dbhands.ini - id 12 corresponds to the right hand
                        }
                        if(propriedade->get(1).asString() == "left") {
                            stateFile << idsp->get((i-1)).asInt() <<"_inhand_" << "11" << "() "; //according to dbhands.ini - id 12 corresponds to the right hand
                        }
                        break;
                    }

                    default: {
                        cout << "warning: not known attribute " << propriedade->get(0).asString().c_str() << endl;

                        break;
                    }
                } // end switch
            } // end for property
        } // end for object

        stateFile.close();
        return true;
    } // end if
    cout << "database is empty!"<< endl;
    return false; // return fail if the database is empty
}

Bottle TranslatorModule::query2d(const int32_t ObjectID_r){
    Bottle *receive,dataBase,ids2,*idsp;
    Bottle send;
    int ObjectID = ObjectID_r;
    cout << "Received query2d command in RPC" << endl;

    cout << "querydesc2d " << ObjectID_r << " received" << endl;
    readingThread->guard.lock();
    dataBase = readingThread->_data;
    ids2 = readingThread->_ids;
    readingThread->guard.unlock();
    idsp = ids2.get(1).asList();
    idsp = idsp->get(1).asList();
    for(int i=0;i<idsp->size();i++) {
        if(idsp->get(i).asInt() == ObjectID_r) {
            ObjectID = i;
            break;
        }
    }
    Bottle *objecto = dataBase.get(ObjectID+1).asList();

    if (objecto!=NULL)
    {

        //cout << ObjectID << endl;
        for(int j=0;j<objecto->size();j++) { // for each properties
            Bottle *propriedade = objecto->get(j).asList();
            switchCase r = hashtable(propriedade->get(0).asString());
            //cout << propriedade->get(0).asString().c_str() << endl;
            // TODO: replace switch/case with if(propriedade->get(0).asString()=="desc2d")
            switch(r) {
                case desc: {
                    send.clear();
                    send.addList()=*propriedade->get(1).asList();
//                        cout << send.toString().c_str() << endl;
                    //translatorPort.write();
                    cout << "DONE querydesc2d" << endl;
                    return send;
                }
            }
        }

    }
    send.clear();
    return send;
}
Bottle TranslatorModule::querytool2d(const int32_t ObjectID_r){
    Bottle *receive,dataBase,ids2,*idsp;
    Bottle send;
    int ObjectID = ObjectID_r;
    cout << "Received querytool2d command in RPC" << endl;

    cout << "querytooldesc2d " << ObjectID_r << " received" << endl;
    readingThread->guard.lock();
    dataBase = readingThread->_data;
    ids2 = readingThread->_ids;
    readingThread->guard.unlock();
    idsp = ids2.get(1).asList();
    idsp = idsp->get(1).asList();
    for(int i=0;i<idsp->size();i++) {
        if(idsp->get(i).asInt() == ObjectID_r) {
            ObjectID = i;
            break;
        }
    }
    Bottle *objecto = dataBase.get(ObjectID+1).asList();

    if (objecto!=NULL)
    {

        //cout << ObjectID << endl;
        for(int j=0;j<objecto->size();j++) { // for each properties
            Bottle *propriedade = objecto->get(j).asList();
            switchCase r = hashtable(propriedade->get(0).asString());
            //cout << propriedade->get(0).asString().c_str() << endl;
            // TODO: replace switch/case with if(propriedade->get(0).asString()=="tooldesc2d")
            switch(r) {
                case tooldesc2d: {
                    send.clear();
                    send.addList()=*propriedade->get(1).asList();
                    //translatorPort.write();
                    cout << "DONE querytooldesc2d" << endl;
                    return send;
                }
            }
        }

    }
    send.clear();
    return send;
}
bool TranslatorModule::attach(RpcServer &source)
{
    return this->yarp().attachAsServer(source);
}
// End IDL functions


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
    handlerPortName = "/" + moduleName + "/rpc:i";
    handlerPort.open(handlerPortName.c_str());
    attach(handlerPort);
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
        cout << "Context FOUND! Full path of state file:" << endl;
        stateFileName = path+stateName;
        cout << stateFileName<< endl;
    }

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
        cmd.addFloat64(0.1);
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
