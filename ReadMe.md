# 日志结构文件系统块访问深度预测

因为日志结构（Log-structure）文件块较为碎片化地分布在文件系统中，所以在文件的read上会带来比较多寻道操作，从而提升了延迟和开销。

而文件系统块访问的深度预测就可以很好地提前读取要访问的文件块，这样子就可以隐藏访问延迟。

## 获取trace

在访问深度的预测上，我们需要使用使用已有的访问历史记录。有一个网站可以让我们查到block级别的访问记录：

[SNIA IOTTA Repository BLOCK I/O TRACES](http://iotta.snia.org/tracetypes/3)

我们使用的是一个来自于微软的trace集，也就是`MSR-Cambridge`，我们使用里面的`mds_0`。得到的是一个csv文件，根据Readme的内容，这个csv文件一共分7列，每一列包含下面几个内容：

> The files are gzipped csv (comma-separated text) files. The fields in
> the csv are:
>
> Timestamp,Hostname,DiskNumber,Type,Offset,Size,ResponseTime
>
> Timestamp is the time the I/O was issued in "Windows filetime"
> Hostname is the hostname (should be the same as that in the trace file name)
> DiskNumber is the disknumber (should be the same as in the trace file name)
> Type is "Read" or "Write"
> Offset is the starting offset of the I/O in bytes from the start of the logical
> disk.
> Size is the transfer size of the I/O request in bytes.
> ResponseTime is the time taken by the I/O to complete, in Windows filetime
> units.

所以实际上所以我们先要编程对csv文件进行整理，我们只关注read类型，然后因为实际上在csv文件中只提供offset，我们需要设定一个块的大小来将这个csv文件中offset转化为一系列块号。

所以我们我们要写一个程序来来进行现有数据的整理。我们只需要一个序列，那就是操作的访问块号。

而在时序的问题上整个csv文件是按照整个从小到大的顺序排列的。所以就保持现有的顺序就可以了。

我们编写了`select-useful-data`，这个函（文件）可以将所有读的块号读取出来，写到一个新的`block_count`文件当中。



