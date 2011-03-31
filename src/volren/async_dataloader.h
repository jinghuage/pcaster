#ifndef ASYNC_DATALOADER_H_
#define ASYNC_DATALOADER_H_


#include <stdlib.h>
#include <string.h>

#include <string>
#include <vector>


#include "uniformgrids/4DUniformRequest.h"
#include "ClientCreator.h"
#include "uniformgrids/4DUniformRequest.h"
#include <log4cxx/basicconfigurator.h>



#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>


#include <boost/program_options.hpp>
namespace po = boost::program_options;

#include <iostream>
#include <fstream>

#include <vector>
#include <queue>

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>



#include "dataloader.h"

class AsyncDataLoader : public CDataLoader
{
public:
    AsyncDataLoader(int, int);
    virtual ~AsyncDataLoader();


    //Progressive data loading, override virtual function from CDataLoader
    void request_data();
    bool update_data();

    void* get_data()
    {
        return m_data_chunks[m_cid]->buffer;
    }


private:

    ClientCreator* m_myCreator;
    boost::shared_ptr<RemoteRequester> m_remote_requester;
    HDF5AsyncFileRequester *m_local_requester;


private: 
    std::vector<BlockRef> m_data_chunks;
    bool m_chunk_ready;

    void init_requests(int);
    DataSpace set_chunk_dataspace(int cid);

    //std::queue<int> m_chunk_request_queue;
    boost::mutex m_mutex;
    std::queue<int> m_chunk_ready_queue;
};


#endif
