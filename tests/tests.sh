echo "Start testing"
FIRSTTESTNUM=10
SECONDTESTNUM=10

printf "Start tests for task 1\n"

for ((i = 1; i <= FIRSTTESTNUM; i++));
do
	printf "${i})\n"
	time ./../bin/csvcraft -i "test${i}.txt" -o "../bin/test${i}_out_sp.csv" -m 1
	printf "\n"
	time ./../bin/csvcraft -i "test${i}.txt" -o "../bin/test${i}_out_mp.csv" -m 1 -p
	printf "\n"
done

time ./../bin/csvcraft -i "lotr.txt" -o "../bin/lotr_out_sp.csv" -m 1
printf "\n"
time ./../bin/csvcraft -i "lotr.txt" -o "../bin/lotr_out_mp.csv" -m 1 -p
printf "\n"

printf "Start tests for task 2\n"

for ((i = 1; i <= SECONDTESTNUM; i++));
do
	printf "${i})\n"
	time ./../bin/csvcraft -i "../bin/test${i}_out_sp.csv" -o "../bin/test${i}_textgen_sp.txt" -m 2 --word-number 10
	printf "\n"
	time ./../bin/csvcraft -i "../bin/test${i}_out_sp.csv" -o "../bin/test${i}_textgen_mp.txt" -m 2 --word-number 10 -p
	printf "\n"
done

printf "lotr normal generation\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_sp.txt" -m 2 --word-number 100
printf "\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_mp.txt" -m 2 --word-number 100 -p
printf "\n"

printf "lotr correct starting word\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_sp_caution.txt" -m 2 --word-number 10 --starting-word "caution"
printf "\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_mp_caution.txt" -m 2 --word-number 10 -p --starting-word "caution"
printf "\n"

printf "lotr wrong starting word\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_sp_error.txt" -m 2 --word-number 10 --starting-word "sium"
printf "\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_mp_error.txt" -m 2 --word-number 10 -p --starting-word "sium"
printf "\n"

printf "lotr negative word number\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_sp_meno1.txt" -m 2 --word-number -1
printf "\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_mp_meno1.txt" -m 2 --word-number -1 -p
printf "\n"

printf "lotr zero word number\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_sp_zero.txt" -m 2 --word-number 0
printf "\n"
time ./../bin/csvcraft -i "../bin/lotr_out_sp.csv" -o "../bin/lotr_textgen_mp_zero.txt" -m 2 --word-number 0 -p
printf "\n"


