#include "4DUniformRequest.h"



#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>

log4cxx::LoggerPtr UniformRequest::mLogger(log4cxx::Logger::getLogger("UniformRequest"));
log4cxx::LoggerPtr UniformCacheRequest::mLogger(log4cxx::Logger::getLogger("UniformCacheRequest"));
log4cxx::LoggerPtr HDF5UniformHandleRequest::mLogger(log4cxx::Logger::getLogger("HDF5UniformHandleRequest"));
log4cxx::LoggerPtr HDF5FileRequester::mLogger(log4cxx::Logger::getLogger("HDF5FileRequester"));
log4cxx::LoggerPtr UniformFileCache::mLogger(log4cxx::Logger::getLogger("UniformFileCache"));
log4cxx::LoggerPtr UniformFileCache::UniformCacheEntry::mLogger(log4cxx::Logger::getLogger("UniformFileCache::UniformCacheEntry"));
log4cxx::LoggerPtr HDF5AsyncFileRequester::mLogger(log4cxx::Logger::getLogger("HDF5AsyncFileRequester"));



std::string UniformRequest::rpcMethodName("Uniform.read");
std::string UniformCacheRequest::rpcMethodName("Uniform.cache");

std::string HDF5UniformOpenRequest::rpcMethodName("HDF5Cactus.open");
std::string HDF5UniformCloseRequest::rpcMethodName("HDF5Cactus.close");

using namespace std;

HDF5FileRequester::HDF5FileRequester(std::string filename, std::string object) {
	LOG4CXX_DEBUG(mLogger, "HDF5FileRequester");
	LOG4CXX_DEBUG(mLogger, "filename: " + filename);
	//TODO: open file, throw error if needed
	input_file = H5Fopen(filename.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	if (input_file < 0) {
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, string("Error opening input file ") + filename);
		throw IOException(string("Error opening file"));
		return;
	}
	dataset = H5Dopen(input_file, object.c_str());
	if (dataset < 0) {
		H5Eprint(NULL);
		H5Fclose(input_file);
		LOG4CXX_ERROR(mLogger, string("Error opening dataset ") + object + string(" on file ") + filename);
		throw IOException(string("Error opening dataset"));
		return;
	}
	//hid_t data_type = H5Dget_type(dataset);
	//TODO: replace asserts with errors!
	//assert (data_type == H5T_STD_U8LE);
	hid_t dataspace = H5Dget_space(dataset);
	assert(dataspace >=0);
	hsize_t maxdims[4];
	hsize_t dims[4];
	int dimensions = H5Sget_simple_extent_dims(dataspace, dims, maxdims);
	assert (dimensions == 4);
√	LOG4CXX_DEBUG(mLogger, string("Dims [0] = ") + boost::lexical_cast<string>(dims[0]) +
			string("; Dims [1] = ") + boost::lexical_cast<string>(dims[1]) +
			string("; Dims [2] = ") + boost::lexical_cast<string>(dims[2]) +
			string("; Dims[3] = ") + boost::lexical_cast<string>(dims[3]));
	H5Sclose(dataspace); //making sure of no leaks



}
HDF5FileRequester::~HDF5FileRequester() {
	LOG4CXX_DEBUG(mLogger, "~HDF5FileRequester");
	H5Dclose(dataset);
	H5Fclose(input_file);
}

# if 0
DataSpace HDF5FileRequester::GetDataSpace () {

}


std::vector<float> HDF5FileRequester::GetBoundingBox (const DataSpace* restrictds) {

}

#endif

