# Script that runs ./sim_trace for all experiments for each protocol
# Requires Python 3.5+
import os
import subprocess

PROTOCOLS = [
    'MI',
    'MSI',
    'MESI',
    'MOSI',
    'MOESI',
    'MOESIF'
]

COMMAND = './sim_trace -t traces/{0} -p {1}'

def main():
    experiments = list(sorted(filter(lambda x: 'experiment' in x, os.listdir('traces'))))

    # Store data for each experiment
    data = {
        'cycles': [],
        'accesses': [],
        'misses': [],
        'upgrades': [],
        'transfers': [],
    }

    exp_data = [[] for _ in experiments]

    for protocol in PROTOCOLS:
        cycles = []
        accesses = []
        misses = []
        upgrades = []
        transfers = []

        for i, e in enumerate(experiments):
            # Run ./sim_trace for a given combination
            cmd = COMMAND.format(e, protocol)
            s = subprocess.run(cmd.split(' '), stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            lines = s.stdout.decode('utf-8').split('\n')
            results = lines[-2].split(',')

            # Store results
            cycles.append(results[0])
            accesses.append(results[1])
            misses.append(results[2])
            upgrades.append(results[3])
            transfers.append(results[4])

            # Store per-exp results
            exp_data[i].append(results)

        data['cycles'].append(cycles)
        data['accesses'].append(accesses)
        data['misses'].append(misses)
        data['upgrades'].append(upgrades)
        data['transfers'].append(transfers)

    # Print data in CSV format
    for k, v in data.items():
        print('Data for ' + k)
        for row in v:
            print(','.join(row))

    for i, e in enumerate(exp_data):
        print('Data for exp ' + str(i+1))
        for row in e:
            print(','.join(row))

if __name__ == '__main__':
    main()
