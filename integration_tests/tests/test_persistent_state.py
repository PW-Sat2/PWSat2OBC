import datetime
from os import path
from pprint import pprint
from unittest import TestCase

from build_config import config
from persistent_state import PersistentStateParser, SailOpeningState


class PersistentStateTest(TestCase):
    def test_parse_generated_state(self):
        state_file = path.join(config['REPORTS_DIR'], 'persistent_state')
        with open(state_file, 'rb') as f:
            data = f.read()

            result = PersistentStateParser.parse(data)
            pprint(result)
            self.assertNotEqual(result, None)
            self.assertEqual(len(result), 7)
            self.assertEqual(result['Antenna Configuration'], ('Deployment Disabled', True))
            self.assertEqual(result['Mission Time'],  {
                'External Time': datetime.timedelta(131831949, 75728, 674000),
                'Internal Time': datetime.timedelta(55818034, 51605, 111000)
            })
            self.assertEqual(result['Time Correction'], {
                'External Clock Weight': 3344,
                'Internal Clock Weight': 1122
            })
            self.assertEqual(result['Sail State'], ('Deployed', SailOpeningState.Open))
            for k in result['Error Counters']:
                self.assertEqual(result['Error Counters'][k], {
                    'Counter Config': {
                        'Decrement': 2,
                        'Increment': 5,
                        'Limit': 128,
                        'Zero': 0
                    }
                })
            self.assertEqual(result['Adcs Configuration'], ('Built In Detumbling Disabled', False))
            self.assertTrue(result['Periodic Message'], {
                'Interval': datetime.timedelta(0, 300),
                'Message': 'We are the Borg. Lower your shields and surrender your ships. We will add your biological and technological distinctiveness to our own. Your culture will adapt to service us. Resistance is futile.\x00\x00\x00\x00',
                'RepeatCount': 3
            })