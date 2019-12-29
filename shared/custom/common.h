#if !defined(COMMON_H)
#define COMMON_H

#include <vector>
#include <stdint.h>
/**
 * Generates permutations according to lexicographic generation
 * using the algorithm from Algorithm L in Chapter 7.2.1.3 of 
 * The Art of Computer Programming Volume 4A Part 1 by Donald E. Knuth.
 * 
 * Basic idea is to try to increase the largest index possible until we have violated
 * the constraint. The sententials are used for the base case of increasing the last index,
 * and also boundes how far we can increase.
 * 
 * @param n the size of the index can take in range {0...n-1}
 * @param t how many positions you want in the permutation
 * @return a contingious vector of the permutations
 **/
template<typename T> static std::vector<T> Generate_Permutations(const T n, const T t){
	// Complete list
	std::vector<T> sup_c;

	// Temp c to keep list in, +2 for the sentenials
	T* c = new T[t+2];

	for(int i = 0; i < t; i++){
		c[i] = i;
		sup_c.push_back(i);
	}

	// Put some sentenials in
	c[t] = n;
	c[t+1] = 0;

	int j = 0;
	while(true){
		j = 0;
		// Keep going until we find the largest index we can increase
		while(c[j]+1 == c[j+1]){
			c[j] = j;
			j++;
		}

		// If we've gone too far to hit the sentenials we stop the algorthm
		if((j+1) > t){
			break;
		}

		// Increase the largest index we can
		c[j] = c[j]+1;

		// We could probably store the indexes as we can increase
		// but we will just do it here instead for simplicity
		for(int i = 0; i < t; i++){
			sup_c.push_back(c[i]);
		}
	}

	sup_c.shrink_to_fit();
	return sup_c;
}

#endif // COMMON_H