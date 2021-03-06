float sum_64(__constant float16* numbers, int offset);

float sum_64_non_vector(const __local float *workBuffer, size_t offset){
	float sum = 0;
	for(int i = 0; i < 4; i += 2){
		float16 nums = 
    vload16(i, (const __local float*)workBuffer)
      +
    vload16(i+1, (const __local float*)workBuffer);
		float8 nums_1 = nums.lo + nums.hi;
		float4 nums_2 = nums_1.lo + nums_1.hi;
		sum += nums_2.x + nums_2.y + nums_2.z + nums_2.w;
	}
  return sum;
/*
		float16 nums1 = vload16(0+offset, (const __local float*)workBuffer);
		float16 nums2 = vload16(1+offset, (const __local float*)workBuffer);


		float16 nums3 = vload16(2+offset, (const __local float*)workBuffer);
		float16 nums4 = vload16(3+offset, (const __local float*)workBuffer);

    float16 num1_2 = nums1+nums2;
    float16 num3_4 = nums3+nums4;

		//printf("Summing %2.2v16f + %2.2v16f + %2.2v16f + %2.2v16f\n", nums1, nums2, nums3, nums4); 

		float16 nums1_2_3_4 = num1_2+num3_4;

		float8 a = nums1_2_3_4.lo + nums1_2_3_4.hi;
		float4 nums_all = a.lo + a.hi;

		return nums_all.x + nums_all.y + nums_all.z + nums_all.w;
*/

}
__kernel void add(
	constant float16 *numbers, 
	int numbersSize,
	__global float *numOut
	) {
	__local float workBuffer[64];
  for(int i = 0; i < 64; i++){
    workBuffer[i] = 0.0f;
	}
 
	workBuffer[get_local_id(0)] = (float)sum_64(numbers, 4*get_local_id(0));
	barrier(CLK_LOCAL_MEM_FENCE);

	if(get_local_id(0) == 0 ){
		numOut[get_group_id(0)] = sum_64_non_vector(workBuffer, 0);
	}
}

float sum_64(__constant float16 *numbers, int offset){
/*
	float16 nums1 = numbers[0+offset];
	float16 nums2 = numbers[1+offset];
	float16 num1_2 = nums1+nums2;

	float16 nums3 = numbers[2+offset];
	float16 nums4 = numbers[3+offset];
	float16 num3_4 = nums3+nums4;

	float16 nums1_2_3_4 = num1_2+num3_4;

	float8 a = nums1_2_3_4.lo + nums1_2_3_4.hi;
	float4 nums_all = a.lo + a.hi;

	return nums_all.x + nums_all.y + nums_all.z + nums_all.w;
*/

	float sum;
	for(int i = 0; i < 4; i += 2){
		float16 nums = numbers[i]+numbers[i+1];
		float8 nums_1 = nums.lo + nums.hi;
		float4 nums_2 = nums_1.lo + nums_1.hi;
		sum += nums_2.x + nums_2.y + nums_2.z + nums_2.w;
	}
	return sum;

}
