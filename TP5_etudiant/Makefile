
mono : tp5_top.desc tp5_top.cpp
	soclib-cc -m debug -P -p tp5_top.desc -o simulator.x

cluster : tp5_cluster_top.desc tp5_cluster_top.cpp
	soclib-cc -m debug -P -p tp5_cluster_top.desc -o simulator_cluster.x

clean_mono :
	soclib-cc -x -X -P -p tp5_top.desc        
	rm -f *.o *.x core tmp *.deps stats trace simulator.x

clean_cluster :
	soclib cc -x -X -P -p tp5_cluster_top.desc        
	rm -f *.o *.x core tmp *.deps stats trace simulator_cluster.x

