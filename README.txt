compillation:
install qt:
pacman -Sy qt gcc make cmake

cd [project directory]
cmake ./
make

run:
./reliability_experiments [filename].csv [gamma] [time probability] [time intensity]

example, you can run from project directory:
./reliability_experiments example/sample.csv 0.54 880 258
example output to stdout:
запущено
середнє на відмову:Tcp= 268.38
y-відсотковий наробіток на відмову: 91.2813
ймовірність безвідмовної роботи: 0.0328346  на час: 880
інтенсивність відмов: 0.00291432  на час: 258

