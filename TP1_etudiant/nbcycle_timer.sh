#!/bin/bash
# Thanks to adrian satin for some tips for this script
simu="simulator.x"
if [[ -f $simu ]]; then
	list=$(./simulator.x 10000 | grep cycle | sed 's|[^0-9]*||')

	total_cycle=0
	nb_test=0
	moy_cycle=0

	for cur_num_cycle in $list
	do
		nb_test=$((nb_test +1))
		total_cycle=$(bc -l <<< ${total_cycle}+${cur_num_cycle})
	done

	temps=0
	fast_temps=0
	IFS=' ' read -ra timer <<< $(cat test.log | sed 's|[^0-9]*||')
	temps=$(bc -l <<< ${timer[1]}-${timer[0]})
	fast_temps=$(bc -l <<< ${timer[3]}-${timer[2]})

	echo -e "\033[32mMoyenne de cycle =\033[0m $(bc -l <<< ${total_cycle}/${nb_test})"
	echo -e "\033[32mTemps d'execution dans le makefile fast : \033[0m $temps \033[32ms\033[0m"
	echo -e "\033[32mTemps d'execution dans le makefile fast : \033[0m $fast_temps \033[32ms\033[0m"
else
	echo -e "\033[31mVous devez compiler\033[0m"
fi
