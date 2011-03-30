
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>



//#if defined(__REMOTE_HDF5__) || defined(__LOCAL_HDF5__)

#include "uniformgrids/4DUniformRequest.h"
#include "ClientCreator.h"
#include "uniformgrids/4DUniformRequest.h"
#include <log4cxx/basicconfigurator.h>



#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <fstream>
#include <string>



#include "async_dataloader.h"
#include "pcaster_options.h"



//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
AsyncDataLoader::AsyncDataLoader(int rank, int runsize):
    CDataLoader(rank, runsize),
    m_myCreator(0),
    m_local_requester(0)
{ 
    init_requests(rank);
}


AsyncDataLoader::~AsyncDataLoader()
{

#ifdef __REMOTE_HDF5__
     std::string hdf5_filename = pcaster_options::DATA_NAME + ".h5";

     boost::shared_ptr<HDF5UniformCloseRequest> myCloseReq(new HDF5UniformCloseRequest(hdf5_filename, NULL));
     m_remote_requester->beginRead(myCloseReq);
     
     try {
         m_remote_requester->endRead(myCloseReq);
     } catch (Exception e) {
         printf("Exception: %d\n", e.what().c_str());
     }
     delete m_myCreator; m_myCreator = 0;
     m_remote_requester.reset();
#endif
     
#ifdef __LOCAL_HDF5__
     delete m_local_requester; m_local_requester = 0;
#endif


}



//-----------------------------------------------------------------------------
//todo: resolve chunk origin inside local brick -- may not work with remote 
//data server
//-----------------------------------------------------------------------------
DataSpace AsyncDataLoader::set_chunk_dataspace(int cid)
{
    DataSpace selection(m_full_dataDims);

    resolve_chunk_origin(cid); //resolve chunk origin inside local brick

    for(int i=0; i<3; i++)
    {
        int d, o;
        d = m_chunkDims[i]; // + padding[2*i] + padding[2*i+1];
        o = m_brickOrigin[i] + m_chunkOrigin[i]; // - padding[2*i];
	//printf("%d: dim %d, origin %d\n",i, d, o);
        selection.setSelection(i, o, d, 1);
    }

    selection.setSelection(3, m_chunkOrigin[3], 1, 1);
    return selection;
}



//-----------------------------------------------------------------------------
//for callback functions, pass by value or address. Pass by reference will not
//also make a copy so will not work the same as pass by address
//-----------------------------------------------------------------------------
void dataCB (AsyncResultRef res, 
             int cid,
             boost::mutex* mutex,
             std::queue<int>* readyqueue) 

{
  static int counter = 0;
    
  DataRequestIfaceRef op = boost::dynamic_pointer_cast<DataRequestIface>(res);
  assert(op.get());
  op->endRead();
    
  {
      boost::mutex::scoped_lock lock(*mutex);
      readyqueue->push(cid);
  } 

#ifdef _DEBUG7
  printf("Callback called ! counter %d\n\n\n", counter++);
#endif

}



//-----------------------------------------------------------------------------
//used in application init, init the data request for progressive update later
// -- virtual function
//for data distribution: DL_REMOTE -- send request to Networked data server
//-----------------------------------------------------------------------------
void AsyncDataLoader::request_data()
{
//    fprintf(stderr, "**** %s:%s() ****\n", __FILE__, __func__);


    assert(m_dataDist == DL_REMOTE);
    assert(m_dataLoadmode == DL_RPC);


    int N = m_chunkNum[0] * m_chunkNum[1] * m_chunkNum[2];
    int size = m_chunkDims[0] * m_chunkDims[1] * m_chunkDims[2] * sizeof(char);


    std::vector<DataSpace> selectionList;

    //allocate data pointer for all N chunks;
    //if data too large may not need to allocate all N, maybe smaller number
    //if(m_data_chunks == 0) m_data_chunks = (void**)malloc(N * sizeof(void*));
    //for(int i=0; i<N; i++) m_data_chunks = malloc(size);
    m_data_chunks.clear();
    m_data_chunks.resize(N);


    std::string hdf5_filename = pcaster_options::DATA_NAME + ".h5";





    for(int i=0; i<N; i++)
    {
        //m_chunk_request_queue.push(i);

        fprintf(stderr, "%d: request No. %d\n", m_rank, i);

        selectionList.push_back(set_chunk_dataspace(i));


        assert(size == selectionList[i].selectionElemCount());
	boost::shared_ptr<Block> data(
            new Block(selectionList[i].selectionElemCount()));
        m_data_chunks[i] = data;

	//dataCB is data retrieve function pointer
        //similar to a thread function pointer? what's the "_1"?
        AsyncResultCallback testCB = boost::bind(&dataCB, 
                                                 _1, 
                                                 i,
                                                 &m_mutex,
                                                 &m_chunk_ready_queue);

	boost::shared_ptr<UniformRequest> myReadReq(
            new UniformRequest(selectionList[i], 
                               data, 
                               hdf5_filename, 
                               testCB));

	//printf("Got here\n");

#ifdef __REMOTE_HDF5__
        m_remote_requester->beginRead(myReadReq);
#else
	m_local_requester->beginRead(myReadReq);
#endif
	//printf("Request sent\n");

    }


    fprintf(stderr, "%d: Sent out %d requests\n", m_rank, N);
     
}



