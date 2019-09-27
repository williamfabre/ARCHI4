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
	temps=$(bc -l <<< ${timer[3]}-${timer[2]}) # depiler dans le bon sens
	fast_temps=$(bc -l <<< ${timer[1]}-${timer[0]}) # on a compiler le fast en premier


	timer_simu_start=`date +%s`
	`./simulator.x 10000000`
	timer_simu_end=`date +%s`
	runtime_simu=$((timer_simu_end - timer_simu_start))

	fast_timer_simu_start=`date +%s`
	`./fast_simulator.x 10000000`
	fast_timer_simu_end=`date +%s`
	fast_runtime_simu=$((fast_timer_simu_end - fast_timer_simu_start))


	# Nombre de cycle
	echo -e "\033[32mMoyenne de cycle =\033[0m $(bc -l <<< ${total_cycle}/${nb_test})"

	# Temps de compilation
	echo -e "\033[32mTemps de compilation pour la version normale: \033[0m $temps \033[32ms\033[0m"
	echo -e "\033[32mTemps de compilation pour la version fast: \033[0m $fast_temps \033[32ms\033[0m"

	# Temps d'execution
	echo -e "\033[32mTemps d'execution du script simulator.x pour 10 000 000 de cycles: \033[0m $runtime_simu\033[32ms\033[0m"
	echo -e "\033[32mTemps d'execution du script fast_simulator.x pour 10 000 000 de cycles: \033[0m $fast_runtime_simu\033[32ms\033[0m"
else
	echo -e "\033[31mVous devez compiler\033[0m"
fi
