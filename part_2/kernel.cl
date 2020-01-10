__kernel void add(
	__global int *partitionPermutations, 
	__global int *numbers, 
	int dividers,
	int numbersSize,
	__local int *maxSums,
	__global int *numOut
	) {
 
	// Get the index of the current element to be processed
	int globali = get_global_id(0);
	int locali = get_local_id(0);
	// printf("%d\t%d\t%d\n", get_global_id(0), get_group_id(0), get_local_id(0));

	/**
	 * We need to walk each partition, sum it, and then determine if it is the largest.
	 * The first and last partition is done out of the loop to simply the logic of the primary loop.
	 */

	// each index has number of dividers of numbers in it
	int startIndex = globali * dividers;
	int maxSum = 0;

	int sum = 0;
	// we need to sum the first number when the first partition index is zero
	// we add one to the end of the loop index
	for(int j = 0; j < partitionPermutations[startIndex]+1; j++){
		sum += numbers[j];
		// printf("%d\tsum: %d\t", globali, sum);
	}
	maxSum = max(maxSum, sum);
	sum = 0;

	// do the last sum out of the loop
	for(int i = 0; i < dividers-1; i++){
		int sumStart = partitionPermutations[startIndex+i]+1;
		int sumEnd = partitionPermutations[startIndex+i+1]+1;
		int sum = 0;
		for(int j = sumStart; j < sumEnd; j++){
			sum += numbers[j];
		}
		maxSum = max(maxSum, sum);

	}
	int sumStart = partitionPermutations[startIndex+dividers-1]+1;
	// printf("Start, end: %d, %d\n", sumStart, numbersSize);
	sum = 0;
	for(int j = sumStart; j < numbersSize; j++){
		sum += numbers[j];
		printf("%d\tsum: %d\t", globali, sum);
	}
	maxSum = max(sum, maxSum);
	// printf("%d\t%d", globali, maxSum);

	// store the results
	// maxSums[locali] = maxSum;
	atomic_min(&numOut[0], maxSum);
}