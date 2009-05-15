//pbo_mpi_test.cpp
//ask each mpi process to render one block,
//stage1: boundingbox will be sufficient
//readback and loading in using pbos (can have a queue)
//use big volume and subimage paging. Tex coords determine drawing

#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include "data_loader.h"





#define max(a, b) (a)>(b)?(a):(b)


data_loader::data_loader()
{
    m_vol = NULL;
}

data_loader::~data_loader()
{

#ifdef __REMOTE_HDF5__
    delete myCreator;
#endif

#ifdef __LOCAL_HDF5__
    delete m_local_requester;
#endif

}


void data_loader::init_from_rawvol(int grp, int rank)
{

    assert( rank==grp-1 );

    m_cks_grp = new admbase_chunk_grp;
    m_cks_grp->g_pos[0] = 0.0;
    m_cks_grp->g_pos[1] = 0.0;
    m_cks_grp->g_pos[2] = 0.0;
    m_cks_grp->g_scale[0] = 1.0;
    m_cks_grp->g_scale[1] = 1.0;
    m_cks_grp->g_scale[2] = 1.0;
    m_cks_grp->tos[0] = 0.0;
    m_cks_grp->tos[1] = 1.0;
    m_cks_grp->tos[2] = 0.0;
    m_cks_grp->tos[3] = 1.0;
    m_cks_grp->tos[4] = 0.0;
    m_cks_grp->tos[5] = 1.0;

    //evenly distribute chunks to nodes
    m_cks_grp->numofchunks_grp = 1;
    printf("num of chunks for this node: %d\n", m_cks_grp->numofchunks_grp);
    m_cks = new admbase_chunk* [m_cks_grp->numofchunks_grp];

    m_cks_grp->dims[0] = 256;
    m_cks_grp->dims[1] = 256;
    m_cks_grp->dims[2] = 256;
    m_cks_grp->origin[0] = 0;
    m_cks_grp->origin[1] = 0;
    m_cks_grp->origin[2] = 0;

    m_cks[0] = new admbase_chunk;
    m_cks[0]->dims[0] = 256;
    m_cks[0]->dims[1] = 256;
    m_cks[0]->dims[2] = 256;
    m_cks[0]->origin[0] = 0;
    m_cks[0]->origin[1] = 0;
    m_cks[0]->origin[2] = 0;


    printf("int volume model\n");
    if(m_vol == NULL) m_vol = init_volmodel();
    m_vol->w = 256;
    m_vol->h = 256;
    m_vol->d = 256;

    load_Volume_Data("floats256x256x256.raw", m_vol);
    //m_vol->wholedata = malloc(256*256*256*4);
    //memset(m_vol->wholedata, 128, 256*256*256*4);
    m_cks[0]->data = m_vol->wholedata;

}


void data_loader::init_from_h5vol(int grp, int rank, bool attrs,
                                  bool float_volume, float vsize)
{
/*    //global metadata
    if(grp < 8) m_metadata = init_admbase(true);
    else m_metadata = init_admbase(false);

    //based on grp and rank, each node will assign itself some chunks of data
    //and put them together into one volume texture space
    //this combination of native chunks should also form a chunk group. 

    m_cks_grp = new admbase_chunk_grp;
    //evenly distribute chunks to nodes
    m_cks_grp->numofchunks_grp = m_metadata->numofchunks / grp;
    printf("num of chunks for this node: %d\n", m_cks_grp->numofchunks_grp);

    m_cks = new admbase_chunk* [m_cks_grp->numofchunks_grp];
    
//     int s, e, i, fid;
//     s = rank * m_cks_grp->numofchunks_grp;
//     e = s + m_cks_grp->numofchunks_grp -1;
//     for(fid = s; fid <= e; fid++)
//     {
//         i = fid-s;

    int i, j, k, cki, ckj, ckk;
    int _pi, _pj, _pk;
    int fid, pid, ckid = 0;
    int *nck;
    nck = m_metadata->numofck;


    pid = rank;
    if(grp == 1)
    {
        ckk = nck[2];
        ckj = nck[1];
        cki = nck[0];
    }
    else if(grp == 2)
    {
        cki = nck[0]; 
        ckj = nck[1]; 
        ckk = nck[2]/2; 
    }
    else if(grp == 8)
    {
        ckk = nck[2]/2;
        ckj = nck[1]/2;
        cki = nck[0]/2;
    }
    else
    {
        printf("data_loader: now only support mpi runsize 1, 2 or 8\n");
        exit(1);
    }
    int pi = pid/4;
    int pj = (pid%4) / 2;
    int pk = (pid%4) % 2;
    for(k=0; k<nck[2]; k++)
    {
        _pk = k/ckk;
        for(j=0; j<nck[1]; j++)
        {
            _pj = j/ckj;
            for(i=0; i<nck[0]; i++)
            {
                _pi = i/cki;

                fid = k*nck[0]*nck[1] + j*nck[0] + i;

                if( (pi == _pi) && (pj == _pj) && (pk == _pk) )
                {
                    printf("file %d go to process %d\n", fid, pid);


                    m_cks[ckid] = new admbase_chunk;
                    read_h5file(fid, m_cks[ckid], float_volume);

                    //compute_global_attrs(overlap, i, vsize);

                    printf("m_cks[%d]: dims(%d,%d,%d), origin(%d,%d,%d)\n\n", 
                           i, m_cks[ckid]->dims[0], m_cks[ckid]->dims[1], m_cks[ckid]->dims[2],
                           m_cks[ckid]->chunk_origin[0], 
                           m_cks[ckid]->chunk_origin[1],
                           m_cks[ckid]->chunk_origin[2]);

                    ckid ++;
                }            
            }
        }
    }

    assert(ckid == m_cks_grp->numofchunks_grp);

//grp
if(attrs) compute_grp_attrs(vsize);*/
}


