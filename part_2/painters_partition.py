def parition(n, k):
    if len(n) == 1:
        return n[0]
    if k == 1:
        return sum(n)

    a = []
    for i in range(len(n)):
        #print(n[i+1:])
        b =  [parition(n[:i+1], k-1), sum(n[i+1:] )]
        a.append( max(
          b 
        ))
    return min(a)


def gen_permutations(n, t):
   """
    Lexicographic generation where each number is always strictly greater than the previous.

    :param n: the size of the index can take in range {0...n-1}.

    :param t: how many positions you want in permutaton.

    :returns: list of all the permutations

    This is based off Algorithm L in Chapter 7.2.1.3 of
    The Art of Computer Programming Volume 4A Part 1 by Donald E. Knuth
   """
   sup_c = []
   # Algorithm considers the inverse list first, but we are going to do the inverse
   c = list(range(t))
   sup_c.append(c.copy())

   # Drop some sentinels in
   c.append(n)
   c.append(0)

   j = 0
   # use a blind while loop so we can break at the right point internally
   while True:
        j = 0
        # This will eventually cause j to be set as the sentinel but we break before that
        while c[j]+1 == c[j+1]:
            c[j] = j
            j += 1
        # When we're about to violate the sentital, stop
        if j+1 > t:
            break
        # increase the largest valid index so we can increase to that
        c[j] = c[j]+1
        # cut out the sentinels
        sup_c.append(c[:t].copy())

   return sup_c


if __name__ == "__main__":
    input = [6, 7, 4, 9]
    partitions = 3
    print(parition(input, partitions))
    #print(gen_permutations(10, 3))
