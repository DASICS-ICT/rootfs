echo 1 > /sys/module/dmatest/parameters/verbose
echo dma0chan0 > /sys/module/dmatest/parameters/channel
echo 2000 > /sys/module/dmatest/parameters/timeout
echo 1 > /sys/module/dmatest/parameters/iterations
echo 1 > /sys/module/dmatest/parameters/run