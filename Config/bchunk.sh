boundary="myboundary"
data="Hello World"

# Create the data with the boundary
data_with_boundary="5\r\n$boundary\r\nHello\r\n6\r\n$boundary\r\n World\r\n0\r\n$boundary\r\n"

# Send the request using nc
echo -ne "POST / HTTP/1.1\r\nHost: 127.0.0.1:7029\r\nTransfer-Encoding: chunked\r\n\r\n$data_with_boundary" | nc 127.0.0.1 7029