void HDF5FileRequester::Read(const DataSpace ds, BlockRef& data) {
	LOG4CXX_DEBUG(mLogger, "Read");

	assert(ds.selectionSize().size() == 4);

	size_t my_val = (size_t) ds.selectionElemCount() * sizeof (unsigned char);

	if (data->buffer == NULL && data->size == 0)
		data->alloc(my_val);

	LOG4CXX_DEBUG(mLogger, std::string("start - X - ") + boost::lexical_cast<std::string>(ds.selectionStart(0)) +  std::string(" start - Y - ") + boost::lexical_cast<std::string>(ds.selectionStart(1)) + std::string (" start - Z - ") + boost::lexical_cast<std::string>(ds.selectionStart(2)) + std::string(" start - timestep - ") + boost::lexical_cast<std::string>(ds.selectionStart(3)));
	LOG4CXX_DEBUG(mLogger, std::string("size - X - ") + boost::lexical_cast<std::string>(ds.selectionSize(0)) +  std::string(" size - Y - ") + boost::lexical_cast<std::string>(ds.selectionSize(1)) + std::string (" size - Z - ") + boost::lexical_cast<std::string>(ds.selectionSize(2)) + std::string(" size - timesteps - ") + boost::lexical_cast<std::string>(ds.selectionSize(3)));


	hsize_t     start[4];       hsize_t     start_out[4];
	hsize_t     stride[4];      hsize_t     stride_out[4];
	hsize_t     count[4];       hsize_t     count_out[4];
	hsize_t     block[4];       hsize_t     block_out[4];

	//reversed indices so that we read in fortran order as needed
	//by openGL!
	//TODO: Check to see if correct!!
	//Contiguous in memory
	//selectionStart(3) is in time,
	//selectionStart(2) is on z axis,
	//selectionStart(1) is on y axix
	//selectionStart(0) is on x axix.
	//the same applies to selectionSize and selectionStrides
	//the definition of start, size, strides can be found
	//in HDF5 hyperslab description
	//http://www.hdfgroup.org/HDF5/doc/H5.intro.html#Intro-PMSelectHyper
	//size is equivalent to hyperslab count
	start[0]  = ds.selectionStart(3);  start_out[0] = 0;
	start[1]  = ds.selectionStart(2);  start_out[1] = 0;
	start[2]  = ds.selectionStart(1);  start_out[2] = 0;
	start[3]  = ds.selectionStart(0);  start_out[3] = 0;

	stride[0] = ds.selectionStrides(3); stride_out[0] = 1;
	stride[1] = ds.selectionStrides(2); stride_out[1] = 1;
	stride[2] = ds.selectionStrides(1); stride_out[2] = 1;
	stride[3] = ds.selectionStrides(0); stride_out[3] = 1;

	count[0]  = ds.selectionSize(3);  count_out[0] = count[0];
	count[1]  = ds.selectionSize(2);  count_out[1] = count[1];
	count[2]  = ds.selectionSize(1);  count_out[2] = count[2];
	count[3]  = ds.selectionSize(0);  count_out[3] = count[3];

	block[0]  = 1;  block_out[0] = 1;
	block[1]  = 1;  block_out[1] = 1;
	block[2]  = 1;  block_out[2] = 1;
	block[3]  = 1;  block_out[3] = 1;
	herr_t      status;

	hid_t dataspace   = H5Dget_space (dataset);

	status      = H5Sselect_hyperslab        (dataspace,
			H5S_SELECT_SET,
			start, stride,
			count, block);

	assert (H5Sget_select_npoints      (dataspace) == my_val);

	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "Cannot select data slab");
		throw IOException("Cannot select data slab");
		return;
	}

	hid_t memspace    = H5Screate_simple  (4, count_out, NULL);


	status      = H5Sselect_hyperslab        (memspace,
			H5S_SELECT_SET,
			start_out , stride_out,
			count_out,  block_out);

	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "Cannot select mem slab");
		throw IOException("Cannot select mem slab");
		return;
	}

	assert (H5Sget_select_npoints      (memspace) == my_val);

	hid_t plist       = H5Pcreate     (H5P_DATASET_XFER);
	//is this hint really useful?
	status = H5Pset_buffer  (plist, my_val, NULL, NULL);
	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "Cannot set data buffer");
		throw IOException("Cannot set buffer");
		return;
	}

	//read data into "data" buffer, fortran ordering!
	//endianess doesn't matter for uchar
	status      = H5Dread (dataset,
			H5T_NATIVE_UCHAR, memspace,
			dataspace, plist, data->buffer);
	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "READ ERROR!");
		throw IOException("Read error");
		return;
	}

	H5Pclose (plist);
	H5Sclose (dataspace);
	H5Sclose (memspace);

}

