output_dir=$1
if [ -z $output_dir ]; then
	echo "*************************************************"
	echo "Passe o nome do diretório dos arquivos de entrada"
	echo "*************************************************"
	exit
fi
printf '%-13s %-10s %-10s %-10s %-8s %-6s %-10s\n\n' "FILE" "TIME" "GAP" "SOL" "NOS" "EDGES" "DENSIDADE"
for file in `ls $output_dir`; do
	path=$output_dir/$file
	set -- `cat $path| grep DIMENSOES |  awk '{print $2}'`
	nos=$1
	set -- `cat $path| grep DIMENSOES |  awk '{print $3}'`
	arestas=$1
	set -- `cat $path| grep Total |  awk '{print $4}'`
	time=$1
	set -- `cat $path| grep GAP |  awk  '{print $3}'`
	gap=$1
	set -- `cat $path| grep "Solution value" |  awk  '{print $4}'`
	sol=$1

	pot=$((nos*nos))
	densidade=`echo "scale=2;$arestas/$pot" | bc`
	if [ -z $time ]; then
		time="-1"
	fi
	if [ -z $gap ]; then
		gap="-1"
	fi
	if [ -z $sol ]; then
		sol="-1"
	fi
	time=$time"s"
	printf "%-13s %-10s %-10s %-10s %-8s %-6s %-10s\n" $file $time $gap $sol $nos $arestas $densidade
done