//-----------------------------caution!!---------------------------------------
//this is only correct when the whole volume is in texture and draw single chunk
//don't use it when draw chunk_grp
//-----------------------------------------------------------------------------
void data_loader::compute_global_attrs(int n, float volume_size)
{
    int cdim, maxborder;
    float scale, left, right, center;
    admbase_chunk* md = m_cks[n];
    admbase_metadata* m = m_metadata;


    for(int i=0; i<3; i++)
    {
        cdim = m->dims[i] / m->numofck[i];
        maxborder = md->dims[i] - cdim;
        md->cid[i] = (md->origin[i] + maxborder) / cdim;

//         if(overlap_by_bb)
//         {
//             scale = 1.0 * md->dims[i] / m->dims[i];
//             left = 1.0 * md->chunk_origin[i] / m->dims[i];
//         }
//         else
        {
            scale  = 1.0  / m->numofck[i];
            left = 1.0 * md->cid[i] / m->numofck[i];
        }

        right = left + scale;
        center = (left + right)/2.0;

        md->tos[2*i+0] = left;
        md->tos[2*i+1] = right;

        md->g_scale[i] = scale;
        md->g_pos[i] = volume_size * (center - 0.5);
    }

    //printf("--------------------------------------------------\n");
   //  printf("chunk location+scale in global coord: (%f, %f, %f, %f, %f, %f)\n", 
//            md->g_pos[0],md->g_pos[1],md->g_pos[2],
//            md->g_scale[0],md->g_scale[1],md->g_scale[2]);
    //printf("--------------------------------------------------\n");
    
}


void data_loader::compute_grp_attrs(float volume_size)
{
    float scale, left, right, center;
    admbase_metadata* m = m_metadata;

    int l, r, gmin[3], gmax[3];
    for(int i=0; i<3; i++)
    {
        gmin[i] = m_cks[0]->origin[i];
        gmax[i] = gmin[i] + m_cks[0]->dims[i];
    }

    for(int k=1; k<m_cks_grp->numofchunks_grp; k++)
    {       
        for(int i=0; i<3; i++)
        {
            l = m_cks[k]->origin[i];
            r = l + m_cks[k]->dims[i];
            if(l < gmin[i]) gmin[i] = l;
            if(r > gmax[i]) gmax[i] = r;
        }
    }

    int n, cdim, cid;
    int bl, br;
    for(int i=0; i<3; i++) 
    {
        m_cks_grp->origin[i] = gmin[i];
        m_cks_grp->dims[i] = gmax[i] - gmin[i];

        n = (int)( 1.0 * m->dims[i] / m_cks_grp->dims[i] + 0.5 );
        cdim = m->dims[i] / n;
        cid = (int)( 1.0 * m_cks_grp->origin[i] / cdim + 0.5);

//         if(overlap_by_bb)
//         {
//             scale = 1.0 * m_cks_grp->dims[i] / m->dims[i];
//             left = 1.0 * m_cks_grp->origin[i] / m->dims[i];
//         }
//         else
        {
            scale  = 1.0  / n;
            left =  cid * scale;
        }

        right = left + scale;
        center = (left + right)/2.0;

        bl = cid * cdim - m_cks_grp->origin[i];
        br = cdim + bl;
        m_cks_grp->tos[2*i+0] = 1.0 * bl/m_cks_grp->dims[i];
        m_cks_grp->tos[2*i+1] = 1.0 * br/m_cks_grp->dims[i];
        
        m_cks_grp->g_scale[i] = scale;
        m_cks_grp->g_pos[i] = volume_size * (center - 0.5);
        m_cks_grp->g_boundary[i] = left;
        m_cks_grp->g_boundary[i+3] = right;
    }

    printf("--------------------------------------------------\n");

    printf("m_cks_grp: dims(%d,%d,%d), origin(%d,%d,%d)\n", 
           m_cks_grp->dims[0], m_cks_grp->dims[1], m_cks_grp->dims[2],
           m_cks_grp->origin[0], 
           m_cks_grp->origin[1],
           m_cks_grp->origin[2]);
    printf("chunkgrp location+scale in global coord: (%f, %f, %f, %f, %f, %f)\n", 
           m_cks_grp->g_pos[0],m_cks_grp->g_pos[1],m_cks_grp->g_pos[2],
           m_cks_grp->g_scale[0],m_cks_grp->g_scale[1],m_cks_grp->g_scale[2]);
    printf("chunkgrp tos: (%f, %f, %f, %f, %f, %f)\n", 
           m_cks_grp->tos[0],m_cks_grp->tos[1],m_cks_grp->tos[2],
           m_cks_grp->tos[3],m_cks_grp->tos[4],m_cks_grp->tos[5]);

    printf("--------------------------------------------------\n");
    
}



