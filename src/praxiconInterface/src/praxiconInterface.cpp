#include <iostream>
#include <list>

#include <yarp/os/Bottle.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Module.h>
#include <yarp/os/Network.h>

#include "include/iCub/soapPraxiconTreePortBindingProxy.h"

using namespace std;
using namespace yarp::os;

class PraxiconInterface:public RFModule
{
    Port handlerPort; //a port to handle messages
    Port speechPort;
    
    string moduleName; 
    string inputSpeechPort;
    string handlerPortName;
    Bottle speech;
    Bottle response;
    string relations[10];

    PraxiconTreePortBinding q;
    ns1__queryPRAXICON praxiconQuery;
    ns1__queryPRAXICONResponse responsePRAXICON;
    ns1__iitResponse* res;
    ns1__iitAction* action;
    ns1__iitObject* object1;
    ns1__iitObject* object2;

public:

    double getPeriod()
    {
        return 0.1; //module periodicity (seconds)
    }

    void queryPraxicon(std::vector<xsd__anyType *> objAround, std::string praxMess, std::vector<xsd__anyType *> objMissing){
        cout << "Praxicon Query: " << praxMess << endl;
        praxiconQuery.query = &praxMess;
        praxiconQuery.noObjects = objMissing;
        praxiconQuery.objectsAround = objAround;
            
        if(q.__ns1__queryPRAXICON(&praxiconQuery, &responsePRAXICON) == SOAP_OK)
        {
            res = responsePRAXICON.return_;
            if (res!=NULL || res->response.size()!=3){
                for (int i = 0; i < (int) res->response.size(); i++)
                {
                    action = res->response.at(i);
                    object1 = action->object1;
                    object2 = action->object2;
                    
                    cout<<"object1: "  << *(object1->name)    <<endl;
                    cout<<"movement: " << *(action->movement) <<endl;
                    cout<<"object2: "  << *(object2->name)    <<endl;
                  
                    relations[i] += *(object1->name);
                    relations[i] += " ";
                    relations[i] += *(action->movement);
                    relations[i] += " ";
                    relations[i] += *(object2->name);
                }
            }
        }
    }

    bool updateModule()
    {
        cout << "Waiting for a message..."<< endl;
        speech.clear();
        response.clear();
        speechPort.read(speech, true);
        
        //if (speech!=NULL)
        if (!speech.isNull()) // gsaponaro
        {
            //faking it
            //--------------------------------------------------
            /*Bottle list1;
            list1.addString("available");
            list1.addString("bun-bottom");
            list1.addString("cheese");
            list1.addString("tomato");
			list1.addString("bun-top");

            Bottle tempQuery;
            tempQuery.addString("query");
            tempQuery.addString("make a sandwich");
            
            Bottle list2;
            list2.addString("missing");
            list2.addString("stirrer");
            list2.addString("plate");

            speech.addList() = list1;
            speech.addList() = tempQuery;
            speech.addList() = list2;/**/
            //----------------------------------------------------

            //DO LIST PROCESSING...

			//fprintf (stdout, "the query is %s \n", speech.toString().c_str());

            //LOOK FOR LIST OF AVAILABLE OBJECTS
            //create soap vectors
            std::vector<xsd__anyType *> objectsAround;
            xsd__anyType known[100];
            cout << endl;
            string knownItems[100]; 
            cout << "List of Available object: " << endl;
            for (int i=1; i < speech.findGroup("available").size(); i++ )
            {
                knownItems[i] = speech.findGroup("available").get(i).toString().c_str();
                known[i].__item = (char *)knownItems[i].c_str();
                cout  << known[i].__item <<endl;
                objectsAround.push_back(&known[i]);
            }

            //LOOK FOR QUERY
            std::string query = speech.findGroup("query").toString().c_str();

            std::string::iterator it = query.begin();
            const char * mess = query.c_str();
            char * pch;
            pch = (char*) memchr (mess, '"', strlen(mess));

            if (pch!=NULL){
                query.erase (query.begin(),query.begin()+7);
                it=query.end()-1;
                query.erase (it);
            }
            //cout << endl;
            //cout << "Query: " << endl;
            //cout << query << endl;
            
            //Fill up unknown lists
            std::string missing = speech.findGroup("missing").toString().c_str();

            //create soap vectors
            std::vector<xsd__anyType *> noObjects;
            xsd__anyType unknown[100];
            string unknownItems[100]; 
            cout << endl;
            cout << "List of unavailable object: " << endl;
            for (int i=1; i < speech.findGroup("missing").size(); i++ )
            {
                unknownItems[i] = speech.findGroup("missing").get(i).toString().c_str();
                unknown[i].__item = (char *)unknownItems[i].c_str();
                cout  << unknown[i].__item <<endl;
                noObjects.push_back(&unknown[i]);
            }
            cout << endl;

            //send query to the praxicon
            queryPraxicon(objectsAround, query, noObjects);

            if (res!=NULL || res->response.size()!=3){
                cout << "response size " << res->response.size() << endl;
                for (int rep = 0; rep< (int)res->response.size(); rep++)
                {
                    response.addString( relations[rep].c_str() );
                    relations[rep].clear();
                }
            }
            speechPort.reply(response);
        }
        return true;
    }

