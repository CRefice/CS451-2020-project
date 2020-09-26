mkdir -p logs

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

NUM_PROCESSES=2
python finishedSignal.py --host localhost --port 10000 -p $NUM_PROCESSES &
python barrier.py --host localhost --port 11000 -p $NUM_PROCESSES &

# Wait for barriers to finish initialization
sleep 1

for i in $(seq 1 $NUM_PROCESSES); do
	(cd code && ./run-process.sh $i) &
done

wait