#if defined(__LOCAL_HDF5__) || defined(__REMOTE_HDF5__)


DataSpace data_loader::compute_local_selection(int timestep)
{
    

    DataSpace selection(m_dims);
    
    int border = 2;
    
    switch( m_rank)
    {
    case 0:        
        selection.setSelection(0, 0, m_dims[0]/2 + border, 1);
        selection.setSelection(1, 0, m_dims[1]/2 + border, 1);
        selection.setSelection(2, 0, m_dims[2]/2 + border, 1);
        break;
    case 1:        
        selection.setSelection(0, m_dims[0]/2-border, m_dims[0]/2 + border, 1);
        selection.setSelection(1, 0, m_dims[1]/2 + border, 1);
        selection.setSelection(2, 0, m_dims[2]/2 + border, 1);
        break;
    case 2:        
        selection.setSelection(0, 0, m_dims[0]/2 + border, 1);
        selection.setSelection(1, m_dims[1]/2-border, m_dims[1]/2 + border, 1);
        selection.setSelection(2, 0, m_dims[2]/2 + border, 1);
        break;
    case 3:        
        selection.setSelection(0, m_dims[0]/2-border, m_dims[0]/2 + border, 1);
        selection.setSelection(1, m_dims[1]/2-border, m_dims[1]/2 + border, 1);
        selection.setSelection(2, 0, m_dims[2]/2 + border, 1);
        break;
    case 4:        
        selection.setSelection(0, 0, m_dims[0]/2 + border, 1);
        selection.setSelection(1, 0, m_dims[1]/2 + border, 1);
        selection.setSelection(2, m_dims[2]/2-border, m_dims[2]/2 + border, 1);
        break;
    case 5:        
        selection.setSelection(0, m_dims[0]/2-border, m_dims[0]/2 + border, 1);
        selection.setSelection(1, 0, m_dims[1]/2 + border, 1);
        selection.setSelection(2, m_dims[2]/2-border, m_dims[2]/2 + border, 1);
        break;
    case 6:        
        selection.setSelection(0, 0, m_dims[0]/2 + border, 1);
        selection.setSelection(1, m_dims[1]/2-border, m_dims[1]/2 + border, 1);
        selection.setSelection(2, m_dims[2]/2-border, m_dims[2]/2 + border, 1);
        break;
    case 7:        
        selection.setSelection(0, m_dims[0]/2-border, m_dims[0]/2 + border, 1);
        selection.setSelection(1, m_dims[1]/2-border, m_dims[1]/2 + border, 1);
        selection.setSelection(2, m_dims[2]/2-border, m_dims[2]/2 + border, 1);
        break;

    }
    selection.setSelection(3, timestep, 1, 1);
    return selection;
}




