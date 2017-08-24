# Load to PyConsole using %load <path>
import csv
import os

from system import System


class DetumblingCrossValidation(object):
    def __init__(self, system):
        super(DetumblingCrossValidation, self).__init__()
        self._system = system
        self._sample_index = 0
        
        self._samples = self._load_data()

    def set_up(self):
        self._sample_index = 0

        self._system.imtq.on_mtm_measurement = self._on_measurement
        self._system.imtq.on_actuation_dipole = self._on_dipole

    def _on_measurement(self):
        self._sample_index += 1
        return self._samples[self._sample_index - 1]['MTM']

    def _on_dipole(self, dipole):
        with open('D:\\tmp\\dipoles.txt', 'a') as f:
            # f.write('{}\n'.format(dipole))
            f.write('{},{},{}\n'.format(dipole[0], dipole[1], dipole[2]))

    def _load_data(self):
        p = os.path.join(os.path.dirname(__file__), '..', '..', 'unit_tests', 'others', 'adcs', 'experimental', 'data',
                         'bdot_crossvalidation.csv')

        def as_values(items):
            for d in items:
                yield {
                    'Sample': int(d['Sample']),
                    'MTM': map(lambda v: int(float(v) * 1e7), [d['MtmX'], d['MtmY'], d['MtmZ']])
                }

        with open(p, 'r') as f:
            sample = ''.join([f.readline() for x in xrange(0, 3)])
            dialect = csv.Sniffer().sniff(sample)
            f.seek(0)
            reader = csv.DictReader(f, fieldnames=('Sample', 'MtmX', 'MtmY', 'MtmZ'), restval='Ignore', dialect=dialect)

            return list(as_values(reader))


dcv = DetumblingCrossValidation(system)
dcv.set_up()