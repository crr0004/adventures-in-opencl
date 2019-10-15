__kernel void add(__global int *numbers) {
 
    // Get the index of the current element to be processed
    int i = get_global_id(0);
    if(i > 0){
        numbers[i] = numbers[i] + numbers[i-1];
        printf("%d: %d", i, numbers[i]);
    }
}