# Questions to answer

- What difference does the vector size make?
- At what point is it better to invoke multiple kernel calls instead of running a loop in the work item?
- Is it better to do branchless summation for the workgroup or use a branch?
- What impact does buffering input sizes to align to 64 make?
- What size does the input have to be for it to be faster on the GPU instead of the CPU?
- For input / 64*64 > 1 when is it faster to run the last summation on the GPU or CPU?
