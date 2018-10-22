from struct import pack

from bitarray import bitarray

from emulator.beacon_parser.full_beacon_parser import FullBeaconParser
from emulator.beacon_parser.parser import BitReader, BeaconStorage


class BeaconFrame(object):
    def __init__(self, payload):
        self._payload = payload
        try:
            all_bits = bitarray(endian='little')
            all_bits.frombytes(''.join(map(lambda x: pack('B', x), payload)))

            reader = BitReader(all_bits)
            store = BeaconStorage()

            parsers = FullBeaconParser().GetParsers(reader, store)
            parsers.reverse()

            while len(parsers) > 0:
                parser = parsers.pop()
                parser.parse()

            self._parsed = store.storage
        except:
            self._parsed = None

    def payload(self):
        return self._payload

    def _get_proper_crc(self):
        try:
            from build_config import config
            return config['VALID_CRC']
        except Exception as e:
            return ""

    def __repr__(self):
        if self._parsed is None:
            return '{}: v    {} (parse failed)'.format(hex(id(self)), self.__class__.__name__)

        proper_ctc = self._get_proper_crc()

        v = lambda group, key: str(self._parsed[group][key])

        crc_ok = "\t"
        if v('02: Program State', '0056: Program CRC') != proper_ctc:
            crc_ok = "!!!!!\t"

        lines = [
            '{}: v    {}'.format(hex(id(self)), self.__class__.__name__),
            '\tBP VOLT {}, {}'.format(
                v('14: Controller A', '1019: BATC.VOLT_A'),
                v('15: Controller B', '1204: BATC.VOLT_B')
            ),
            '\tBP TEMP {}, {}, {}'.format(
                v('14: Controller A', '1062: BP.Temperature A'),
                v('14: Controller A', '1075: BP.Temperature B'),
                v('15: Controller B', '1194: BP.Temperature'),
            ),
            '\tCOMM PA TEMP NOW {} LAST {}'.format(
                v('11: Comm', '0756: [Now] Power Amplifier Temperature '),
                v('11: Comm', '0605: [Last transmission] Power Amplifier Temperature')
            ),
            '{}OBC CRC {}'.format(
                crc_ok,
                v('02: Program State', '0056: Program CRC')
            ),
            '\tGYRO UNCAL {}, {}, {}'.format(
                v('10: Gyroscope', '0510: X measurement'),
                v('10: Gyroscope', '0526: Y measurement'),
                v('10: Gyroscope', '0542: Z measurement')
            )
        ]

        return '\n'.join(lines)