void HDF5FileRequester::ReadFloat(const DataSpace ds, BlockRef& data) {
	LOG4CXX_DEBUG(mLogger, "Read");

	assert(ds.selectionSize().size() == 4);

	size_t my_val = (size_t) ds.selectionElemCount() * sizeof (float);

	if (data->buffer == NULL && data->size == 0)
		data->alloc(my_val);

	LOG4CXX_DEBUG(mLogger, std::string("start - X - ") + boost::lexical_cast<std::string>(ds.selectionStart(0)) +  std::string(" start - Y - ") + boost::lexical_cast<std::string>(ds.selectionStart(1)) + std::string (" start - Z - ") + boost::lexical_cast<std::string>(ds.selectionStart(2)) + std::string(" start - timestep - ") + boost::lexical_cast<std::string>(ds.selectionStart(3)));
	LOG4CXX_DEBUG(mLogger, std::string("size - X - ") + boost::lexical_cast<std::string>(ds.selectionSize(0)) +  std::string(" size - Y - ") + boost::lexical_cast<std::string>(ds.selectionSize(1)) + std::string (" size - Z - ") + boost::lexical_cast<std::string>(ds.selectionSize(2)) + std::string(" size - timesteps - ") + boost::lexical_cast<std::string>(ds.selectionSize(3)));


	hsize_t     start[4];       hsize_t     start_out[4];
	hsize_t     stride[4];      hsize_t     stride_out[4];
	hsize_t     count[4];       hsize_t     count_out[4];
	hsize_t     block[4];       hsize_t     block_out[4];

	start[0]  = ds.selectionStart(3);  start_out[0] = 0;
	start[1]  = ds.selectionStart(2);  start_out[1] = 0;
	start[2]  = ds.selectionStart(1);  start_out[2] = 0;
	start[3]  = ds.selectionStart(0);  start_out[3] = 0;

	stride[0] = ds.selectionStrides(3); stride_out[0] = 1;
	stride[1] = ds.selectionStrides(2); stride_out[1] = 1;
	stride[2] = ds.selectionStrides(1); stride_out[2] = 1;
	stride[3] = ds.selectionStrides(0); stride_out[3] = 1;

	count[0]  = ds.selectionSize(3);  count_out[0] = count[0];
	count[1]  = ds.selectionSize(2);  count_out[1] = count[1];
	count[2]  = ds.selectionSize(1);  count_out[2] = count[2];
	count[3]  = ds.selectionSize(0);  count_out[3] = count[3];

	block[0]  = 1;  block_out[0] = 1;
	block[1]  = 1;  block_out[1] = 1;
	block[2]  = 1;  block_out[2] = 1;
	block[3]  = 1;  block_out[3] = 1;
	herr_t      status;

	hid_t dataspace   = H5Dget_space (dataset);

	status      = H5Sselect_hyperslab        (dataspace,
			H5S_SELECT_SET,
			start, stride,
			count, block);

	assert (H5Sget_select_npoints      (dataspace) * sizeof(float) == my_val);

	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "Cannot select data slab");
		throw IOException("Cannot select data slab");
		return;
	}

	hid_t memspace    = H5Screate_simple  (4, count_out, NULL);


	status      = H5Sselect_hyperslab        (memspace,
			H5S_SELECT_SET,
			start_out , stride_out,
			count_out,  block_out);

	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "Cannot select mem slab");
		throw IOException("Cannot select mem slab");
		return;
	}

	assert (H5Sget_select_npoints      (memspace) * sizeof(float)== my_val);

	hid_t plist       = H5Pcreate     (H5P_DATASET_XFER);
	//is this hint really useful?
	status = H5Pset_buffer  (plist, my_val, NULL, NULL);
	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "Cannot set data buffer");
		throw IOException("Cannot set buffer");
		return;
	}

	//read data into "data" buffer, fortran ordering!
	status      = H5Dread (dataset,
			H5T_NATIVE_FLOAT, memspace,
			dataspace, plist, data->buffer);
	if ( status < 0 )
	{
		H5Eprint(NULL);
		LOG4CXX_ERROR(mLogger, "READ ERROR!");
		throw IOException("Read error");
		return;
	}

	H5Pclose (plist);
	H5Sclose (dataspace);
	H5Sclose (memspace);

}

