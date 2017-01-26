from unittest import TestCase
from nose_parameterized import parameterized

from utils import ensure_byte_list


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
