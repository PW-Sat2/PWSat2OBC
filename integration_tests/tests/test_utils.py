from unittest import TestCase
from parameterized import parameterized

from utils import ensure_byte_list, ensure_string

class UtilsTests(TestCase):
    @parameterized.expand([
        (1, [1]),
        ([1, 2], [1, 2]),
        (['A', 'B', 'C'], [65, 66, 67]),
        ('A', [65]),
        ('ABC', [65, 66, 67]),
        ((1, 2, 3), [1, 2, 3])
    ])
    def test_ensure_byte_list(self, input, output):
        result = ensure_byte_list(input)
        self.assertEqual(result, output)

    @parameterized.expand([
        ([1], '\x01'),
        ([1, 2], '\x01\x02'),
        (['A', 'B', 'C'], 'ABC'),
        ('A', 'A'),
        ('ABC', 'ABC'),
        ((1, 2, 3), '\x01\x02\x03')
    ])
    def test_ensure_string(self, input, output):
        result = ensure_string(input)

        self.assertEqual(result, output)
