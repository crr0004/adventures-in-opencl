__kernel void add(__global int *numbers, __global int *numOut) {
 
	// Get the index of the current element to be processed
	int i = get_global_id(0)*2;
	int locali = get_local_id(0);
	//printf("%d\t%d\t%d\n", get_global_id(0), get_group_id(0), get_local_id(0));

	
	//vstore4(va + vb, i, numOut);
}