void data_loader::init_requests(int rank)
{

#if defined( __LOCAL_HDF5__) || defined(__REMOTE_HDF5__)
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
	("filename,f", po::value<std::string>(&filename)->default_value("cached:/data/psi0re.h5"), "name of file to open remotely (prefix \"cached:\" to use the cache, )")
        ("local_filename", po::value<std::string>(&local_filename)->default_value("somedefaultfile.h5"), "name of file to open locally")
	//multitoken bug fixed in 1.39.0 ...
	//("dims", po::value< std::vector<int> >(&dims)->multitoken(), "Dimensions of overall space - superflous I know, will fix later")
	//("start-coords", po::value< std::vector<int> >(&start)->multitoken(), "Size of selection")
	//("size", po::value< std::vector<int> >(&size)->multitoken(), "Start coordinates for selection")
	//("strides", po::value< std::vector<int> >(&strides)->multitoken(), "Strides for selection")
	("dims", po::value< std::vector<int> >(&m_dims), "Dimensions of overall space - superflous I know, will fix later")
	("debug_level", po::value<int>(&debug_level)->default_value(0), "Debug level (0 - lowest = NONE, 1 = FATAL, 2 = ERROR, 3 = WARN, 4 = INFO, 5 = DEBUG, 6 = TRACE, 7 -highest = ALL");


	po::variables_map vm;
        std::string configfile("blackhole_scale.cfg");
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

#endif

#ifdef __REMOTE_HDF5__
        myCreator = new ClientCreator(transport.c_str(), hostname, port,
			server_active , (data_protocol.size() == 0 ? NULL : data_protocol.c_str()), data_port, 
			(data_host.size() == 0 ? NULL : data_host.c_str()), data_mtu, data_cc, data_dataRate,
			client_udtrecvbuf, client_udprecvbuf, client_udtsendbuf, client_udpsendbuf,
			client_tcprecvbuf, client_tcpsendbuf,
			server_udtrecvbuf, server_udprecvbuf, server_udtsendbuf, server_udpsendbuf,
			server_tcprecvbuf, server_tcpsendbuf);
	m_remote_requester = myCreator->getRequester();
#endif


#ifdef __LOCAL_HDF5__
        m_local_requester = new HDF5FileRequester(local_filename);
#endif

        //m_dims = dims;
        m_rank = rank;
	m_remote_filename = filename;
}


void data_loader::request_data(int timestep)
{
    
#ifdef __REMOTE_HDF5__
    m_data = remote_data_request(timestep);
#endif

#ifdef __LOCAL_HDF5__

    m_data = local_data_request(timestep);
#endif

}


void data_loader::compute_selection_attributes(float volume_size)
{
    m_cks_grp = new admbase_chunk_grp;

    int n, cdim, cid;
    int bl, br;
    float scale, left, right, center;
    for(int i=0; i<3; i++) 
    {

        n = (int)( 1.0 * m_dims[i] / m_selection.selectionSize(i) + 0.5 );
        cdim = m_dims[i] / n;
        cid = (int)( 1.0 * m_selection.selectionStart(i) / cdim + 0.5);

//         if(overlap_by_bb)
//         {
//             scale = 1.0 * m_cks_grp->dims[i] / m->dims[i];
//             left = 1.0 * m_cks_grp->origin[i] / m->dims[i];
//         }
//         else
        {
            scale  = 1.0  / n;
            left =  cid * scale;
        }

        right = left + scale;
        center = (left + right)/2.0;

        bl = cid * cdim - m_selection.selectionStart(i);
        br = cdim + bl;
        m_cks_grp->tos[2*i+0] = 1.0 * bl/m_selection.selectionSize(i);
        m_cks_grp->tos[2*i+1] = 1.0 * br/m_selection.selectionSize(i);
        
        m_cks_grp->g_scale[i] = scale;
        m_cks_grp->g_pos[i] = volume_size * (center - 0.5);
        m_cks_grp->g_boundary[i] = left;
        m_cks_grp->g_boundary[i+3] = right;
    }
}

#endif

#ifdef __LOCAL_HDF5__


boost::shared_ptr<Block> data_loader::local_data_request( int timestep)
{

    m_selection = compute_local_selection(timestep);
    boost::shared_ptr<Block> data(new Block(m_selection.selectionElemCount()));

    m_local_requester->Read(m_selection, (unsigned char*) data->buffer);

    return data;

}
#endif

#ifdef __REMOTE_HDF5__

static boost::mutex mutex;
static boost::condition condition;
static int done = 0;

void testCB (AsyncResultRef res) {
    boost::mutex::scoped_lock lock(mutex);
    //DataRequestIfaceRef op = boost::dynamic_pointer_cast<DataRequestIface>(res);
    //assert(op.get());
    //op->endRead();
    done++;
    condition.notify_all();
}

