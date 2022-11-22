# SocksProxy
用IOCP完成端口开发的支持Socks4和Socks5的高并发服务器程序源码。
讲解文章：https://blog.csdn.net/jaye8090/article/details/123648814
代码基于MFC开发，从成熟项目中分离出来。所有网络操作均使用Windows下性能最高的IOCP完成端口网络模型，只需要少数几个工作线程就能处理成千上万个并发连接，能达到很高的性能和网络吞吐量。CPU占用率很低，并且稳定无BUG，可长期运行。
