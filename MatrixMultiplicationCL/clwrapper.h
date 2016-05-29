#pragma once
#include <CL/cl.h>

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

/*
A platform is a specific OpenCL implementation, for instance "Intel(R) OpenCL" or "NVIDIA CUDA".
*/
std::vector<cl_platform_id> getPlatformIDs() {
	std::vector<cl_platform_id> platform_ids;
	cl_platform_id platform_id[128];
	cl_uint num_platforms;
	cl_int ret = clGetPlatformIDs(128, platform_id, &num_platforms);
	for (int i = 0; i < num_platforms; i++) {
		platform_ids.push_back(platform_id[i]);
	}
	return platform_ids;
}

std::string getPlatformName(cl_platform_id platform_id) {
	char buffer[256];
	size_t size;
	clGetPlatformInfo(platform_id, CL_PLATFORM_NAME, 256, &buffer, &size);
	std::string sbuffer(buffer);
	return sbuffer.substr(0, size);
}

/*
Device: what runs the OpenCL code: GPU or CPU
*/
cl_device_id getFirstDevice(cl_platform_id platform_id) {
	cl_device_id device_id = NULL;
	cl_uint ret_num_devices;
	cl_int ret;
	ret = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
	return device_id;
}

cl_mem createReadOnlyDeviceBuffer(cl_context context, size_t size) {
	cl_int ret;
	cl_mem mem_obj = clCreateBuffer(context, CL_MEM_READ_ONLY, size, NULL, &ret);
	return mem_obj;
}

cl_mem createWriteOnlyDeviceBuffer(cl_context context, size_t size) {
	cl_int ret;
	cl_mem mem_obj = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size, NULL, &ret);
	return mem_obj;
}

cl_mem createReadWriteDeviceBuffer(cl_context context, size_t size) {
	cl_int ret;
	cl_mem mem_obj = clCreateBuffer(context, CL_MEM_READ_WRITE, size, NULL, &ret);
	return mem_obj;
}


void copyToDeviceBuffer(cl_command_queue command_queue, cl_mem mem_obj, void *buffer, size_t size) {
	cl_int ret;
	ret = clEnqueueWriteBuffer(command_queue, mem_obj, CL_TRUE, 0, size, buffer, 0, NULL, NULL);
}

void copyFromDeviceBuffer(cl_command_queue command_queue, cl_mem mem_obj, void *buffer, size_t size) {
	cl_int ret;
	ret = clEnqueueReadBuffer(command_queue, mem_obj, CL_TRUE, 0, size, buffer, 0, NULL, NULL);
}

cl_program createProgramFromFile(const char *file, cl_context context) {
	std::ifstream fin(file);
	std::stringstream buffer;
	buffer << fin.rdbuf();
	std::string src = buffer.str();
	const size_t size = src.size() * 8;
	cl_int ret;
	const char *csrc = src.c_str();
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&csrc, (const size_t *)&size, &ret);
	return program;
}
