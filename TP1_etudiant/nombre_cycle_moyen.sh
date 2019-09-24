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

	echo -e "\033[32mMoyenne de cycle =\033[0m $(bc -l <<< ${total_cycle}/${nb_test})"
else
	echo -e "\033[31mVous devez compiler\033[0m"
fi