boost::shared_ptr<Block> data_loader::remote_data_request(int timestep)
{
    m_selection = compute_local_selection(timestep);
    boost::shared_ptr<Block> data(new Block(m_selection.selectionElemCount()));

    boost::shared_ptr<UniformRequest> myReadReq(new UniformRequest(m_selection, data, m_remote_filename, testCB));

    {
        //boost::shared_ptr<UniformRequest> myReadReq(new UniformRequest(ds, filename, testCB));

        m_remote_requester->beginRead(myReadReq);
    }
    {
        boost::mutex::scoped_lock lock(mutex);
        while (!done)
            condition.wait(lock);
    }

    m_remote_requester->endRead(myReadReq);

    return data;

}
#endif


void data_loader::init_TF_manual()
{

    m_vol = init_volmodel();

//basically colormap for isosurfaces.
    int n = 12;
    float colormap[12*3] = 
        {0., 0.45, 0.,
         0., 0.35, 0.23,
         0., 0.4, 0.1,
         0., 0.25, 0.12,
         0.38, 0.11, 0.14,
         0.38, 0., 0.16,
         0.38, 0.41, 0.,
         0.38, 0.11, 0.05,
         0.47, 0.17, 0,
         0.47, 0., 0.5,
         0.47, 0.32, 0.,
         0.47, 0., 0.09,
        };
    
    m_vol->TFsize = 256;
    m_vol->TF = new unsigned char[256*4];
    memset(m_vol->TF, 0, 256*4);
    int i, j, r, ipeak;
    int ispan = 255/(n+1);
    int hf = ispan/2;

    for(j=0; j<n; j++)
    {
        ipeak = (j+1) * ispan - 1; //for byte volume normal transferfunc
        
        //for(r=-hf; r<hf+1; r++)
        for(r=-3; r<4; r++)
        //r=0;
        {
            i = ipeak + r;
            m_vol->TF[4*i+0] = (unsigned char)(255 * colormap[3*j+0] * (hf-abs(r)) / ispan);
            m_vol->TF[4*i+1] = (unsigned char)(255 * colormap[3*j+1] * (hf-abs(r)) / ispan);
            m_vol->TF[4*i+2] = (unsigned char)(255 * colormap[3*j+2] * (hf-abs(r)) / ispan);
            m_vol->TF[4*i+3] = 255/n;
//             printf("set TF[%d]:(%d,%d,%d,%d)\n", i,
//                    m_vol->TF[4*i+0], m_vol->TF[4*i+1], m_vol->TF[4*i+2], m_vol->TF[4*i+3]);
        }    
    }
}


static void HSVtoRGB( double* r, double* g, double* b, 
                      double h, double s, double v)
{
    if ( s == 0 )
    {
        *r = v;
        *g = v;
        *b = v;
    }
    else
    {
        double var_h = h * 6;
        double var_i = floor( var_h );
        double var_1 = v * ( 1 - s );
        double var_2 = v * ( 1 - s * ( var_h - var_i ) );
        double var_3 = v * ( 1 - s * ( 1 - ( var_h - var_i ) ) );

        if      ( var_i == 0 ) { *r = v     ; *g = var_3 ; *b = var_1; }
        else if ( var_i == 1 ) { *r = var_2 ; *g = v     ; *b = var_1; }
        else if ( var_i == 2 ) { *r = var_1 ; *g = v     ; *b = var_3; }
        else if ( var_i == 3 ) { *r = var_1 ; *g = var_2 ; *b = v;     }
        else if ( var_i == 4 ) { *r = var_3 ; *g = var_1 ; *b = v;     }
        else                   { *r = v     ; *g = var_1 ; *b = var_2; }

    }
}


void data_loader::init_TF_auto()
{
    printf("init TF auto\n");

    m_vol = init_volmodel();
    m_vol->TFsize = 256;
    m_vol->TF = new unsigned char[256*4];
    memset(m_vol->TF, 0, 256*4);

    double Colorizer = 0.5;
    double r, g, b;

    for(int i=0; i<256; i++)
    {
        double value = 1.0*i/255;
        double  sat = 1-0.3*value*Colorizer;
        if (sat<0) sat=0;
        double  val = sat*sat;

        HSVtoRGB( &r, &g, &b, 10*360*value*Colorizer, sat, val);


        int NumberOfPeaks = 6; //(0,32)
        int Sharpness = 2; //(0, 100)
        //double alpha = fabs(sin(12.0));
        //printf("alpha=%f\n", alpha);
        double alpha = fabs(sin(3.141592*value*NumberOfPeaks));
        alpha = pow(alpha, Sharpness);

        m_vol->TF[4*i+0] = (unsigned char)(128*r);
        m_vol->TF[4*i+1] = (unsigned char)(128*g);
        m_vol->TF[4*i+2] = (unsigned char)(128*b);
        m_vol->TF[4*i+3] = (unsigned char)(45*alpha);
    }

}


