__kernel void add(__global float *numbers, __global float *numOut) {
 
    // Get the index of the current element to be processed
    int i = get_global_id(0);
    float4 va = vload4(i, numbers);

	numOut[i] = result;
}