mkdir -p logs

trap "kill 0" EXIT

NUM_PROCESSES=4
python finishedSignal.py --host localhost --port 10000 -p $NUM_PROCESSES &
python barrier.py --host localhost --port 11000 -p $NUM_PROCESSES &

# Wait for barriers to finish initialization
sleep 0.2

for i in $(seq 1 $NUM_PROCESSES); do
	(cd code && ./run-process.sh $i) 2> logs/process-${i}.cerr &
done

wait
