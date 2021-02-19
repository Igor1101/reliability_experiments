compillation:
pacman -Sy qt

cd [project directory]
cmake ./
make

run:
./reliability_experiments [filename].csv [gamma] [time probability] [time intensity]