    /*
    * Message handler. Just echo all received messages.
    */
    bool respond(const Bottle& command, Bottle& reply) 
    {
        if (command.get(0).asString()=="quit")
            return false;     
        else
            reply=command;
        return true;
    }

    /* 
    * Configure function. Receive a previously initialized
    * resource finder object. Use it to configure your module.
    * Open port and attach it to message handler.
    */
    bool configure(yarp::os::ResourceFinder &rf)
    {

        /* get the module name which will form the stem of all module port names */
        moduleName            = rf.check("name", 
                           Value("praxInterface"), 
                           "module name (string)").asString();

        /*
        * before continuing, set the module name before getting any other parameters, 
        * specifically the port names which are dependent on the module name
        */

        setName(moduleName.c_str());

        /* get the name of the input and output ports, automatically prefixing the module name by using getName() */

        inputSpeechPort       = "/";
        inputSpeechPort       += getName(
                               rf.check("port", 
                               Value("/speech:i"),
                               "Input speech port (string)").asString()
                               );

        /* do all initialization here */
        /* open ports  */ 
           
        if (!speechPort.open(inputSpeechPort.c_str())) {
            cout << getName() << ": unable to open port " << inputSpeechPort << endl;
            return false;  // unable to open; let RFModule know so that it won't run
        }

       /*
        * attach a port of the same name as the module (prefixed with a /) to the module
        * so that messages received from the port are redirected to the respond method
        */

        handlerPortName =  "/";
        handlerPortName += getName();         // use getName() rather than a literal 
     
        if (!handlerPort.open(handlerPortName.c_str())) {           
            cout << getName() << ": Unable to open port " << handlerPortName << endl;  
            return false;
        }

        attach(handlerPort);  
        action = NULL;
        object1 = NULL;
        object2 = NULL;
        res = NULL;
        return true;
    }

    /*
    * Interrupt function.
    */
    bool interruptModule()
    {
        cout<<"Interrupting your module, for port cleanup"<<endl;
        speechPort.interrupt();
        handlerPort.interrupt();
        return true;
    }

    /*
    * Close function, to perform cleanup.
    */
    bool close()
    {
        cout<<"Calling close function\n";
        speechPort.close();
        handlerPort.close();
        return true;
    }
};

int main(int argc, char * argv[])
{
    Network yarp;

    PraxiconInterface module;
    ResourceFinder rf;
    rf.configure("ICUB_ROOT", argc, argv);
    rf.setVerbose(true);

    cout<<"Configure module..."<<endl;
    module.configure(rf);
    module.runModule();
    cout<<"Main returning..."<<endl;
    return 0;
}
