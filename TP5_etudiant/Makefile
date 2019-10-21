
mono : tp4_top.desc tp4_top.cpp
	soclib-cc -m debug -P -p tp4_top.desc -o simulator.x

cluster : tp5_cluster_top.desc tp5_cluster_top.cpp
	soclib-cc -m debug -P -p tp5_cluster_top.desc -o simulator_cluster.x

clean_mono :
	soclib-cc -x -X -P -p tp4_top.desc        
	rm -f *.o *.x core tmp *.deps stats trace simulator.x

clean_cluster :
	soclib cc -x -X -P -p tp5_cluster_top.desc        
	rm -f *.o *.x core tmp *.deps stats trace simulator_cluster.x