HDF5AsyncFileRequester::HDF5AsyncFileRequester(std::string filename) :
	mFile(filename) {
	LOG4CXX_DEBUG(mLogger, "HDF5AsyncFileRequester");
	mRequestManager = new RequestManager(boost::bind(&HDF5AsyncFileRequester::ReadData, &mFile, _1));
}

HDF5AsyncFileRequester::~HDF5AsyncFileRequester() {
	LOG4CXX_DEBUG(mLogger, "~HDF5AsyncFileRequester");
	delete mRequestManager;
}

AsyncResultRef HDF5AsyncFileRequester::beginRead (boost::shared_ptr<DataRequestIface> opr) {
	//boost::shared_ptr<SimpleFileRequest> op = boost::dynamic_pointer_cast<SimpleFileRequest>(opr);
	//assert(op.get()); //this is not a file request
	return mRequestManager->beginOp (opr);
}

BlockRef HDF5AsyncFileRequester::endRead (AsyncResultRef handle) {
	return mRequestManager->endOp (boost::dynamic_pointer_cast<DataOperation>(handle));
}

void HDF5AsyncFileRequester::ReadData(HDF5FileRequester* file, OperationRef opr) {
	boost::shared_ptr<UniformRequest> op = boost::dynamic_pointer_cast<UniformRequest>(opr);

	assert(op.get());

	LOG4CXX_DEBUG(mLogger, std::string("ReadData"));
	try {
		file->Read(op->mSpace, op->data);
	} catch (IOException e) {
		op->SetException(e);
	}
}



void UniformFileCache::PreCache(const DataSpace ds) {

	//search if this is already cached return
	for (unsigned int i = 0; i < mCache.size(); i++) {
		if (mCache[i].space.includesSelection(ds)) {
			LOG4CXX_DEBUG(mLogger,  std::string("Request for cache ignored, this is already in cache"));
			return;
		}
	}

	size_t data_size = (size_t) ds.selectionElemCount() * sizeof (unsigned char);

	boost::shared_ptr<unsigned char> data ((unsigned char*) malloc(data_size));

	assert(data.get());

	BlockRef myBlock(new Block(data.get(), data_size, 0));
	//read data from the actual file, using parent class
	HDF5FileRequester::Read(ds, myBlock);


	UniformCacheEntry newEntry(ds, data);
	mCache.push_back(newEntry);

	//TODO: could check if the new entry includes some
	//of the old ones .. who then should be deleted
}

