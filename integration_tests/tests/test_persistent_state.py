from unittest import TestCase
from persistent_state import PersistentStateParser
import struct

class PersistentStateTest(TestCase):

    def test_parsing_empty_buffer(self):
        buffer = "\0" * 276
        result = PersistentStateParser.parse_partial(buffer)

        self.assertNotEqual(result, None)
        self.assertEqual(len(result[0]), 7)
        self.assertTrue(result[0][0].has_key('Antenna Configuration'))
        self.assertTrue(result[0][1].has_key('Mission Time'))
        self.assertTrue(result[0][2].has_key('Time Correction'))
        self.assertTrue(result[0][3].has_key('Boot State'))
        self.assertTrue(result[0][4].has_key('Sail State'))
        self.assertTrue(result[0][5].has_key('Error Counters'))
        self.assertTrue(result[0][6].has_key('Periodic Message'))
        self.assertEqual(len(result[1]), 0)
