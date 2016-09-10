//clang++ -std=c++11 lesson2_kernels.cpp -framework OpenCL

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


	//PLATFORM
	std::vector<cl::Platform> platformList;
	cl::Platform::get(&platformList);
	checkErr(platformList.size()!= 0 ? CL_SUCCESS : -1, "cl::Platform::get");
	//PLATFORM

	//INFOS
	std::cerr << "Platform number is: " << platformList.size() << std::endl;
	std::string platformVendor;
	platformList[0].getInfo((cl_platform_info)CL_PLATFORM_VENDOR, &platformVendor);
	std::cerr << "Platform is by: " << platformVendor << "\n";
	//INFOS

	//CONTEXT
	cl_context_properties cprops[3] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)(platformList[0])(),
		0
	};
	cl::Context context(dType, cprops, NULL, NULL, &err);
	checkErr(err, "Context::Context()");
	//CONTEXT

	//DEVICES
	std::vector<cl::Device> devices;
	devices = context.getInfo<CL_CONTEXT_DEVICES>();
	checkErr(devices.size() > 0 ? CL_SUCCESS : -1, "devices.size() > 0");
	//DEVICES

	//PROGRAM
	std::ifstream file("lesson2_kernels.cl");
	checkErr(file.is_open() ? CL_SUCCESS:-1, "lesson2_kernel.cl");
	std::string prog(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
	cl::Program::Sources source(1, std::make_pair(prog.c_str(), prog.length() + 1));
	cl::Program program(context, source);
	err = program.build(devices, "");
	checkErr(err, "Program::build()");
	//PROGRAM

	//BUFFERS
    cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, sizeof(int) * 10);
    cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(int) * 10);
    cl::Buffer buffer_C(context, CL_MEM_READ_WRITE, sizeof(int) * 10);

    int A[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int B[] = {0, 1, 2, 0, 1, 2, 0, 1, 2, 0};
	//BUFFERS


	//QUEUE
	cl::CommandQueue queue(context, devices[0], 0, &err);
	checkErr(err, "CommandQueue::CommandQueue()");
	queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, sizeof(int) * 10, A);
	queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(int) * 10, B);
	//QUEUE

	//KERNEL
	cl::Kernel kernel_add = cl::Kernel(program,"simple_add");
	kernel_add.setArg(0, buffer_A);
	kernel_add.setArg(1, buffer_B);
	kernel_add.setArg(2, buffer_C);
	queue.enqueueNDRangeKernel(kernel_add, cl::NullRange, cl::NDRange(10), cl::NullRange);
	queue.finish();
	//KERNEL

	
	int C[10];
	//read result C from the device to array C
	queue.enqueueReadBuffer(buffer_C, CL_TRUE, 0, sizeof(int) * 10, C);

	//std::cout << "Dimensions : " << cl::NDRange::dimensions() << std::endl;
	std::cout << "result: \n";
	for (int i = 0; i < 10; i++){
		std::cout << C[i] << " ";
	}

	return EXIT_SUCCESS;

}
