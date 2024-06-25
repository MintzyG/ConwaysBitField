#!/usr/bin/env bash 

counter=1
tmp=0
amount=0
while [ $counter -le 1000 ]
do
    tmp=$(./a.out)
    amount=$((tmp+amount))
    echo -e $tmp
    ((counter++))
done

amount=$((amount/1000))

echo -e
echo $amount

echo All done
