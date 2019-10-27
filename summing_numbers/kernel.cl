__kernel void add(__global int *numbers, __global int *numOut, __local int *numReduce) {
 
	// Get the index of the current element to be processed
	int i = get_global_id(0)*2;
	int locali = get_local_id(0);
	int2 va = vload2(i, numbers);
	int2 vb = vload2(i+1, numbers);
	int2 vc = va + vb;
	numReduce[locali] = vc[0] + vc[1];
	//printf("%d\t%d\t%d\n", get_global_id(0), get_group_id(0), get_local_id(0));
	printf("%d\t%d: %d + %d + %d + %d = %d\n", i, locali, va[0], va[1], vb[0], vb[1], numReduce[locali]);

	
	//vstore4(va + vb, i, numOut);
}