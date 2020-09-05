__kernel void add( __global float *numOut, int numbersSize) {
float sum = numOut[0];
for(int i = 1; i < numbersSize; i++){
	sum += numOut[i];
}
numOut[0] = sum;

}