//-----------------------------------------------------------------------------
//used in application frame update (idle) to update new data -- virtual function
//get data and update dataDims|Origin progresively until all data is updated
//for data distribution: DL_REMOTE
//and load mode: DL_RPC. -- connect with remote data server 
//-----------------------------------------------------------------------------
bool AsyncDataLoader::update_data()
{
    static int counter = 0;
    bool retval = false;
    boost::mutex::scoped_lock lock(m_mutex);
    if(!m_chunk_ready_queue.empty())
    {
        retval = true;
        int cid = m_chunk_ready_queue.front();
        m_chunk_ready_queue.pop();

        resolve_chunk_origin(cid);
        resolve_av_attr();
        //m_data = m_data_chunks[cid]->buffer;
        m_cid = cid;
        printf("Node %d procesed block %d\n", m_rank, m_cid);
    }
   

    return retval;

}




//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void AsyncDataLoader::init_requests(int rank)
{

	log4cxx::BasicConfigurator::configure();
	int debug_level;
	int port;
	std::string transport;
	std::string hostname;
	std::string filename;
        std::string local_filename;

	//std::vector<int> dims;

	int server_active; 
	std::string data_protocol;
	int data_port;
	std::string data_host;

	int data_mtu;
	int data_cc;
	int data_dataRate;
	int client_udtrecvbuf;
	int client_udprecvbuf;
	int client_udtsendbuf;
	int client_udpsendbuf;
	int client_tcprecvbuf;
	int client_tcpsendbuf;
	int server_udtrecvbuf;
	int server_udprecvbuf;
	int server_udtsendbuf;
	int server_udpsendbuf;
	int server_tcprecvbuf;
	int server_tcpsendbuf;


	po::options_description myOptions("Program options");

	myOptions.add_options()
	("hostname", po::value<std::string>(&hostname)->default_value("localhost"), "hostname or IP address to use to connect to")
	("port", po::value<int>(&port)->default_value(40888), "Port to connect to")
	("protocol,p", po::value<std::string>(&transport)->default_value("tcp"), "transport protocol for control channel")
	("server_active,a", po::value<int>(&server_active)->default_value(0), "Should server create an active data connection (options - 0:no, 1:yes)")
	("data_protocol", po::value<std::string>(&data_protocol)->default_value(""), "Override data protocol server default (options: tcp, udt , \"\" - no override)")
	("data_port", po::value<int>(&data_port)->default_value(-1), "Override data port server default with value (default = -1 (no override))")
	("data_host", po::value<std::string>(&data_host)->default_value(""), "Override data protocol server default (default - \"\" - no override)")
	("data_mtu", po::value<int>(&data_mtu)->default_value(-1), "MTU for UDT- default value = unchanged")
	("congestion_control,c", po::value<int>(&data_cc)->default_value(-1), "congestion control algorithm for UDT - default value = native, options 1 - TCP; 2 - Blast UDP (rate-based : please set data rate as well); 3 - BIC TCP; 4 - Scalable TCP; 5 - Hi-speed TCP; 6 - TCP Westwood")
	("data_rate,d", po::value<int>(&data_dataRate)->default_value(-1), "default data rate for UDT and rate-based congestion control(option 2) in mbps, default value - UDT default")
	("client_udtrecvbuf", po::value<int>(&client_udtrecvbuf)->default_value(-1), "UDT receive buffer size on this (client) side- default value = unchanged")
	("server_udtrecvbuf", po::value<int>(&server_udtrecvbuf)->default_value(-1), "override UDT receive buffer size on the other (server) side- default value = unchanged")
	("client_udprecvbuf", po::value<int>(&client_udprecvbuf)->default_value(-1), "UDP receive buffer size for UDT on client side - default value = unchanged")
	("server_udprecvbuf", po::value<int>(&server_udprecvbuf)->default_value(-1), "override UDP receive buffer size for UDT on server side- default value = unchanged")
	("client_udtsendbuf", po::value<int>(&client_udtsendbuf)->default_value(-1), "UDT send buffer size client side - default value = unchanged")
	("server_udtsendbuf", po::value<int>(&server_udtsendbuf)->default_value(-1), "override UDT send buffer size server side - default value = unchanged")
	("client_udpsendbuf", po::value<int>(&client_udpsendbuf)->default_value(-1), "UDP send buffer size for UDT client side - default value = unchanged")
	("server_udpsendbuf", po::value<int>(&server_udpsendbuf)->default_value(-1), "override UDP send buffer size for UDT server side - default value = unchanged")
	("client_tcprecvbuf", po::value<int>(&client_tcprecvbuf)->default_value(-1), "TCP receive buffer size client side - default value = unchanged")
	("server_tcprecvbuf", po::value<int>(&server_tcprecvbuf)->default_value(-1), "override TCP receive buffer size server side- default value = unchanged")
	("client_tcpsendbuf", po::value<int>(&client_tcpsendbuf)->default_value(-1), "TCP send buffer size client side - default value = unchanged")
	("server_tcpsendbuf", po::value<int>(&server_tcpsendbuf)->default_value(-1), "override TCP send buffer size server side - default value = unchanged")
	("help,h", "produce help message\nThe command line-parameters have priority over the configuration file")
        //("filename,f", po::value<std::string>(&filename)->default_value("cached:/data/psi0re.h5"), "name of file to open remotely (prefix \"cached:\" to use the cache, )")
        //("local_filename", po::value<std::string>(&local_filename)->default_value("somedefaultfile.h5"), "name of file to open locally")
	//multitoken bug fixed in 1.39.0 ...
	//("dims", po::value< std::vector<int> >(&dims)->multitoken(), "Dimensions of overall space - superflous I know, will fix later")
	//("start-coords", po::value< std::vector<int> >(&start)->multitoken(), "Size of selection")
	//("size", po::value< std::vector<int> >(&size)->multitoken(), "Start coordinates for selection")
	//("strides", po::value< std::vector<int> >(&strides)->multitoken(), "Strides for selection")
	//("dims", po::value< std::vector<int> >(&m_dims), "Dimensions of overall space - superflous I know, will fix later")
	//("number_subblocks", po::value<unsigned int>(&m_nr_blocks), "Number of subblocks that this node will work with (for progressive rendering)")
	("debug_level", po::value<int>(&debug_level)->default_value(0), "Debug level (0 - lowest = NONE, 1 = FATAL, 2 = ERROR, 3 = WARN, 4 = INFO, 5 = DEBUG, 6 = TRACE, 7 -highest = ALL");


	po::variables_map vm;
        std::string configfile("network_options.cfg");
#ifdef __REMOTE_HDF5__
	configfile = configfile + std::string(".") + boost::lexical_cast<std::string>(rank);
#endif
	//po::store(po::parse_command_line(argc, argv, myOptions), vm);
	std::ifstream ifs(configfile.c_str());
	po::store(po::parse_config_file(ifs, myOptions), vm);

	notify(vm);

	
	switch (debug_level) {
	case 0:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
		break;
	case 1:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getFatal());
		break;
	case 2:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getError());
		break;
	case 3:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getWarn());
		break;
	case 4:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getInfo());
		break;
	case 5:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getDebug());
		break;
	case 6:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getTrace());
		break;
	case 7:
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getAll());
		break;
	default:
		printf("Unrecognized debug level, turning off\n");
		log4cxx::Logger::getRootLogger()->setLevel(log4cxx::Level::getOff());
	}
	printf("Initialized debug, level %d\n", debug_level);

        std::string hdf5_filename = pcaster_options::DATA_NAME + ".h5";

