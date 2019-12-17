def parition(n, k):
    if len(n) == 1:
        return n[0]
    if k == 1:
        return sum(n)

    a = []
    for i in range(len(n)):
        print(n[i+1:])
        b =  [parition(n[:i+1], k-1), sum(n[i+1:] )]
        a.append( max(
          b 
        ))
    return min(a)

if __name__ == "__main__":
    input = [1,2,3,4,5,6,7,8,9]
    partitions = 3
    print(parition(input, partitions))