void UniformFileCache::ReadCache(const DataSpace ds, BlockRef& data){
	//search if this is cached


	assert(ds.selectionSize().size() == 4);
	assert(data->buffer == NULL);
	assert(data->size == 0);
	size_t data_size = (size_t) ds.selectionElemCount() * sizeof (unsigned char);


	for (unsigned int i = 0; i < mCache.size(); i++) {
		if (mCache[i].space.includesSelection(ds)) {
			LOG4CXX_DEBUG(mLogger,  std::string("This is in cache!"));

			if (mCache[i].space.isIdentical(ds)) {
				//TODO: maybe this memcopy is not needed!
				LOG4CXX_DEBUG(mLogger, string("Found identical cache block, copy"));
				data = BlockRef(new Block(mCache[i].data.get(), data_size, 0));
				//memcpy(data->buffer, mCache[i].data.get(), data_size);
			} else {
				LOG4CXX_DEBUG(mLogger, string("Found larger cache block, selecting and copying relevant data"));

				//TODO: could boost::multiarray help?
				std::vector<int> startInMystrides(ds.nDims());
				std::vector<int> stridesInMystrides(ds.nDims());
				std::vector<int> offsets(ds.nDims());
				std::vector<int> selection_offsets(ds.nDims());

				boost::int64_t mult = 1;
				//correct, because this selection is contiguous
				for (unsigned int k = 0; k < offsets.size(); k++) {
					offsets[k] = mult;
					LOG4CXX_DEBUG(mLogger, string("offsets[") + boost::lexical_cast<string>(k) + string("]=") + boost::lexical_cast<string>(mult));
					mult *= mCache[i].space.selectionSize(k);
				}



				for (int k = 0; k < ds.nDims(); k++) {
					//const int startOffset = ds.selectionStart(i) - mCache[i].space.selectionStart(i);
					startInMystrides[k] = ds.selectionStart(k) / mCache[i].space.selectionStrides(k);
					LOG4CXX_DEBUG(mLogger, string("startInMystrides[") + boost::lexical_cast<string>(k) + string("]=") + boost::lexical_cast<string>(startInMystrides[k]));
					stridesInMystrides[k] = ds.selectionStrides(k) / mCache[i].space.selectionStrides(k);
					LOG4CXX_DEBUG(mLogger, string("stridesInMystrides[") + boost::lexical_cast<string>(k) + string("]=") + boost::lexical_cast<string>(stridesInMystrides[k]));
				}
				boost::int64_t startElemOffset = 0;
				for (unsigned int k = 0; k < offsets.size(); k++) {
					selection_offsets[k] = offsets[k] * stridesInMystrides[k];
					LOG4CXX_DEBUG(mLogger, string("selection_offsets[") + boost::lexical_cast<string>(k) + string("]=") + boost::lexical_cast<string>(selection_offsets[k]));
					startElemOffset += startInMystrides[k] * offsets[k];
				}
				LOG4CXX_DEBUG(mLogger, string("startElemOffset =") + boost::lexical_cast<string>(startElemOffset));




				//HACK: 4D
				boost::int64_t orig_offset = 0;
				boost::int64_t offset = startElemOffset;
				//OPTIMIZE IF BOTH CONTIGUOUS!
				if (
				    (selection_offsets[0] == 1 || ds.selectionElemCount() == 1) &&
				    ((selection_offsets[0] * ds.selectionSize(0) == selection_offsets[1]) || (ds.selectionSize(3) == 1 && ds.selectionSize(2) == 1 && ds.selectionSize(1) == 1) ) &&
				    ((selection_offsets[1] * ds.selectionSize(1) == selection_offsets[2]) || (ds.selectionSize(3) == 1 && ds.selectionSize(2) == 1)) &&
				    ((selection_offsets[2] * ds.selectionSize(2) == selection_offsets[3]) || (ds.selectionSize(3) == 1))
				   ){
					//SELECTION IS ALSO CONTIGUOUS!
					LOG4CXX_DEBUG(mLogger, string("selection is contiguous, single memcopy!"));
					//data = BlockRef(new Block((void*)(mCache[i].data.get() + offset), data_size, 0));
					data = BlockRef(new Block((void*)(mCache[i].data.get() + offset), data_size, 0));
					//memcpy(data->buffer, (void*)(mCache[i].data.get() + offset), data_size);

				} else {
					LOG4CXX_DEBUG(mLogger, string("selection is not contiguous, copy one by one"));

					//SELECTION IS NOT CONTIGUOUS IN MEMORY
					data->alloc(data_size);

					for (int t = 0; t < ds.selectionSize(3); t++) {
						for (int k = 0; k < ds.selectionSize(2); k++) {
							for (int j = 0; j < ds.selectionSize(1); j++) {
								for (int l = 0; l < ds.selectionSize(0); l++) {
									//boost::int64_t offset = startElemOffset;
									//TODO optimize this!

									//offset += (i * selection_offsets[0] + j * selection_offsets[1] +
									//		k * selection_offsets[2] + t * selection_offsets[3]);

									((unsigned char*)(data->buffer))[orig_offset] = mCache[i].data.get()[offset];
									orig_offset += 1;
									offset += selection_offsets[0];
								}
								offset -= selection_offsets[0] * ds.selectionSize(0) ;
								offset += selection_offsets[1];
							}
							offset -= selection_offsets[1] * ds.selectionSize(1) ;
							offset += selection_offsets[2];
						}
						offset -= selection_offsets[2] * ds.selectionSize(2) ;
						offset += selection_offsets[3];
					}
					assert(offset == selection_offsets[3] * ds.selectionSize(3));
					assert(orig_offset == data_size);
				}
			}
			return;
		}
	}
	LOG4CXX_DEBUG(mLogger, "Not in cache! Request will be served from file");
	Read(ds, data);
	assert(data->buffer);
}
