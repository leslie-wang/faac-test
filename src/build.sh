g++ pcm2aac.cpp -o pcm2aac -L/opt/local/lib -lfaac -I/opt/local/include
g++ liveaac.cpp -o liveaac -L/opt/local/lib -lfaac -lportaudio -I/opt/local/include
g++ recordaac_pa.cpp -o recordaac_pa -L/opt/local/lib -lfaac -lportaudio -I/opt/local/include
g++ paex_record.cpp -o paex_record -L/opt/local/lib -lportaudio -I/opt/local/include
