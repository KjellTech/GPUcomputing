#include <vector>
#include <cstdlib>

#include "clwrapper.h"
#include "stopwatch.h"

void cpuMultiply(float *M, const float *M1, const float *M2, int nr1, int nc1r2, int nc2) {
	Stopwatch sw("cpu");
	const int nr = nr1;
	const int nc = nc2;
	for (int r = 0; r < nr1; r++) {
		for (int c = 0; c < nc2; c++) {
			float m = 0.0f;
			for (int i = 0; i < nc1r2; i++) {
				int ind1 = r + i*nr1;
				int ind2 = i + c*nc1r2;
				m += M1[ind1] * M2[ind2];
			}
			int ind = r + c*nr1;
			M[ind] = m;
		}
	}
}

cl_context context;
cl_command_queue command_queue;
cl_device_id device_id;
cl_kernel kernel;

void initializeGPU() {
	std::vector<cl_platform_id> platform_ids = getPlatformIDs();
	for (int i = 0; i < platform_ids.size(); i++)
	{
		std::string platform_name = getPlatformName(platform_ids[i]);
		std::cout << platform_name << "\n";
	}

	device_id = getFirstDevice(platform_ids[1]);

	cl_int ret;
	// Create an OpenCL context
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);

	// Create a command queue
	command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

	const char *file = "kernel.cl";
	cl_program program = createProgramFromFile(file, context);

	// Build the program
	ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);

	// Create the OpenCL kernel
	kernel = clCreateKernel(program, "matMul", &ret);
}

void gpuMultiply(float *M, const float *M1, const float *M2, int nr1, int nc1r2, int nc2) {
	Stopwatch sw("gpu");
	
	cl_int ret;

	// Allocate global GPU memory arrays:
	size_t N1 = nr1 * nc1r2 * sizeof(float);
	size_t N2 = nc1r2 * nc2 * sizeof(float);
	size_t N = nr1 * nc2 * sizeof(float);
	cl_mem M1_mem_obj = createReadOnlyDeviceBuffer(context, N1);
	cl_mem M2_mem_obj = createReadOnlyDeviceBuffer(context, N2);
	cl_mem M_mem_obj  = createWriteOnlyDeviceBuffer(context, N);
	
	// Copy from CPU to GPU memory arrays:
	copyToDeviceBuffer(command_queue, M1_mem_obj, (void*) M1, N1);
	copyToDeviceBuffer(command_queue, M2_mem_obj, (void*) M2, N2);

	

	// Set the arguments of the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&M_mem_obj);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&M1_mem_obj);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&M2_mem_obj);
	ret = clSetKernelArg(kernel, 3, sizeof(int), (void *)&nr1);
	ret = clSetKernelArg(kernel, 4, sizeof(int), (void *)&nc1r2);
	ret = clSetKernelArg(kernel, 5, sizeof(int), (void *)&nc2);

	size_t global_work_size[2] = { nr1, nc2 };
	size_t  local_work_size[2] = { 16, 16 };
	ret = clEnqueueNDRangeKernel(command_queue, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, NULL);
	copyFromDeviceBuffer(command_queue, M_mem_obj, M, N);
}

float rmsError(const std::vector<float> &M1, const std::vector<float> &M2) {
	float E = 0.0f;
	int n = M1.size();
	for (int i = 0; i < n; i++) {
		float d = M1[i] - M2[i];
		E += d*d;
	}
	E /= (1.0f*n);
	return sqrt(E);
}

int main()
{
	initializeGPU();


	int N = 128;
	for (int i = 0; i < 4; i++) {
		std::cout << "N=" << N << std::endl;
		std::vector<float> R(N * N);
		for (int i = 0; i < N * N; i++) {
			float r = rand() % 100;
			R[i] = r;
		}
		std::vector<float> Mc(N * N);
		std::vector<float> Mg(N * N);

		cpuMultiply(&Mc[0], &R[0], &R[0], N, N, N); // 2 ms, 20 ms, 296 ms, 3391 ms

		gpuMultiply(&Mg[0], &R[0], &R[0], N, N, N);  // 1 ms, 2 ms, 12 ms, 75 ms

		float E = rmsError(Mc, Mg);
		std::cout << "rmsError=" << E << "\n";
		N *= 2;
	}
	int k;
	std::cin >> k;
    return 0;
}

