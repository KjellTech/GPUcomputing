

__kernel void matMul(__global float* M, __global float* M1, __global float* M2, int nr1, int nc1r2, int nc2)
{
	int r = get_global_id(0);
	int c = get_global_id(1);
	float m = 0.0f;
	for(int i = 0; i < nc1r2; i++) {
		int ind1 = r + i*nr1;
		int ind2 = i + c*nc1r2;
		m += M1[ind1] * M2[ind2];
	}
	int ind = r + c*nr1;
	M[ind] = m;
}
