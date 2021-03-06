//clang++ -std=c++11 lesson1_kernels.cpp -framework OpenCL

#include <utility>
//#define __NO_STD_VECTOR
#include "cl.hpp"


#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>

const std::string hw("Hello World\n");

inline void	checkErr(cl_int err, const char *name) {

	if (err != CL_SUCCESS) {
		std::cerr << "ERROR : " << name << "(" << err << ")" << std::endl;
		exit(EXIT_FAILURE);
	}
}

int			main(int argc, char **argv) {

	cl_int	err;


	cl_device_type dType = CL_DEVICE_TYPE_DEFAULT;
	std::string comp = "";
	if (argc > 1)
		comp = argv[1];

	if (comp == "GPU") {
		dType = CL_DEVICE_TYPE_GPU;
		std::cout << "Using GPU " << "(" << dType << ")" << std::endl;
	}
	else if (comp == "CPU") {
		dType = CL_DEVICE_TYPE_CPU;
		std::cout << "Using CPU " << "(" << dType << ")" << std::endl;
	}
	else
		std::cout << "Using Default " << "(" << dType << ")" << std::endl;


	std::vector<cl::Platform> platformList;
	cl::Platform::get(&platformList);
	checkErr(platformList.size()!= 0 ? CL_SUCCESS : -1, "cl::Platform::get");
	std::cerr << "Platform number is: " << platformList.size() << std::endl;
	std::string platformVendor;
	platformList[0].getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &platformVendor);
	std::cerr << "Platform is by: " << platformVendor << "\n";

	cl_context_properties cprops[3] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)(platformList[0])(),
		0
	};
	//cl::Context context(CL_DEVICE_TYPE_CPU, cprops, NULL, NULL, &err);
	//cl::Context context(CL_DEVICE_TYPE_GPU, cprops, NULL, NULL, &err);
	cl::Context context(dType, cprops, NULL, NULL, &err);
	checkErr(err, "Context::Context()");



	char * outH = new char[hw.length() + 1];
	cl::Buffer outCL(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, hw.length() + 1, outH, &err);
	checkErr(err, "Buffer::Buffer()");


	std::vector<cl::Device> devices;
	devices = context.getInfo<CL_CONTEXT_DEVICES>();
	checkErr(devices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");


	std::ifstream file("lesson1_kernels.cl");
	checkErr(file.is_open() ? CL_SUCCESS:-1, "lesson1_kernel.cl");

	std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length() + 1));
	cl::Program program(context, source);
	err = program.build(devices, "");
	checkErr(err, "Program::build()");


	cl::Kernel kernel(program, "hello", &err);
	checkErr(err, "Kernel::Kernel()");
	err = kernel.setArg(0, outCL);
	checkErr(err, "Kernel::setArg()");


	cl::CommandQueue queue(context, devices[0], 0, &err);
	checkErr(err, "CommandQueue::CommandQueue()");
	cl::Event event;
	err = queue.enqueueNDRangeKernel(kernel, cl::NullRange, cl::NDRange(hw.length() + 1), cl::NDRange(1, 1), NULL, &event);
	checkErr(err, "ComamndQueue::enqueueNDRangeKernel()");


	event.wait();
	err = queue.enqueueReadBuffer(outCL, CL_TRUE, 0, hw.length() + 1, outH);
	checkErr(err, "ComamndQueue::enqueueReadBuffer()");
	std::cout << outH << std::endl;

	return EXIT_SUCCESS;

}