#ifdef __REMOTE_HDF5__
        m_myCreator = new ClientCreator(transport.c_str(), hostname, port,
			server_active , (data_protocol.size() == 0 ? NULL : data_protocol.c_str()), data_port, 
			(data_host.size() == 0 ? NULL : data_host.c_str()), data_mtu, data_cc, data_dataRate,
			client_udtrecvbuf, client_udprecvbuf, client_udtsendbuf, client_udpsendbuf,
			client_tcprecvbuf, client_tcpsendbuf,
			server_udtrecvbuf, server_udprecvbuf, server_udtsendbuf, server_udpsendbuf,
			server_tcprecvbuf, server_tcpsendbuf);
	m_remote_requester = m_myCreator->getRequester();

        boost::shared_ptr<HDF5UniformOpenRequest> myOpenReq(new HDF5UniformOpenRequest(hdf5_filename, NULL));
        m_remote_requester->beginRead(myOpenReq);
        
        try {
            m_remote_requester->endRead(myOpenReq);
        } catch (Exception e) {
            printf("Exception: %d\n", e.what().c_str());
        }
#endif


#ifdef __LOCAL_HDF5__

        m_local_requester = new HDF5AsyncFileRequester(hdf5_filename);
#endif


	printf("returning from init_Requests\n");
}




